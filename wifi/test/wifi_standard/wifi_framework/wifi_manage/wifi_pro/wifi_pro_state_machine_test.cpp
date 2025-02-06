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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include "define.h"
#include "wifi_log.h"
#include "wifi_errcode.h"
#include "state_machine.h"
#include "wifi_pro_common.h"
#include "iscan_service.h"
#include "wifi_logger.h"
#include "wifi_msg.h"
#include "wifi_pro_state_machine.h"
#include "wifi_service_manager.h"
#include "wifi_pro_utils.h"
#include "net_conn_client.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"
#include "network_status_history_manager.h"
#include "self_cure_state_machine.h"
#include "self_cure_utils.h"
#include "ip_qos_monitor.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {

constexpr int TEN = 10;

class WifiProStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        wifiProStateMachine_ = new WifiProStateMachine();
        pWifiProStateMachine_ = std::make_unique<WifiProStateMachine>();
        defaultState_ = std::make_unique<WifiProStateMachine::DefaultState>(wifiProStateMachine_);
        wifiProEnableState_ = std::make_unique<WifiProStateMachine::WifiProEnableState>(wifiProStateMachine_);
        wifiProDisabledState_ = std::make_unique<WifiProStateMachine::WifiProDisabledState>(wifiProStateMachine_);
        wifiConnectedState_ = std::make_unique<WifiProStateMachine::WifiConnectedState>(wifiProStateMachine_);
        wifiDisconnectedState_ = std::make_unique<WifiProStateMachine::WifiDisconnectedState>(wifiProStateMachine_);
        wifiHasNetState_ = std::make_unique<WifiProStateMachine::WifiHasNetState>(wifiProStateMachine_);
        wifiNoNetState_ = std::make_unique<WifiProStateMachine::WifiNoNetState>(wifiProStateMachine_);
        wifiPortalState_ = std::make_unique<WifiProStateMachine::WifiPortalState>(wifiProStateMachine_);
        pWifiProStateMachine_->Initialize();
    }

    virtual void TearDown()
    {
        if (pWifiProStateMachine_->pCurrWifiInfo_ != nullptr) {
            pWifiProStateMachine_->pCurrWifiInfo_.reset();
        }
        
        if (pWifiProStateMachine_->pCurrWifiDeviceConfig_ != nullptr) {
            pWifiProStateMachine_->pCurrWifiDeviceConfig_.reset();
        }
        if (wifiProStateMachine_ != nullptr) {
            delete wifiProStateMachine_;
            wifiProStateMachine_ = nullptr;
        }

        pWifiProStateMachine_.reset();
    }

    WifiProStateMachine *wifiProStateMachine_;
    std::unique_ptr<WifiProStateMachine> pWifiProStateMachine_;
    std::unique_ptr<WifiProStateMachine::DefaultState> defaultState_;
    std::unique_ptr<WifiProStateMachine::WifiProEnableState> wifiProEnableState_;
    std::unique_ptr<WifiProStateMachine::WifiProDisabledState> wifiProDisabledState_;
    std::unique_ptr<WifiProStateMachine::WifiConnectedState> wifiConnectedState_;
    std::unique_ptr<WifiProStateMachine::WifiDisconnectedState> wifiDisconnectedState_;
    std::unique_ptr<WifiProStateMachine::WifiHasNetState> wifiHasNetState_;
    std::unique_ptr<WifiProStateMachine::WifiNoNetState> wifiNoNetState_;
    std::unique_ptr<WifiProStateMachine::WifiPortalState> wifiPortalState_;
};

HWTEST_F(WifiProStateMachineTest, IsReachWifiScanThresholdTest01, TestSize.Level1)
{
    // wifi signal 4 bars
    int32_t signalLevel = SIG_LEVEL_4;
    int32_t ret = pWifiProStateMachine_->IsReachWifiScanThreshold(signalLevel);
    EXPECT_EQ(ret, false);
}

HWTEST_F(WifiProStateMachineTest, IsReachWifiScanThresholdTest02, TestSize.Level1)
{
    // The wifi signal is lower than 3 bars
    int32_t signalLevel = SIG_LEVEL_2;
    int32_t ret = pWifiProStateMachine_->IsReachWifiScanThreshold(signalLevel);
    EXPECT_EQ(ret, true);
}

