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

#include "wifiscansa_fuzzer.h"
#include "wifi_fuzz_common_func.h"
#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <mutex>
#include "securec.h"
#include "define.h"
#include "wifi_logger.h"
#include "iscan_service.h"
#include "wifi_scan_service_impl.h"
#include "wifi_scan_mgr_service_impl.h"
#include "wifi_settings.h"
#include "wifi_toggler_manager.h"
#include "wifi_errcode.h"

namespace OHOS {
namespace Wifi {
constexpr int TWO = 2;
constexpr int U32_AT_SIZE_ZERO = 4;

OHOS::Wifi::WifiScanServiceImpl pWifiScanServiceImpl;
WifiScanMgrServiceImpl pWifiScanMgrServiceImpl;
class IWifiScanCallbackMock : public IWifiScanCallback {
public:
    IWifiScanCallbackMock()
    {
        LOGE("IWifiScanCallbackMock");
    }

    virtual ~IWifiScanCallbackMock()
    {
        LOGE("~IWifiScanCallbackMock");
    }

public:
    void OnWifiScanStateChanged(int state) override
    {
        LOGE("OnWifiScanStateChanged Mock");
    }

    OHOS::sptr<OHOS::IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

ErrCode WifiTogglerManager::ScanOnlyToggled(int isOpen)
{
    return WIFI_OPT_SUCCESS;
}

void SetScanControlInfoFuzzTest()
{
    ScanControlInfo info;
    pWifiScanServiceImpl.SetScanControlInfo(info);
}

void ScanFuzzTest(const uint8_t* data, size_t size)
{
    bool status = (static_cast<int>(data[0]) % TWO) ? true : false;
    pWifiScanServiceImpl.Scan(status);
}

void AdvanceScanFuzzTest()
{
    WifiScanParams params;
    pWifiScanServiceImpl.AdvanceScan(params);
}

void IsWifiScanAllowedFuzzTest(const uint8_t* data, size_t size)
{
    bool status = (static_cast<int>(data[0]) % TWO) ? true : false;
    pWifiScanServiceImpl.IsWifiScanAllowed(status);
}

void GetScanInfoListFuzzTest(const uint8_t* data, size_t size)
{
    std::vector<WifiScanInfo> result;
    bool status = (static_cast<int>(data[0]) % TWO) ? true : false;
    pWifiScanServiceImpl.GetScanInfoList(result, status);
}

void SetScanOnlyAvailableFuzzTest(const uint8_t* data, size_t size)
{
    bool status = (static_cast<int>(data[0]) % TWO) ? true : false;
    pWifiScanServiceImpl.SetScanOnlyAvailable(status);
}

void StartWifiPnoScanFuzzTest(const uint8_t* data, size_t size)
{
    bool isStartAction = (static_cast<int>(data[0]) % TWO) ? true : false;
    int periodMs = static_cast<int>(data[0]);
    int suspendReason = static_cast<int>(data[0]);
    pWifiScanServiceImpl.StartWifiPnoScan(isStartAction, periodMs, suspendReason);
}

void GetSupportedFeaturesFuzzTest(const uint8_t* data, size_t size)
{
    long features = static_cast<long>(data[0]);
    pWifiScanServiceImpl.GetSupportedFeatures(features);
}

void SaBasicDumpFuzzTest(const uint8_t* data, size_t size)
{
    std::string result;
    pWifiScanServiceImpl.SaBasicDump(result);
}

void RegisterCallBackFuzzTest(const uint8_t* data, size_t size)
{
    sptr<IWifiScanCallback> callback = new (std::nothrow)IWifiScanCallbackMock();
    std::vector<std::string> event;
    event.push_back(std::string(reinterpret_cast<const char*>(data), size));
    pWifiScanServiceImpl.RegisterCallBack(callback, event);
}

void WifiScanServiceImplFuzzTest(const uint8_t* data, size_t size)
{
    std::string appId = std::string(reinterpret_cast<const char*>(data), size);
    pWifiScanServiceImpl.IsAllowedThirdPartyRequest(appId);
    pWifiScanServiceImpl.IsRemoteDied();
    pWifiScanServiceImpl.IsInScanMacInfoWhiteList();
}

void WifiScanMgrServiceImplFuzzTest(const uint8_t* data, size_t size)
{
    int32_t fd = static_cast<int32_t>(data[0]);
    std::vector<std::u16string> args;
    
    pWifiScanMgrServiceImpl.Dump(fd, args);
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= OHOS::Wifi::U32_AT_SIZE_ZERO)) {
        return 0;
    }

    OHOS::Wifi::SetScanControlInfoFuzzTest();
    OHOS::Wifi::ScanFuzzTest(data, size);
    OHOS::Wifi::AdvanceScanFuzzTest();
    OHOS::Wifi::IsWifiScanAllowedFuzzTest(data, size);
    OHOS::Wifi::GetScanInfoListFuzzTest(data, size);
    OHOS::Wifi::SetScanOnlyAvailableFuzzTest(data, size);
    OHOS::Wifi::StartWifiPnoScanFuzzTest(data, size);
    OHOS::Wifi::GetSupportedFeaturesFuzzTest(data, size);
    OHOS::Wifi::SaBasicDumpFuzzTest(data, size);
    OHOS::Wifi::RegisterCallBackFuzzTest(data, size);
    OHOS::Wifi::WifiScanServiceImplFuzzTest(data, size);
    OHOS::Wifi::WifiScanMgrServiceImplFuzzTest(data, size);

    return 0;
}
}
}
