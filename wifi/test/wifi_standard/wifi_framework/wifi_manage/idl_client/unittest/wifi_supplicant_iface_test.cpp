/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "../../..//services/wifi_standard/wifi_framework/wifi_manage/idl_client/idl_interface/i_wifi_public_func.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include "securec.h"

#define NUMBER 5

using ::testing::Return;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
class WifiSupplicantifaceTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown(){};

    void RpcClientCallTest1()
    {
        RpcClient* client = nullptr;
        const char* func = nullptr;
        RpcClientCall(client, func);
    }

    void RpcClientCallTest2()
    {
        RpcClient client;
        client.callLockFlag = NUMBER;
        client.threadRunFlag = NUMBER;
        client.waitReply = NUMBER;
        const char* func = nullptr;
        if (strcpy_s(func, sizeof(func), "RpcClientCall") != EOK) {
            return;
        }
        RpcClientCall(&client, func);
    }
};
HWTEST_F(WifiSupplicantifaceTest, RpcClientCallTest1, TestSize.Level1)
{
    RpcClientCallTest1();
}

HWTEST_F(WifiSupplicantifaceTest, RpcClientCallTest2, TestSize.Level1)
{
    RpcClientCallTest2();
}
}  // namespace Wifi
}  // namespace OHOS
