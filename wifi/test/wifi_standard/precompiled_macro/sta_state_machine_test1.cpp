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
#include "internal_message.h"
#include "sta_define.h"
#include "define.h"
#include "sta_state_machine.h"
#include "sta_service.h"
#include "wifi_app_state_aware.h"
#include "wifi_internal_msg.h"
#include "wifi_msg.h"
#include "mock_wifi_sta_hal_interface.h"
#include "wifi_error_no.h"
#include "log.h"
#include "wifi_config_center.h"
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
static std::string g_errLog;
void StaMachLogCallback(const LogType type, const LogLevel level,
    const unsigned int domain, const char *tag, const char *msg)
{
    g_errLog = msg;
}
static const std::string RANDOMMAC_SSID = "testwifi";
static const std::string RANDOMMAC_PASSWORD = "testwifi";
static const std::string RANDOMMAC_BSSID = "01:23:45:67:89:a0";
constexpr int TEST_FAIL_REASON = 16;
constexpr int TEN = 10;

class StaStateMachineTest : public testing::Test {
public:
static void SetUpTestCase() {}
static void TearDownTestCase()
{
    WifiAppStateAware& wifiAppStateAware = WifiAppStateAware::GetInstance();
    wifiAppStateAware.appChangeEventHandler.reset();
    wifiAppStateAware.mAppStateObserver = nullptr;
}
virtual void SetUp()
{
    pStaStateMachine.reset(new StaStateMachine());
    pStaStateMachine->InitStaStateMachine();
    pStaStateMachine->InitWifiLinkedInfo();
    LOG_SetCallback(StaMachLogCallback);
}
virtual void TearDown()
{
    pStaStateMachine.reset();
}
std::unique_ptr<StaStateMachine> pStaStateMachine;

void StartConnectToNetworkSuccess()
{
    MockWifiStaHalInterface::GetInstance().SetRetResult(WIFI_HAL_OPT_OK);
    pStaStateMachine->StartConnectToNetwork(0, "wifitest/123", 0);
}

void OnWifiWpa3SelfCureSuccessTest()
{
    int failreason = TEST_FAIL_REASON;
    WifiDeviceConfig config;
    config.lastConnectTime = 1;
    config.ssid = "1234";
    std::vector<WifiScanInfo> scanResults;
    WifiScanInfo scanInfo;
    scanInfo.ssid = "1234";
    scanInfo.capabilities = "PSK+SAE";
    scanResults.push_back(scanInfo);
    int networkId = 0;
    pStaStateMachine->OnWifiWpa3SelfCure(failreason, networkId);
}

void InitRandomMacInfoTest()
{
    const std::string bssid = "1234";
    WifiDeviceConfig deviceConfig;
    deviceConfig.keyMgmt = KEY_MGMT_NONE;
    std::vector<WifiScanInfo> scanResults;
    WifiScanInfo scanInfo;
    scanInfo.ssid = "1234";
    scanInfo.securityType = WifiSecurity::OPEN;
    scanResults.push_back(scanInfo);
    WifiStoreRandomMac randomMacInfo;
    pStaStateMachine->InitRandomMacInfo(deviceConfig, bssid, randomMacInfo);
}

void SetRandomMacConfigTest()
{
    WifiStoreRandomMac randomMacInfo;
    WifiDeviceConfig deviceConfig;
    std::string currentMac;
    pStaStateMachine->SetRandomMacConfig(randomMacInfo, deviceConfig, currentMac);
}

void SetRandomMacSuccess1()
{
    WifiDeviceConfig deviceConfig;
    deviceConfig.wifiPrivacySetting = WifiPrivacyConfig::RANDOMMAC;
    deviceConfig.keyMgmt = KEY_MGMT_WPA_PSK;
    pStaStateMachine->SetRandomMac(deviceConfig, "");
}

void SetRandomMacFail1()
{
    WifiDeviceConfig deviceConfig;
    deviceConfig.wifiPrivacySetting = WifiPrivacyConfig::RANDOMMAC;
    deviceConfig.keyMgmt = KEY_MGMT_SAE;
    WifiStoreRandomMac randomMacInfo;
    randomMacInfo.ssid = RANDOMMAC_SSID;
    randomMacInfo.keyMgmt = KEY_MGMT_WEP;
    randomMacInfo.preSharedKey = RANDOMMAC_PASSWORD;
    randomMacInfo.peerBssid = RANDOMMAC_BSSID;
    pStaStateMachine->MacAddressGenerate(randomMacInfo);
    pStaStateMachine->SetRandomMac(deviceConfig, "");
}

void SetRandomMacFail2()
{
    WifiDeviceConfig deviceConfig;
    WifiStoreRandomMac randomMacInfo;
    randomMacInfo.ssid = RANDOMMAC_SSID;
    randomMacInfo.keyMgmt = KEY_MGMT_WPA_PSK;
    randomMacInfo.preSharedKey = RANDOMMAC_PASSWORD;
    randomMacInfo.peerBssid = RANDOMMAC_BSSID;
    pStaStateMachine->MacAddressGenerate(randomMacInfo);
    deviceConfig.wifiPrivacySetting = WifiPrivacyConfig::RANDOMMAC;
    deviceConfig.keyMgmt = KEY_MGMT_WPA_PSK;
    std::string MacAddress = "11:22:33:44:55:66";
    pStaStateMachine->SetRandomMac(deviceConfig, "");
}

void PreWpaEapUmtsAuthEventTest()
{
    pStaStateMachine->PreWpaEapUmtsAuthEvent();
}

void HandleStaBssidChangedEventTest()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string bssid = "wifitest";
    msg->SetMessageObj(bssid);
    msg->SetMessageName(WIFI_SVR_CMD_STA_BSSID_CHANGED_EVENT);
    pStaStateMachine->pApLinkedState->HandleStaBssidChangedEvent(msg);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void HandleStaBssidChangedEventTest1()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string  reason =  "ASSOC_COMPLETE";
    msg->AddStringMessageBody(reason);
    msg->SetMessageName(WIFI_SVR_CMD_STA_BSSID_CHANGED_EVENT);
    pStaStateMachine->pApLinkedState->HandleStaBssidChangedEvent(msg);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void DealStartRoamCmdInApLinkedStateSuccess()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    MockWifiStaHalInterface::GetInstance().SetRetResult(WIFI_HAL_OPT_OK);
    pStaStateMachine->pApLinkedState->DealStartRoamCmdInApLinkedState(msg);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void ApRoamingStateExeMsgSuccess()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string bssid = "wifitest";
    msg->SetMessageObj(bssid);
    msg->SetMessageName(WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT);
    EXPECT_FALSE(pStaStateMachine->pApRoamingState->ExecuteStateMsg(msg));
}

