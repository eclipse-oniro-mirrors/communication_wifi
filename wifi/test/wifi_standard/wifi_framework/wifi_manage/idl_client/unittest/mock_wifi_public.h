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
#ifndef OHOS_MOCK_WIFI_PUBILC_H
#define OHOS_MOCK_WIFI_PUBILC_H

#include <gmock/gmock.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client.h"

namespace OHOS {
namespace Wifi {
class MockWifiPublic {
public:
    MOCK_METHOD1(RemoteCall, int(RpcClient *client));
    static MockWifiPublic &GetInstance(void);
    static void SetMockFlag(bool flag);
    static bool GetMockFlag(void);

private:
    MockWifiPublic();
    ~MockWifiPublic()
    {}
};
}  // namespace Wifi
}  // namespace OHOS

extern "C" {}

#endif