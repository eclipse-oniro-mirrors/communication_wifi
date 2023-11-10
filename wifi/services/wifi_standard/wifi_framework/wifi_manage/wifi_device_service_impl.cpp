/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "wifi_device_service_impl.h"
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include "wifi_permission_utils.h"
#include "wifi_internal_msg.h"
#include "wifi_auth_center.h"
#include "wifi_config_center.h"
#ifdef OHOS_ARCH_LITE
#include "wifi_internal_event_dispatcher_lite.h"
#else
#include "wifi_internal_event_dispatcher.h"
#include "xcollie/watchdog.h"
#include "wifi_sa_manager.h"
#include "wifi_settings.h"
#include "mac_address.h"
#include "wifi_p2p_service_impl.h"
#include "wifi_country_code_manager.h"
#endif
#include "wifi_manager.h"
#include "wifi_service_manager.h"
#include "wifi_protect_manager.h"
#include "wifi_logger.h"
#include "define.h"
#include "wifi_common_util.h"
#include "wifi_protect_manager.h"
#include "wifi_global_func.h"

DEFINE_WIFILOG_LABEL("WifiDeviceServiceImpl");
namespace OHOS {
namespace Wifi {
#ifdef OHOS_ARCH_LITE
std::mutex WifiDeviceServiceImpl::g_instanceLock;
std::shared_ptr<WifiDeviceServiceImpl> WifiDeviceServiceImpl::g_instance = nullptr;
std::shared_ptr<WifiDeviceServiceImpl> WifiDeviceServiceImpl::GetInstance()
{
    if (g_instance == nullptr) {
        std::lock_guard<std::mutex> autoLock(g_instanceLock);
        if (g_instance == nullptr) {
            std::shared_ptr<WifiDeviceServiceImpl> service = std::make_shared<WifiDeviceServiceImpl>();
            g_instance = service;
        }
    }
    return g_instance;
}

void WifiDeviceServiceImpl::OnStart()
{
    if (mState == ServiceRunningState::STATE_RUNNING) {
        WIFI_LOGW("Service has already started.");
        return;
    }
    
    WifiManager::GetInstance();
    mState = ServiceRunningState::STATE_RUNNING;
    WIFI_LOGI("Start sta service!");
}

void WifiDeviceServiceImpl::OnStop()
{
    mState = ServiceRunningState::STATE_NOT_START;
    WIFI_LOGI("Stop sta service!");
}
#endif


WifiDeviceServiceImpl::WifiDeviceServiceImpl()
#ifdef OHOS_ARCH_LITE
    : mState(ServiceRunningState::STATE_NOT_START)
#endif
{
    WIFI_LOGI("enter WifiDeviceServiceImpl");
}

#ifndef OHOS_ARCH_LITE
WifiDeviceServiceImpl::WifiDeviceServiceImpl(int instId) : WifiDeviceStub(instId)
{
    WIFI_LOGI("enter WifiDeviceServiceImpl");
    using TimeOutCallback = std::function<void()>;
    const uint32_t TIMEOUT_APP_EVENT = 3000;
    const uint32_t TIMEOUT_THERMAL_EVENT = 3000;
    if (eventSubscriber_ == nullptr && appEventTimerId == 0) {
        TimeOutCallback timeoutCallback = std::bind(&WifiDeviceServiceImpl::RegisterAppRemoved, this);
        WifiTimer::GetInstance()->Register(timeoutCallback, appEventTimerId, TIMEOUT_APP_EVENT, false);
    }

    if (thermalLevelSubscriber_ == nullptr && thermalTimerId == 0) {
        TimeOutCallback timeoutCallback = std::bind(&WifiDeviceServiceImpl::RegisterThermalLevel, this);
        WifiTimer::GetInstance()->Register(timeoutCallback, thermalTimerId, TIMEOUT_THERMAL_EVENT, false);
    }
}
#endif

WifiDeviceServiceImpl::~WifiDeviceServiceImpl()
{
    WIFI_LOGI("enter ~WifiDeviceServiceImpl");
#ifndef OHOS_ARCH_LITE
    if (eventSubscriber_ != nullptr) {
        UnRegisterAppRemoved();
    }

    if (thermalLevelSubscriber_ != nullptr) {
        UnRegisterThermalLevel();
    }
#endif
}

ErrCode WifiDeviceServiceImpl::EnableWifi()
{
    ErrCode errCode = CheckCanEnableWifi();
    if (errCode != WIFI_OPT_SUCCESS) {
        return errCode;
    }
#ifdef OHOS_ARCH_LITE
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetWifiMidState(m_instId);
    if (curState != WifiOprMidState::CLOSED) {
        WIFI_LOGI("current wifi state is %{public}d", static_cast<int>(curState));
        if (curState == WifiOprMidState::CLOSING) { /* when current wifi is closing, return */
            return WIFI_OPT_OPEN_FAIL_WHEN_CLOSING;
        } else {
            return WIFI_OPT_OPEN_SUCC_WHEN_OPENED;
        }
    }

#ifdef FEATURE_AP_SUPPORT
    WifiOprMidState apState = WifiConfigCenter::GetInstance().GetApMidState(0);
    if (apState != WifiOprMidState::CLOSED) {
#ifdef FEATURE_STA_AP_EXCLUSION
        errCode = WifiManager::GetInstance().AutoStopApService(AutoStartOrStopServiceReason::STA_AP_EXCLUSION);
        if (errCode != WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED) {
            return errCode;
        }
#else
        WIFI_LOGW("current ap state is %{public}d, please close SoftAp first!",
            static_cast<int>(curState));
        return WIFI_OPT_NOT_SUPPORTED;
#endif
    }
#endif

    if (!WifiConfigCenter::GetInstance().SetWifiMidState(curState, WifiOprMidState::OPENING, m_instId)) {
        WIFI_LOGI("set wifi mid state opening failed!");
        return WIFI_OPT_OPEN_SUCC_WHEN_OPENED;
    }

    errCode = WIFI_OPT_FAILED;
    do {
        if (WifiServiceManager::GetInstance().CheckAndEnforceService(WIFI_SERVICE_STA) < 0) {
            break;
        }
        IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
        if (pService == nullptr) {
            WIFI_LOGE("Create %{public}s service failed!", WIFI_SERVICE_STA);
            break;
        }

        errCode = pService->RegisterStaServiceCallback(WifiManager::GetInstance().GetStaCallback());
        if (errCode != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("Register sta service callback failed!");
            break;
        }
        errCode = pService->EnableWifi();
        if (errCode != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("service enable sta failed, ret %{public}d!", static_cast<int>(errCode));
            break;
        }
    } while (false);
    if (errCode != WIFI_OPT_SUCCESS) {
        WifiConfigCenter::GetInstance().SetWifiMidState(WifiOprMidState::OPENING, WifiOprMidState::CLOSED, m_instId);
        WifiServiceManager::GetInstance().UnloadService(WIFI_SERVICE_STA, m_instId);
        return errCode;
    }
#ifdef FEATURE_P2P_SUPPORT
    sptr<WifiP2pServiceImpl> p2pService = WifiP2pServiceImpl::GetInstance();
    if (p2pService != nullptr && p2pService->EnableP2p() != WIFI_OPT_SUCCESS) {
        // only record to log
        WIFI_LOGE("Enable P2p failed!");
    }
#endif

    WifiSettings::GetInstance().SyncWifiConfig();
    WifiManager::GetInstance().StopUnloadStaSaTimer();
    if (WifiConfigCenter::GetInstance().GetOperatorWifiType(m_instId) ==
        static_cast<int>(OperatorWifiType::USER_OPEN_WIFI_IN_AIRPLANEMODE)) {
        WIFI_LOGI("EnableWifi, user opened wifi in airplane mode!");
        return WIFI_OPT_SUCCESS;
    }

    int operatorWifiType = static_cast<int>(OperatorWifiType::USER_OPEN_WIFI_IN_NO_AIRPLANEMODE);
    if (WifiConfigCenter::GetInstance().GetAirplaneModeState() == MODE_STATE_OPEN) {
        operatorWifiType = static_cast<int>(OperatorWifiType::USER_OPEN_WIFI_IN_AIRPLANEMODE);
        WIFI_LOGI("EnableWifi, current airplane mode is opened, user open wifi!");
    }
    WifiConfigCenter::GetInstance().SetOperatorWifiType(operatorWifiType, m_instId);
    return WIFI_OPT_SUCCESS;
#else
    if (m_instId == 0) {
        WifiSettings::GetInstance().SetWifiToggledState(true);
    }
    errCode = WifiManager::GetInstance().WifiToggled(1, m_instId);
    return errCode;
#endif
}

ErrCode WifiDeviceServiceImpl::DisableWifi()
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("DisableWifi: NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DisableWifi:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DisableWifi:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
#ifdef OHOS_ARCH_LITE
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetWifiMidState(m_instId);
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGI("current wifi state is %{public}d", static_cast<int>(curState));
        if (curState == WifiOprMidState::OPENING) { /* when current wifi is opening, return */
            return WIFI_OPT_CLOSE_FAIL_WHEN_OPENING;
        } else {
            return WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED;
        }
    }

#ifdef FEATURE_P2P_SUPPORT
    sptr<WifiP2pServiceImpl> p2pService = WifiP2pServiceImpl::GetInstance();
    if (p2pService != nullptr) {
        ErrCode errCode = p2pService->DisableP2p();
        if (errCode != WIFI_OPT_SUCCESS && errCode != WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED) {
            WIFI_LOGE("Disable P2p failed!");
            return WIFI_OPT_FAILED;
        }
    }
#endif