void GetIpStateStateIsPublicESSTest()
{
    std::vector<WifiScanInfo> scanResults;
    WifiScanInfo scanInfo;
    scanInfo.ssid = "1234";
    scanInfo.capabilities = "123";
    scanResults.push_back(scanInfo);
    WifiLinkedInfo linkedInfo;
    linkedInfo.ssid = "1234";
    pStaStateMachine->pGetIpState->IsPublicESS();
}

void DealApRoamingStateTimeoutTest1()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    pStaStateMachine->pApRoamingState->DealApRoamingStateTimeout(msg);
}

void ApRoamingStateExeMsgSuccess1()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string bssid = "wifitest";
    msg->SetMessageObj(bssid);
    msg->SetMessageName(WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT);
    EXPECT_FALSE(pStaStateMachine->pApRoamingState->ExecuteStateMsg(msg));
}

void DealScreenStateChangedEventTest1()
{
    InternalMessagePtr msg = nullptr;
    pStaStateMachine->DealScreenStateChangedEvent(msg);
    InternalMessagePtr msg1 = std::make_shared<InternalMessage>();
    msg1->SetMessageName(WIFI_SVR_CMD_STA_WPA_EAP_UMTS_AUTH_EVENT);
    pStaStateMachine->DealScreenStateChangedEvent(msg1);
}

