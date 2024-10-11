/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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
#include "sta_service.h"
#include <gtest/gtest.h>
#include "mock_wifi_manager.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"
#include "sta_state_machine.h"
#include "mock_dhcp_service.h"
#include "mock_sta_auto_connect_service.h"
#include "sta_define.h"
#include "wifi_msg.h"
#include "wifi_internal_msg.h"
#include "wifi_error_no.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Ref;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
constexpr int NETWORK_ID = 15;
constexpr int BAND = 2;
constexpr int FREQUENCY = 2437;
constexpr int TIMESTAMP = -750366468;
constexpr int UID = 5225;
constexpr int TWO = 2;
constexpr int COMM_NET = 1151;

class StaServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        pStaService = std::make_unique<StaService>();
        pStaService->pStaStateMachine = new StaStateMachine();
        pStaService->pStaAutoConnectService = new MockStaAutoConnectService(pStaService->pStaStateMachine);
    }
    virtual void TearDown()
    {
        pStaService.reset();
    }

    void StaServiceInitStaServiceSuccess();
    void StaServiceEnableWifiSuccess();
    void StaServiceConnectToWifiDeviceConfigSuccess();
    void StaServiceConnectToWifiDeviceConfigFail1();
    void StaServiceConnectToWifiDeviceConfigFail2();
    void StaServiceConnectToWifiDeviceConfigFail3();
    void StaServiceConnectToNetworkIdSuccess();
    void StaServiceConnectToNetworkIdFail1();
    void StaServiceReAssociateSuccess();
    void StaServiceAddDeviceConfigSuccess();
    void StaServiceAddDeviceConfigFail1();
    void StaServiceAddDeviceConfigFail2();
    void StaServiceAddDeviceConfigFail3();
    void StaServiceUpdateDeviceConfigSuccess();
    void StaServiceRemoveDeviceConfigSuccess();
    void StaServiceRemoveDeviceConfigFail1();
    void StaServiceRemoveDeviceConfigFail2();
    void StaServiceEnableDeviceConfigSuccess();
    void StaServiceEnableDeviceConfigFail1();
    void StaServiceEnableDeviceConfigFail2();
    void StaServiceDisableDeviceConfigSuccess();
    void StaServiceDisableDeviceConfigFail1();
    void StaServiceDisconnectSuccess();
    void StaServiceStartWpsSuccess();
    void StaServiceCancelWpsSuccess();
    void StaServiceAutoConnectServiceSuccess();
    void StaServiceRegisterStaServiceCallbackSuccess();
    void StaServiceRegisterStaServiceCallbackFail();
    void StaServiceSetSuspendModeTest();
    void StaServiceAddCandidateConfigTestSucc();
    void StaServiceAddCandidateConfigTestFail0();
    void StaServiceAddCandidateConfigTestFail1();
    void StaServiceRemoveCandidateConfigTestSucc();
    void StaServiceRemoveCandidateConfigTestFail();
    void StaServiceRemoveAllCandidateConfigTestSucc();
    void StaServiceConnectToCandidateConfigTestSucc0();
    void StaServiceConnectToCandidateConfigTestSucc1();
    void StaServiceConnectToCandidateConfigTestFail();
    void StaServiceRemoveAllDeviceTestSucc();
    void StaServiceRemoveAllDeviceTestFail0();
    void StaServiceRemoveAllDeviceTestFail1();
    void StaServiceReConnectTestSucc();
    void StaServiceSetPowerModeTest();
    void StaServiceOnSystemAbilityChangedTest();
    void StaServiceStartPortalCertificationTest();
    void DisableAutoJoin();
    void EnableAutoJoin();
    void RegisterAutoJoinCondition();
    void DeregisterAutoJoinCondition();
    void RegisterFilterBuilder();
    void DeregisterFilterBuilder();
    void EnableHiLinkHandshakeFailTest();
    void EnableHiLinkHandshakeSuceessTest();
    void DeliverStaIfaceDataSuccessTest();
    void InitStaServiceFailed();
    void GetDataSlotIdTest();
    void GetImsiTest();
    void GetPlmnTest();
    void GetMccTest();
    void GetMncTest();
    void UpdateEapConfigTest();
    void OnWifiCountryCodeChangedTest();
    void StartPortalCertificationTest();
    void HandleForegroundAppChangedActionTest();
    void EnableHiLinkHandshakeTest();
    void DeliverStaIfaceDataTest();
    void StartRoamToNetworkTest();
    int StartConnectToUserSelectNetworkSuccessTest();
    int StartConnectToUserSelectNetworkSuccessFail();
    void SetTxPowerTest();