    if (!WifiConfigCenter::GetInstance().SetWifiMidState(curState, WifiOprMidState::CLOSING, m_instId)) {
        WIFI_LOGI("set wifi mid state opening failed! may be other activity has been operated");
        return WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED;
    }
    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        WifiConfigCenter::GetInstance().SetWifiMidState(WifiOprMidState::CLOSED, m_instId);
        WifiServiceManager::GetInstance().UnloadService(WIFI_SERVICE_STA, m_instId);
        return WIFI_OPT_SUCCESS;
    }
    ErrCode ret = pService->DisableWifi();
    if (ret != WIFI_OPT_SUCCESS) {
        WifiConfigCenter::GetInstance().SetWifiMidState(WifiOprMidState::CLOSING, WifiOprMidState::RUNNING, m_instId);
    } else {
        WifiConfigCenter::GetInstance().SetStaLastRunState(false, m_instId);
        WifiManager::GetInstance().GetAirplaneModeByDatashare();
        if (WifiConfigCenter::GetInstance().GetOperatorWifiType(m_instId) ==
            static_cast<int>(OperatorWifiType::USER_OPEN_WIFI_IN_AIRPLANEMODE)) {
            if (WifiConfigCenter::GetInstance().GetAirplaneModeState() == MODE_STATE_OPEN) {
                WifiConfigCenter::GetInstance().SetOperatorWifiType(
                    static_cast<int>(OperatorWifiType::USER_CLOSE_WIFI_IN_AIRPLANEMODE), m_instId);
                WIFI_LOGI("DisableWifi, current airplane mode is opened, user close wifi!");
            }
        } else {
            WifiConfigCenter::GetInstance().SetOperatorWifiType(
                static_cast<int>(OperatorWifiType::USER_CLOSE_WIFI_IN_NO_AIRPLANEMODE), m_instId);
        }
    }
    return ret;
#else
    if (m_instId == 0) {
        WifiSettings::GetInstance().SetWifiToggledState(false);
    }
    ErrCode errCode = WifiManager::GetInstance().WifiToggled(0, m_instId);
    return errCode;
#endif
}