HWTEST_F(WifiProStateMachineTest, IsReachWifiScanThresholdTest03, TestSize.Level1)
{
    // The wifi signal is equal to 3 bars, and there are switching records within 14 days
    int32_t signalLevel = SIG_LEVEL_3;
    WifiLinkedInfo wifiLinkedInfo;
    wifiLinkedInfo.supplicantState = SupplicantState::INVALID;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(wifiLinkedInfo), Return(0)));
    pWifiProStateMachine_->pCurrWifiInfo_ = std::make_shared<WifiLinkedInfo>(wifiLinkedInfo);

    WifiDeviceConfig wifiDeviceConfig;
    wifiDeviceConfig.lastTrySwitchWifiTimestamp = WifiProUtils::GetCurrentTimeMs();
    pWifiProStateMachine_->pCurrWifiDeviceConfig_ = std::make_shared<WifiDeviceConfig>(wifiDeviceConfig);
    bool ret = pWifiProStateMachine_->IsReachWifiScanThreshold(signalLevel);
    EXPECT_EQ(ret, true);
}

HWTEST_F(WifiProStateMachineTest, IsReachWifiScanThresholdTest04, TestSize.Level1)
{
    // The wifi signal is equal to 3 bars, and there is no connection record within 14 days.
    int32_t signalLevel = SIG_LEVEL_3;
    WifiLinkedInfo wifiLinkedInfo;
    wifiLinkedInfo.supplicantState = SupplicantState::INVALID;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(wifiLinkedInfo), Return(0)));
    pWifiProStateMachine_->pCurrWifiInfo_ = std::make_shared<WifiLinkedInfo>(wifiLinkedInfo);

    WifiDeviceConfig wifiDeviceConfig;
    wifiDeviceConfig.lastTrySwitchWifiTimestamp = 0;
    pWifiProStateMachine_->pCurrWifiDeviceConfig_ = std::make_shared<WifiDeviceConfig>(wifiDeviceConfig);
    bool ret = pWifiProStateMachine_->IsReachWifiScanThreshold(signalLevel);
    EXPECT_EQ(ret, false);
}

HWTEST_F(WifiProStateMachineTest, HandleRssiChangedInLinkMonitorStateTest, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_RSSI_CHANGED);
    msg->SetParam1(-30);
    auto pWiFiLinkMonitorState = pWifiProStateMachine_->pWifiHasNetState_;
    pWiFiLinkMonitorState->rssiLevel2Or3ScanedCounter_ = 1;
    pWiFiLinkMonitorState->HandleRssiChangedInHasNet(msg);
    EXPECT_EQ(pWiFiLinkMonitorState->rssiLevel2Or3ScanedCounter_, 1);
}

HWTEST_F(WifiProStateMachineTest, RefreshConnectedNetWorkTest01, TestSize.Level1)
{
    WifiLinkedInfo wifiLinkedInfo;
    wifiLinkedInfo.supplicantState = SupplicantState::AUTHENTICATING;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(wifiLinkedInfo), Return(0)));
    pWifiProStateMachine_->RefreshConnectedNetWork();
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, RefreshConnectedNetWorkTest02, TestSize.Level1)
{
    WifiLinkedInfo wifiLinkedInfo;
    wifiLinkedInfo.supplicantState = SupplicantState::AUTHENTICATING;
    wifiLinkedInfo.networkId = 1;

    std::vector<WifiDeviceConfig> configs;
    WifiDeviceConfig wifiDeviceConfig;
    wifiDeviceConfig.networkId = 1;
    configs.push_back(wifiDeviceConfig);

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(wifiLinkedInfo), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(configs), Return(0)));

    pWifiProStateMachine_->RefreshConnectedNetWork();
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, SetSwitchReasonTest01, TestSize.Level1)
{
    pWifiProStateMachine_->SetSwitchReason(WIFI_SWITCH_REASON_POOR_RSSI);
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, IsSwitchingOrSelfCuringTest01, TestSize.Level1)
{
    pWifiProStateMachine_->isWifi2WifiSwitching_ = false;
    pWifiProStateMachine_->isWifiNoInternet_ = false;
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, IsSwitchingOrSelfCuringTest02, TestSize.Level1)
{
    pWifiProStateMachine_->isWifi2WifiSwitching_ = true;
    pWifiProStateMachine_->isWifiNoInternet_ = false;
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, IsSwitchingOrSelfCuringTest03, TestSize.Level1)
{
    pWifiProStateMachine_->isWifi2WifiSwitching_ = false;
    pWifiProStateMachine_->isWifiNoInternet_ = true;
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, IsDisableWifiAutoSwitchTest01, TestSize.Level1)
{
    EXPECT_EQ(pWifiProStateMachine_->IsDisableWifiAutoSwitch(), true);
}

