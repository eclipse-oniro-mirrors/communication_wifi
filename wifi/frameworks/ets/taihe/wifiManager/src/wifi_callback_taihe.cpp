/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "wifi_callback_taihe.h"
#include "wifi_utils_taihe.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiCallbackTaihe");

std::mutex g_candidateConnectMutex;
std::condition_variable g_candidateConnectCV;
int g_candidateApprovalStatus = -1;
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiStateChangeVec = {};
std::shared_mutex g_wifiStateChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiConnectionChangeVec = {};
std::shared_mutex g_wifiConnectionChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiRssiChangeVec = {};
std::shared_mutex g_wifiRssiChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiStreamChangeVec = {};
std::shared_mutex g_wifiStreamChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiDeviceConfigChangeVec = {};
std::shared_mutex g_wifiDeviceConfigChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiScanStateChangeVec = {};
std::shared_mutex g_wifiScanStateChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiHotspotStateChangeVec = {};
std::shared_mutex g_wifiHotspotStateChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::ohos::wifiManager::StationInfo const&)>>>
    g_wifiHotspotStaJoinVec = {};
std::shared_mutex g_wifiHotspotStaJoinLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::ohos::wifiManager::StationInfo const&)>>>
    g_wifiHotspotStaLeaveVec = {};
std::shared_mutex g_wifiHotspotStaLeaveLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiP2pStateChangeVec = {};
std::shared_mutex g_wifiP2pStateChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::ohos::wifiManager::WifiP2pLinkedInfo const&)>>>
    g_wifiP2pConnectionChangeVec = {};
std::shared_mutex g_wifiP2pConnectionChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::ohos::wifiManager::WifiP2pDevice const&)>>>
    g_wifiP2pDeviceChangeVec = {};
std::shared_mutex g_wifiP2pDeviceChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::taihe::array_view<::ohos::wifiManager::WifiP2pDevice>)>>>
    g_wifiP2pPeerDeviceChangeVec = {};
std::shared_mutex g_wifiP2pPeerDeviceChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<
    void(::ohos::wifiManager::UndefinedType const&)>>>
    g_wifiP2pPersistentGroupChangeVec = {};
std::shared_mutex g_wifiP2pPersistentGroupChangeLock;
 
std::vector<::taihe::optional<::taihe::callback<void(int)>>>
    g_wifiP2pDiscoveryChangeVec = {};
std::shared_mutex g_wifiP2pDiscoveryChangeLock;

std::vector<::taihe::optional<::taihe::callback<void(
    ::ohos::wifiManager::P2pServiceDiscoveryInfo const&)>>>
    g_wifiP2pServiceDiscoveryVec = {};
std::shared_mutex g_wifiP2pServiceDiscoveryLock;

// device
void WifiIdlDeviceEventCallback::OnWifiStateChanged(int state)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiStateChangeLock);
    WIFI_LOGI("OnWifiStateChanged event: %{public}d [0:DISABLING, 1:DISABLED, 2:ENABLING, 3:ENABLED]",
        state);
    for (auto callback : g_wifiStateChangeVec) {
        if (callback.has_value()) {
            (*callback)(state);
        }
    }
}

void WifiIdlDeviceEventCallback::OnWifiConnectionChanged(int state, const WifiLinkedInfo &info)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiStateChangeLock);
    WIFI_LOGI("OnWifiConnectionChanged event: %{public}d [4:CONNECTED, 6:DISCONNECTED, 7:SPECIAL_CONNECT]",
        state);
    for (auto callback : g_wifiConnectionChangeVec) {
        if (callback.has_value()) {
            (*callback)(state);
        }
    }
}

void WifiIdlDeviceEventCallback::OnWifiRssiChanged(int rssi)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiRssiChangeLock);
    WIFI_LOGI("OnWifiRssiChanged event: %{public}d", rssi);
    for (auto callback : g_wifiRssiChangeVec) {
        if (callback.has_value()) {
            (*callback)(rssi);
        }
    }
}

void WifiIdlDeviceEventCallback::OnWifiWpsStateChanged(int state, const std::string &pinCode)
{
}

void WifiIdlDeviceEventCallback::OnStreamChanged(int direction)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiStreamChangeLock);
    WIFI_LOGD("OnStreamChanged event: %{public}d [0:DATA_NONE, 1:DATA_IN, 2:DATA_OUT, 3:DATA_INOUT]",
        direction);
    for (auto callback : g_wifiStreamChangeVec) {
        if (callback.has_value()) {
            (*callback)(direction);
        }
    }
}