void OnDhcpOfferResultTest()
{
    int status = 0;
    const char *ifname = "wlan0";
    DhcpResult result;
    pStaStateMachine->pDhcpResultNotify->OnDhcpOfferResult(status, ifname, &result);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void DealDhcpResultTest()
{
    int ipType = 0;
    pStaStateMachine->pDhcpResultNotify->DealDhcpResult(ipType);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void TryToCloseDhcpClientTest()
{
    int ipType = 1;
    pStaStateMachine->pDhcpResultNotify->TryToCloseDhcpClient(ipType);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void TryToCloseDhcpClientTest1()
{
    int ipType = 2;
    pStaStateMachine->pDhcpResultNotify->TryToCloseDhcpClient(ipType);
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void DealDhcpResultFailedTest()
{
    pStaStateMachine->pDhcpResultNotify->DealDhcpResultFailed();
    EXPECT_NE(pStaStateMachine->currentTpType, TEN);
}

void DealDhcpOfferResultTest()
{
    pStaStateMachine->pDhcpResultNotify->DealDhcpOfferResult();
}

void TransHalDeviceConfigTest()
{
    WifiDeviceConfig config;
    config.keyMgmt = "SAE";
    std::vector<WifiScanInfo> scanResults;
    WifiScanInfo scanInfo;
    scanInfo.ssid = "1234";
    scanResults.push_back(scanInfo);
    WifiHalDeviceConfig halDeviceConfig;
    pStaStateMachine->TransHalDeviceConfig(halDeviceConfig, config);
}

void DealReassociateCmdSuccess()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    std::string bssid = "wifitest";
    msg->SetMessageObj(bssid);
    MockWifiStaHalInterface::GetInstance().SetRetResult(WIFI_HAL_OPT_OK);
    pStaStateMachine->DealReassociateCmd(msg);
}

void DealReConnectCmdInSeparatedStateSuccess()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    pStaStateMachine->linkedInfo.connState = ConnState::CONNECTING;
    MockWifiStaHalInterface::GetInstance().SetRetResult(WIFI_HAL_OPT_OK);
    pStaStateMachine->pSeparatedState->DealReConnectCmdInSeparatedState(msg);
}

void DealReConnectCmdSuccess()
{
    InternalMessagePtr msg = std::make_shared<InternalMessage>();
    pStaStateMachine->linkedInfo.connState = ConnState::CONNECTING;
    MockWifiStaHalInterface::GetInstance().SetRetResult(WIFI_HAL_OPT_OK);
    pStaStateMachine->pSeparatedState->DealReConnectCmdInSeparatedState(msg);
}

void StartDisConnectToNetworkSuccess()
{
    pStaStateMachine->StartDisConnectToNetwork();
}
};

HWTEST_F(StaStateMachineTest, StartConnectToNetworkSuccess, TestSize.Level1)
{
    StartConnectToNetworkSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, OnWifiWpa3SelfCureSuccessTest, TestSize.Level1)
{
    OnWifiWpa3SelfCureSuccessTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, InitRandomMacInfoTest, TestSize.Level1)
{
    InitRandomMacInfoTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, SetRandomMacConfigTest, TestSize.Level1)
{
    SetRandomMacConfigTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, PreWpaEapUmtsAuthEventTest, TestSize.Level1)
{
    PreWpaEapUmtsAuthEventTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, HandleStaBssidChangedEventTest, TestSize.Level1)
{
    HandleStaBssidChangedEventTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, HandleStaBssidChangedEventTest1, TestSize.Level1)
{
    HandleStaBssidChangedEventTest1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, ApRoamingStateExeMsgSuccess, TestSize.Level1)
{
    ApRoamingStateExeMsgSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, GetIpStateStateIsPublicESSTest, TestSize.Level1)
{
    GetIpStateStateIsPublicESSTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
HWTEST_F(StaStateMachineTest, ApRoamingStateExeMsgSuccess1, TestSize.Level1)
{
    ApRoamingStateExeMsgSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealScreenStateChangedEventTest1, TestSize.Level1)
{
    DealScreenStateChangedEventTest1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, OnDhcpOfferResultTest, TestSize.Level1)
{
    OnDhcpOfferResultTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealDhcpResultTest, TestSize.Level1)
{
    DealDhcpResultTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, TryToCloseDhcpClientTest, TestSize.Level1)
{
    TryToCloseDhcpClientTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, TryToCloseDhcpClientTest1, TestSize.Level1)
{
    TryToCloseDhcpClientTest1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealDhcpResultFailedTest, TestSize.Level1)
{
    DealDhcpResultFailedTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealDhcpOfferResultTest, TestSize.Level1)
{
    DealDhcpOfferResultTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, SetRandomMacSuccess1, TestSize.Level1)
{
    SetRandomMacSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, SetRandomMacFail1, TestSize.Level1)
{
    SetRandomMacFail1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, SetRandomMacFail2, TestSize.Level1)
{
    SetRandomMacFail2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, TransHalDeviceConfigTest, TestSize.Level1)
{
    TransHalDeviceConfigTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealReConnectCmdInSeparatedStateSuccess, TestSize.Level1)
{
    DealReConnectCmdInSeparatedStateSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealReassociateCmdSuccess, TestSize.Level1)
{
    DealReassociateCmdSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealReConnectCmdSuccess, TestSize.Level1)
{
    DealReConnectCmdSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, DealStartRoamCmdInApLinkedStateSuccess, TestSize.Level1)
{
    DealStartRoamCmdInApLinkedStateSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(StaStateMachineTest, StartDisConnectToNetworkSuccess, TestSize.Level1)
{
    StartDisConnectToNetworkSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
}
}