HWTEST_F(WifiProStateMachineTest, IsDisableWifiAutoSwitchTest02, TestSize.Level1)
{
    pWifiProStateMachine_->isDisableWifiAutoSwitch_ = true;
    EXPECT_EQ(pWifiProStateMachine_->IsDisableWifiAutoSwitch(), false);
}

HWTEST_F(WifiProStateMachineTest, IsCallingInCsTest01, TestSize.Level1)
{
    EXPECT_EQ(pWifiProStateMachine_->IsCallingInCs(), false);
}

HWTEST_F(WifiProStateMachineTest, IsFullscreenTest01, TestSize.Level1)
{
    EXPECT_EQ(pWifiProStateMachine_->IsFullscreen(), false);
}

HWTEST_F(WifiProStateMachineTest, UpdateWifiSwitchTimeStampTest01, TestSize.Level1)
{
    WifiLinkedInfo linkedInfo;
    WifiDeviceConfig config;
    linkedInfo.networkId = 1;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(linkedInfo), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillRepeatedly(Return(0));

    EXPECT_EQ(pWifiProStateMachine_->isWifi2WifiSwitching_, false);
}

HWTEST_F(WifiProStateMachineTest, HandleWifi2WifiFailedTest01, TestSize.Level1)
{
    pWifiProStateMachine_->HandleWifi2WifiFailed();
    EXPECT_EQ(pWifiProStateMachine_->isWifi2WifiSwitching_, false);
}

HWTEST_F(WifiProStateMachineTest, FastScanTest01, TestSize.Level1)
{
    std::vector<WifiScanInfo> scanInfoList;
    WifiScanInfo scanInfoList1;
    scanInfoList1.frequency = 2412;
    scanInfoList.push_back(scanInfoList1);
    pWifiProStateMachine_->instId_ = 0;
    pWifiProStateMachine_->FastScan(scanInfoList);
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, IsSatisfiedWifi2WifiConditionTest01, TestSize.Level1)
{
    pWifiProStateMachine_->isWifi2WifiSwitching_ = false;
    pWifiProStateMachine_->isWifiNoInternet_ = true;
    pWifiProStateMachine_->IsSatisfiedWifi2WifiCondition();
    EXPECT_EQ(pWifiProStateMachine_->isWifi2WifiSwitching_, false);
}

HWTEST_F(WifiProStateMachineTest, TryNoNetSwitchTest, TestSize.Level1)
{
    NetworkSelectionResult networkSelectionResult;
    networkSelectionResult.wifiDeviceConfig.networkId = 1;
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = false;
    wifiNoNetState_->pWifiProStateMachine_->wifiSwitchReason_ = WIFI_SWITCH_REASON_NO_INTERNET;
    wifiNoNetState_->pWifiProStateMachine_->TryWifi2Wifi(networkSelectionResult);

    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    wifiNoNetState_->pWifiProStateMachine_->wifiSwitchReason_ = WIFI_SWITCH_REASON_NO_INTERNET;
    wifiNoNetState_->pWifiProStateMachine_->TryWifi2Wifi(networkSelectionResult);

    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = false;
    wifiNoNetState_->pWifiProStateMachine_->wifiSwitchReason_ = WIFI_SWITCH_REASON_POOR_RSSI;
    wifiNoNetState_->pWifiProStateMachine_->TryWifi2Wifi(networkSelectionResult);
    EXPECT_NE(pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiProEnableStateTransitionNetStateTest01, TestSize.Level1)
{
    wifiProEnableState_->pWifiProStateMachine_ = new WifiProStateMachine();
    wifiProEnableState_->pWifiProStateMachine_->instId_ = 0;
    
    WifiLinkedInfo linkedInfo;
    linkedInfo.connState = ConnState::CONNECTED;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(linkedInfo), Return(0)));

    wifiProEnableState_->TransitionNetState();
    EXPECT_NE(wifiProEnableState_->pWifiProStateMachine_, nullptr);
}