void WifiIdlDeviceEventCallback::OnDeviceConfigChanged(ConfigChange value)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiDeviceConfigChangeLock);
    WIFI_LOGI("OnDeviceConfigChanged event: %{public}d", static_cast<int>(value));
    int result = static_cast<int>(value);
    for (auto callback : g_wifiDeviceConfigChangeVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

void WifiIdlDeviceEventCallback::OnCandidateApprovalStatusChanged(CandidateApprovalStatus status)
{
    WIFI_LOGI("OnCandidateApprovalStatusChanged event: %{public}d", static_cast<int>(status));
    std::unique_lock<std::mutex> lock(g_candidateConnectMutex);
    g_candidateApprovalStatus = static_cast<int>(status);
    g_candidateConnectCV.notify_one();
}

OHOS::sptr<OHOS::IRemoteObject> WifiIdlDeviceEventCallback::AsObject()
{
    return nullptr;
}

// scan
void WifiIdlScanEventCallback::OnWifiScanStateChanged(int state)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiScanStateChangeLock);
    WIFI_LOGI("scan received state changed event: %{public}d", state);
    for (auto callback : g_wifiScanStateChangeVec) {
        (*callback)(state);
    }
}

OHOS::sptr<OHOS::IRemoteObject> WifiIdlScanEventCallback::AsObject()
{
    return nullptr;
}


// hotspot
void WifiIdlHotspotEventCallback::OnHotspotStateChanged(int state)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiHotspotStateChangeLock);
    WIFI_LOGI("Hotspot received state changed event: %{public}d", state);
    for (auto callback : g_wifiHotspotStateChangeVec) {
        if (callback.has_value()) {
            (*callback)(state);
        }
    }
}

