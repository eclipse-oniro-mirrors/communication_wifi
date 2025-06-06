/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "wifi_scan_callback_stub.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstddef>
#include <cstdint>
#include "securec.h"
#include "wifi_logger.h"
#include "wifi_manager_service_ipc_interface_code.h"

using ::testing::Return;
using ::testing::ext::TestSize;
DEFINE_WIFILOG_LABEL("WifiScanCallbackStubTest");

namespace OHOS {
namespace Wifi {
static std::string g_errLog;
void WifiScanLogCallback(const LogType type, const LogLevel level, const unsigned int domain,
                         const char *tag, const char *msg)
{
    g_errLog = msg;
}
constexpr int NUMBER = 2;
class WifiScanCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(){}
    static void TearDownTestCase(){}
    virtual void SetUp()
    {
        pWifiScan = std::make_unique<WifiScanCallbackStub>();
        LOG_SetCallback(WifiScanLogCallback);
    }
    virtual void TearDown()
    {
        pWifiScan.reset();
    }
public:
    std::unique_ptr<WifiScanCallbackStub> pWifiScan;
};

class IWifiScanCallbackMock : public IWifiScanCallback {
public:
    IWifiScanCallbackMock()
    {
        WIFI_LOGI("IWifiScanCallbackMock");
    }

    virtual ~IWifiScanCallbackMock()
    {
        WIFI_LOGI("~IWifiScanCallbackMock");
    }

public:
    void OnWifiScanStateChanged(int state) override
    {
        WIFI_LOGI("OnWifiScanStateChanged Mock");
    }

    OHOS::sptr<OHOS::IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

HWTEST_F(WifiScanCallbackStubTest, OnWifiScanStateChangedTest, TestSize.Level1)
{
    int state = NUMBER;
    pWifiScan->OnWifiScanStateChanged(state);
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(WifiScanCallbackStubTest, RegisterCallBackTest, TestSize.Level1)
{
    sptr<IWifiScanCallback> userCallback =  new (std::nothrow) IWifiScanCallbackMock();
    pWifiScan->RegisterCallBack(userCallback);
    pWifiScan->RegisterCallBack(userCallback);
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(WifiScanCallbackStubTest, OnRemoteRequestTest, TestSize.Level1)
{
    uint32_t code = NUMBER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(IWifiScanCallback::GetDescriptor()));
    pWifiScan->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(pWifiScan->OnRemoteRequest(code, data, reply, option), 1);
}

HWTEST_F(WifiScanCallbackStubTest, OnRemoteRequestTest1, TestSize.Level1)
{
    WIFI_LOGI("OnRemoteRequestTest1 ENTER");
    uint32_t code = static_cast<uint32_t>(ScanInterfaceCode::WIFI_CBK_CMD_SCAN_STATE_CHANGE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(IWifiScanCallback::GetDescriptor()));
    pWifiScan->OnRemoteRequest(code, data, reply, option);
    sptr<IWifiScanCallback> userCallback =  new (std::nothrow) IWifiScanCallbackMock();
    pWifiScan->RegisterCallBack(userCallback);
    EXPECT_TRUE(data.WriteInterfaceToken(IWifiScanCallback::GetDescriptor()));
    pWifiScan->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(pWifiScan->OnRemoteRequest(code, data, reply, option), 1);
}

}  // namespace Wifi
}  // namespace OHOS

