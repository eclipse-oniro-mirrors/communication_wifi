/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sta_auto_connect_service.h"
#include "wifi_logger.h"
#include "wifi_sta_hal_interface.h"
#include "wifi_config_center.h"
#include "wifi_common_util.h"
#include "block_connect_service.h"
#include <sys/time.h>

DEFINE_WIFILOG_LABEL("StaAutoConnectService");

namespace OHOS {
namespace Wifi {
constexpr int CONNECT_CHOICE_INVALID = 0;
constexpr int CONNECT_CHOICE_TIMEOUT_MS = 50 * 1000;
constexpr int CONNECT_CHOICE_MAX_LOOP_TIMES = 2;

StaAutoConnectService::StaAutoConnectService(StaStateMachine *staStateMachine, int instId)
    : pStaStateMachine(staStateMachine),
      pSavedDeviceAppraisal(nullptr),
      firmwareRoamFlag(true),
      selectDeviceLastTime(0),
      pAppraisals {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
      m_instId(instId)
{}

StaAutoConnectService::~StaAutoConnectService()
{
    WIFI_LOGI("Enter ~StaAutoConnectService.\n");
    if (pSavedDeviceAppraisal != nullptr) {
        delete pSavedDeviceAppraisal;
        pSavedDeviceAppraisal = nullptr;
    }
}

ErrCode StaAutoConnectService::InitAutoConnectService()
{
    WIFI_LOGI("Enter InitAutoConnectService.\n");

    pSavedDeviceAppraisal = new (std::nothrow) StaSavedDeviceAppraisal(firmwareRoamFlag);
    if (pSavedDeviceAppraisal == nullptr) {
        WIFI_LOGE("savedDeviceAppraisal is null\n");
        return WIFI_OPT_FAILED;
    }
    pNetworkSelectionManager = std::make_unique<NetworkSelectionManager>();
    int savedPriority = WifiSettings::GetInstance().GetSavedDeviceAppraisalPriority(m_instId);
    if (RegisterDeviceAppraisal(pSavedDeviceAppraisal, savedPriority)) {
        WIFI_LOGI("RegisterSavedDeviceAppraisal succeeded.\n");
    }
    return WIFI_OPT_SUCCESS;
}

void StaAutoConnectService::SetAutoConnectStateCallback(const std::vector<StaServiceCallback> &callbacks)
{
    WIFI_LOGI("Enter SetAutoConnectStateCallback.\n");
    mStaCallbacks = callbacks;
}

bool StaAutoConnectService::OverrideCandidateWithUserSelectChoice(NetworkSelectionResult &candidate)
{
    WifiDeviceConfig tmpConfig = candidate.wifiDeviceConfig;
    int originalCandidateNetwordId = candidate.wifiDeviceConfig.networkId;
    int curentLoopIdx = 0;
    while (tmpConfig.networkSelectionStatus.connectChoice != INVALID_NETWORK_ID) {
        curentLoopIdx++;
        if (curentLoopIdx > CONNECT_CHOICE_MAX_LOOP_TIMES) {
            WIFI_LOGI("%{public}s reach max loop threshold connectChoice: %{public}d",
                __FUNCTION__, tmpConfig.networkId);
            break;
        }
        struct timespec times = {0, 0};
        clock_gettime(CLOCK_BOOTTIME, &times);
        long currentTime = static_cast<int64_t>(times.tv_sec) * MSEC + times.tv_nsec / (MSEC * MSEC);
        long choiceSetToGet = currentTime - tmpConfig.networkSelectionStatus.connectChoiceTimestamp;
        if (choiceSetToGet < CONNECT_CHOICE_INVALID || choiceSetToGet > CONNECT_CHOICE_TIMEOUT_MS) {
            WIFI_LOGI("%{public}s connectChoice: %{public}d update time is expired", __FUNCTION__, tmpConfig.networkId);
            break;
        }
        if (WifiSettings::GetInstance().GetDeviceConfig(tmpConfig.networkId, tmpConfig) != 0) {
            WIFI_LOGI("%{public}s cannot find connectChoice: %{public}d", __FUNCTION__, tmpConfig.networkId);
            break;
        }
        if (tmpConfig.networkSelectionStatus.seenInLastQualifiedNetworkSelection) {
            WIFI_LOGI("%{public}s cannot seen connectChoice in last auto connect: %{public}d",
                __FUNCTION__, tmpConfig.networkId);
            break;
        }
        if (tmpConfig.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED) {
            candidate.wifiDeviceConfig = tmpConfig;
        }
    }
    if (candidate.wifiDeviceConfig.networkId != originalCandidateNetwordId) {
        WIFI_LOGE("%{public}s original networdId:%{public}d, override networkId %{public}d, ssid: %{public}s",
            __FUNCTION__, originalCandidateNetwordId, candidate.wifiDeviceConfig.networkId,
            SsidAnonymize(candidate.wifiDeviceConfig.ssid).c_str());
        return true;
    }
    return false;
}

void StaAutoConnectService::OnScanInfosReadyHandler(const std::vector<InterScanInfo> &scanInfos)
{
    WIFI_LOGD("Enter OnScanInfosReadyHandler.\n");

    WifiLinkedInfo info;
    WifiConfigCenter::GetInstance().GetLinkedInfo(info, m_instId);
    if (info.supplicantState == SupplicantState::ASSOCIATING ||
        info.supplicantState == SupplicantState::ASSOCIATED ||
        info.supplicantState == SupplicantState::AUTHENTICATING ||
        info.supplicantState == SupplicantState::FOUR_WAY_HANDSHAKE ||
        info.supplicantState == SupplicantState::GROUP_HANDSHAKE) {
        WIFI_LOGE("Supplicant is under transient state.\n");
        return;
    }

    std::vector<std::string> blockedBssids;
    if (!AllowAutoSelectDevice(info) || !IsAllowAutoJoin()) {
        return;
    }
    BlockConnectService::GetInstance().UpdateAllNetworkSelectStatus();
    NetworkSelectionResult networkSelectionResult;
    if (pNetworkSelectionManager->SelectNetwork(networkSelectionResult, NetworkSelectType::AUTO_CONNECT, scanInfos)) {
        std::string bssid = "";
        if (!OverrideCandidateWithUserSelectChoice(networkSelectionResult)) {
            bssid = networkSelectionResult.interScanInfo.bssid;
        }
        int networkId = networkSelectionResult.wifiDeviceConfig.networkId;
        std::string &ssid = networkSelectionResult.interScanInfo.ssid;
        WIFI_LOGI("AutoSelectDevice networkId: %{public}d, ssid: %{public}s, bssid: %{public}s.", networkId,
                  SsidAnonymize(ssid).c_str(), MacAnonymize(bssid).c_str());
        auto message = pStaStateMachine->CreateMessage(WIFI_SVR_CMD_STA_CONNECT_SAVED_NETWORK);
        message->SetParam1(networkId);
        message->SetParam2(NETWORK_SELECTED_BY_AUTO);
        message->AddStringMessageBody(bssid);
        pStaStateMachine->SendMessage(message);
    } else {
        WIFI_LOGI("AutoSelectDevice return fail.");
    }
    for (const auto &callBackItem : mStaCallbacks) {
        if (callBackItem.OnAutoSelectNetworkRes != nullptr) {
            callBackItem.OnAutoSelectNetworkRes(networkSelectionResult.wifiDeviceConfig.networkId, m_instId);
        }
    }
}

bool StaAutoConnectService::EnableOrDisableBssid(std::string bssid, bool enable, int reason)
{
    WIFI_LOGI("Enter EnableOrDisableBssid.\n");
    if (bssid.empty()) {
        WIFI_LOGI("bssid is empty.\n");
        return false;
    }

    return true;
}

bool StaAutoConnectService::RegisterDeviceAppraisal(StaDeviceAppraisal *appraisal, int priority)
{
    WIFI_LOGI("Enter RegisterDeviceAppraisal.\n");
    if (priority < 0 || priority >= MIN_APPRAISAL_PRIORITY) {
        WIFI_LOGE("Out of array range.\n");
        return false;
    }
    if (pAppraisals[priority] != nullptr) {
        WIFI_LOGE("Appraisals is not empty.\n");
        return false;
    }
    pAppraisals[priority] = appraisal;
    return true;
}

ErrCode StaAutoConnectService::AutoSelectDevice(WifiDeviceConfig &electedDevice,
    const std::vector<InterScanInfo> &scanInfos, std::vector<std::string> &blockedBssids, WifiLinkedInfo &info)
{
    WIFI_LOGI("Enter SelectNetwork.\n");
    if (scanInfos.empty()) {
        WIFI_LOGE("scanInfo is empty.");
        return WIFI_OPT_FAILED;
    }

    /* Whether network selection handover is required */
    if (!AllowAutoSelectDevice(scanInfos, info)) {
        WIFI_LOGE("Network switching is not required.\n");
        return WIFI_OPT_FAILED;
    }

    std::vector<InterScanInfo> availableScanInfos;
    /* Filter out unnecessary networks. */
    GetAvailableScanInfos(availableScanInfos, scanInfos, blockedBssids, info);
    if (availableScanInfos.empty()) {
        WIFI_LOGE("No scanInfo available.\n");
        return WIFI_OPT_FAILED;
    }
    /*
     * Check the registered network appraisal from highest priority to lowest
     * priority until the selected network
     */
    for (auto registeredAppraisal : pAppraisals) {
        if (registeredAppraisal != nullptr) {
            ErrCode code = registeredAppraisal->DeviceAppraisals(electedDevice, availableScanInfos, info);
            if (code == WIFI_OPT_SUCCESS) {
                time_t now = time(0);
                selectDeviceLastTime = static_cast<int>(now);
                WIFI_LOGI("electedDevice generation.\n");
                return WIFI_OPT_SUCCESS;
            }
        }
    }

    if (RoamingSelection(electedDevice, availableScanInfos, info)) {
        WIFI_LOGI("Roaming network generation.\n");
        return WIFI_OPT_SUCCESS;
    }
    WIFI_LOGE("No electedDevice.\n");
    return WIFI_OPT_FAILED;
}

bool StaAutoConnectService::RoamingSelection(
    WifiDeviceConfig &electedDevice, std::vector<InterScanInfo> &availableScanInfos, WifiLinkedInfo &info)
{
    for (auto scanInfo : availableScanInfos) {
        if (info.connState == ConnState::CONNECTED && scanInfo.ssid == info.ssid && scanInfo.bssid != info.bssid) {
            WIFI_LOGD("Discover roaming networks.\n");
            if (RoamingEncryptionModeCheck(electedDevice, scanInfo, info)) {
                return true;
            }
        }
    }
    return false;
}

bool StaAutoConnectService::RoamingEncryptionModeCheck(
    WifiDeviceConfig &electedDevice, InterScanInfo scanInfo, WifiLinkedInfo &info)
{
    WifiDeviceConfig network;
    if (WifiSettings::GetInstance().GetDeviceConfig(scanInfo.ssid, DEVICE_CONFIG_INDEX_SSID, network) == 0) {
        std::string mgmt = scanInfo.capabilities;
        if (mgmt.find("WPA-PSK") != std::string::npos || mgmt.find("WPA2-PSK") != std::string::npos) {
            mgmt = "WPA-PSK";
        } else if (mgmt.find("EAP") != std::string::npos) {
            mgmt = "WPA-EAP";
        } else if (mgmt.find("SAE") != std::string::npos) {
            mgmt = "SAE";
        } else {
            if (mgmt.find("WEP") != std::string::npos && network.wepTxKeyIndex == 0) {
                WIFI_LOGE("The roaming network is a WEP network, but the connected network is not a WEP network.\n");
                return false;
            } else if (mgmt.find("WEP") == std::string::npos && network.wepTxKeyIndex != 0) {
                WIFI_LOGE("The connected network is a WEP network, but the roaming network is not a WEP network.\n");
                return false;
            }
            mgmt = "NONE";
        }
        if (mgmt == network.keyMgmt) {
            WIFI_LOGD("The Current network bssid %{public}s signal strength is %{public}d",
                MacAnonymize(info.bssid).c_str(), info.rssi);
            WIFI_LOGD("The Roaming network bssid %{public}s signal strength is %{public}d",
                MacAnonymize(scanInfo.bssid).c_str(), scanInfo.rssi);
            int rssi = scanInfo.rssi - info.rssi;
            if (rssi > MIN_ROAM_RSSI_DIFF) {
                WIFI_LOGD("Roming network rssi - Current network rssi > 6.");
                electedDevice.ssid = scanInfo.ssid;
                electedDevice.bssid = scanInfo.bssid;
                return true;
            } else {
                WIFI_LOGD("Roming network rssi - Current network rssi < 6.");
            }
        } else {
            WIFI_LOGE("The encryption mode does not match.\n");
        }
    }
    return false;
}

bool StaAutoConnectService::AllowAutoSelectDevice(OHOS::Wifi::WifiLinkedInfo &info)
{
    if (info.connState == DISCONNECTED || info.connState == UNKNOWN) {
        return true;
    }
    WIFI_LOGI("Current linkInfo state:[%{public}d %{public}s] is not in DISCONNECTED state, skip network selection.",
        info.connState, magic_enum::Enum2Name(info.connState).c_str());
    return false;
}

bool StaAutoConnectService::AllowAutoSelectDevice(const std::vector<InterScanInfo> &scanInfos, WifiLinkedInfo &info)
{
    WIFI_LOGI("Allow auto select device, connState=%{public}d %{public}s, detailedState=%{public}d %{public}s\n",
        info.connState, magic_enum::Enum2Name(info.connState).c_str(), info.detailedState,
        magic_enum::Enum2Name(info.detailedState).c_str());
    if (scanInfos.empty()) {
        WIFI_LOGE("No network,skip network selection.\n");
        return false;
    }

    switch (info.detailedState) {
        case DetailedState::WORKING:
            /* Configure whether to automatically switch the network. */
            if (!WifiSettings::GetInstance().GetWhetherToAllowNetworkSwitchover(m_instId)) {
                WIFI_LOGE("Automatic network switching is not allowed in user configuration.\n");
                return false;
            }
            /* Indicates whether the minimum interval is the minimum interval since the last network selection. */
            if (selectDeviceLastTime != 0) {
                int gap = static_cast<int>(time(0)) - selectDeviceLastTime;
                if (gap < MIN_SELECT_NETWORK_TIME) {
                    WIFI_LOGE("%ds time before we selected the network(30s).\n", gap);
                    return false;
                }
            }

            if (!CurrentDeviceGoodEnough(scanInfos, info)) {
                WIFI_LOGI("The current network is insuffice.\n");
                return true;
            }
            return false;

        case DetailedState::DISCONNECTED:
        case DetailedState::CONNECTION_TIMEOUT:
        case DetailedState::FAILED:
        case DetailedState::CONNECTION_REJECT:
        case DetailedState::CONNECTION_FULL:
            WIFI_LOGI("Auto Select is allowed, detailedState: %{public}d\n", info.detailedState);
            return true;
        case DetailedState::PASSWORD_ERROR:
            WIFI_LOGI("Password error, auto connect to ap quickly.\n");
            return true;
        case DetailedState::NOTWORKING:
            WIFI_LOGI("The current network cannot access the Internet.\n");
            /* Configure whether to automatically switch the network. */
            if (!WifiSettings::GetInstance().GetWhetherToAllowNetworkSwitchover(m_instId)) {
                WIFI_LOGE("Automatic network switching is not allowed in user configuration.\n");
                return false;
            }
            return true;

        default:
            WIFI_LOGE("not allowed auto select!\n");
            return false;
    }
    return false;
}

bool StaAutoConnectService::CurrentDeviceGoodEnough(const std::vector<InterScanInfo> &scanInfos, WifiLinkedInfo &info)
{
    WIFI_LOGI("Enter CurrentDeviceGoodEnough.\n");

    WifiDeviceConfig network;

    /* The network is deleted */
    if (WifiSettings::GetInstance().GetDeviceConfig(info.networkId, network) == -1) {
        WIFI_LOGE("The network is deleted.\n");
        return false;
    }

    int userLastSelectedNetworkId = WifiConfigCenter::GetInstance().GetUserLastSelectedNetworkId(m_instId);
    if (userLastSelectedNetworkId != INVALID_NETWORK_ID && userLastSelectedNetworkId == network.networkId) {
        time_t userLastSelectedNetworkTimeVal = WifiConfigCenter::GetInstance().GetUserLastSelectedNetworkTimeVal(
            m_instId);
        time_t now = time(0);
        int interval = static_cast<int>(now - userLastSelectedNetworkTimeVal);
        if (interval <= TIME_FROM_LAST_SELECTION) {
            WIFI_LOGI("(60s)Current user recent selections time is %ds.\n", interval);
            return true;
        }
    }

    /* Temporary network unqualified */
    if (network.isEphemeral) {
        WIFI_LOGE("The network is isEphemeral.\n");
        return false;
    }

    if (network.keyMgmt == "NONE" || network.keyMgmt.size() == 0) {
        WIFI_LOGE("This network No keyMgmt.\n");
        return false;
    }

    /* The signal strength on the live network does not meet requirements. */
    if (info.rssi < RSSI_DELIMITING_VALUE) {
        WIFI_LOGE("Signal strength insuffice %{public}d < -65.\n", info.rssi);
        return false;
    }
    /*
     * The network is a 2.4 GHz network and is not qualified when the 5G network
     * is available.
     */
    if (Whether24GDevice(info.frequency)) {
        if (WhetherDevice5GAvailable(scanInfos)) {
            WIFI_LOGE("5 GHz is available when the current frequency band is 2.4 GHz.\n");
            return false;
        }
    }
    return true;
}

bool StaAutoConnectService::WhetherDevice5GAvailable(const std::vector<InterScanInfo> &scanInfos)
{
    WIFI_LOGI("Enter WhetherDevice5GAvailable.\n");
    for (auto scaninfo : scanInfos) {
        if (Whether5GDevice(scaninfo.frequency)) {
            return true;
        }
    }
    return false;
}

bool StaAutoConnectService::Whether24GDevice(int frequency)
{
    if (frequency > MIN_24_FREQUENCY && frequency < MAX_24_FREQUENCY) {
        return true;
    } else {
        return false;
    }
}

bool StaAutoConnectService::Whether5GDevice(int frequency)
{
    if (frequency > MIN_5_FREQUENCY && frequency < MAX_5_FREQUENCY) {
        return true;
    } else {
        return false;
    }
}

void StaAutoConnectService::GetAvailableScanInfos(std::vector<InterScanInfo> &availableScanInfos,
    const std::vector<InterScanInfo> &scanInfos, std::vector<std::string> &blockedBssids, WifiLinkedInfo &info)
{
    WIFI_LOGI("Enter GetAvailableScanInfos.\n");
    if (scanInfos.empty()) {
        return;
    }
    bool scanInfosContainCurrentBssid = false;

    for (auto scanInfo : scanInfos) {
        if (scanInfo.ssid.size() == 0) {
            continue;
        }

        /* Check whether the scanning result contains the current BSSID. */
        if (info.connState == ConnState::CONNECTED && scanInfo.bssid == info.bssid) {
            scanInfosContainCurrentBssid = true;
        }

        auto itr = find(blockedBssids.begin(), blockedBssids.end(), scanInfo.bssid);
        if (itr != blockedBssids.end()) { /* Skip Blocklist Network */
            WIFI_LOGD("Skip blocklistedBssid network, ssid: %{public}s.\n", SsidAnonymize(scanInfo.ssid).c_str());
            continue;
        }

        /* Skipping networks with weak signals */
        if (scanInfo.frequency < MIN_5GHZ_BAND_FREQUENCY) {
            if (scanInfo.rssi <= MIN_RSSI_VALUE_24G) {
                WIFI_LOGD("Skip network %{public}s with low 2.4G signals %{public}d.\n",
                    SsidAnonymize(scanInfo.ssid).c_str(), scanInfo.rssi);
                continue;
            }
        } else {
            if (scanInfo.rssi <= MIN_RSSI_VALUE_5G) {
                WIFI_LOGD("Skip network %{public}s with low 5G signals %{public}d.\n",
                    SsidAnonymize(scanInfo.ssid).c_str(), scanInfo.rssi);
                continue;
            }
        }
        availableScanInfos.push_back(scanInfo);
    }
    /*
     * Some scan requests may not include channels for the currently connected
     * network, so the currently connected network will not appear in the scan
     * results. We will not act on these scans to avoid network switching that may
     * trigger disconnections.
     */
    if (info.connState == ConnState::CONNECTED && !scanInfosContainCurrentBssid) {
        WIFI_LOGI("scanInfo is be cleared.\n");
        availableScanInfos.clear();
    }
    return;
}

void StaAutoConnectService::DisableAutoJoin(const std::string &conditionName)
{
    std::lock_guard<std::mutex> lock(autoJoinMutex);
    WIFI_LOGI("Auto Join is disabled by %{public}s.", conditionName.c_str());
    autoJoinConditionsMap.insert_or_assign(conditionName, []() { return false; });
}

void StaAutoConnectService::EnableAutoJoin(const std::string &conditionName)
{
    std::lock_guard<std::mutex> lock(autoJoinMutex);
    WIFI_LOGI("Auto Join disabled by %{public}s is released.", conditionName.c_str());
    autoJoinConditionsMap.erase(conditionName);
}

void StaAutoConnectService::RegisterAutoJoinCondition(const std::string &conditionName,
                                                      const std::function<bool()> &autoJoinCondition)
{
    if (!autoJoinCondition) {
        WIFI_LOGE("the condition of %{public}s is empty.", conditionName.c_str());
        return;
    }
    std::lock_guard<std::mutex> lock(autoJoinMutex);
    WIFI_LOGI("Auto Join condition of %{public}s is registered.", conditionName.c_str());
    if (autoJoinConditionsMap.size() > REGISTERINFO_MAX_NUM) {
        WIFI_LOGW("%{public}s fail autoJoinConditionsMap size is: %{public}d, over 1000",
            __FUNCTION__, static_cast<int>(autoJoinConditionsMap.size()));
        return;
    }
    autoJoinConditionsMap.insert_or_assign(conditionName, autoJoinCondition);
}

void StaAutoConnectService::DeregisterAutoJoinCondition(const std::string &conditionName)
{
    std::lock_guard<std::mutex> lock(autoJoinMutex);
    WIFI_LOGI("Auto Join condition of %{public}s is deregistered.", conditionName.c_str());
    autoJoinConditionsMap.erase(conditionName);
}

bool StaAutoConnectService::IsAllowAutoJoin()
{
    std::lock_guard<std::mutex> lock(autoJoinMutex);
    for (auto condition = autoJoinConditionsMap.rbegin(); condition != autoJoinConditionsMap.rend(); ++condition) {
        if (!condition->second.operator()()) {
            WIFI_LOGI("Auto Join is not allowed because of %{public}s.", condition->first.c_str());
            return false;
        }
    }
    return true;
}
}  // namespace Wifi
}  // namespace OHOS
