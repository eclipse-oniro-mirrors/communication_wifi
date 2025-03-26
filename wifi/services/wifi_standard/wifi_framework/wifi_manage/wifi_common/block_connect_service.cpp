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
#include "block_connect_service.h"
#include "wifi_config_center.h"
#include "wifi_system_timer.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("BlockConnectService");
constexpr int FREQUENT_DISCONNECT_COUNT = 5;
constexpr int64_t FREQUENT_DISCONNECT_TIME_INTERVAL_MAX = 10 * 60 * 1000 * 1000;
constexpr int64_t FREQUENT_DISCONNECT_TIME_INTERVAL_MID = 1 * 60 * 1000 * 1000;
constexpr int64_t FREQUENT_DISCONNECT_TIME_INTERVAL_MIN = 0.5 * 60 * 1000 * 1000;
constexpr int64_t TIMEOUT_CLEAR_SET = 4 * 60 * 1000;

BlockConnectService &BlockConnectService::GetInstance()
{
    static BlockConnectService gStaBlockConnectService;
    return gStaBlockConnectService;
}

BlockConnectService::BlockConnectService()
{
    // Initialize any necessary variables or data structures
    blockConnectPolicies = {
        {DisabledReason::DISABLED_ASSOCIATION_REJECTION,
         DisablePolicy(5 * 60 * 1000 * 1000, 3, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_AUTHENTICATION_FAILURE,
         DisablePolicy(5 * 60 * 1000 * 1000, 3, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_DHCP_FAILURE,
         DisablePolicy(5 * 60 * 1000 * 1000, 2, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_NO_INTERNET_TEMPORARY,
         DisablePolicy(5 * 60 * 1000 * 1000, 1, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_AUTHENTICATION_NO_CREDENTIALS,
         DisablePolicy(-1, 3, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_NO_INTERNET_PERMANENT,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_BY_WIFI_MANAGER,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_BY_WRONG_PASSWORD,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_AUTHENTICATION_NO_SUBSCRIPTION,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_AUTHENTICATION_PRIVATE_EAP_ERROR,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_NETWORK_NOT_FOUND,
         DisablePolicy(5 * 60 * 1000 * 1000, 2, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_CONSECUTIVE_FAILURES,
         DisablePolicy(5 * 60 * 1000 * 1000, 1, WifiDeviceConfigStatus::DISABLED)},
        {DisabledReason::DISABLED_BY_SYSTEM,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_EAP_AKA_FAILURE,
         DisablePolicy(-1, 1, WifiDeviceConfigStatus::PERMEMANTLY_DISABLED)},
        {DisabledReason::DISABLED_DISASSOC_REASON,
         DisablePolicy(5 * 60 * 1000 * 1000, 5, WifiDeviceConfigStatus::DISABLED)},
    };

    validReasons = {
        static_cast<int>(DisconnectDetailReason::UNSPECIFIED),
        static_cast<int>(DisconnectDetailReason::PREV_AUTH_NOT_VALID),
        static_cast<int>(DisconnectDetailReason::DISASSOC_DUE_TO_INACTIVITY),
        static_cast<int>(DisconnectDetailReason::DISASSOC_AP_BUSY),
        static_cast<int>(DisconnectDetailReason::DISASSOC_STA_HAS_LEFT),
        static_cast<int>(DisconnectDetailReason::DISASSOC_IEEE_802_1X_AUTH_FAILED),
        static_cast<int>(DisconnectDetailReason::DISASSOC_LOW_ACK)
    };

    mLastConnectedApInfo = {"", -1, 0};
}

// Destructor
BlockConnectService::~BlockConnectService()
{
    // Clean up any resources
    blockConnectPolicies.clear();
}

void BlockConnectService::Exit()
{
    // Implement the logic to exit the service
    // Clean up any resources
    blockConnectPolicies.clear();
    mLastConnectedApInfo = {"", -1, 0};
}

// Method to check if auto connect is enabled for a given WifiDeviceConfig
bool BlockConnectService::ShouldAutoConnect(const WifiDeviceConfig &config)
{
    // Return true if auto connect is enabled, false otherwise
    WIFI_LOGD("ENTER shouldAutoConnect %d",
        config.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED);
    return config.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED;
}

// Update the selection status of all saved networks and check if disabled networks have expired
bool BlockConnectService::UpdateAllNetworkSelectStatus()
{
    WIFI_LOGD("ENTER updateAllNetworkSelectStatus");
    // Implement the logic to update the selection status of all saved networks
    // and check if disabled networks have expired
    // Return true if successful, false otherwise
    int64_t timestamp = GetElapsedMicrosecondsSinceBoot();
    std::vector<WifiDeviceConfig> results;
    if (WifiSettings::GetInstance().GetDeviceConfig(results) != 0) {
        WIFI_LOGE("Failed to get device config");
        return false;
    }
    for (auto &config : results) {
        WifiSettings::GetInstance().ClearNetworkCandidateScanResult(config.networkId);
        if ((config.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED) &&
            (config.networkSelectionStatus.networkSelectionDisableReason == DisabledReason::DISABLED_NONE)) {
            continue;
        }
        DisablePolicy policy = CalculateDisablePolicy(config.networkSelectionStatus.networkSelectionDisableReason);
        if (policy.disableStatus == WifiDeviceConfigStatus::PERMEMANTLY_DISABLED) {
            LogDisabledConfig(config);
            continue;
        }
        if (policy.disableStatus == WifiDeviceConfigStatus::ENABLED ||
            (config.networkSelectionStatus.networkDisableTimeStamp > 0 &&
            timestamp - config.networkSelectionStatus.networkDisableTimeStamp >= policy.disableTime)) {
            config.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;
            config.networkSelectionStatus.networkSelectionDisableReason = DisabledReason::DISABLED_NONE;
            config.networkSelectionStatus.networkDisableTimeStamp = -1;
            config.networkSelectionStatus.networkDisableCount = 0;
            WifiSettings::GetInstance().AddDeviceConfig(config);
        }
        LogDisabledConfig(config);
    }
    return true;
}

// Enable the selection status of a target network
bool BlockConnectService::EnableNetworkSelectStatus(int targetNetworkId)
{
    WIFI_LOGD("ENTER EnableNetworkSelectStatus");
    // Implement the logic to enable the selection status of a target network
    // Return true if successful, false otherwise
    WifiDeviceConfig targetNetwork;
    if (WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, targetNetwork)) {
        WIFI_LOGE("Failed to get device config %{public}d", targetNetworkId);
        return false;
    }
    targetNetwork.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;
    targetNetwork.networkSelectionStatus.networkSelectionDisableReason = DisabledReason::DISABLED_NONE;
    targetNetwork.networkSelectionStatus.networkDisableTimeStamp = -1;
    targetNetwork.networkSelectionStatus.networkDisableCount = 0;
    WifiSettings::GetInstance().AddDeviceConfig(targetNetwork);
    WIFI_LOGI("EnableNetworkSelectStatus %{public}d %{public}s enabled",
        targetNetworkId, SsidAnonymize(targetNetwork.ssid).c_str());
    return true;
}

DisablePolicy BlockConnectService::CalculateDisablePolicy(DisabledReason disableReason)
{
    // Implement the logic to calculate the disable reason based on the disconnect reason
    // Return the disable reason
    std::map<DisabledReason, DisablePolicy>::iterator it = blockConnectPolicies.find(disableReason);
    if (it == blockConnectPolicies.end()) {
        return DisablePolicy(-1, 0, WifiDeviceConfigStatus::ENABLED);
    }
    return it->second;
}

// Clear the blocklist information of a target network with reason for wpa_supplicant disconnection
bool BlockConnectService::UpdateNetworkSelectStatus(int targetNetworkId, DisabledReason disableReason, int wpaReason)
{
    // Implement the logic to clear the blocklist information of a target network
    // Return true if successful, false otherwise
    WIFI_LOGD("ENTER updateNetworkSelectStatus");
    if (disableReason == DisabledReason::DISABLED_DISASSOC_REASON) {
        if (std::find(validReasons.begin(), validReasons.end(), wpaReason) == validReasons.end()) {
            return false;
        }
    }
    return UpdateNetworkSelectStatus(targetNetworkId, disableReason);
}

// Clear the blocklist information of a target network
bool BlockConnectService::UpdateNetworkSelectStatus(int targetNetworkId, DisabledReason disableReason)
{
    // Implement the logic to clear the blocklist information of a target network
    // Return true if successful, false otherwise
    WIFI_LOGD("ENTER updateNetworkSelectStatus");
    WifiDeviceConfig targetNetwork;
    int64_t timestamp = GetElapsedMicrosecondsSinceBoot();
    if (WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, targetNetwork)) {
        WIFI_LOGE("Failed to get device config %{public}d", targetNetworkId);
        return false;
    }
    DisablePolicy disablePolicy = CalculateDisablePolicy(disableReason);
    if (disablePolicy.disableStatus == WifiDeviceConfigStatus::ENABLED) {
        targetNetwork.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;
        targetNetwork.networkSelectionStatus.networkSelectionDisableReason = disableReason;
        targetNetwork.networkSelectionStatus.networkDisableTimeStamp = -1;
        targetNetwork.networkSelectionStatus.networkDisableCount = 0;
        return true;
    }
    if (targetNetwork.networkSelectionStatus.networkSelectionDisableReason == disableReason) {
        targetNetwork.networkSelectionStatus.networkDisableCount++;
    } else {
        targetNetwork.networkSelectionStatus.networkDisableCount = 1;
        targetNetwork.networkSelectionStatus.networkSelectionDisableReason = disableReason;
    }

    if (targetNetwork.networkSelectionStatus.networkDisableCount >= disablePolicy.disableCount) {
        targetNetwork.networkSelectionStatus.status = disablePolicy.disableStatus;
        targetNetwork.networkSelectionStatus.networkSelectionDisableReason = disableReason;
    }
    targetNetwork.networkSelectionStatus.networkDisableTimeStamp = timestamp;
    WifiSettings::GetInstance().AddDeviceConfig(targetNetwork);
    WIFI_LOGI("updateNetworkSelectStatus networkId %{public}d %{public}s %{public}d",
        targetNetworkId, SsidAnonymize(targetNetwork.ssid).c_str(), disableReason);
    return true;
}

// Check if the given BSSID has frequent disconnects with the last connected network
bool BlockConnectService::IsFrequentDisconnect(std::string bssid, int wpaReason)
{
    // Implement the logic to check if the given BSSID has frequent disconnects
    // with the last connected network
    // Return true if frequent disconnects, false otherwise
    int64_t timestamp = GetElapsedMicrosecondsSinceBoot();
    int64_t time_duration = timestamp - mLastConnectedApInfo.lastDisconnectTimestamp;
    WIFI_LOGD("ENTER isFrequentDisconnect %{public}" PRId64"  %{public}s", time_duration, MacAnonymize(bssid).c_str());
    WIFI_LOGD("mLastConnectedApInfo alreadyConnectedCount %{public}d", mLastConnectedApInfo.alreadyConnectedCount);
    mLastConnectedApInfo.lastDisconnectTimestamp = timestamp;
    if (mLastConnectedApInfo.bssid != bssid) {
        mLastConnectedApInfo.bssid = bssid;
        mLastConnectedApInfo.alreadyConnectedCount = 1;
        return false;
    }

    if (time_duration > FREQUENT_DISCONNECT_TIME_INTERVAL_MAX) {
        mLastConnectedApInfo.bssid = bssid;
        mLastConnectedApInfo.alreadyConnectedCount = 1;
        return false;
    }
    if (wpaReason == static_cast<int>(DisconnectDetailReason::DEAUTH_STA_IS_LEFING) ||
        wpaReason == static_cast<int>(DisconnectDetailReason::DISASSOC_STA_HAS_LEFT)) {
        if (time_duration < FREQUENT_DISCONNECT_TIME_INTERVAL_MIN) {
            WIFI_LOGD("isFrequentDisconnect case min %{public}s %{public}d  duration %{public}" PRId64,
                MacAnonymize(bssid).c_str(), wpaReason, time_duration);
            mLastConnectedApInfo.alreadyConnectedCount++;
        }
    } else if (time_duration < FREQUENT_DISCONNECT_TIME_INTERVAL_MID) {
        WIFI_LOGD("isFrequentDisconnect case mid %{public}s %{public}d duration %{public}" PRId64,
            MacAnonymize(bssid).c_str(), wpaReason, time_duration);
        mLastConnectedApInfo.alreadyConnectedCount++;
    }
    if (mLastConnectedApInfo.alreadyConnectedCount >= FREQUENT_DISCONNECT_COUNT) {
        WIFI_LOGI("isFrequentDisconnect %{public}s %{public}d count %{public}d",
            MacAnonymize(bssid).c_str(), wpaReason, mLastConnectedApInfo.alreadyConnectedCount);
        return true;
    }
    return false;
}

// Check if the given targetNetworkId is blocked due to wrong password
bool BlockConnectService::IsWrongPassword(int targetNetworkId)
{
    // Implement the logic to check if the given targetNetworkId is blocked due to wrong password
    // Return true if blocked due to wrong password, false otherwise
    WifiDeviceConfig targetNetwork;
    if (WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, targetNetwork)) {
        WIFI_LOGE("Failed to get device config %{public}d", targetNetworkId);
        return false;
    }

    if (targetNetwork.numAssociation == 0) {
        return true;
    }
    return false;
}

void BlockConnectService::EnableAllNetworksByEnteringSettings(std::vector<DisabledReason> enableReasons)
{
    WIFI_LOGI("ENTER EnableAllNetworksByEnteringSettings");
    std::vector<WifiDeviceConfig> results;
    if (WifiSettings::GetInstance().GetDeviceConfig(results) != 0) {
        WIFI_LOGE("Failed to get device config");
        return;
    }
    for (auto &config : results) {
        if (config.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED) {
            continue;
        }
        if (std::find(enableReasons.begin(), enableReasons.end(),
            config.networkSelectionStatus.networkSelectionDisableReason) != enableReasons.end()) {
            config.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;
            config.networkSelectionStatus.networkSelectionDisableReason = DisabledReason::DISABLED_NONE;
            config.networkSelectionStatus.networkDisableTimeStamp = -1;
            config.networkSelectionStatus.networkDisableCount = 0;
            WifiSettings::GetInstance().AddDeviceConfig(config);
        }
    }
}

void BlockConnectService::OnReceiveSettingsEnterEvent(bool isEnter)
{
    WIFI_LOGI("ENTER OnReceiveSettingsEnterEvent %{public}d", static_cast<int>(isEnter));
    if (isEnter) {
        std::vector<DisabledReason> enableReasons = {
            DisabledReason::DISABLED_AUTHENTICATION_FAILURE,
            DisabledReason::DISABLED_ASSOCIATION_REJECTION,
            DisabledReason::DISABLED_DHCP_FAILURE,
            DisabledReason::DISABLED_CONSECUTIVE_FAILURES,
        };
        EnableAllNetworksByEnteringSettings(enableReasons);
        ReleaseUnusableBssidSet();
    }
}

void BlockConnectService::LogDisabledConfig(const WifiDeviceConfig &config)
{
    if (config.networkSelectionStatus.status == WifiDeviceConfigStatus::ENABLED) {
        WIFI_LOGD("%{public}s config is ENABLED", SsidAnonymize(config.ssid).c_str());
        return;
    }
    if (config.networkSelectionStatus.status == WifiDeviceConfigStatus::DISABLED) {
        WIFI_LOGI("%{public}s config is DISABLED due to reason: %{public}d",
            SsidAnonymize(config.ssid).c_str(), config.networkSelectionStatus.networkSelectionDisableReason);
        return;
    }
    if (config.networkSelectionStatus.status == WifiDeviceConfigStatus::PERMEMANTLY_DISABLED) {
        WIFI_LOGI("%{public}s  networkId :%{public}d config is PERMEMANTLY DISABLED due to reason: %{public}d",
            SsidAnonymize(config.ssid).c_str(), config.networkId,
            config.networkSelectionStatus.networkSelectionDisableReason);
        return;
    }
}

void BlockConnectService::DealStaStopped(int instId)
{
    if (instId != 0) {
        WIFI_LOGD("sta stopped, but instId is %{public}d", instId);
        return;
    }
    if (WifiConfigCenter::GetInstance().GetScreenState() == MODE_STATE_OPEN) {
        ReleaseUnusableBssidSet();
    }
}

void BlockConnectService::NotifyWifiConnFailedInfo(int targetNetworkId, std::string bssid, DisabledReason disableReason)
{
    WifiDeviceConfig targetNetwork;
    if (WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, targetNetwork)) {
        WIFI_LOGE("Failed to get device config %{public}d", targetNetworkId);
        return;
    }
    std::lock_guard<std::mutex> lock(bssidMutex_);
    if (disableReason == DisabledReason::DISABLED_ASSOCIATION_REJECTION
        || disableReason == DisabledReason::DISABLED_AUTHENTICATION_FAILURE) {
        if (targetNetwork.ssid != curUnusableSsid_ ||
            !WifiSettings::GetInstance().InKeyMgmtBitset(targetNetwork, curUnusableKeyMgmt_)) {
            autoJoinUnusableBssidSet_.clear();
        }
        if (!bssid.empty()) {
            autoJoinUnusableBssidSet_.insert(bssid);
            curUnusableSsid_ = targetNetwork.ssid;
            curUnusableKeyMgmt_ = targetNetwork.keyMgmt;
        }
    }
}

void BlockConnectService::ReleaseUnusableBssidSet()
{
    StopClearSetTimer();
    std::lock_guard<std::mutex> lock(bssidMutex_);
    autoJoinUnusableBssidSet_.clear();
    curUnusableSsid_ = "";
    curUnusableKeyMgmt_ = "";
}

bool BlockConnectService::IsBssidMatchUnusableSet(std::string bssid)
{
    std::lock_guard<std::mutex> lock(bssidMutex_);
    for (auto curBssid : autoJoinUnusableBssidSet_) {
        if (bssid == curBssid) {
            WIFI_LOGI("current bssid %{public}s match unusable bssid set.", MacAnonymize(bssid).c_str());
            return true;
        }
    }
    return false;
}

void BlockConnectService::StartClearSetTimer()
{
    WIFI_LOGD("%{public}s, enter", __FUNCTION__);
    std::lock_guard<std::mutex> lock(clearSetTimerMutex_);
    if (clearSetTimerId_ != 0) {
        WIFI_LOGI("%{public}s, clearSetTimerId_ is not zero", __FUNCTION__);
        return;
    }
    std::shared_ptr<WifiSysTimer> clearSetTimer =
        std::make_shared<WifiSysTimer>(false, 0, true, false);
    std::function<void()> callback = [this]() { this->ClearSetTimerCallback(); };
    clearSetTimer->SetCallbackInfo(callback);
    clearSetTimerId_ = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(clearSetTimer);
    int64_t currentTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
    MiscServices::TimeServiceClient::GetInstance()->StartTimer(clearSetTimerId_, currentTime + TIMEOUT_CLEAR_SET);
    WIFI_LOGI("%{public}s, succuss", __FUNCTION__);
}

void BlockConnectService::StopClearSetTimer()
{
    WIFI_LOGI("enter %{public}s, ", __FUNCTION__);
    std::lock_guard<std::mutex> lock(clearSetTimerMutex_);
    if (clearSetTimerId_ == 0) {
        WIFI_LOGE("%{public}s, clearSetTimerId_ is zero", __FUNCTION__);
        return;
    } else {
        MiscServices::TimeServiceClient::GetInstance()->StopTimer(clearSetTimerId_);
        MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(clearSetTimerId_);
        clearSetTimerId_ = 0;
        return;
    }
}

void BlockConnectService::ClearSetTimerCallback()
{
    std::lock_guard<std::mutex> lock(bssidMutex_);
    autoJoinUnusableBssidSet_.clear();
    curUnusableSsid_ = "";
    curUnusableKeyMgmt_ = "";
}
}
}