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

#include "wifihotspotstub_fuzzer.h"
#include "wifi_fuzz_common_func.h"

#include <cstddef>
#include <cstdint>

#include "wifi_hotspot_stub.h"
#include "message_parcel.h"
#include "securec.h"
#include "define.h"
#include "wifi_manager_service_ipc_interface_code.h"
#include "wifi_hotspot_service_impl.h"
#include "wifi_log.h"

namespace OHOS {
namespace Wifi {
constexpr size_t U32_AT_SIZE_ZERO = 4;
constexpr size_t MAP_HOTSPOT_NUMS = 22;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"ohos.wifi.IWifiHotspotService";
std::shared_ptr<WifiHotspotStub> pWifiHotspotStub = std::make_shared<WifiHotspotServiceImpl>();

void OnGetSupportedFeaturesTest(const uint8_t* data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    pWifiHotspotStub->OnRemoteRequest(static_cast<uint32_t>(DevInterfaceCode::WIFI_SVR_CMD_GET_SUPPORTED_FEATURES),
        datas, reply, option);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    uint32_t code = U32_AT(data) % MAP_HOTSPOT_NUMS +
        static_cast<uint32_t>(HotspotInterfaceCode::WIFI_SVR_CMD_ENABLE_WIFI_AP);
    LOGI("wifihotspotstub_fuzzer code(0x%{public}x) size(%{public}zu)", code, size);
    MessageParcel datas;
    datas.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    datas.WriteInt32(0);
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    pWifiHotspotStub->OnRemoteRequest(code, datas, reply, option);
    OnGetSupportedFeaturesTest(data, size);
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= OHOS::Wifi::U32_AT_SIZE_ZERO)) {
        return 0;
    }
    OHOS::Wifi::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
}
}