public:
    std::unique_ptr<StaService> pStaService;
};

void StaServiceTest::StaServiceInitStaServiceSuccess()
{
    WifiHalRoamCapability capability;
    capability.maxBlocklistSize = 1;
    std::vector<int32_t> band_2G_channel = { 1, 2, 3, 4, 5, 6, 7 };
    std::vector<int32_t> band_5G_channel = { 149, 168, 169 };
    ChannelsTable temp = { { BandType::BAND_2GHZ, band_2G_channel }, { BandType::BAND_5GHZ, band_5G_channel } };
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveLinkedInfo(_, _)).WillRepeatedly(
        Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsScoreSlope(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsInitScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsSameBssidScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsSameNetworkScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsFrequency5GHzScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsLastSelectionScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetScoretacticsSecurityScore(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSavedDeviceAppraisalPriority(_)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetExternDeviceAppraisalPriority()).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), ReloadDeviceConfig()).Times(AtLeast(0));
    std::vector<StaServiceCallback> callbacks;
    callbacks.push_back(WifiManager::GetInstance().GetStaCallback());
    EXPECT_TRUE(pStaService->InitStaService(callbacks) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceEnableWifiSuccess()
{
    EXPECT_TRUE(pStaService->EnableStaService() == WIFI_OPT_SUCCESS);
    EXPECT_TRUE(pStaService->DisableStaService() == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToWifiDeviceConfigSuccess()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), AddDeviceConfig(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->ConnectToDevice(config) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToWifiDeviceConfigFail1()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.keyMgmt = "123456";
    config.ssid = "networkId";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(-1)));
    EXPECT_CALL(WifiSettings::GetInstance(), AddDeviceConfig(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->ConnectToDevice(config) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToWifiDeviceConfigFail2()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.keyMgmt = "123456";
    config.ssid = "networkId";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(-1)));
    EXPECT_TRUE(pStaService->ConnectToDevice(config) == WIFI_OPT_SUCCESS);
}


void StaServiceTest::StaServiceConnectToWifiDeviceConfigFail3()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.keyMgmt = "123456";
    config.ssid = "networkId";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(0)));
    EXPECT_TRUE(pStaService->ConnectToDevice(config) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToNetworkIdSuccess()
{
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _)).WillRepeatedly(Return(0));
    EXPECT_TRUE(pStaService->ConnectToNetwork(networkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToNetworkIdFail1()
{
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(pStaService->ConnectToNetwork(networkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceReAssociateSuccess()
{
    EXPECT_TRUE(pStaService->ReAssociate() == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceAddDeviceConfigSuccess()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = 1;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    config.wifiEapConfig.eap = "TLS";
    config.wifiEapConfig.certEntry.push_back(1);
    config.wifiEapConfig.clientCert = "client certificate";
    config.wifiEapConfig.privateKey = "//12302345//";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), AddDeviceConfig(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->AddDeviceConfig(config) != INVALID_NETWORK_ID);
}

void StaServiceTest::StaServiceAddDeviceConfigFail1()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(-1)));
    EXPECT_CALL(WifiSettings::GetInstance(), AddDeviceConfig(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->AddDeviceConfig(config) != INVALID_NETWORK_ID);
}

void StaServiceTest::StaServiceAddDeviceConfigFail2()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(-1)));
    EXPECT_TRUE(pStaService->AddDeviceConfig(config) != INVALID_NETWORK_ID);
}


void StaServiceTest::StaServiceAddDeviceConfigFail3()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(0)));
    EXPECT_TRUE(pStaService->AddDeviceConfig(config) != INVALID_NETWORK_ID);
}

void StaServiceTest::StaServiceUpdateDeviceConfigSuccess()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:A1";
    config.band = 2;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "12345678";

    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));

    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(config.ssid, config.keyMgmt, _))
        .WillOnce(DoAll(SetArgReferee<2>(config), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), AddDeviceConfig(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->UpdateDeviceConfig(config) != INVALID_NETWORK_ID);
}

