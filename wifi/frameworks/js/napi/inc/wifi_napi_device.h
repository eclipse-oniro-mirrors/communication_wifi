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

#ifndef WIFI_NAPI_DEVICE_H_
#define WIFI_NAPI_DEVICE_H_

#include "wifi_napi_utils.h"
#include "wifi_device.h"
#include "wifi_scan.h"

namespace OHOS {
namespace Wifi {
napi_value EnableWifi(napi_env env, napi_callback_info info);
napi_value DisableWifi(napi_env env, napi_callback_info info);
napi_value IsWifiActive(napi_env env, napi_callback_info info);
napi_value Scan(napi_env env, napi_callback_info info);
napi_value StartScan(napi_env env, napi_callback_info info);
napi_value GetScanInfos(napi_env env, napi_callback_info info);
napi_value GetScanInfoResults(napi_env env, napi_callback_info info);
napi_value GetScanResults(napi_env env, napi_callback_info info);
napi_value GetScanInfoList(napi_env env, napi_callback_info info);
napi_value AddDeviceConfig(napi_env env, napi_callback_info info);
napi_value AddUntrustedConfig(napi_env env, napi_callback_info info);
napi_value RemoveUntrustedConfig(napi_env env, napi_callback_info info);
napi_value AddCandidateConfig(napi_env env, napi_callback_info info);
napi_value RemoveCandidateConfig(napi_env env, napi_callback_info info);
napi_value ConnectToCandidateConfig(napi_env env, napi_callback_info info);
napi_value ConnectToCandidateConfigWithUserAction(napi_env env, napi_callback_info info);
napi_value GetCandidateConfigs(napi_env env, napi_callback_info info);
napi_value ConnectToNetwork(napi_env env, napi_callback_info info);
napi_value ConnectToDevice(napi_env env, napi_callback_info info);
napi_value IsConnected(napi_env env, napi_callback_info info);
napi_value Disconnect(napi_env env, napi_callback_info info);
napi_value GetSignalLevel(napi_env env, napi_callback_info info);
napi_value ReConnect(napi_env env, napi_callback_info info);
napi_value ReAssociate(napi_env env, napi_callback_info info);
napi_value GetIpInfo(napi_env env, napi_callback_info info);
napi_value GetIpv6Info(napi_env env, napi_callback_info info);
napi_value GetLinkedInfo(napi_env env, napi_callback_info info);
napi_value GetLinkedInfoSync(napi_env env, napi_callback_info info);
napi_value GetMultiLinkedInfo(napi_env env, napi_callback_info info);
napi_value GetDisconnectedReason(napi_env env, napi_callback_info info);
napi_value IsMeteredHotspot(napi_env env, napi_callback_info info);
napi_value RemoveDevice(napi_env env, napi_callback_info info);
napi_value RemoveAllNetwork(napi_env env, napi_callback_info info);
napi_value AllowAutoConnect(napi_env env, napi_callback_info info);
napi_value DisableNetwork(napi_env env, napi_callback_info info);
napi_value GetCountryCode(napi_env env, napi_callback_info info);
napi_value GetDeviceConfigs(napi_env env, napi_callback_info info);
napi_value UpdateNetwork(napi_env env, napi_callback_info info);
napi_value GetSupportedFeatures(napi_env env, napi_callback_info info);
napi_value IsFeatureSupported(napi_env env, napi_callback_info info);
napi_value GetDeviceMacAddress(napi_env env, napi_callback_info info);
napi_value IsBandTypeSupported(napi_env env, napi_callback_info info);
napi_value Get5GHzChannelList(napi_env env, napi_callback_info info);
napi_value SetScanOnlyAvailable(napi_env env, napi_callback_info info);
napi_value GetScanOnlyAvailable(napi_env env, napi_callback_info info);
napi_value StartPortalCertification(napi_env env, napi_callback_info info);
napi_value GetWifiProtect(napi_env env, napi_callback_info info);
napi_value PutWifiProtect(napi_env env, napi_callback_info info);
napi_value FactoryReset(napi_env env, napi_callback_info info);
napi_value EnableHiLinkHandshake(napi_env env, napi_callback_info info);
napi_value EnableSemiWifi(napi_env env, napi_callback_info info);
napi_value GetWifiDetailState(napi_env env, napi_callback_info info);
napi_value ConfigStaticIp(const napi_env &env, const napi_value &object, WifiDeviceConfig &cppConfig);
napi_value ConfigStaticIpv4(const napi_env &env, const napi_value &object, WifiDeviceConfig &cppConfig);
napi_value ConfigStaticIpv6(const napi_env &env, const napi_value &object, WifiDeviceConfig &cppConfig);

class ScanInfoAsyncContext : public AsyncContext {
public:
    std::vector<WifiScanInfo> vecScanInfos;

    ScanInfoAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred){}

    ScanInfoAsyncContext() = delete;

    virtual ~ScanInfoAsyncContext(){}
};

class DeviceConfigContext : public AsyncContext {
public:
    WifiDeviceConfig *config;
    int networkId;
    bool isCandidate;

    DeviceConfigContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred) {
            config = nullptr;
            networkId = -1;
            isCandidate = false;
        }

    DeviceConfigContext() = delete;

    virtual ~DeviceConfigContext() {}
};

class LinkedInfoAsyncContext : public AsyncContext {
public:
    WifiLinkedInfo linkedInfo;

    LinkedInfoAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr) :
        AsyncContext(env, work, deferred){}

    LinkedInfoAsyncContext() = delete;

    virtual ~LinkedInfoAsyncContext(){}
};
}  // namespace Wifi
}  // namespace OHOS

#endif