void WifiIdlHotspotEventCallback::OnHotspotStaJoin(const StationInfo &info)
{
    // bssid -> macAddress
    std::shared_lock<std::shared_mutex> guard(g_wifiHotspotStaJoinLock);
    WIFI_LOGI("Hotspot received sta join event");
    ::ohos::wifiManager::StationInfo result = {info.bssid};
    for (auto callback : g_wifiHotspotStaJoinVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

void WifiIdlHotspotEventCallback::OnHotspotStaLeave(const StationInfo &info)
{
    // bssid -> macAddress
    std::shared_lock<std::shared_mutex> guard(g_wifiHotspotStaLeaveLock);
    WIFI_LOGI("Hotspot received sta leave event");
    ::ohos::wifiManager::StationInfo result = {info.bssid};
    result.reasonCode = ::taihe::optional<int32_t>(std::in_place_t{}, info.reasonCode);
    result.locallyGenerated = ::taihe::optional<bool>(std::in_place_t{}, info.locallyGenerated);
    result.rssi = ::taihe::optional<int32_t>(std::in_place_t{}, info.rssi);
    for (auto callback : g_wifiHotspotStaLeaveVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}
OHOS::sptr<OHOS::IRemoteObject> WifiIdlHotspotEventCallback::AsObject()
{
    return nullptr;
}

// p2p
void WifiIdlP2pEventCallback::OnP2pStateChanged(int state)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pStateChangeLock);
    WIFI_LOGI("received p2p state changed event: %{public}d", state);
    for (auto callback : g_wifiP2pStateChangeVec) {
        if (callback.has_value()) {
            (*callback)(state);
        }
    }
}

void WifiIdlP2pEventCallback::OnP2pPersistentGroupsChanged(void)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pPersistentGroupChangeLock);
    WIFI_LOGI("received persistent group changed event");
    auto result = ::ohos::wifiManager::UndefinedType::make_undefined();
    for (auto callback : g_wifiP2pPersistentGroupChangeVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

void WifiIdlP2pEventCallback::OnP2pThisDeviceChanged(const WifiP2pDevice& device)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pDeviceChangeLock);
    WIFI_LOGI("received this device changed event");
    ::taihe::string_view deviceName = static_cast<::taihe::string_view>(device.GetDeviceName());
    ::taihe::string_view deviceAddress = static_cast<::taihe::string_view>(device.GetDeviceAddress());
    ::ohos::wifiManager::DeviceAddressType deviceAddressType =
        static_cast<::ohos::wifiManager::DeviceAddressType::key_t>(device.GetDeviceAddressType());
    ::taihe::string_view primaryDeviceType = static_cast<::taihe::string_view>(device.GetPrimaryDeviceType());
    ::ohos::wifiManager::P2pDeviceStatus deviceStatus =
        static_cast<::ohos::wifiManager::P2pDeviceStatus::key_t>(device.GetP2pDeviceStatus());
    int groupCapabilities = static_cast<int>(device.GetGroupCapabilitys());
    ::ohos::wifiManager::WifiP2pDevice result = {deviceName, deviceAddress, deviceAddressType,
        primaryDeviceType, deviceStatus, groupCapabilities};
    for (auto callback : g_wifiP2pDeviceChangeVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

void WifiIdlP2pEventCallback::OnP2pPeersChanged(const std::vector<WifiP2pDevice>& devices)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pPeerDeviceChangeLock);
    WIFI_LOGI("received p2p peers changed event, devices count: %{public}d", static_cast<int>(devices.size()));
    std::vector<::ohos::wifiManager::WifiP2pDevice> result;
    for (WifiP2pDevice device : devices) {
        ::taihe::string_view deviceName = static_cast<::taihe::string_view>(device.GetDeviceName());
        ::taihe::string_view deviceAddress = static_cast<::taihe::string_view>(device.GetDeviceAddress());
        ::ohos::wifiManager::DeviceAddressType deviceAddressType =
            static_cast<::ohos::wifiManager::DeviceAddressType::key_t>(device.GetDeviceAddressType());
        ::taihe::string_view primaryDeviceType =
            static_cast<::taihe::string_view>(device.GetPrimaryDeviceType());
        ::ohos::wifiManager::P2pDeviceStatus deviceStatus =
            static_cast<::ohos::wifiManager::P2pDeviceStatus::key_t>(device.GetP2pDeviceStatus());
        int groupCapabilities = static_cast<int>(device.GetGroupCapabilitys());
        ::ohos::wifiManager::WifiP2pDevice tmpDevice = {deviceName, deviceAddress, deviceAddressType,
            primaryDeviceType, deviceStatus, groupCapabilities};
        result.emplace_back(tmpDevice);
    }
    ::taihe::array<::ohos::wifiManager::WifiP2pDevice> finalResult =
        ::taihe::array<::ohos::wifiManager::WifiP2pDevice>(taihe::copy_data_t{}, result.data(), result.size());
    for (auto callback : g_wifiP2pPeerDeviceChangeVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

void WifiIdlP2pEventCallback::OnP2pServicesChanged(const std::vector<WifiP2pServiceInfo>& srvInfo)
{
}

void WifiIdlP2pEventCallback::OnP2pConnectionChanged(const WifiP2pLinkedInfo& info)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pPeerDeviceChangeLock);
    WIFI_LOGI("received p2p connection changed event, state: %{public}d",
        static_cast<int>(info.GetConnectState()));
    ::ohos::wifiManager::P2pConnectState state =
        static_cast<::ohos::wifiManager::P2pConnectState::key_t>(info.GetConnectState());
    ani_boolean isOwner = static_cast<ani_boolean>(info.IsGroupOwner());
    ::taihe::string_view ownerAddr = static_cast<::taihe::string_view>(info.GetGroupOwnerAddress());
    ::ohos::wifiManager::WifiP2pLinkedInfo result = {state, isOwner, ownerAddr};
    for (auto callback : g_wifiP2pConnectionChangeVec) {
        (*callback)(result);
    }
}

void WifiIdlP2pEventCallback::OnP2pDiscoveryChanged(bool isChange)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pDiscoveryChangeLock);
    WIFI_LOGI("received discovery state changed event");
    int result = static_cast<int>(isChange);
    for (auto callback : g_wifiP2pDiscoveryChangeVec) {
        (*callback)(result);
    }
}

void WifiIdlP2pEventCallback::OnP2pActionResult(P2pActionCallback action, ErrCode code)
{
}

void WifiIdlP2pEventCallback::OnConfigChanged(CfgType type, char* data, int dataLen)
{
}

void WifiIdlP2pEventCallback::OnP2pGcJoinGroup(const OHOS::Wifi::GcInfo &info)
{
    WIFI_LOGI("received OnP2pGcJoinGroup event");
}

void WifiIdlP2pEventCallback::OnP2pGcLeaveGroup(const OHOS::Wifi::GcInfo &info)
{
    WIFI_LOGI("received OnP2pGcLeave event");
}