void StaServiceTest::StaServiceRemoveDeviceConfigSuccess()
{
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetChangeDeviceConfig(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _)).Times(AtLeast(0)).WillRepeatedly(Return(0));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->RemoveDevice(networkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceRemoveDeviceConfigFail1()
{
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _)).WillRepeatedly(Return(-1));
    EXPECT_TRUE(pStaService->RemoveDevice(networkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceRemoveDeviceConfigFail2()
{
    int networkId = NETWORK_ID;
    EXPECT_TRUE(pStaService->RemoveDevice(networkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceEnableDeviceConfigSuccess()
{
    int networkId = NETWORK_ID;
    bool attemptEnable = true;
    EXPECT_CALL(WifiSettings::GetInstance(),
        SetDeviceState(networkId, (int)WifiDeviceConfigStatus::ENABLED, attemptEnable))
        .WillRepeatedly(Return(0));
    EXPECT_TRUE(pStaService->EnableDeviceConfig(networkId, attemptEnable) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceEnableDeviceConfigFail1()
{
    int networkId = NETWORK_ID;
    bool attemptEnable = true;
    EXPECT_CALL(WifiSettings::GetInstance(),
        SetDeviceState(networkId, (int)WifiDeviceConfigStatus::ENABLED, attemptEnable))
        .WillRepeatedly(Return(-1));
    EXPECT_TRUE(pStaService->EnableDeviceConfig(networkId, attemptEnable) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceEnableDeviceConfigFail2()
{
    int networkId = NETWORK_ID;
    bool attemptEnable = true;
    EXPECT_TRUE(pStaService->EnableDeviceConfig(networkId, attemptEnable) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceDisableDeviceConfigSuccess()
{
    int networkId = NETWORK_ID;
    bool attemptEnable = false;
    EXPECT_CALL(WifiSettings::GetInstance(),
        SetDeviceState(networkId, (int)WifiDeviceConfigStatus::DISABLED, attemptEnable))
        .WillRepeatedly(Return(0));
    EXPECT_TRUE(pStaService->DisableDeviceConfig(networkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceDisableDeviceConfigFail1()
{
    int networkId = NETWORK_ID;
    bool attemptEnable = false;
    EXPECT_CALL(WifiSettings::GetInstance(),
        SetDeviceState(networkId, (int)WifiDeviceConfigStatus::DISABLED, attemptEnable))
        .WillRepeatedly(Return(-1));
    EXPECT_TRUE(pStaService->DisableDeviceConfig(networkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceDisconnectSuccess()
{
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->Disconnect() == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceStartWpsSuccess()
{
    WpsConfig config;
    config.setup = SetupMethod::PBC;
    config.pin = "12345678";
    config.bssid = "01:23:45:67:89:AB";
    EXPECT_TRUE(pStaService->StartWps(config) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceCancelWpsSuccess()
{
    EXPECT_TRUE(pStaService->CancelWps() == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceAutoConnectServiceSuccess()
{
    std::vector<InterScanInfo> scanInfos;
    InterScanInfo scanInfo;
    scanInfo.bssid = "2a:76:93:47:e2:8a";
    scanInfo.ssid = "HMWIFI_W2_EAP_G2_03";
    scanInfo.capabilities = "[RSN-EAP-CCMP][WPA2-EAP-CCMP][ESS]";
    scanInfo.frequency = FREQUENCY;
    scanInfo.timestamp = TIMESTAMP;
    scanInfos.push_back(scanInfo);
    EXPECT_TRUE(pStaService->AutoConnectService(scanInfos) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceRegisterStaServiceCallbackSuccess()
{
    std::vector<StaServiceCallback> callbacks;
    callbacks.push_back(WifiManager::GetInstance().GetStaCallback());
    pStaService->RegisterStaServiceCallback(callbacks);
}

void StaServiceTest::StaServiceRegisterStaServiceCallbackFail()
{
    std::vector<StaServiceCallback> callbacks;
    callbacks.push_back(WifiManager::GetInstance().GetStaCallback());
    pStaService->RegisterStaServiceCallback(callbacks);
}

void StaServiceTest::StaServiceAddCandidateConfigTestSucc()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    int uid = UID;
    int netWorkId = NETWORK_ID;
    std::vector<WifiDeviceConfig> configs;
    configs.push_back(config);
    EXPECT_CALL(WifiSettings::GetInstance(), GetAllCandidateConfig(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(configs), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
        .WillOnce(DoAll(SetArgReferee<TWO>(config), Return(0)));  // 2: The third parameter
    EXPECT_TRUE(pStaService->AddCandidateConfig(uid, config, netWorkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceAddCandidateConfigTestFail0()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "WEP";
    int uid = UID;
    int netWorkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetAllCandidateConfig(_, _)).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->AddCandidateConfig(uid, config, netWorkId) == WIFI_OPT_NOT_SUPPORTED);
}

void StaServiceTest::StaServiceAddCandidateConfigTestFail1()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    int uid = UID;
    int netWorkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetAllCandidateConfig(_, _)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
        .WillOnce(DoAll(SetArgReferee<TWO>(config), Return(0)));  // 2: The third parameter
    pStaService->AddCandidateConfig(uid, config, netWorkId);
}

void StaServiceTest::StaServiceRemoveCandidateConfigTestSucc()
{
    int uid = UID;
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
    .WillRepeatedly(Return(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _)).Times(AtLeast(0)).WillRepeatedly(Return(1));
    EXPECT_CALL(WifiSettings::GetInstance(), RemoveDevice(_)).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig()).Times(AtLeast(1));
    pStaService->RemoveCandidateConfig(uid, networkId);
}

void StaServiceTest::StaServiceRemoveCandidateConfigTestFail()
{
    int uid = UID;
    int networkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
    .WillRepeatedly(Return(-1));
    EXPECT_TRUE(pStaService->RemoveCandidateConfig(uid, networkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceRemoveAllCandidateConfigTestSucc()
{
    int uid = UID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetAllCandidateConfig(_, _)).Times(AtLeast(1));
    EXPECT_TRUE(pStaService->RemoveAllCandidateConfig(uid) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToCandidateConfigTestSucc0()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    int uid = UID;
    int netWorkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
        .WillOnce(DoAll(SetArgReferee<TWO>(config), Return(0)));
    EXPECT_TRUE(pStaService->ConnectToCandidateConfig(uid, netWorkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToCandidateConfigTestSucc1()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "NONE";
    config.lastConnectTime = 1;
    int uid = UID;
    int netWorkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
        .WillOnce(DoAll(SetArgReferee<TWO>(config), Return(0)));
    EXPECT_TRUE(pStaService->ConnectToCandidateConfig(uid, netWorkId) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceConnectToCandidateConfigTestFail()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    int uid = UID;
    int netWorkId = NETWORK_ID;
    EXPECT_CALL(WifiSettings::GetInstance(), GetCandidateConfig(_, _, _))
        .WillOnce(DoAll(SetArgReferee<TWO>(config), Return(-1)));
    EXPECT_TRUE(pStaService->ConnectToCandidateConfig(uid, netWorkId) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceRemoveAllDeviceTestSucc()
{
    EXPECT_CALL(WifiSettings::GetInstance(), ClearDeviceConfig()).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig())
        .WillRepeatedly(Return(0));
    pStaService->RemoveAllDevice();
}

void StaServiceTest::StaServiceRemoveAllDeviceTestFail0()
{
    EXPECT_TRUE(pStaService->RemoveAllDevice() == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceRemoveAllDeviceTestFail1()
{
    EXPECT_CALL(WifiSettings::GetInstance(), ClearDeviceConfig()).Times(AtLeast(1));
    EXPECT_CALL(WifiSettings::GetInstance(), SyncDeviceConfig())
        .WillRepeatedly(Return(1));
    EXPECT_TRUE(pStaService->RemoveAllDevice() == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceReConnectTestSucc()
{
    EXPECT_TRUE(pStaService->ReConnect() == WIFI_OPT_SUCCESS);
}
/**
 * @tc.name: Set suspend mode test
 * @tc.desc: Set suspend mode test function.
 * @tc.type: FUNC
 * @tc.require: issueI5JRBB
 */
void StaServiceTest::StaServiceSetSuspendModeTest()
{
    EXPECT_TRUE(pStaService->SetSuspendMode(false) == WIFI_OPT_SUCCESS);
    EXPECT_TRUE(pStaService->SetSuspendMode(false) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceSetPowerModeTest()
{
    EXPECT_TRUE(pStaService->SetPowerMode(false) == WIFI_OPT_FAILED);
    EXPECT_TRUE(pStaService->SetPowerMode(true) == WIFI_OPT_FAILED);
}

void StaServiceTest::StaServiceOnSystemAbilityChangedTest()
{
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetWifiState(0)).Times(AtLeast(0));
    EXPECT_TRUE(pStaService->OnSystemAbilityChanged(COMM_NET, false) == WIFI_OPT_SUCCESS);
    EXPECT_TRUE(pStaService->OnSystemAbilityChanged(COMM_NET, true) == WIFI_OPT_SUCCESS);
    EXPECT_TRUE(pStaService->OnSystemAbilityChanged(1, true) == WIFI_OPT_SUCCESS);
}

void StaServiceTest::StaServiceStartPortalCertificationTest()
{
    EXPECT_TRUE(pStaService->StartPortalCertification() == WIFI_OPT_SUCCESS);
    pStaService->pStaStateMachine = nullptr;
    EXPECT_TRUE(pStaService->StartPortalCertification() == WIFI_OPT_FAILED);
}

void StaServiceTest::DisableAutoJoin()
{
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->DisableAutoJoin("testCondition"));
}

void StaServiceTest::EnableAutoJoin()
{
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->EnableAutoJoin("testCondition"));
}

void StaServiceTest::RegisterAutoJoinCondition()
{
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->RegisterAutoJoinCondition("testCondition", []() {return true;}));
}

void StaServiceTest::DeregisterAutoJoinCondition()
{
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->DeregisterAutoJoinCondition("testCondition"));
}

void StaServiceTest::RegisterFilterBuilder()
{
    FilterBuilder filterBuilder = [](auto &compositeWifiFilter) {};
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->RegisterFilterBuilder(FilterTag::SAVED_NETWORK_TRACKER_FILTER_TAG,
                                                                   "testFilterBuilder",
                                                                   filterBuilder));
}

void StaServiceTest::DeregisterFilterBuilder()
{
    EXPECT_EQ(WIFI_OPT_SUCCESS, pStaService->DeregisterFilterBuilder(FilterTag::SAVED_NETWORK_TRACKER_FILTER_TAG,
                                                                     "testFilterBuilder"));
}

void StaServiceTest::EnableHiLinkHandshakeFailTest()
{
    WifiDeviceConfig config;
    std::string cmd = "ENABLE=1 BSSID=01:23:45:67:89:AB";
    pStaService->EnableHiLinkHandshake(config, cmd);
}

void StaServiceTest::EnableHiLinkHandshakeSuceessTest()
{
    WifiDeviceConfig config;
    std::string cmd = "ENABLE=0 BSSID=01:23:45:67:89:AB";
    pStaService->EnableHiLinkHandshake(config, cmd);
}

void StaServiceTest::DeliverStaIfaceDataSuccessTest()
{
    std::string mac = "01:23:45:67:89:AB";
    pStaService->DeliverStaIfaceData(mac);
}

void StaServiceTest::GetDataSlotIdTest()
{
    pStaService->GetDataSlotId(0);
    pStaService->GetDataSlotId(-1);
}

void StaServiceTest::GetImsiTest()
{
    int32_t slotId = 0;
    pStaService->GetImsi(slotId);
}

void StaServiceTest::GetPlmnTest()
{
    int32_t slotId = 0;
    pStaService->GetPlmn(slotId);
}

void StaServiceTest::GetMccTest()
{
    std::string imsi = "";
    pStaService->GetMcc(imsi);
}

void StaServiceTest::GetMncTest()
{
    std::string imsi = "1234";
    const int mncLen = 4;
    pStaService->GetMnc(imsi, mncLen);
}

void StaServiceTest::UpdateEapConfigTest()
{
    const WifiDeviceConfig config;
    WifiEapConfig wifiEapConfig;
    pStaService->UpdateEapConfig(config, wifiEapConfig);
}

void StaServiceTest::OnWifiCountryCodeChangedTest()
{
    const std::string wifiCountryCode = "CN";
}

void StaServiceTest::StartPortalCertificationTest()
{
    pStaService->StartPortalCertification();
}

void StaServiceTest::HandleForegroundAppChangedActionTest()
{
    AppExecFwk::AppStateData appData;
    pStaService->HandleForegroundAppChangedAction(appData);
}

void StaServiceTest::EnableHiLinkHandshakeTest()
{
    WifiDeviceConfig config;
    std::string bssid = "11:22:33:44";
    pStaService->EnableHiLinkHandshake(config, bssid);
}

void StaServiceTest::DeliverStaIfaceDataTest()
{
    const std::string currentMac = "11:22:33:44";
    pStaService->DeliverStaIfaceData(currentMac);
}

void StaServiceTest::StartRoamToNetworkTest()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:AB";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123456";
    WifiLinkedInfo info;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _)).Times(AtLeast(0));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));
    pStaService->StartRoamToNetwork(0, "11:22:33:44");
}

int StaServiceTest::StartConnectToUserSelectNetworkSuccessTest()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:CD";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "123";
    WifiLinkedInfo info;
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(0)));
    return static_cast<int>(pStaService->StartConnectToUserSelectNetwork(0, "11:22:33:44"));
}

int StaServiceTest::StartConnectToUserSelectNetworkSuccessFail()
{
    WifiDeviceConfig config;
    config.bssid = "01:23:45:67:89:EF";
    config.band = BAND;
    config.networkId = NETWORK_ID;
    config.ssid = "networkId";
    config.keyMgmt = "456";
    WifiLinkedInfo info;
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(_, _))
    .Times(AtLeast(0)).WillOnce(DoAll(SetArgReferee<1>(config), Return(1)));
    return static_cast<int>(pStaService->StartConnectToUserSelectNetwork(0, "11:22:33:44"));
}

void StaServiceTest::SetTxPowerTest()
{
    pStaService->SetTxPower(0);
}

HWTEST_F(StaServiceTest, StaServiceStartPortalCertificationTest, TestSize.Level1)
{
    StaServiceStartPortalCertificationTest();
}

HWTEST_F(StaServiceTest, StaServiceOnSystemAbilityChangedTest, TestSize.Level1)
{
    StaServiceOnSystemAbilityChangedTest();
}

HWTEST_F(StaServiceTest, StaServiceSetPowerModeTest, TestSize.Level1)
{
    StaServiceSetPowerModeTest();
}

HWTEST_F(StaServiceTest, StaServiceEnableWifiSuccess, TestSize.Level1)
{
    StaServiceEnableWifiSuccess();
}

HWTEST_F(StaServiceTest, StaServiceConnectToWifiDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceConnectToWifiDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceConnectToWifiDeviceConfigFail1, TestSize.Level1)
{
    StaServiceConnectToWifiDeviceConfigFail1();
}

HWTEST_F(StaServiceTest, StaServiceConnectToWifiDeviceConfigFail2, TestSize.Level1)
{
    StaServiceConnectToWifiDeviceConfigFail2();
}

HWTEST_F(StaServiceTest, StaServiceConnectToWifiDeviceConfigFail3, TestSize.Level1)
{
    StaServiceConnectToWifiDeviceConfigFail3();
}

HWTEST_F(StaServiceTest, StaServiceConnectToNetworkIdSuccess, TestSize.Level1)
{
    StaServiceConnectToNetworkIdSuccess();
}

HWTEST_F(StaServiceTest, StaServiceConnectToNetworkIdFail1, TestSize.Level1)
{
    StaServiceConnectToNetworkIdFail1();
}

HWTEST_F(StaServiceTest, StaServiceReAssociateSuccess, TestSize.Level1)
{
    StaServiceReAssociateSuccess();
}

HWTEST_F(StaServiceTest, StaServiceAddDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceAddDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceAddDeviceConfigFail1, TestSize.Level1)
{
    StaServiceAddDeviceConfigFail1();
}

HWTEST_F(StaServiceTest, StaServiceAddDeviceConfigFail2, TestSize.Level1)
{
    StaServiceAddDeviceConfigFail2();
}

HWTEST_F(StaServiceTest, StaServiceAddDeviceConfigFail3, TestSize.Level1)
{
    StaServiceAddDeviceConfigFail3();
}

HWTEST_F(StaServiceTest, StaServiceUpdateDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceUpdateDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceRemoveDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceRemoveDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceRemoveDeviceConfigFail1, TestSize.Level1)
{
    StaServiceRemoveDeviceConfigFail1();
}

HWTEST_F(StaServiceTest, StaServiceRemoveDeviceConfigFail2, TestSize.Level1)
{
    StaServiceRemoveDeviceConfigFail2();
}

HWTEST_F(StaServiceTest, StaServiceEnableDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceEnableDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceEnableDeviceConfigFail1, TestSize.Level1)
{
    StaServiceEnableDeviceConfigFail1();
}

HWTEST_F(StaServiceTest, StaServiceEnableDeviceConfigFail2, TestSize.Level1)
{
    StaServiceEnableDeviceConfigFail2();
}

HWTEST_F(StaServiceTest, StaServiceDisableDeviceConfigSuccess, TestSize.Level1)
{
    StaServiceDisableDeviceConfigSuccess();
}

HWTEST_F(StaServiceTest, StaServiceDisableDeviceConfigFail1, TestSize.Level1)
{
    StaServiceDisableDeviceConfigFail1();
}

HWTEST_F(StaServiceTest, StaServiceDisconnectSuccess, TestSize.Level1)
{
    StaServiceDisconnectSuccess();
}

HWTEST_F(StaServiceTest, StaServiceStartWpsSuccess, TestSize.Level1)
{
    StaServiceStartWpsSuccess();
}

HWTEST_F(StaServiceTest, StaServiceCancelWpsSuccess, TestSize.Level1)
{
    StaServiceCancelWpsSuccess();
}

HWTEST_F(StaServiceTest, StaServiceAutoConnectServiceSuccess, TestSize.Level1)
{
    StaServiceAutoConnectServiceSuccess();
}

HWTEST_F(StaServiceTest, StaServiceRegisterStaServiceCallbackSuccess, TestSize.Level1)
{
    StaServiceRegisterStaServiceCallbackSuccess();
}

HWTEST_F(StaServiceTest, StaServiceRegisterStaServiceCallbackFail, TestSize.Level1)
{
    StaServiceRegisterStaServiceCallbackFail();
}

HWTEST_F(StaServiceTest, StaServiceAddCandidateConfigTestSucc, TestSize.Level1)
{
    StaServiceAddCandidateConfigTestSucc();
}

HWTEST_F(StaServiceTest, StaServiceAddCandidateConfigTestFail0, TestSize.Level1)
{
    StaServiceAddCandidateConfigTestFail0();
}

HWTEST_F(StaServiceTest, StaServiceAddCandidateConfigTestFail1, TestSize.Level1)
{
    StaServiceAddCandidateConfigTestFail1();
}

HWTEST_F(StaServiceTest, StaServiceRemoveCandidateConfigTestSucc, TestSize.Level1)
{
    StaServiceRemoveCandidateConfigTestSucc();
}

HWTEST_F(StaServiceTest, StaServiceRemoveCandidateConfigTestFail, TestSize.Level1)
{
    StaServiceRemoveCandidateConfigTestFail();
}

HWTEST_F(StaServiceTest, StaServiceRemoveAllCandidateConfigTestSucc, TestSize.Level1)
{
    StaServiceRemoveAllCandidateConfigTestSucc();
}

HWTEST_F(StaServiceTest, StaServiceConnectToCandidateConfigTestSucc0, TestSize.Level1)
{
    StaServiceConnectToCandidateConfigTestSucc0();
}

HWTEST_F(StaServiceTest, StaServiceConnectToCandidateConfigTestSucc1, TestSize.Level1)
{
    StaServiceConnectToCandidateConfigTestSucc1();
}

HWTEST_F(StaServiceTest, StaServiceConnectToCandidateConfigTestFail, TestSize.Level1)
{
    StaServiceConnectToCandidateConfigTestFail();
}

HWTEST_F(StaServiceTest, StaServiceRemoveAllDeviceTestSucc, TestSize.Level1)
{
    StaServiceRemoveAllDeviceTestSucc();
}

HWTEST_F(StaServiceTest, StaServiceRemoveAllDeviceTestFail0, TestSize.Level1)
{
    StaServiceRemoveAllDeviceTestFail0();
}

HWTEST_F(StaServiceTest, StaServiceRemoveAllDeviceTestFail1, TestSize.Level1)
{
    StaServiceRemoveAllDeviceTestFail1();
}

HWTEST_F(StaServiceTest, StaServiceReConnectTestSucc, TestSize.Level1)
{
    StaServiceReConnectTestSucc();
}
/**
 * @tc.name: Set suspend mode test
 * @tc.desc: Set suspend mode test function.
 * @tc.type: FUNC
 * @tc.require: issueI5JRBB
 */
HWTEST_F(StaServiceTest, StaServiceSetSuspendMode, TestSize.Level1)
{
    StaServiceSetSuspendModeTest();
}

HWTEST_F(StaServiceTest, DisableAutoJoin, TestSize.Level1)
{
    DisableAutoJoin();
}

HWTEST_F(StaServiceTest, EnableAutoJoin, TestSize.Level1)
{
    EnableAutoJoin();
}

HWTEST_F(StaServiceTest, RegisterAutoJoinCondition, TestSize.Level1)
{
    RegisterAutoJoinCondition();
}

HWTEST_F(StaServiceTest, DeregisterAutoJoinCondition, TestSize.Level1)
{
    DeregisterAutoJoinCondition();
}

HWTEST_F(StaServiceTest, RegisterFilterBuilder, TestSize.Level1)
{
    RegisterFilterBuilder();
}

HWTEST_F(StaServiceTest, DeregisterFilterBuilder, TestSize.Level1)
{
    DeregisterFilterBuilder();
}

HWTEST_F(StaServiceTest, EnableHiLinkHandshakeSuceessTest, TestSize.Level1)
{
    EnableHiLinkHandshakeSuceessTest();
}

HWTEST_F(StaServiceTest, EnableHiLinkHandshakeFailTest, TestSize.Level1)
{
    EnableHiLinkHandshakeFailTest();
}

HWTEST_F(StaServiceTest, DeliverStaIfaceDataSuccessTest, TestSize.Level1)
{
    DeliverStaIfaceDataSuccessTest();
}

HWTEST_F(StaServiceTest, GetDataSlotIdTest, TestSize.Level1)
{
    GetDataSlotIdTest();
}

HWTEST_F(StaServiceTest, GetImsiTest, TestSize.Level1)
{
    GetImsiTest();
}

HWTEST_F(StaServiceTest, GetPlmnTest, TestSize.Level1)
{
    GetPlmnTest();
}

HWTEST_F(StaServiceTest, GetMccTest, TestSize.Level1)
{
    GetMccTest();
}

HWTEST_F(StaServiceTest, GetMncTest, TestSize.Level1)
{
    GetMncTest();
}

HWTEST_F(StaServiceTest, UpdateEapConfigTest, TestSize.Level1)
{
    UpdateEapConfigTest();
}

HWTEST_F(StaServiceTest, OnWifiCountryCodeChangedTest, TestSize.Level1)
{
    OnWifiCountryCodeChangedTest();
}

HWTEST_F(StaServiceTest, StartPortalCertificationTest, TestSize.Level1)
{
    StartPortalCertificationTest();
}

HWTEST_F(StaServiceTest, HandleForegroundAppChangedActionTest, TestSize.Level1)
{
    HandleForegroundAppChangedActionTest();
}

HWTEST_F(StaServiceTest, EnableHiLinkHandshakeTest, TestSize.Level1)
{
    EnableHiLinkHandshakeTest();
}

HWTEST_F(StaServiceTest, DeliverStaIfaceDataTest, TestSize.Level1)
{
    DeliverStaIfaceDataTest();
}

HWTEST_F(StaServiceTest, StartRoamToNetworkTest, TestSize.Level1)
{
    StartRoamToNetworkTest();
}

HWTEST_F(StaServiceTest, StartConnectToUserSelectNetworkSuccessTest, TestSize.Level1)
{
    EXPECT_EQ(0, StartConnectToUserSelectNetworkSuccessTest());
}

HWTEST_F(StaServiceTest, StartConnectToUserSelectNetworkSuccessFail, TestSize.Level1)
{
    EXPECT_EQ(1, StartConnectToUserSelectNetworkSuccessFail());
}
 
HWTEST_F(StaServiceTest, SetTxPowerTest, TestSize.Level1)
{
    SetTxPowerTest();
}
} // namespace Wifi
} // namespace OHOS
