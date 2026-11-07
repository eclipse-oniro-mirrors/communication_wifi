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

#include <gtest/gtest.h>
#include "wifi_p2p_temp_disc_event.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::Wifi;
using ::testing::ext::TestSize;

class WifiP2pTempDiscEventTest : public testing::Test {
public:
    void SetUp() override
    {
        event = std::make_unique<WifiP2pTempDiscEvent>();
    }
    void TearDown() override
    {
        event.reset();
    }
    std::unique_ptr<WifiP2pTempDiscEvent> event;
};

HWTEST_F(WifiP2pTempDiscEventTest, ConstructorTest, TestSize.Level1)
{
    EXPECT_NE(event.get(), nullptr);
}

HWTEST_F(WifiP2pTempDiscEventTest, DestructorTest, TestSize.Level1)
{
    {
        WifiP2pTempDiscEvent e;
    }
    SUCCEED();
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDiscEventTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::PBC_REQ);
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::PBC_REQ);
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDiscEventPbcRespTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::PBC_RESP);
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::PBC_RESP);
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDiscEventEnterPinTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::ENTER_PIN);
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::ENTER_PIN);
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDiscEventShowPinTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::SHOW_PIN);
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::SHOW_PIN);
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDiscEventUnknownTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::UNKNOWN);
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::UNKNOWN);
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDeviceTest, TestSize.Level1)
{
    WifiP2pDevice device;
    device.SetDeviceName("TestDevice");
    device.SetDeviceAddress("00:11:22:33:44:55");
    event->SetDevice(device);
    const auto &dev = event->GetDevice();
    EXPECT_EQ(dev.GetDeviceName(), "TestDevice");
    EXPECT_EQ(dev.GetDeviceAddress(), "00:11:22:33:44:55");
}

HWTEST_F(WifiP2pTempDiscEventTest, SetDeviceDefaultTest, TestSize.Level1)
{
    WifiP2pDevice device;
    event->SetDevice(device);
    const auto &dev = event->GetDevice();
    EXPECT_TRUE(dev.GetDeviceName().empty());
}

HWTEST_F(WifiP2pTempDiscEventTest, SetPinTest, TestSize.Level1)
{
    event->SetPin("12345678");
    EXPECT_EQ(event->GetPin(), "12345678");
}

HWTEST_F(WifiP2pTempDiscEventTest, SetPinEmptyTest, TestSize.Level1)
{
    event->SetPin("");
    EXPECT_TRUE(event->GetPin().empty());
}

HWTEST_F(WifiP2pTempDiscEventTest, SetPinSpecialCharsTest, TestSize.Level1)
{
    event->SetPin("abc!@#123");
    EXPECT_EQ(event->GetPin(), "abc!@#123");
}

HWTEST_F(WifiP2pTempDiscEventTest, GetPinDefaultTest, TestSize.Level1)
{
    EXPECT_TRUE(event->GetPin().empty());
}

HWTEST_F(WifiP2pTempDiscEventTest, SetAllFieldsTest, TestSize.Level1)
{
    event->SetDiscEvent(DiscEvent::SHOW_PIN);
    WifiP2pDevice device;
    device.SetDeviceName("MyDevice");
    event->SetDevice(device);
    event->SetPin("87654321");
    EXPECT_EQ(event->GetDiscEvent(), DiscEvent::SHOW_PIN);
    EXPECT_EQ(event->GetDevice().GetDeviceName(), "MyDevice");
    EXPECT_EQ(event->GetPin(), "87654321");
}
