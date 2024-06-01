/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "wifiscanstub_fuzzer.h"
#include "wifi_fuzz_common_func.h"

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include "wifi_device_stub.h"
#include "wifi_device_service_impl.h"
#include "wifi_scan_stub.h"
#include "wifi_scan_mgr_stub.h"
#include "message_parcel.h"
#include "securec.h"
#include "define.h"
#include "wifi_manager_service_ipc_interface_code.h"
#include "wifi_scan_service_impl.h"
#include "wifi_scan_mgr_service_impl.h"
#include "wifi_log.h"
#include <mutex>
#include "wifi_config_center.h"
#include "wifi_settings.h"
#include "wifi_common_def.h"
#include "wifi_manager.h"
#include "wifi_net_agent.h"

namespace OHOS {
namespace Wifi {
constexpr size_t U32_AT_SIZE_ZERO = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"ohos.wifi.IWifiScan";
const std::u16string FORMMGR_INTERFACE_TOKEN_DEVICE = u"ohos.wifi.IWifiDeviceService";
static bool g_isInsted = false;
static std::mutex g_instanceLock;
std::shared_ptr<WifiDeviceStub> pWifiDeviceStub = std::make_shared<WifiDeviceServiceImpl>();
std::shared_ptr<WifiScanStub> pWifiScanServiceImpl = std::make_shared<WifiScanServiceImpl>();
void MyExit()
{
    WifiManager::GetInstance().GetWifiStaManager()->StopUnloadStaSaTimer();
    WifiManager::GetInstance().GetWifiScanManager()->StopUnloadScanSaTimer();
    WifiManager::GetInstance().GetWifiHotspotManager()->StopUnloadApSaTimer();
    WifiManager::GetInstance().GetWifiP2pManager()->StopUnloadP2PSaTimer();
    WifiAppStateAware::GetInstance().appChangeEventHandler.reset();
    WifiNetAgent::GetInstance().netAgentEventHandler.reset();
    WifiSettings::GetInstance().mWifiEncryptionThread.reset();
    WifiManager::GetInstance().Exit();
    sleep(5);
    printf("exiting\n");
}

bool Init()
{
    if (!g_isInsted) {
        if (WifiManager::GetInstance().Init() < 0) {
            LOGE("WifiManager init failed!");
            return false;
        }
        atexit(MyExit);
        g_isInsted = true;
    }
    return true;
}
bool OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    std::unique_lock<std::mutex> autoLock(g_instanceLock);
    if (!g_isInsted) {
        if (!Init()) {
            LOGE("OnRemoteRequest Init failed!");
            return false;
        }
    }
    MessageParcel reply;
    MessageOption option;
    pWifiScanServiceImpl->OnRemoteRequest(code, data, reply, option);
    return true;
}

void OnSetScanControlInfoFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_SET_SCAN_CONTROL_INFO), datas);
}

void OnScanByParamsFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_SPECIFIED_PARAMS_SCAN), datas);
}

void OnIsWifiClosedScanFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_IS_SCAN_ALWAYS_ACTIVE), datas);
}

void OnGetScanInfoListFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_GET_SCAN_INFO_LIST), datas);
}

void OnRegisterCallBackFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_REGISTER_SCAN_CALLBACK), datas);
}

void OnStartWifiPnoScanFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_START_PNO_SCAN), datas);
}

void OnScanFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_FULL_SCAN), datas);
}

void OnSetScanOnlyAvailableTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_SET_WIFI_SCAN_ONLY), datas);
}

void OnGetScanOnlyAvailableTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    if (!datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN)) {
        LOGE("WriteInterfaceToken failed!");
        return;
    }
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    OnRemoteRequest(static_cast<uint32_t>(ScanInterfaceCode::WIFI_SVR_CMD_GET_WIFI_SCAN_ONLY), datas);
}

void OnEnableWifiFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN_DEVICE);
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    MessageParcel reply;
    MessageOption option;
    pWifiDeviceStub->OnRemoteRequest(static_cast<uint32_t>(DevInterfaceCode::WIFI_SVR_CMD_ENABLE_WIFI),
        datas, reply, option);
}

void OnDisableWifiFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN_DEVICE);
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    MessageParcel reply;
    MessageOption option;
    pWifiDeviceStub->OnRemoteRequest(static_cast<uint32_t>(DevInterfaceCode::WIFI_SVR_CMD_DISABLE_WIFI),
        datas, reply, option);
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= OHOS::Wifi::U32_AT_SIZE_ZERO)) {
        return 0;
    }
    Init();
    OHOS::Wifi::OnEnableWifiFuzzTest(data, size);
    OHOS::Wifi::OnSetScanControlInfoFuzzTest(data, size);
    OHOS::Wifi::OnSetScanOnlyAvailableTest(data, size);
    OHOS::Wifi::OnGetScanOnlyAvailableTest(data, size);
    OHOS::Wifi::OnScanFuzzTest(data, size);
    OHOS::Wifi::OnScanByParamsFuzzTest(data, size);
    OHOS::Wifi::OnIsWifiClosedScanFuzzTest(data, size);
    OHOS::Wifi::OnGetScanInfoListFuzzTest(data, size);
    OHOS::Wifi::OnRegisterCallBackFuzzTest(data, size);
    OHOS::Wifi::OnStartWifiPnoScanFuzzTest(data, size);
    OHOS::Wifi::OnDisableWifiFuzzTest(data, size);
    sleep(4);
    return 0;
}
}
}