HWTEST_F(WifiProStateMachineTest, DefaultStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_REMOVE_BLOCK_LIST);
    std::string test = "test";
    msg->SetMessageObj(test);
    EXPECT_EQ(defaultState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, DefaultStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    EXPECT_EQ(defaultState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, DefaultStateHandleRemoveBlockListTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string bssid = "TEST";
    msg->SetMessageObj(bssid);
    defaultState_->HandleRemoveBlockList(msg);
    EXPECT_NE(defaultState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, DefaultStateHandleWifiProSwitchChangedTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    defaultState_->HandleWifiProSwitchChanged(msg);
    EXPECT_EQ(defaultState_->pWifiProStateMachine_->isWifiProEnabled_, false);
}

HWTEST_F(WifiProStateMachineTest, WifiProEnableStateExecuteStateMsgTest01, TestSize.Level1)
{
    wifiProEnableState_->GoOutState();
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    EXPECT_EQ(wifiProEnableState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiProEnableStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_NOTIFY_WIFI_PRO_SWITCH_CHANGED);
    EXPECT_EQ(wifiProEnableState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiProEnableStateHandleWifiConnectStateChangedInEnableTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetParam1(17);
    WifiLinkedInfo linkedInfo;
    linkedInfo.ssid = "test";
    msg->SetMessageObj(linkedInfo);
    wifiProEnableState_->HandleWifiConnectStateChangedInEnable(msg);

    msg->SetParam1(23);
    linkedInfo.ssid = "test1";
    msg->SetMessageObj(linkedInfo);
    wifiProEnableState_->HandleWifiConnectStateChangedInEnable(msg);

    msg->SetParam1(0);
    linkedInfo.ssid = "test2";
    msg->SetMessageObj(linkedInfo);
    wifiProEnableState_->HandleWifiConnectStateChangedInEnable(msg);
    EXPECT_NE(wifiProEnableState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiProDisabledStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(0);
    msg->SetParam1(1);
    wifiProDisabledState_->pWifiProStateMachine_ = new WifiProStateMachine();
    EXPECT_EQ(wifiProDisabledState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiProDisabledStateGoInStateTest01, TestSize.Level1)
{
    wifiProDisabledState_->GoInState();
    EXPECT_NE(wifiProDisabledState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiProDisabledStateGoOutStateTest01, TestSize.Level1)
{
    wifiProDisabledState_->GoOutState();
    EXPECT_NE(wifiProDisabledState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiProDisabledStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(1);
    EXPECT_EQ(wifiProDisabledState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateGoInStateTest01, TestSize.Level1)
{
    wifiConnectedState_->GoInState();
    EXPECT_NE(wifiConnectedState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateGoOutStateTest01, TestSize.Level1)
{
    wifiConnectedState_->GoOutState();
    EXPECT_NE(wifiConnectedState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(20);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(18);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest03, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(19);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest05, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    msg->SetParam1(23);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest06, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    msg->SetParam1(22);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateExecuteStateMsgTest07, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_DISCONNECT_DISCONNECTED);
    msg->SetParam1(23);
    EXPECT_EQ(wifiConnectedState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiConnectedStateInitConnectedStateTest01, TestSize.Level1)
{
    wifiConnectedState_->pWifiProStateMachine_ = new WifiProStateMachine();
    wifiConnectedState_->pWifiProStateMachine_->duanBandHandoverType_ = 1;
    wifiConnectedState_->InitConnectedState();

    wifiConnectedState_->pWifiProStateMachine_->duanBandHandoverType_ = 2;
    wifiConnectedState_->InitConnectedState();
    EXPECT_NE(wifiConnectedState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiDisconnectedStateGoInStateTest01, TestSize.Level1)
{
    wifiDisconnectedState_->GoInState();
    EXPECT_NE(wifiDisconnectedState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiDisconnectedStateGoOutStateTest01, TestSize.Level1)
{
    wifiDisconnectedState_->GoOutState();
    EXPECT_NE(wifiDisconnectedState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiDisconnectedStateExecuteStateMsgTest04, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI2WIFI_FAILED);
    EXPECT_EQ(wifiDisconnectedState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_RSSI_CHANGED);
    wifiHasNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_RSSI_CHANGED);
    wifiHasNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_ = false;
    wifiHasNetState_->pWifiProStateMachine_->isWifiNoInternet_ = false;
    wifiHasNetState_->pWifiProStateMachine_->instId_ = 1;
    WifiLinkedInfo linkedInfo;
    linkedInfo.rssi = 1;
    linkedInfo.band = 1;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<0>(linkedInfo), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillRepeatedly(Return(4));
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest03, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    msg->SetParam1(23);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest04, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_REQUEST_SCAN_DELAY);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest05, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_HANDLE_SCAN_RESULT);
    std::vector<InterScanInfo> scanInfos;
    msg->SetMessageObj(scanInfos);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest06, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(19);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest07, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(18);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateExecuteStateMsgTest08, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(20);
    EXPECT_EQ(wifiHasNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateTryStartScanTest01, TestSize.Level1)
{
    bool hasSwitchRecord = true;
    int32_t signalLevel = 2;
    wifiHasNetState_->TryStartScan(hasSwitchRecord, signalLevel);
    EXPECT_NE(wifiHasNetState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateTryStartScanTest02, TestSize.Level1)
{
    bool hasSwitchRecord = true;
    int32_t signalLevel = 1;
    wifiHasNetState_->rssiLevel0Or1ScanedCounter_ = 1;
    wifiHasNetState_->TryStartScan(hasSwitchRecord, signalLevel);
    EXPECT_NE(wifiHasNetState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateRequestHttpDetectTest01, TestSize.Level1)
{
    wifiHasNetState_->RequestHttpDetect();
    EXPECT_NE(wifiHasNetState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiHasNetStateHandleWifiQoeSlowTest01, TestSize.Level1)
{
    wifiHasNetState_->HandleWifiQoeSlow();
    EXPECT_NE(wifiHasNetState_->pWifiProStateMachine_->wifiSwitchReason_, TEN);
}

HWTEST_F(WifiProStateMachineTest, WifiNoNetStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    wifiNoNetState_->pWifiProStateMachine_ = new WifiProStateMachine();
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(19);
    EXPECT_EQ(wifiNoNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiNoNetStateExecuteStateMsgTest04, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    msg->SetMessageName(EVENT_REQUEST_SCAN_DELAY);
    EXPECT_EQ(wifiNoNetState_->ExecuteStateMsg(msg), true);
}

HWTEST_F(WifiProStateMachineTest, wifiNoNetStateExecuteStateMsgTest05, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    msg->SetParam1(23);
    EXPECT_EQ(wifiNoNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, wifiNoNetStateExecuteStateMsgTest06, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI2WIFI_FAILED);
    msg->SetParam1(19);
    EXPECT_EQ(wifiNoNetState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, wifiNoNetStateIsSatisfiedWifi2WifiConditionMsgTest, TestSize.Level1)
{
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    wifiNoNetState_->pWifiProStateMachine_->isDisableWifiAutoSwitch_ = true;
    wifiNoNetState_->pWifiProStateMachine_->IsSatisfiedWifi2WifiCondition();

    wifiNoNetState_->pWifiProStateMachine_->isDisableWifiAutoSwitch_ = false;
    wifiNoNetState_->pWifiProStateMachine_->IsSatisfiedWifi2WifiCondition();
    EXPECT_EQ(wifiNoNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_, false);
}

HWTEST_F(WifiProStateMachineTest, wifiNoNetStateTrySelfCureTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    wifiNoNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_ = true;
    wifiNoNetState_->pWifiProStateMachine_->TrySelfCure(false);
    EXPECT_EQ(wifiNoNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_, true);
}

HWTEST_F(WifiProStateMachineTest, wifiNoNetStateTrySelfCureTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    wifiNoNetState_->pWifiProStateMachine_->isWifiNoInternet_ = true;
    wifiNoNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_ = false;
    wifiNoNetState_->pWifiProStateMachine_->TrySelfCure(true);
    EXPECT_EQ(wifiNoNetState_->pWifiProStateMachine_->isWifi2WifiSwitching_, false);
}

HWTEST_F(WifiProStateMachineTest, WifiPortalStateGoInStateTest01, TestSize.Level1)
{
    wifiPortalState_->GoInState();
    wifiPortalState_->GoOutState();
    EXPECT_EQ(wifiPortalState_->pWifiProStateMachine_->isWifiNoInternet_, false);
}

HWTEST_F(WifiProStateMachineTest, WifiPortalStateExecuteStateMsgTest01, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(19);
    EXPECT_EQ(wifiPortalState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiPortalStateExecuteStateMsgTest02, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_CHECK_WIFI_INTERNET_RESULT);
    msg->SetParam1(20);
    EXPECT_EQ(wifiPortalState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiPortalStateExecuteStateMsgTest03, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI2WIFI_FAILED);
    msg->SetParam1(19);
    EXPECT_EQ(wifiPortalState_->ExecuteStateMsg(msg), false);
}

HWTEST_F(WifiProStateMachineTest, WifiPortalStateExecuteStateMsgTest04, TestSize.Level1)
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    msg->SetMessageName(EVENT_WIFI_CONNECT_STATE_CHANGED);
    msg->SetParam1(23);
    EXPECT_EQ(wifiPortalState_->ExecuteStateMsg(msg), false);
}

} // namespace Wifi
} // namespace OHOS
