/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "ap_define.h"
#include "ap_idle_state.h"
#include "mock_pendant.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"
#include "mock_wifi_ap_hal_interface.h"

using namespace OHOS;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
static std::string g_errLog = "wifitest";
class ApIdleState_test : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        const int SLEEP_TIEM = 20;
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetThreadStatusFlag(_)).Times(AtLeast(0));

        pMockPendant = new MockPendant();

        pApIdleState = new ApIdleState(pMockPendant->GetMockApStateMachine());

        usleep(SLEEP_TIEM);
    }
    virtual void TearDown()
    {
        delete pApIdleState;

        pApIdleState = nullptr;

        delete pMockPendant;

        pMockPendant = nullptr;
    }

public:
    MockPendant *pMockPendant;

    ApIdleState *pApIdleState;
};

HWTEST_F(ApIdleState_test, GoInState, TestSize.Level1)
{
    pApIdleState->GoInState();
    EXPECT_FALSE(g_errLog.find("processWiTasDecisiveMessage") != std::string::npos);
}
HWTEST_F(ApIdleState_test, GoOutState, TestSize.Level1)
{
    pApIdleState->GoOutState();
    EXPECT_FALSE(g_errLog.find("processWiTasDecisiveMessage") != std::string::npos);
}

HWTEST_F(ApIdleState_test, ExecuteStateMsg_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));

    InternalMessagePtr msg = std::make_shared<InternalMessage>();

    msg->SetMessageName(static_cast<int>(ApStatemachineEvent::CMD_UPDATE_HOTSPOTCONFIG_RESULT));

    EXPECT_TRUE(pApIdleState->ExecuteStateMsg(msg));

    msg->SetMessageName(static_cast<int>(ApStatemachineEvent::CMD_START_HOTSPOT));

    EXPECT_TRUE(pApIdleState->ExecuteStateMsg(msg));
}

HWTEST_F(ApIdleState_test, ExecuteStateMsg_FAILED, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();

    msg->SetMessageName(static_cast<int>(ApStatemachineEvent::CMD_SET_HOTSPOT_CONFIG));

    EXPECT_FALSE(pApIdleState->ExecuteStateMsg(msg));

    msg->SetMessageName(static_cast<int>(ApStatemachineEvent::CMD_DISCONNECT_STATION));

    EXPECT_FALSE(pApIdleState->ExecuteStateMsg(msg));

    EXPECT_FALSE(pApIdleState->ExecuteStateMsg(nullptr));
}
} // namespace Wifi
} // namespace OHOS