void WifiIdlP2pEventCallback::OnP2pPrivatePeersChanged(const std::string &priWfdInfo)
{
    WIFI_LOGI("received OnP2pPrivatePeersChanged event");
}

void WifiIdlP2pEventCallback::OnP2pChrErrCodeReport(const int errCode)
{
    WIFI_LOGI("received OnP2pChrErrCodeReport event");
}

static void NotifyP2pServiceDiscovery(const ::ohos::wifiManager::P2pServiceDiscoveryInfo &result)
{
    for (auto callback : g_wifiP2pServiceDiscoveryVec) {
        if (callback.has_value()) {
            (*callback)(result);
        }
    }
}

static void NotifyDnsSdService(const P2pServiceDiscoveryInfo &info, const WifiP2pDevice &device)
{
    ::ohos::wifiManager::P2pDnsSdInfo arm {
        info.dnsSdName,
        info.dnsSdRegistrationType,
        MakeWifiP2pDevice(device),
    };
    NotifyP2pServiceDiscovery(::ohos::wifiManager::P2pServiceDiscoveryInfo::make_dnsSd(std::move(arm)));
}

static void NotifyDnsSdTxtRecord(const P2pServiceDiscoveryInfo &info, const WifiP2pDevice &device)
{
    ::taihe::map<::taihe::string, ::taihe::string> txtMap;
    txtMap.reserve(info.dnsSdTxtRecord.size());
    for (const auto &item : info.dnsSdTxtRecord) {
        txtMap.emplace(::taihe::string(item.first), ::taihe::string(item.second));
    }
    ::ohos::wifiManager::P2pDnsSdTxtInfo arm {
        info.dnsSdName,
        txtMap,
        MakeWifiP2pDevice(device),
    };
    NotifyP2pServiceDiscovery(::ohos::wifiManager::P2pServiceDiscoveryInfo::make_dnsSdTxt(std::move(arm)));
}

static void NotifyUpnpService(const P2pServiceDiscoveryInfo &info, const WifiP2pDevice &device)
{
    std::vector<::taihe::string> names;
    for (const auto &name : info.upnpUniqueServiceNames) {
        names.emplace_back(name);
    }
    auto nameArray = ::taihe::array<::taihe::string>(taihe::copy_data_t{}, names.data(), names.size());
    ::ohos::wifiManager::P2pUpnpInfo arm {
        nameArray,
        MakeWifiP2pDevice(device),
    };
    NotifyP2pServiceDiscovery(::ohos::wifiManager::P2pServiceDiscoveryInfo::make_upnp(std::move(arm)));
}

static void NotifyGenericService(const P2pServiceDiscoveryInfo &info, const WifiP2pDevice &device)
{
    ::taihe::array<uint8_t> dataArray(taihe::copy_data_t{}, info.responseData.data(), info.responseData.size());
    ::ohos::wifiManager::P2pGenericInfo arm {
        static_cast<::ohos::wifiManager::P2pServiceProtocolType::key_t>(info.protocolType),
        dataArray,
        MakeWifiP2pDevice(device),
    };
    NotifyP2pServiceDiscovery(::ohos::wifiManager::P2pServiceDiscoveryInfo::make_generic(std::move(arm)));
}

void WifiIdlP2pEventCallback::OnP2pServiceDiscoveryResult(const OHOS::Wifi::P2pServiceDiscoveryInfo &info,
    const WifiP2pDevice &device)
{
    std::shared_lock<std::shared_mutex> guard(g_wifiP2pServiceDiscoveryLock);
    WIFI_LOGI("received OnP2pServiceDiscoveryResult, type=%{public}d", static_cast<int>(info.type));
    switch (info.type) {
        case P2pServiceDiscoveryResultType::DNS_SD_SERVICE:
            NotifyDnsSdService(info, device);
            break;
        case P2pServiceDiscoveryResultType::DNS_SD_TXT_RECORD:
            NotifyDnsSdTxtRecord(info, device);
            break;
        case P2pServiceDiscoveryResultType::UPNP_SERVICE:
            NotifyUpnpService(info, device);
            break;
        case P2pServiceDiscoveryResultType::GENERIC_SERVICE:
        default:
            NotifyGenericService(info, device);
            break;
    }
}

OHOS::sptr<OHOS::IRemoteObject> WifiIdlP2pEventCallback::AsObject()
{
    return nullptr;
}
}  // namespace Wifi
}  // namespace OHOS