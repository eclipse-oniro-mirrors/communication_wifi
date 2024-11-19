/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_MOCK_BLOCKCONNECTSERVICE_H
#define OHOS_MOCK_BLOCKCONNECTSERVICE_H
 
#include "wifi_ap_msg.h"
#include "wifi_msg.h"
#include <gmock/gmock.h>
#include "wifi_internal_msg.h"
 
namespace OHOS {
namespace Wifi {
class MockBlockConnectService {
public:
    virtual ~MockBlockConnectService() = default;
    virtual bool ShouldAutoConnect(const WifiDeviceConfig &config) = 0;
    virtual bool UpdateAllNetworkSelectStatus() = 0;
    virtual bool UpdateNetworkSelectStatus(int targetNetworkId, DisabledReason disableReason, int wpaReason) = 0;
    virtual bool UpdateNetworkSelectStatus(int targetNetworkId, DisabledReason disableReason) = 0;
    virtual bool EnableNetworkSelectStatus(int targetNetworkId) = 0;
    virtual bool IsFrequentDisconnect(std::string bssid, int wpaReason) = 0;
    virtual bool IsWrongPassword(int targetNetworkId) = 0;
};
 
class BlockConnectService : public MockBlockConnectService {
public:
    BlockConnectService() = default;
    ~BlockConnectService() = default;
    static BlockConnectService &GetInstance(void);
 
 
    MOCK_METHOD1(ShouldAutoConnect, bool(const WifiDeviceConfig &config));
    MOCK_METHOD0(UpdateAllNetworkSelectStatus, bool());
    MOCK_METHOD3(UpdateNetworkSelectStatus, bool(int targetNetworkId, DisabledReason disableReason, int wpaReason));
    MOCK_METHOD2(UpdateNetworkSelectStatus, bool(int targetNetworkId, DisabledReason disableReason));
    MOCK_METHOD1(EnableNetworkSelectStatus, bool(int targetNetworkId));
    MOCK_METHOD2(IsFrequentDisconnect, bool(std::string bssid, int wpaReason));
    MOCK_METHOD1(IsWrongPassword, bool(int targetNetworkId));
};
}  // namespace OHOS
}  // namespace Wifi
#endif