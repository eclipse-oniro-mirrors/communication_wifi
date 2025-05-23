/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ap_state_machine.h"
#include "wifi_log.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"
#include "mock_pendant.h"
#include "mock_wifi_ap_hal_interface.h"
#include "operator_overload.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Eq;
using ::testing::Return;
namespace OHOS {
namespace Wifi {
static std::string g_errLog = "wifitest";
class ApStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        const int SLEEP_TIME = 20;
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetThreadStatusFlag(_)).Times(AtLeast(0));
        pMockPendant = new MockPendant();
        pMockApStationsManager = &(pMockPendant->GetMockApStationsManager());
        pMockApRootState = &(pMockPendant->GetMockApRootState());
        pMockApIdleState = &(pMockPendant->GetMockApIdleState());
        pMockApStartedState = &(pMockPendant->GetMockApStartedState());
        pMockApMonitor = &(pMockPendant->GetMockApMonitor());
        pApStateMachine = new ApStateMachine(*pMockApStationsManager, *pMockApRootState, *pMockApIdleState,
            *pMockApStartedState, *pMockApMonitor);
        RegisterApServiceCallbacks();
        EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
            .WillOnce(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
        usleep(SLEEP_TIME);
    }
    virtual void TearDown()
    {
        delete pApStateMachine;
        delete pMockPendant;
        pMockPendant = nullptr;
        pApStateMachine = nullptr;
        pMockApStationsManager = nullptr;
        pMockApRootState = nullptr;
        pMockApIdleState = nullptr;
        pMockApStartedState = nullptr;
        pMockApMonitor = nullptr;
    }
public:
    ErrCode RegisterApServiceCallbacks()
    {
        std::function<void(const StationInfo &, int)> OnStationEvent =
            [&](const StationInfo &sta, int id) { m_sta = sta; };
        IApServiceCallbacks callbacks = {"", [&](ApState state, int id, int hotspotMode) {
            mBState = state;
            hotspotMode_ = hotspotMode;},
            OnStationEvent, OnStationEvent};
        return pApStateMachine->RegisterApServiceCallbacks(callbacks);
    }
    ErrCode UnRegisterApServiceCallbacks()
    {
        IApServiceCallbacks callbacks;
        return pApStateMachine->RegisterApServiceCallbacks(callbacks);
    }
public:
    void WrapOnApStateChange(ApState state)
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetHotspotState(Eq(static_cast<int>(state)), 0)).WillOnce(
            Return(0));
        pApStateMachine->OnApStateChange(state);
    }

    void WrapBroadCastStationJoin(const StationInfo &staInfo)
    {
        pApStateMachine->BroadCastStationChange(staInfo, ApStatemachineEvent::CMD_STATION_JOIN);
    }

    void WrapBroadCastStationLeave(const StationInfo &staInfo)
    {
        pApStateMachine->BroadCastStationChange(staInfo, ApStatemachineEvent::CMD_STATION_LEAVE);
    }
    void WrapBroadCastStationChangeDefult(const StationInfo &staInfo)
    {
        pApStateMachine->BroadCastStationChange(staInfo, ApStatemachineEvent::CMD_START_HOTSPOT);
    }
    void WarpRegisterEventHandler() const
    {
        pApStateMachine->RegisterEventHandler();
    }
public:
    MockPendant *pMockPendant;
    MockApStationsManager *pMockApStationsManager;
    MockApRootState *pMockApRootState;
    MockApIdleState *pMockApIdleState;
    MockApStartedState *pMockApStartedState;
    MockApMonitor *pMockApMonitor;
    ApStateMachine *pApStateMachine;
    ApState mBState;
    int hotspotMode_;
    StationInfo m_sta;
};
TEST_F(ApStateMachineTest, OnApStateChange)
{
    ApState BroadState = ApState::AP_STATE_IDLE;
    WrapOnApStateChange(BroadState);

    BroadState = ApState::AP_STATE_STARTING;
    WrapOnApStateChange(BroadState);

    BroadState = ApState::AP_STATE_STARTED;
    WrapOnApStateChange(BroadState);

    BroadState = ApState::AP_STATE_CLOSING;
    WrapOnApStateChange(BroadState);

    BroadState = ApState::AP_STATE_CLOSED;
    WrapOnApStateChange(BroadState);

    BroadState = ApState::AP_STATE_CLOSED;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetHotspotState(Eq(static_cast<int>(BroadState)), 0)).WillOnce(
        Return(1));
    pApStateMachine->OnApStateChange(BroadState);
}

TEST_F(ApStateMachineTest, BroadCastStationJoin)
{
    const StationInfo BroadCastStation = {"test1", "aa:bb:cc:dd:ee:ff", 1, "127.0.0.1"};
    WrapBroadCastStationJoin(BroadCastStation);
    EXPECT_EQ(BroadCastStation, m_sta);
}

TEST_F(ApStateMachineTest, BroadCastStationLeave)
{
    const StationInfo BroadCastStation = {"test1", "aa:bb:cc:dd:ee:ff", 1, "127.0.0.1"};
    WrapBroadCastStationChangeDefult(BroadCastStation);
    WrapBroadCastStationLeave(BroadCastStation);
    EXPECT_EQ(BroadCastStation, m_sta);
}

TEST_F(ApStateMachineTest, RegisterEventHandler)
{
    WarpRegisterEventHandler();
    EXPECT_FALSE(g_errLog.find("processWiTasDecisiveMessage") != std::string::npos);
}

TEST_F(ApStateMachineTest, GetPowerModelTest)
{
    HotspotMode mode = HotspotMode::NONE;
    pApStateMachine->SetHotspotMode(HotspotMode::SOFTAP);
    pApStateMachine->GetHotspotMode(mode);
    EXPECT_EQ(mode, HotspotMode::SOFTAP);
}

TEST_F(ApStateMachineTest, SetHotspotModeTest)
{
    HotspotMode mode = HotspotMode::NONE;
    pApStateMachine->SetHotspotMode(HotspotMode::RPT);
    pApStateMachine->GetHotspotMode(mode);
    EXPECT_EQ(mode, HotspotMode::RPT);
}
} // namespace Wifi
} // namespace OHOS