ErrCode WifiDeviceServiceImpl::InitWifiProtect(const WifiProtectType &protectType, const std::string &protectName)
{
    /* refer to WifiProtectManager::GetInstance().InitWifiProtect, DO NOT support now! */
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetWifiProtectRef(const WifiProtectMode &protectMode, const std::string &protectName)
{
#ifdef OHOS_ARCH_LITE
    /* refer to WifiProtectManager::GetInstance().GetWifiProtect, DO NOT support now! */
    return WIFI_OPT_SUCCESS;
#else
    if (!WifiProtectManager::GetInstance().GetWifiProtect(protectMode, protectName)) {
        WIFI_LOGE("App %{public}s set protect mode %{public}d failed.",
            protectName.c_str(), static_cast<int>(protectMode));
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
#endif
}

ErrCode WifiDeviceServiceImpl::PutWifiProtectRef(const std::string &protectName)
{
#ifdef OHOS_ARCH_LITE
    /* refer to WifiProtectManager::GetInstance().PutWifiProtect, DO NOT support now! */
    return WIFI_OPT_SUCCESS;
#else
    if (!WifiProtectManager::GetInstance().PutWifiProtect(protectName)) {
        WIFI_LOGE("App %{public}s remove protect mode failed.", protectName.c_str());
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
#endif
}

bool WifiDeviceServiceImpl::CheckConfigEap(const WifiDeviceConfig &config)
{
    if (config.keyMgmt != KEY_MGMT_EAP) {
        WIFI_LOGE("CheckConfigEap: keyMgmt is not EAP!");
        return false;
    }
    if (config.wifiEapConfig.eap == EAP_METHOD_TLS) {
        if (config.wifiEapConfig.identity.empty() ||
            (config.wifiEapConfig.certEntry.size() == 0 &&
            (config.wifiEapConfig.clientCert.empty() ||
            config.wifiEapConfig.privateKey.empty()))) {
            WIFI_LOGE("CheckConfigEap: with invalid TLS params!");
            return false;
        }
        return true;
    } else if (config.wifiEapConfig.eap == EAP_METHOD_PEAP) {
        if (config.wifiEapConfig.identity.empty() || config.wifiEapConfig.password.empty()) {
            WIFI_LOGE("CheckConfigEap: with invalid PEAP params!");
            return false;
        }
        return true;
    } else {
        WIFI_LOGE("EAP:%{public}s unsupported!", config.wifiEapConfig.eap.c_str());
    }
    return false;
}

bool WifiDeviceServiceImpl::CheckConfigPwd(const WifiDeviceConfig &config)
{
    if ((config.ssid.length() <= 0) || (config.keyMgmt.length()) <= 0) {
        WIFI_LOGE("CheckConfigPwd: invalid ssid or keyMgmt!");
        return false;
    }

    WIFI_LOGI("CheckConfigPwd: keyMgmt = %{public}s!", config.keyMgmt.c_str());
    if (config.keyMgmt == KEY_MGMT_EAP) {
        return CheckConfigEap(config);
    }

    if ((config.keyMgmt != KEY_MGMT_NONE && config.keyMgmt != KEY_MGMT_WEP) &&
        config.preSharedKey.empty()) {
        WIFI_LOGE("CheckConfigPwd: preSharedKey is empty!");
        return false;
    }

    int len = config.preSharedKey.length();
    bool isAllHex = std::all_of(config.preSharedKey.begin(), config.preSharedKey.end(), isxdigit);
    WIFI_LOGI("CheckConfigPwd, ssid: %{public}s, psk len: %{public}d", SsidAnonymize(config.ssid).c_str(), len);
    if (config.keyMgmt == KEY_MGMT_WEP) {
        for (int i = 0; i != WEPKEYS_SIZE; ++i) {
            if (!config.wepKeys[i].empty()) { // wep
                int wepLen = config.wepKeys[i].size();
                if (wepLen == WEP_KEY_LEN1 || wepLen == WEP_KEY_LEN2 || wepLen == WEP_KEY_LEN3) {
                    return true;
                }
                constexpr int MULTIPLE_HEXT_TO_ASCII = 2;
                if (wepLen == (WEP_KEY_LEN1 * MULTIPLE_HEXT_TO_ASCII) ||
                    wepLen == (WEP_KEY_LEN2 * MULTIPLE_HEXT_TO_ASCII) ||
                    wepLen == (WEP_KEY_LEN3 * MULTIPLE_HEXT_TO_ASCII)) {
                    return isAllHex;
                }
                WIFI_LOGE("CheckConfigPwd: invalid wepLen: %{public}d!", wepLen);
                return false;
            }
        }
        return true;
    }
    if (config.keyMgmt == KEY_MGMT_NONE) {
        return config.preSharedKey.empty();
    }
    int minLen = config.keyMgmt == KEY_MGMT_SAE ? MIN_SAE_LEN : MIN_PSK_LEN;
    int maxLen = isAllHex ? MAX_HEX_LEN : MAX_PRESHAREDKEY_LEN;
    if (len < minLen || len > maxLen) {
        WIFI_LOGE("CheckConfigPwd: preSharedKey length error: %{public}d", len);
        return false;
    }
    return true;
}

ErrCode WifiDeviceServiceImpl::CheckCallingUid(int &uid)
{
#ifndef OHOS_ARCH_LITE
    uid = GetCallingUid();
    if (!IsForegroundApp(uid)) {
        return WIFI_OPT_INVALID_PARAM;
    }
    return WIFI_OPT_SUCCESS;
#else
    return WIFI_OPT_NOT_SUPPORTED;
#endif
}

ErrCode WifiDeviceServiceImpl::CheckRemoveCandidateConfig(void)
{
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("CheckRemoveCandidateConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        WIFI_LOGE("CheckRemoveCandidateConfig:IsStaServiceRunning not running!");
        return WIFI_OPT_STA_NOT_OPENED;
    }

    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::RemoveCandidateConfig(const WifiDeviceConfig &config)
{
    ErrCode ret = CheckRemoveCandidateConfig();
    if (ret != WIFI_OPT_SUCCESS) {
        return ret;
    }
    /* check the caller's uid */
    int uid = 0;
    if (CheckCallingUid(uid) != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("CheckCallingUid failed!");
        return WIFI_OPT_INVALID_PARAM;
    }
    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        WIFI_LOGE("pService is nullptr!");
        return WIFI_OPT_STA_NOT_OPENED;
    }
    /* get all candidate configs */
    std::vector<WifiDeviceConfig> configs;
    if (WifiConfigCenter::GetInstance().GetCandidateConfigs(uid, configs) != 0) {
        WIFI_LOGE("NOT find the caller's configs!");
        return WIFI_OPT_INVALID_CONFIG;
    }
    /* find the networkId of the removed config */
    int networkId = INVALID_NETWORK_ID;
    size_t size = configs.size();
    for (size_t i = 0; i < size; i++) {
        if (configs[i].ssid == config.ssid) {
            networkId = configs[i].networkId;
            WIFI_LOGI("find the removed config, networkId:%{public}d!", networkId);
            break;
        }
    }
    /* removed the config */
    if (networkId != INVALID_NETWORK_ID) {
        return pService->RemoveCandidateConfig(uid, networkId);
    }
    return WIFI_OPT_INVALID_CONFIG;
}

ErrCode WifiDeviceServiceImpl::RemoveCandidateConfig(int networkId)
{
    ErrCode ret = CheckRemoveCandidateConfig();
    if (ret != WIFI_OPT_SUCCESS) {
        return ret;
    }
    int uid = 0;
    if (CheckCallingUid(uid) != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("CheckCallingUid failed!");
        return WIFI_OPT_INVALID_PARAM;
    }
    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        WIFI_LOGE("pService is nullptr!");
        return WIFI_OPT_STA_NOT_OPENED;
    }
    if (networkId == INVALID_NETWORK_ID) {
        return pService->RemoveAllCandidateConfig(uid);
    } else {
        return pService->RemoveCandidateConfig(uid, networkId);
    }
}

ErrCode WifiDeviceServiceImpl::AddDeviceConfig(const WifiDeviceConfig &config, int &result, bool isCandidate)
{
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("AddDeviceConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!isCandidate) {
        if (!WifiAuthCenter::IsSystemAppByToken()) {
            WIFI_LOGE("AddDeviceConfig: NOT System APP, PERMISSION_DENIED!");
            return WIFI_OPT_NON_SYSTEMAPP;
        }
        if (WifiPermissionUtils::VerifySetWifiConfigPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("AddDeviceConfig:VerifySetWifiConfigPermission PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
        }
    }

    if (!CheckConfigPwd(config)) {
        WIFI_LOGE("CheckConfigPwd failed!");
        return WIFI_OPT_INVALID_PARAM;
    }

    if (isCandidate && config.bssid.length() != 0 && CheckMacIsValid(config.bssid) != 0) {
        WIFI_LOGE("AddDeviceConfig:VerifyBSSID failed!");
        return WIFI_OPT_INVALID_PARAM;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    WifiDeviceConfig updateConfig = config;
#ifdef SUPPORT_RANDOM_MAC_ADDR
    WifiMacAddrInfo macAddrInfo;
    macAddrInfo.bssid = config.bssid;
    macAddrInfo.bssidType = config.bssidType;
    std::string macAddr =
        WifiSettings::GetInstance().GetMacAddrPairs(WifiMacAddrInfoType::WIFI_SCANINFO_MACADDR_INFO,
            macAddrInfo);
    if (macAddr.empty()) {
        WIFI_LOGW("%{public}s: record not found, bssid:%{private}s, bssidType:%{public}d",
            __func__, config.bssid.c_str(), config.bssidType);
    } else {
        WIFI_LOGI("%{public}s: the record is exists, bssid:%{private}s, bssidType:%{public}d, randomMac:%{private}s",
            __func__, config.bssid.c_str(), config.bssidType, macAddr.c_str());
        /* random MAC address are translated into real MAC address */
        if (!config.bssid.empty() &&
            config.bssidType == RANDOM_DEVICE_ADDRESS) {
            updateConfig.bssid = macAddr;
            updateConfig.bssidType = REAL_DEVICE_ADDRESS;
            WIFI_LOGI("%{public}s: the record is updated, bssid:%{private}s, bssidType:%{public}d",
                __func__, updateConfig.bssid.c_str(), updateConfig.bssidType);
        }
    }
#endif
    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (isCandidate) {
        int uid = 0;
        if (CheckCallingUid(uid) != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("CheckCallingUid failed!");
            return WIFI_OPT_INVALID_PARAM;
        }
        return pService->AddCandidateConfig(uid, updateConfig, result);
    }

    int retNetworkId = pService->AddDeviceConfig(updateConfig);
    if (retNetworkId < 0) {
        return WIFI_OPT_FAILED;
    }
    result = retNetworkId;
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::UpdateDeviceConfig(const WifiDeviceConfig &config, int &result)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("UpdateDeviceConfig: NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("UpdateDeviceConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifySetWifiConfigPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("UpdateDeviceConfig:VerifySetWifiConfigPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    int retNetworkId = pService->UpdateDeviceConfig(config);
    if (retNetworkId <= INVALID_NETWORK_ID) {
        return WIFI_OPT_FAILED;
    }
    result = retNetworkId;
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::RemoveDevice(int networkId)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("RemoveDevice: NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RemoveDevice:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RemoveDevice:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (networkId < 0) {
        return WIFI_OPT_INVALID_PARAM;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->RemoveDevice(networkId);
}

ErrCode WifiDeviceServiceImpl::RemoveAllDevice()
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("RemoveAllDevice:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RemoveAllDevice:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RemoveAllDevice:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->RemoveAllDevice();
}

ErrCode WifiDeviceServiceImpl::GetDeviceConfigs(std::vector<WifiDeviceConfig> &result, bool isCandidate)
{
    if (!isCandidate && !WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("GetDeviceConfigs:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifyGetWifiInfoInternalPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetDeviceConfigs:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");

        if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("GetDeviceConfigs:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
        }

    #ifndef SUPPORT_RANDOM_MAC_ADDR
        if (WifiPermissionUtils::VerifyGetScanInfosPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("GetDeviceConfigs:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
        }
    #endif

        if (!isCandidate) {
            if (WifiPermissionUtils::VerifyGetWifiConfigPermission() == PERMISSION_DENIED) {
                WIFI_LOGE("GetDeviceConfigs:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
                return WIFI_OPT_PERMISSION_DENIED;
            }
        }
    }

    if (isCandidate) {
        int uid = 0;
        if (CheckCallingUid(uid) != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("CheckCallingUid failed!");
            return WIFI_OPT_INVALID_PARAM;
        }
        WifiConfigCenter::GetInstance().GetCandidateConfigs(uid, result);
    } else {
        WifiConfigCenter::GetInstance().GetDeviceConfig(result);
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::EnableDeviceConfig(int networkId, bool attemptEnable)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("EnableDeviceConfig:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("EnableDeviceConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (networkId < 0) {
        return WIFI_OPT_INVALID_PARAM;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->EnableDeviceConfig(networkId, attemptEnable);
}

ErrCode WifiDeviceServiceImpl::DisableDeviceConfig(int networkId)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("DisableDeviceConfig:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DisableDeviceConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("DisableDeviceConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (networkId < 0) {
        return WIFI_OPT_INVALID_PARAM;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->DisableDeviceConfig(networkId);
}

ErrCode WifiDeviceServiceImpl::ConnectToNetwork(int networkId, bool isCandidate)
{
    if (isCandidate) {
        if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("ConnectToCandidateConfig:VerifySetWifiInfoPermission PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
        }
    } else {
        if (!WifiAuthCenter::IsSystemAppByToken()) {
            WIFI_LOGE("ConnectToCandidateConfig:NOT System APP, PERMISSION_DENIED!");
            return WIFI_OPT_NON_SYSTEMAPP;
        }
        if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("ConnectToNetwork:VerifyWifiConnectionPermission PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
        }
    }

    if (!IsStaServiceRunning()) {
        WIFI_LOGE("ConnectToNetwork: sta service is not running!");
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (networkId < 0) {
        WIFI_LOGE("ConnectToNetwork: invalid networkId = %{public}d!", networkId);
        return WIFI_OPT_INVALID_PARAM;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        WIFI_LOGE("ConnectToNetwork: pService is nullptr!");
        return WIFI_OPT_STA_NOT_OPENED;
    }

    if (isCandidate) {
        int uid = 0;
        if (CheckCallingUid(uid) != WIFI_OPT_SUCCESS) {
            WIFI_LOGE("ConnectToNetwork CheckCallingUid failed!");
            return WIFI_OPT_INVALID_PARAM;
        }
        WifiSettings::GetInstance().SetDeviceState(networkId, static_cast<int>(WifiDeviceConfigStatus::ENABLED), false);
        WifiLinkedInfo linkedInfo;
        WifiConfigCenter::GetInstance().GetLinkedInfo(linkedInfo, m_instId);
        if (linkedInfo.connState == ConnState::CONNECTING || linkedInfo.connState == ConnState::CONNECTED) {
            bool isSame = linkedInfo.networkId == networkId;
            WIFI_LOGE("ConnectToNetwork isCandidate isConnected isSame:%{public}s!", isSame ? "true" : "false");
            return isSame ? WIFI_OPT_SUCCESS : WIFI_OPT_FAILED;
        }
        return pService->ConnectToCandidateConfig(uid, networkId);
    }
    return pService->ConnectToNetwork(networkId);
}

ErrCode WifiDeviceServiceImpl::ConnectToDevice(const WifiDeviceConfig &config)
{
    WIFI_LOGI("%{public}s: device address %{private}s, addressType:%{public}d",
        __func__, config.bssid.c_str(), config.bssidType);
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("ConnectToDevice:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ConnectToDevice:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ConnectToDevice:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifySetWifiConfigPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ConnectToDevice:VerifySetWifiConfigPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!CheckConfigPwd(config)) {
        WIFI_LOGE("CheckConfigPwd failed!");
        return WIFI_OPT_INVALID_PARAM;
    }
    if (!IsStaServiceRunning()) {
        WIFI_LOGE("ConnectToDevice: sta service is not running!");
        return WIFI_OPT_STA_NOT_OPENED;
    }
    WifiDeviceConfig updateConfig = config;
#ifdef SUPPORT_RANDOM_MAC_ADDR
    if (MacAddress::IsValidMac(config.bssid)) {
        if (config.bssidType > REAL_DEVICE_ADDRESS) {
            WIFI_LOGE("%{public}s: invalid bssidType:%{public}d", __func__, config.bssidType);
            return WIFI_OPT_INVALID_PARAM;
        }
        WifiMacAddrInfo macAddrInfo;
        macAddrInfo.bssid = config.bssid;
        macAddrInfo.bssidType = config.bssidType;
        std::string randomMacAddr =
            WifiSettings::GetInstance().GetMacAddrPairs(WifiMacAddrInfoType::WIFI_SCANINFO_MACADDR_INFO,
                macAddrInfo);
        if (randomMacAddr.empty()) {
            WIFI_LOGW("%{public}s: record not found, bssid:%{private}s, bssidType:%{public}d",
                __func__, macAddrInfo.bssid.c_str(), macAddrInfo.bssidType);
        } else {
            WIFI_LOGI("%{public}s: find the record, bssid:%{private}s, bssidType:%{public}d, randomMac:%{private}s",
                __func__, config.bssid.c_str(), config.bssidType, randomMacAddr.c_str());
            /* random MAC address are translated into real MAC address */
            if (config.bssidType == RANDOM_DEVICE_ADDRESS) {
                updateConfig.bssid = randomMacAddr;
                updateConfig.bssidType = REAL_DEVICE_ADDRESS;
                WIFI_LOGI("%{public}s: the record is updated, bssid:%{private}s, bssidType:%{public}d, randomMac:%{private}s",
                    __func__, updateConfig.bssid.c_str(), updateConfig.bssidType, randomMacAddr.c_str());
            }
        }
    }
#endif

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        WIFI_LOGE("ConnectToNetwork: pService is nullptr!");
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->ConnectToDevice(updateConfig);
}

ErrCode WifiDeviceServiceImpl::IsConnected(bool &isConnected)
{
    WifiLinkedInfo linkedInfo;

    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("IsConnected:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    WifiConfigCenter::GetInstance().GetLinkedInfo(linkedInfo, m_instId);
    isConnected = (linkedInfo.connState == ConnState::CONNECTED);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::ReConnect()
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("ReConnect:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ReConnect:VerifySetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ReConnect:VerifyWifiConnectionPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->ReConnect();
}

ErrCode WifiDeviceServiceImpl::ReAssociate(void)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("ReAssociate:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }

    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
            WIFI_LOGE("ReAssociate:VerifySetWifiInfoPermission PERMISSION_DENIED!");
            return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("ReAssociate:VerifyWifiConnectionPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->ReAssociate();
}

ErrCode WifiDeviceServiceImpl::Disconnect(void)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("Disconnect:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("Disconnect:VerifySetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("Disconnect:VerifyWifiConnectionPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->Disconnect();
}

ErrCode WifiDeviceServiceImpl::StartWps(const WpsConfig &config)
{
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("StartWps:VerifySetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->StartWps(config);
}

ErrCode WifiDeviceServiceImpl::CancelWps(void)
{
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("CancelWps:VerifySetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(m_instId);
    if (pService == nullptr) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    return pService->CancelWps();
}

ErrCode WifiDeviceServiceImpl::IsWifiActive(bool &bActive)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("IsWifiActive:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    bActive = IsStaServiceRunning();
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetWifiState(int &state)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetWifiState:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    state = WifiConfigCenter::GetInstance().GetWifiState(m_instId);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetLinkedInfo(WifiLinkedInfo &info)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetLinkedInfo:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }

    WifiConfigCenter::GetInstance().GetLinkedInfo(info, m_instId);
    if (WifiPermissionUtils::VerifyGetWifiLocalMacPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetLinkedInfo:VerifyGetWifiLocalMacPermission() PERMISSION_DENIED!");
        /* Clear mac addr */
        info.macAddress = "";
    }

    WIFI_LOGD("GetLinkedInfo, networkId=%{public}d, ssid=%{public}s, rssi=%{public}d, frequency=%{public}d",
              info.networkId, SsidAnonymize(info.ssid).c_str(), info.rssi, info.frequency);
    WIFI_LOGD("GetLinkedInfo, connState=%{public}d, supplicantState=%{public}d, detailedState=%{public}d,\
     wifiStandard=%{public}d RxMaxSpeed=%{public}d TxmaxSpeed=%{public}d rxSpeed=%{public}d txSpeed=%{public}d",
              info.connState, info.supplicantState, info.detailedState, info.wifiStandard,
              info.maxSupportedRxLinkSpeed, info.maxSupportedTxLinkSpeed, info.rxLinkSpeed, info.txLinkSpeed);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetDisconnectedReason(DisconnectedReason &reason)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("GetDisconnectedReason:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetDisconnectedReason:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    if (WifiPermissionUtils::VerifyGetWifiConfigPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetDisconnectedReason:VerifyGetWifiConfigPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    WifiLinkedInfo info;
    WifiConfigCenter::GetInstance().GetLinkedInfo(info, m_instId);
    WIFI_LOGI("%{public}s, connState=%{public}d, detailedState=%{public}d",
        __func__, info.connState, info.detailedState);
    if (info.connState == ConnState::CONNECTING || info.connState == ConnState::CONNECTED) {
        return WIFI_OPT_FAILED;
    }
    WifiConfigCenter::GetInstance().GetDisconnectedReason(reason, m_instId);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetIpInfo(IpInfo &info)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetIpInfo:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    WifiConfigCenter::GetInstance().GetIpInfo(info, m_instId);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetIpv6Info(IpV6Info &info)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetIpv6Info:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    WifiConfigCenter::GetInstance().GetIpv6Info(info, m_instId);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::SetCountryCode(const std::string &countryCode)
{
    if (countryCode.length() != WIFI_COUNTRY_CODE_LEN) {
        return WIFI_OPT_INVALID_PARAM;
    }
    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("SetCountryCode:VerifyWifiConnectionPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
#ifndef OHOS_ARCH_LITE
    return WifiCountryCodeManager::GetInstance().SetWifiCountryCodeFromExternal(countryCode);
#else
    return WIFI_OPT_SUCCESS;
#endif
}

ErrCode WifiDeviceServiceImpl::GetCountryCode(std::string &countryCode)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetCountryCode:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
#ifndef OHOS_ARCH_LITE
    WifiCountryCodeManager::GetInstance().GetWifiCountryCode(countryCode);
    WIFI_LOGI("GetCountryCode: country code is %{public}s", countryCode.c_str());
#endif
    return WIFI_OPT_SUCCESS;
}

#ifdef OHOS_ARCH_LITE
ErrCode WifiDeviceServiceImpl::RegisterCallBack(const std::shared_ptr<IWifiDeviceCallBack> &callback,
    const std::vector<std::string> &event)
#else
ErrCode WifiDeviceServiceImpl::RegisterCallBack(const sptr<IWifiDeviceCallBack> &callback,
    const std::vector<std::string> &event)
#endif
{
    WIFI_LOGI("RegisterCallBack");
    if (callback == nullptr) {
        WIFI_LOGE("Get call back client failed!");
        return WIFI_OPT_FAILED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("RegisterCallBackClient:VerifyWifiConnectionPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    for (const auto &eventName : event) {
        WifiInternalEventDispatcher::GetInstance().SetSingleStaCallback(callback, eventName, m_instId);
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetSignalLevel(const int &rssi, const int &band, int &level)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetSignalLevel:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    level = WifiConfigCenter::GetInstance().GetSignalLevel(rssi, band, m_instId);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetSupportedFeatures(long &features)
{
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetSupportedFeatures:VerifyGetWifiInfoPermission() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    int ret = WifiManager::GetInstance().GetSupportedFeatures(features);
    if (ret < 0) {
        WIFI_LOGE("Failed to get supported features!");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::GetDeviceMacAddress(std::string &result)
{
    WIFI_LOGI("GetDeviceMacAddress");
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("GetDeviceMacAddress:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetDeviceMacAddress:VerifyGetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyGetWifiLocalMacPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("GetDeviceMacAddress:VerifyGetWifiLocalMacPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    /* mac will be got from hal when wifi is enabled. if wifi is disabled, we don't return mac. */
    if (!IsStaServiceRunning()) {
        return WIFI_OPT_STA_NOT_OPENED;
    }
    
    WifiSettings::GetInstance().GetRealMacAddress(result);
    return WIFI_OPT_SUCCESS;
}

bool WifiDeviceServiceImpl::SetLowLatencyMode(bool enabled)
{
    WIFI_LOGI("SetLowLatencyMode");
    /* refer to WifiProtectManager::GetInstance().SetLowLatencyMode, DO NOT support now! */
    return true;
}

ErrCode WifiDeviceServiceImpl::CheckCanEnableWifi(void)
{
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("EnableWifi:NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }
    if (WifiPermissionUtils::VerifySetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("EnableWifi:VerifySetWifiInfoPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyWifiConnectionPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("EnableWifi:VerifyWifiConnectionPermission PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
#ifndef OHOS_ARCH_LITE
    if (WifiManager::GetInstance().IsMdmForbidden()) {
        WIFI_LOGE("EnableWifi: Mdm forbidden PERMISSION_DENIED!");
        return WIFI_OPT_ENTERPRISE_DENIED;
    }
#endif
    /**
     * when airplane mode opened, if the config "can_open_sta_when_airplanemode"
     * opened, then can open sta; other, return forbid.
     */
    WifiManager::GetInstance().GetAirplaneModeByDatashare();
    if (WifiConfigCenter::GetInstance().GetAirplaneModeState() == MODE_STATE_OPEN &&
        !WifiConfigCenter::GetInstance().GetCanOpenStaWhenAirplaneMode(m_instId)) {
        WIFI_LOGI("current airplane mode and can not use sta, open failed!");
        return WIFI_OPT_FORBID_AIRPLANE;
    }
    /* when power saving mode opened, can't open sta, return forbid. */
    if (WifiConfigCenter::GetInstance().GetPowerSavingModeState() == 1) {
        WIFI_LOGI("current power saving mode and can not use sta, open failed!");
        return WIFI_OPT_FORBID_POWSAVING;
    }
    /**
     * Check the interval between the last STA shutdown and the current STA
     * startup.
     */
    double interval = WifiConfigCenter::GetInstance().GetWifiStaInterval(m_instId);
    if (interval <= REOPEN_STA_INTERVAL) {
        int waitMils = REOPEN_STA_INTERVAL - int(interval) + 1;
        WIFI_LOGI("open wifi too frequent, interval since last close is %{public}lf, and wait %{public}d ms",
            interval,
            waitMils);
        usleep(waitMils * MSEC);
    }
    return WIFI_OPT_SUCCESS;
}

bool WifiDeviceServiceImpl::IsStaServiceRunning()
{
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetWifiMidState(m_instId);
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGW("current wifi state is %{public}d", static_cast<int>(curState));
        return false;
    }
    return true;
}

bool WifiDeviceServiceImpl::IsScanServiceRunning()
{
    WifiOprMidState curState = WifiConfigCenter::GetInstance().GetScanMidState(m_instId);
    if (curState != WifiOprMidState::RUNNING) {
        WIFI_LOGW("scan service does not started!");
        return false;
    }
    return true;
}

void WifiDeviceServiceImpl::SaBasicDump(std::string& result)
{
    WifiDeviceServiceImpl impl;
    bool isActive = impl.IsStaServiceRunning();
    result.append("WiFi active state: ");
    std::string strActive = isActive ? "activated" : "inactive";
    result += strActive + "\n\n";

    WifiLinkedInfo linkedInfo;
    WifiConfigCenter::GetInstance().GetLinkedInfo(linkedInfo);
    bool isConnected = linkedInfo.connState == ConnState::CONNECTED;
    result.append("WiFi connection status: ");
    std::string strIsConnected = isConnected ? "connected" : "not connected";
    result += strIsConnected + "\n";
    if (isConnected) {
        std::stringstream ss;
        ss << "  Connection.ssid: " << linkedInfo.ssid << "\n";
        ss << "  Connection.bssid: " << MacAnonymize(linkedInfo.bssid) << "\n";
        ss << "  Connection.rssi: " << linkedInfo.rssi << "\n";

        enum {BAND_2GHZ = 1, BAND_5GHZ = 2, BAND_6GHZ = 3, BAND_60GHZ = 4, BAND_ANY = 5};
        auto funcStrBand = [](int band) {
            std::string retStr;
            switch (band) {
                case BAND_2GHZ:
                    retStr = "2.4GHz";
                    break;
                case BAND_5GHZ:
                    retStr = "5GHz";
                    break;
                case BAND_ANY:
                    retStr = "dual-mode frequency band";
                    break;
                case BAND_6GHZ:
                    retStr = "6GHz";
                    break;
                case BAND_60GHZ:
                    retStr = "60GHz";
                    break;
                default:
                    retStr = "unknown band";
            }
            return retStr;
        };
        ss << "  Connection.band: " << funcStrBand(linkedInfo.band) << "\n";
        ss << "  Connection.frequency: " << linkedInfo.frequency << "\n";
        ss << "  Connection.linkSpeed: " << linkedInfo.linkSpeed << "\n";
        ss << "  Connection.macAddress: " << MacAnonymize(linkedInfo.macAddress) << "\n";
        ss << "  Connection.isHiddenSSID: " << (linkedInfo.ifHiddenSSID ? "true" : "false") << "\n";

        int level = WifiConfigCenter::GetInstance().GetSignalLevel(linkedInfo.rssi, linkedInfo.band);
        ss << "  Connection.signalLevel: " << level << "\n";
        result += ss.str();
    }
    result += "\n";

    std::string cc = "CN";
#ifndef OHOS_ARCH_LITE
    WifiCountryCodeManager::GetInstance().GetWifiCountryCode(cc);
#endif
    result.append("Country Code: ").append(cc);
    result += "\n";
}

bool WifiDeviceServiceImpl::IsRemoteDied(void)
{
    return false;
}

ErrCode WifiDeviceServiceImpl::IsBandTypeSupported(int bandType, bool &supported)
{
    WIFI_LOGI("Enter get bandtype is supported.");
    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("WifiDeviceServiceImpl:IsBandTypeSupported() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (bandType <= (int)BandType::BAND_NONE || bandType >= (int)BandType::BAND_ANY) {
        WIFI_LOGE("IsBandTypeSupported bandType error %{public}d!", bandType);
        return WIFI_OPT_INVALID_PARAM;
    } else {
        ChannelsTable channels;
        WifiSettings::GetInstance().GetValidChannels(channels);
        supported = channels.find((BandType)bandType) != channels.end();
    }
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::Get5GHzChannelList(std::vector<int> &result)
{
    WIFI_LOGI("Enter get 5g channel list.");
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("Get5GHzChannelList: NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }

    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("WifiDeviceServiceImpl:Get5GHzChannelList() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyGetWifiConfigPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("WifiDeviceServiceImpl:Get5GHzChannelList() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    ChannelsTable channels;
    WifiSettings::GetInstance().GetValidChannels(channels);
    if (channels.find(BandType::BAND_5GHZ) != channels.end()) {
        result = channels[BandType::BAND_5GHZ];
    }
    
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::StartPortalCertification()
{
    WIFI_LOGI("Enter StartPortalCertification.");
    if (!WifiAuthCenter::IsSystemAppByToken()) {
        WIFI_LOGE("StartPortalCertification: NOT System APP, PERMISSION_DENIED!");
        return WIFI_OPT_NON_SYSTEMAPP;
    }

    if (WifiPermissionUtils::VerifyGetWifiInfoPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("WifiDeviceServiceImpl:StartPortalCertification() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    if (WifiPermissionUtils::VerifyGetWifiConfigPermission() == PERMISSION_DENIED) {
        WIFI_LOGE("WifiDeviceServiceImpl:StartPortalCertification() PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }

    IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("pService is nullptr!");
        return WIFI_OPT_STA_NOT_OPENED;
    }

    return pService->StartPortalCertification();
}

#ifndef OHOS_ARCH_LITE
void WifiDeviceServiceImpl::StartWatchdog(void)
{
    constexpr int32_t WATCHDOG_INTERVAL_MS = 10000;
    constexpr int32_t WATCHDOG_DELAY_MS = 15000;
    auto taskFunc = []() {
        uint64_t now = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
        uint64_t interval = now - WifiSettings::GetInstance().GetThreadStartTime();
        if ((WifiSettings::GetInstance().GetThreadStatusFlag()) && (interval > WATCHDOG_INTERVAL_MS)) {
            WIFI_LOGE("watchdog happened, thread need restart");
        } else {
            WIFI_LOGD("thread work normally");
        }
    };
    HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("WifiDeviceServiceImpl", taskFunc,
        WATCHDOG_INTERVAL_MS, WATCHDOG_DELAY_MS);
}

void WifiDeviceServiceImpl::RegisterAppRemoved()
{
    std::unique_lock<std::mutex> lock(appEventMutex);
    if (eventSubscriber_) {
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    eventSubscriber_ = std::make_shared<AppEventSubscriber>(subscriberInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(eventSubscriber_)) {
        WIFI_LOGE("AppEvent SubscribeCommonEvent() failed");
    } else {
        WIFI_LOGI("AppEvent SubscribeCommonEvent() OK");
        WifiTimer::GetInstance()->UnRegister(appEventTimerId);
    }
}

void WifiDeviceServiceImpl::UnRegisterAppRemoved()
{
    std::unique_lock<std::mutex> lock(appEventMutex);
    if (!eventSubscriber_) {
        return;
    }
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(eventSubscriber_)) {
        WIFI_LOGE("AppEvent UnSubscribeCommonEvent() failed");
    } else {
        WIFI_LOGI("AppEvent UnSubscribeCommonEvent() OK");
    }
}

void WifiDeviceServiceImpl::RegisterThermalLevel()
{
    std::unique_lock<std::mutex> lock(thermalEventMutex);
    if (thermalLevelSubscriber_) {
        return;
    }
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    thermalLevelSubscriber_ = std::make_shared<ThermalLevelSubscriber>(subscriberInfo);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(thermalLevelSubscriber_)) {
        WIFI_LOGE("THERMAL_LEVEL_CHANGED SubscribeCommonEvent() failed");
    } else {
        WIFI_LOGI("THERMAL_LEVEL_CHANGED SubscribeCommonEvent() OK");
        WifiTimer::GetInstance()->UnRegister(thermalTimerId);
    }
}

void WifiDeviceServiceImpl::UnRegisterThermalLevel()
{
    std::unique_lock<std::mutex> lock(thermalEventMutex);
    if (!thermalLevelSubscriber_) {
        return;
    }
    if (!EventFwk::CommonEventManager::UnSubscribeCommonEvent(thermalLevelSubscriber_)) {
        WIFI_LOGE("THERMAL_LEVEL_CHANGED UnSubscribeCommonEvent() failed");
    } else {
        WIFI_LOGI("THERMAL_LEVEL_CHANGED UnSubscribeCommonEvent() OK");
    }
}

ErrCode WifiDeviceServiceImpl::SetAppFrozen(int uid, bool isFrozen)
{
    if (!WifiAuthCenter::IsNativeProcess()) {
        WIFI_LOGE("SetAppFrozen:NOT NATIVE PROCESS, PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    WifiInternalEventDispatcher::GetInstance().SetAppFrozen(uid, isFrozen);
    return WIFI_OPT_SUCCESS;
}

ErrCode WifiDeviceServiceImpl::ResetAllFrozenApp()
{
    if (!WifiAuthCenter::IsNativeProcess()) {
        WIFI_LOGE("ResetAllFrozenApp:NOT NATIVE PROCESS, PERMISSION_DENIED!");
        return WIFI_OPT_PERMISSION_DENIED;
    }
    WifiInternalEventDispatcher::GetInstance().ResetAllFrozenApp();
    return WIFI_OPT_SUCCESS;
}

AppEventSubscriber::AppEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
    WIFI_LOGI("AppEventSubscriber enter");
}

AppEventSubscriber::~AppEventSubscriber()
{
    WIFI_LOGI("~AppEventSubscriber enter");
}

void AppEventSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    WIFI_LOGI("AppEventSubscriber::OnReceiveEvent : %{public}s.", action.c_str());
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        auto wantTemp = data.GetWant();
        auto uid = wantTemp.GetIntParam(AppExecFwk::Constants::UID, -1);
        if (uid == -1) {
            WIFI_LOGE("%{public}s getPackage uid is illegal.", __func__);
            return;
        }
        WIFI_LOGI("Package removed of uid %{public}d.", uid);
        for (int i = 0; i < STA_INSTANCE_MAX_NUM; ++i) {
            IStaService *pService = WifiServiceManager::GetInstance().GetStaServiceInst(i);
            if (pService == nullptr) {
                WIFI_LOGI("Sta service not opend!");
                std::vector<WifiDeviceConfig> tempConfigs;
                WifiSettings::GetInstance().GetAllCandidateConfig(uid, tempConfigs);
                for (const auto &config : tempConfigs) {
                    if (WifiSettings::GetInstance().RemoveDevice(config.networkId) != WIFI_OPT_SUCCESS) {
                        WIFI_LOGE("RemoveAllCandidateConfig-RemoveDevice() failed!");
                    }
                }
                WifiSettings::GetInstance().SyncDeviceConfig();
                return;
            }
            if (pService->RemoveAllCandidateConfig(uid) != WIFI_OPT_SUCCESS) {
                WIFI_LOGE("RemoveAllCandidateConfig failed");
            }
        }
    }
}

ThermalLevelSubscriber::ThermalLevelSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo) 
{
    WIFI_LOGI("ThermalLevelSubscriber enter");
}

ThermalLevelSubscriber::~ThermalLevelSubscriber()
{
    WIFI_LOGI("~ThermalLevelSubscriber enter");
}

void ThermalLevelSubscriber::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    WIFI_LOGI("ThermalLevelSubscriber::OnReceiveEvent: %{public}s.", action.c_str());
    if (action == OHOS::EventFwk::CommonEventSupport::COMMON_EVENT_THERMAL_LEVEL_CHANGED) {
        static const std::string THERMAL_EVENT_ID = "0";
        int level = data.GetWant().GetIntParam(THERMAL_EVENT_ID, 0);
        WifiSettings::GetInstance().SetThermalLevel(level);
        WIFI_LOGI("ThermalLevelSubscriber SetThermalLevel: %{public}d.", level);
    }
}
#endif
}  // namespace Wifi
}  // namespace OHOS
