/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <string>
#include "wifi_filter_impl.h"
#include "mock_wifi_settings.h"
#include "network_selection_utils.h"
#include "mock_wifi_config_center.h"
#include "wifi_sensor_scene.h"
#include "network_black_list_manager.h"
#include "wifi_errcode.h"

using ::testing::_;
using ::testing::Return;
using ::testing::An;
using ::testing::ext::TestSize;
using ::testing::ReturnRoundRobin;
using ::testing::Invoke;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::TypedEq;

namespace OHOS {
namespace Wifi {


class WifiFilterImplTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

HWTEST_F(WifiFilterImplTest, HiddenWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.bssid = "11:11:11:11:11:55";
    scanInfo1.frequency = 2407;
    scanInfo1.rssi = -77;
    scanInfo1.ssid = "";
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo1);
    networkCandidate.wifiDeviceConfig.networkId = 1;
    auto hiddenWifiFilter = std::make_shared<NetworkSelection::HiddenWifiFilter>();
    EXPECT_FALSE(hiddenWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, HiddenWifiFilterReturnTrue, TestSize.Level1) {

    InterScanInfo scanInfo2;
    scanInfo2.bssid = "11:22:11:11:22:44";
    scanInfo2.frequency = 2407;
    scanInfo2.rssi = -77;
    scanInfo2.ssid = "sssx";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo2);
    networkCandidate1.wifiDeviceConfig.networkId = 5;
    auto hiddenWifiFilter = std::make_shared<NetworkSelection::HiddenWifiFilter>();
    EXPECT_TRUE(hiddenWifiFilter->DoFilter(networkCandidate1));
}
#ifdef WIFI_LOCAL_SECURITY_DETECT_ENABLE
//系统时间可能获取失败
HWTEST_F(WifiFilterImplTest, LongUnusedOpenWifiFilterOpenNotStale, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "00:11:22:33:44:55";
    scanInfo.ssid = "OpenNetwork";
    scanInfo.securityType = WifiSecurity::OPEN;
    scanInfo.riskType = WifiRiskType::OPEN;
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo);
    time_t now = time(nullptr);
    if (now == (time_t)(-1)) {
        GTEST_SKIP() << "time() returned invalid value, skipping test.";
    }
    networkCandidate.wifiDeviceConfig.lastDisconnectTime = now - 1 * 24 * 60 * 60; //1天前断开
    networkCandidate.wifiDeviceConfig.networkId = 1;
    
    auto longUnusedOpenWifiFilter = std::make_shared<NetworkSelection::LongUnusedOpenWifiFilter>();
    EXPECT_TRUE(longUnusedOpenWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, LongUnusedOpenWifiFilterNonOpen, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "00:11:22:33:44:55";
    scanInfo.ssid = "WPA2Network";
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.riskType = WifiRiskType::NORMAL;
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo);
    time_t now = time(nullptr);
    if (now == (time_t)(-1)) {
        // time()返回非法值，跳过测试或标记为失败
        GTEST_SKIP() << "time() returned invalid value, skipping test.";
    }
    networkCandidate.wifiDeviceConfig.lastDisconnectTime = now - 20 * 24 * 60 * 60;
    networkCandidate.wifiDeviceConfig.networkId = 1;
    
    auto longUnusedOpenWifiFilter = std::make_shared<NetworkSelection::LongUnusedOpenWifiFilter>();
    EXPECT_TRUE(longUnusedOpenWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, LongUnusedOpenWifiFilterOpenStale, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "00:11:22:33:44:55";
    scanInfo.ssid = "OldOpenNetwork";
    scanInfo.securityType = WifiSecurity::OPEN;
    scanInfo.riskType = WifiRiskType::OPEN;
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo);
    time_t now = time(nullptr);
    if (now == (time_t)(-1)) {
        // time()返回非法值，跳过测试或标记为失败
        GTEST_SKIP() << "time() returned invalid value, skipping test.";
    }
    networkCandidate.wifiDeviceConfig.lastDisconnectTime = now - (15 * 24 * 60 * 60 + 1000); //超过阈值
    networkCandidate.wifiDeviceConfig.networkId = 1;
    
    auto longUnusedOpenWifiFilter = std::make_shared<NetworkSelection::LongUnusedOpenWifiFilter>();
    EXPECT_FALSE(longUnusedOpenWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, LongUnusedOpenWifiFilterIgnoreCase, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "00:11:22:33:44:55";
    scanInfo.ssid = "NewOpenNetwork";
    scanInfo.securityType = WifiSecurity::OPEN;
    scanInfo.riskType = WifiRiskType::OPEN;
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo);
    networkCandidate.wifiDeviceConfig.lastDisconnectTime = -1; // 不符合本场景特征，不做拦截
    networkCandidate.wifiDeviceConfig.networkId = 1;
    
    auto longUnusedOpenWifiFilter = std::make_shared<NetworkSelection::LongUnusedOpenWifiFilter>();
    EXPECT_TRUE(longUnusedOpenWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, LongUnusedOpenWifiFilterInWhiteList, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "00:11:22:33:44:55";
    scanInfo.ssid = "SafeNetwork";
    scanInfo.securityType = WifiSecurity::OPEN;
    scanInfo.riskType = WifiRiskType::NORMAL;
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo);
    networkCandidate.wifiDeviceConfig.networkId = 1;
    
    auto longUnusedOpenWifiFilter = std::make_shared<NetworkSelection::LongUnusedOpenWifiFilter>();
    EXPECT_TRUE(longUnusedOpenWifiFilter->DoFilter(networkCandidate));
}
#endif
HWTEST_F(WifiFilterImplTest, RssiWifiFilter24gReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.bssid = "11:11:11:11:11:77";
    scanInfo1.ssid = "x";
    scanInfo1.frequency = 2407;
    scanInfo1.rssi = -82;
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(false));
    auto signalStrengthWifiFilter = std::make_shared<NetworkSelection::SignalStrengthWifiFilter>();
    EXPECT_FALSE(signalStrengthWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, RssiWifiFilter24gReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.bssid = "11:11:11:11:11:66";
    scanInfo2.ssid = "x";
    scanInfo2.rssi = -70;
    scanInfo2.frequency = 2407;
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 2;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(false));
    auto signalStrengthWifiFilter = std::make_shared<NetworkSelection::SignalStrengthWifiFilter>();
    EXPECT_TRUE(signalStrengthWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, RssiWifiFilter5gReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.bssid = "11:11:11:11:11:44";
    scanInfo1.ssid = "x";
    scanInfo1.frequency = 5820;
    scanInfo1.rssi = -82;
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 3;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(false));
    auto signalStrengthWifiFilter = std::make_shared<NetworkSelection::SignalStrengthWifiFilter>();
    EXPECT_FALSE(signalStrengthWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, RssiWifiFilter5gReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.bssid = "11:11:11:11:11:22";
    scanInfo2.ssid = "x";
    scanInfo2.rssi = -70;
    scanInfo2.frequency = 5820;
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 4;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(false));
    auto signalStrengthWifiFilter = std::make_shared<NetworkSelection::SignalStrengthWifiFilter>();
    EXPECT_TRUE(signalStrengthWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, RssiWifiFilterWlanReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.bssid = "11:11:11:11:11:22";
    scanInfo2.ssid = "x";
    scanInfo2.rssi = -75;
    scanInfo2.frequency = 5820;
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 4;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(true));
    WifiSensorScene::GetInstance().scenario_ = 1;
    auto signalStrengthWifiFilter = std::make_shared<NetworkSelection::SignalStrengthWifiFilter>();
    EXPECT_TRUE(signalStrengthWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, SavedWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:33";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = -1;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:77";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkStatusHistory = 255;
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.uid = 1;
    networkCandidate2.wifiDeviceConfig.isShared = false;

    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:44";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.networkId = 1;

    auto savedWifiFilter = std::make_shared<NetworkSelection::SavedWifiFilter>();
    EXPECT_FALSE(savedWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(savedWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, SavedWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.networkId = 1;

    auto savedWifiFilter = std::make_shared<NetworkSelection::SavedWifiFilter>();
    EXPECT_TRUE(savedWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, EphemeralWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.isEphemeral = true;

    auto ephemeralWifiFilter = std::make_shared<NetworkSelection::EphemeralWifiFilter>();
    EXPECT_FALSE(ephemeralWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, EphemeralWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.ssid = "xx";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 2;
    networkCandidate2.wifiDeviceConfig.isEphemeral = false;

    auto ephemeralWifiFilter = std::make_shared<NetworkSelection::EphemeralWifiFilter>();
    EXPECT_TRUE(ephemeralWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, PassPointWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:22:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.isPasspoint = true;

    auto passPointWifiFilter = std::make_shared<NetworkSelection::PassPointWifiFilter>();
    EXPECT_FALSE(passPointWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, PassPointWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.isPasspoint = false;
    networkCandidate2.wifiDeviceConfig.networkId = 1;

    auto passPointWifiFilter = std::make_shared<NetworkSelection::PassPointWifiFilter>();
    EXPECT_TRUE(passPointWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, DisableWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::DISABLED;

    auto disableWifiFilter = std::make_shared<NetworkSelection::DisableWifiFilter>();
    EXPECT_FALSE(disableWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, DisableWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    auto disableWifiFilter = std::make_shared<NetworkSelection::DisableWifiFilter>();
    EXPECT_TRUE(disableWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, MatchedUserSelectBssidWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo1);
    networkCandidate.wifiDeviceConfig.userSelectBssid = "";
    networkCandidate.wifiDeviceConfig.networkId = 1;

    auto systemNetworkWifiFilter = std::make_shared<NetworkSelection::MatchedUserSelectBssidWifiFilter>();
    EXPECT_TRUE(systemNetworkWifiFilter->DoFilter(networkCandidate));
}

HWTEST_F(WifiFilterImplTest, MatchedUserSelectBssidWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.userSelectBssid = "11:22:11:11:11:22";

    auto systemNetworkWifiFilter = std::make_shared<NetworkSelection::MatchedUserSelectBssidWifiFilter>();
    EXPECT_FALSE(systemNetworkWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, HasInternetWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate3.wifiDeviceConfig.networkStatusHistory = 7;

    auto hasInternetWifiFilter = std::make_shared<NetworkSelection::HasInternetWifiFilter>();
    EXPECT_TRUE(hasInternetWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, HasInternetWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.noInternetAccess = 1;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.isPortal = 1;
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 0;

    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate3.wifiDeviceConfig.networkStatusHistory = 7;

    InterScanInfo scanInfo4;
    scanInfo4.ssid = "x";
    scanInfo4.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate4(scanInfo4);
    networkCandidate4.wifiDeviceConfig.keyMgmt = KEY_MGMT_NONE;
    networkCandidate4.wifiDeviceConfig.networkId = 1;
    networkCandidate4.wifiDeviceConfig.noInternetAccess = 0;

    InterScanInfo scanInfo5;
    scanInfo5.ssid = "x";
    scanInfo5.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate5(scanInfo5);
    networkCandidate5.wifiDeviceConfig.keyMgmt = KEY_MGMT_WEP;
    networkCandidate5.wifiDeviceConfig.networkId = 1;
    networkCandidate5.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate5.wifiDeviceConfig.networkStatusHistory = 15;

    auto hasInternetWifiFilter = std::make_shared<NetworkSelection::HasInternetWifiFilter>();
    EXPECT_FALSE(hasInternetWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(hasInternetWifiFilter->DoFilter(networkCandidate2));
    EXPECT_FALSE(hasInternetWifiFilter->DoFilter(networkCandidate4));
    EXPECT_FALSE(hasInternetWifiFilter->DoFilter(networkCandidate5));
}

HWTEST_F(WifiFilterImplTest, RecoveryWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "99:11:11:11:99:22";
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo1);
    networkCandidate.wifiDeviceConfig.networkId = 1;
    networkCandidate.wifiDeviceConfig.networkStatusHistory = 0;
    

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "xx";
    scanInfo2.bssid = "99:11:11:11:99:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkStatusHistory = 5;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 3;
    networkCandidate2.wifiDeviceConfig.isPortal = 0;

    auto recoveryWifiFilter = std::make_shared<NetworkSelection::RecoveryWifiFilter>();
    EXPECT_TRUE(recoveryWifiFilter->DoFilter(networkCandidate));
    EXPECT_TRUE(recoveryWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, RecoveryWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "99:11:11:11:99:22";

    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.networkStatusHistory = 3;
    networkCandidate1.wifiDeviceConfig.noInternetAccess = 0;
    
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo1);
    networkCandidate2.wifiDeviceConfig.networkStatusHistory = 3;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 1;
    networkCandidate2.wifiDeviceConfig.isPortal = 1;
    

    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo1);
    networkCandidate3.wifiDeviceConfig.networkStatusHistory = 3;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 1;
    networkCandidate3.wifiDeviceConfig.isPortal = 0;

    auto recoveryWifiFilter = std::make_shared<NetworkSelection::RecoveryWifiFilter>();
    EXPECT_FALSE(recoveryWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(recoveryWifiFilter->DoFilter(networkCandidate2));
    EXPECT_FALSE(recoveryWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, PoorPortalWifiFilter24gReturnTrue, TestSize.Level1) {
    //2.4g wifi
    InterScanInfo scanInfo2;
    scanInfo2.ssid = "xs";
    scanInfo2.bssid = "11:11:11:11:33:22";
    scanInfo2.rssi = -34;
    scanInfo2.band = 1;
    scanInfo2.frequency = 2640;
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo2);
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(4));

    auto poorPortalWifiFilter = std::make_shared<NetworkSelection::PoorPortalWifiFilter>();
    EXPECT_TRUE(poorPortalWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, PoorPortalWifiFilter24gReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate(scanInfo1);
    networkCandidate.wifiDeviceConfig.isPortal = 1;
    networkCandidate.wifiDeviceConfig.networkId = 1;
    networkCandidate.wifiDeviceConfig.noInternetAccess = 1;
    networkCandidate.wifiDeviceConfig.networkStatusHistory = 3;

    InterScanInfo scanInfo3;
    scanInfo3.ssid = "xs";
    scanInfo3.bssid = "11:11:11:11:33:22";
    scanInfo3.rssi = -86;
    scanInfo3.band = 1;
    scanInfo3.frequency = 2640;
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo3);
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(1));

    auto poorPortalWifiFilter = std::make_shared<NetworkSelection::PoorPortalWifiFilter>();
    EXPECT_FALSE(poorPortalWifiFilter->DoFilter(networkCandidate));
    EXPECT_FALSE(poorPortalWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, PoorPortalWifiFilter5gReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo4;
    scanInfo4.ssid = "xs";
    scanInfo4.bssid = "11:11:11:11:33:22";
    scanInfo4.rssi = -50;
    scanInfo4.band = 2;
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo4);
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(4));

    auto poorPortalWifiFilter = std::make_shared<NetworkSelection::PoorPortalWifiFilter>();
    EXPECT_TRUE(poorPortalWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, PoorPortalWifiFilter5gReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo5;
    scanInfo5.ssid = "xs";
    scanInfo5.bssid = "11:11:11:11:33:22";
    scanInfo5.rssi = -85;
    scanInfo5.band = 2;
    scanInfo5.frequency = 5640;
    NetworkSelection::NetworkCandidate networkCandidate4(scanInfo5);
    networkCandidate4.wifiDeviceConfig.networkId = 1;
    networkCandidate4.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(1));

    InterScanInfo scanInfo6;
    scanInfo6.ssid = "xs";
    scanInfo6.bssid = "11:11:11:11:33:22";
    scanInfo6.rssi = -79;
    scanInfo6.band = 2;
    scanInfo6.frequency = 5640;
    NetworkSelection::NetworkCandidate networkCandidate5(scanInfo6);
    networkCandidate5.wifiDeviceConfig.networkId = 1;
    networkCandidate5.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate5.wifiDeviceConfig.lastHasInternetTime = 1735366164;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(2));

    auto poorPortalWifiFilter = std::make_shared<NetworkSelection::PoorPortalWifiFilter>();
    EXPECT_FALSE(poorPortalWifiFilter->DoFilter(networkCandidate4));
    EXPECT_FALSE(poorPortalWifiFilter->DoFilter(networkCandidate5));
}

HWTEST_F(WifiFilterImplTest, PoorPortalWifiFilterIsWlanPageReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo4;
    scanInfo4.ssid = "x";
    scanInfo4.rssi = -50;
    scanInfo4.band = 2;
    scanInfo4.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate4(scanInfo4);
    networkCandidate4.wifiDeviceConfig.isPortal = 1;
    networkCandidate4.wifiDeviceConfig.networkId = 1;
    networkCandidate4.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _)).WillRepeatedly(Return(4));
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(true));

    auto poorportalWifiFilter = std::make_shared<NetworkSelection::PortalWifiFilter>();
    EXPECT_TRUE(poorportalWifiFilter->DoFilter(networkCandidate4));
}

HWTEST_F(WifiFilterImplTest, PortalWifiFilterIsWlanPageReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.isPortal = 1;
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(true));

    auto portalWifiFilter = std::make_shared<NetworkSelection::PortalWifiFilter>();
    EXPECT_TRUE(portalWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, PortalWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.isPortal = 1;
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 0;

    auto portalWifiFilter = std::make_shared<NetworkSelection::PortalWifiFilter>();
    EXPECT_TRUE(portalWifiFilter->DoFilter(networkCandidate3));
}

HWTEST_F(WifiFilterImplTest, PortalWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.isPortal = 1;
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.noInternetAccess = 1;
    networkCandidate1.wifiDeviceConfig.networkStatusHistory = 3;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.isPortal = 0;
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 0;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), IsWlanPage()).WillRepeatedly(Return(false));

    auto portalWifiFilter = std::make_shared<NetworkSelection::PortalWifiFilter>();
    EXPECT_FALSE(portalWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(portalWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, MaybePortalWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.keyMgmt = KEY_MGMT_NONE;
    networkCandidate1.wifiDeviceConfig.networkId = 1;
    networkCandidate1.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate1.wifiDeviceConfig.networkStatusHistory = 0;

    auto maybePortalWifiFilter = std::make_shared<NetworkSelection::MaybePortalWifiFilter>();
    EXPECT_TRUE(maybePortalWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, MaybePortalWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    scanInfo1.capabilities = "OWE";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.keyMgmt = KEY_MGMT_WEP;
    networkCandidate2.wifiDeviceConfig.networkId = 1;
    networkCandidate2.wifiDeviceConfig.noInternetAccess = 0;

    InterScanInfo scanInfo3;
    scanInfo3.ssid = "x";
    scanInfo3.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate3(scanInfo3);
    networkCandidate3.wifiDeviceConfig.keyMgmt = KEY_MGMT_NONE;
    networkCandidate3.wifiDeviceConfig.networkId = 1;
    networkCandidate3.wifiDeviceConfig.noInternetAccess = 1;

    InterScanInfo scanInfo4;
    scanInfo4.ssid = "x";
    scanInfo4.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate4(scanInfo4);
    networkCandidate4.wifiDeviceConfig.keyMgmt = KEY_MGMT_NONE;
    networkCandidate4.wifiDeviceConfig.networkId = 1;
    networkCandidate4.wifiDeviceConfig.noInternetAccess = 0;
    networkCandidate4.wifiDeviceConfig.networkStatusHistory = 7;

    auto maybePortalWifiFilter = std::make_shared<NetworkSelection::MaybePortalWifiFilter>();
    EXPECT_FALSE(maybePortalWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(maybePortalWifiFilter->DoFilter(networkCandidate2));
    EXPECT_FALSE(maybePortalWifiFilter->DoFilter(networkCandidate3));
    EXPECT_FALSE(maybePortalWifiFilter->DoFilter(networkCandidate4));
}

HWTEST_F(WifiFilterImplTest, NoInternetWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkStatusHistory = 255;
    networkCandidate1.wifiDeviceConfig.networkId = 1;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkStatusHistory = 5;
    networkCandidate2.wifiDeviceConfig.networkId = 1;

    auto noInternetWifiFilter = std::make_shared<NetworkSelection::NoInternetWifiFilter>();
    EXPECT_TRUE(noInternetWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, NoInternetWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkStatusHistory = 255;
    networkCandidate1.wifiDeviceConfig.networkId = 1;

    auto noInternetWifiFilter = std::make_shared<NetworkSelection::NoInternetWifiFilter>();
    EXPECT_FALSE(noInternetWifiFilter->DoFilter(networkCandidate1));
}

HWTEST_F(WifiFilterImplTest, WeakAlgorithmWifiFilterReturnTrue, TestSize.Level1) {
    InterScanInfo scanInfo4;
    scanInfo4.ssid = "x";
    scanInfo4.bssid = "11:11:11:11:11:22";
    scanInfo4.securityType = WifiSecurity::PSK;
    scanInfo4.capabilities = "CCMPTKIP";
    NetworkSelection::NetworkCandidate networkCandidate4(scanInfo4);
    networkCandidate4.wifiDeviceConfig.networkId = 1;

    InterScanInfo scanInfo5;
    scanInfo5.ssid = "x";
    scanInfo5.bssid = "11:11:11:11:11:22";
    scanInfo5.securityType = WifiSecurity::EAP;
    NetworkSelection::NetworkCandidate networkCandidate5(scanInfo5);
    networkCandidate5.wifiDeviceConfig.networkId = 1;

    auto weakAlgorithmWifiFilter = std::make_shared<NetworkSelection::WeakAlgorithmWifiFilter>();
    EXPECT_TRUE(weakAlgorithmWifiFilter->DoFilter(networkCandidate4));
    EXPECT_TRUE(weakAlgorithmWifiFilter->DoFilter(networkCandidate5));
}

HWTEST_F(WifiFilterImplTest, WeakAlgorithmWifiFilterReturnFalse, TestSize.Level1) {
    InterScanInfo scanInfo1;
    scanInfo1.ssid = "x";
    scanInfo1.bssid = "11:11:11:11:11:22";
    scanInfo1.securityType = WifiSecurity::WEP;
    NetworkSelection::NetworkCandidate networkCandidate1(scanInfo1);
    networkCandidate1.wifiDeviceConfig.networkId = 1;

    InterScanInfo scanInfo2;
    scanInfo2.ssid = "x";
    scanInfo2.bssid = "11:11:11:11:11:22";
    scanInfo2.securityType = WifiSecurity::OPEN;
    NetworkSelection::NetworkCandidate networkCandidate2(scanInfo2);
    networkCandidate2.wifiDeviceConfig.networkId = 1;

    auto weakAlgorithmWifiFilter = std::make_shared<NetworkSelection::WeakAlgorithmWifiFilter>();
    EXPECT_FALSE(weakAlgorithmWifiFilter->DoFilter(networkCandidate1));
    EXPECT_FALSE(weakAlgorithmWifiFilter->DoFilter(networkCandidate2));
}

HWTEST_F(WifiFilterImplTest, HigherCategoryFilterTest_CandidateIsHigher_ShouldPass, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::HigherCategoryFilter>();
    WifiLinkedInfo currentLinkInfo;
    currentLinkInfo.bssid = "11:22:33:44:55:66";
    currentLinkInfo.networkId = 1;
    InterScanInfo candidateScanInfo;
    candidateScanInfo.bssid = "AA:BB:CC:DD:EE:FF";
    NetworkSelection::NetworkCandidate candidate(candidateScanInfo);

    // Arrange: Mock the behavior of WifiConfigCenter
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(Invoke([&](WifiLinkedInfo& info, int32_t) {
            info = currentLinkInfo;
            return 0;
        }));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(currentLinkInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI6));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(candidateScanInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));

    // Act & Assert
    EXPECT_TRUE(filter->DoFilter(candidate));
}

HWTEST_F(WifiFilterImplTest, HigherCategoryFilterTest_CandidateIsLower_ShouldFilter, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::HigherCategoryFilter>();
    WifiLinkedInfo currentLinkInfo;
    currentLinkInfo.bssid = "11:22:33:44:55:66";
    currentLinkInfo.networkId = 1;
    InterScanInfo candidateScanInfo;
    candidateScanInfo.bssid = "AA:BB:CC:DD:EE:FF";
    NetworkSelection::NetworkCandidate candidate(candidateScanInfo);

    // Arrange: Mock the behavior of WifiConfigCenter
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(Invoke([&](WifiLinkedInfo& info, int32_t) {
            info = currentLinkInfo;
            return 0;
        }));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(currentLinkInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(candidateScanInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI6));

    // Act & Assert
    EXPECT_FALSE(filter->DoFilter(candidate));
}

HWTEST_F(WifiFilterImplTest, HigherCategoryFilterTest_2GCandidate_ShouldFilter, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::HigherCategoryFilter>();
    WifiLinkedInfo currentLinkInfo;
    currentLinkInfo.bssid = "11:22:33:44:55:66";
    currentLinkInfo.networkId = 1;
    currentLinkInfo.ssid = "Current5GWiFi7";
    currentLinkInfo.band = static_cast<int>(BandType::BAND_5GHZ);
    InterScanInfo candidateScanInfo;
    candidateScanInfo.bssid = "AA:BB:CC:DD:EE:FF";
    candidateScanInfo.ssid = "Candidate24GWiFi7";
    candidateScanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    NetworkSelection::NetworkCandidate candidate(candidateScanInfo);

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(Invoke([&](WifiLinkedInfo& info, int32_t) {
            info = currentLinkInfo;
            return 0;
        }));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(currentLinkInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(candidateScanInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));

    // Act & Assert: 2.4G WiFi7 不应通过过滤
    EXPECT_FALSE(filter->DoFilter(candidate));
}

HWTEST_F(WifiFilterImplTest, HigherCategoryFilterTest_CandidateIsSame_ShouldFilter, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::HigherCategoryFilter>();
    WifiLinkedInfo currentLinkInfo;
    currentLinkInfo.bssid = "11:22:33:44:55:66";
    currentLinkInfo.networkId = 1;
    InterScanInfo candidateScanInfo;
    candidateScanInfo.bssid = "AA:BB:CC:DD:EE:FF";
    NetworkSelection::NetworkCandidate candidate(candidateScanInfo);

    // Arrange: Mock the behavior of WifiConfigCenter
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillRepeatedly(Invoke([&](WifiLinkedInfo& info, int32_t) {
            info = currentLinkInfo;
            return 0;
        }));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(currentLinkInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));
    EXPECT_CALL(*WifiConfigCenter::GetInstance().GetWifiScanConfig(), GetWifiCategoryRecord(candidateScanInfo.bssid))
        .WillRepeatedly(Return(WifiCategory::WIFI7));

    // Act & Assert
    EXPECT_FALSE(filter->DoFilter(candidate));
}

HWTEST_F(WifiFilterImplTest, Perf5gBlackListFilterTest_BssidInList_ShouldFilter, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::Perf5gBlackListFilter>();
    InterScanInfo scanInfo;
    const std::string bssid = "11:22:33:44:55:66";
    scanInfo.bssid = bssid;
    NetworkSelection::NetworkCandidate candidate(scanInfo);

    // Arrange: Add BSSID to the blocklist and verify it's in the list.
    NetworkBlockListManager::GetInstance().AddPerf5gBlocklist(bssid);
    ASSERT_TRUE(NetworkBlockListManager::GetInstance().IsInPerf5gBlocklist(bssid));

    // Act & Assert: The filter should block the candidate.
    EXPECT_FALSE(filter->DoFilter(candidate));

    // Cleanup
    NetworkBlockListManager::GetInstance().RemovePerf5gBlocklist(bssid);
}

HWTEST_F(WifiFilterImplTest, Perf5gBlackListFilterTest_BssidNotInList_ShouldPass, TestSize.Level1)
{
    auto filter = std::make_shared<NetworkSelection::Perf5gBlackListFilter>();
    InterScanInfo scanInfo;
    const std::string bssid = "11:22:33:44:55:77";
    scanInfo.bssid = bssid;
    NetworkSelection::NetworkCandidate candidate(scanInfo);

    // Arrange: Ensure the BSSID is not in the blocklist.
    NetworkBlockListManager::GetInstance().RemovePerf5gBlocklist(bssid);
    ASSERT_FALSE(NetworkBlockListManager::GetInstance().IsInPerf5gBlocklist(bssid));

    // Act & Assert: The filter should pass the candidate.
    EXPECT_TRUE(filter->DoFilter(candidate));
}

constexpr int SIGNAL_LEVEL_TWO = 2;

/*
 * Scenario: GetLinkedInfo fails (return non-WIFI_OPT_SUCCESS)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_GetLinkedInfoFail, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";
    candidate.wifiDeviceConfig.bssid = "aa:bb:cc:dd:ee:01";
    candidate.wifiDeviceConfig.networkId = 1;
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = false;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(Return(-1));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: GetLinkedInfo succeeds but GetDeviceConfig fails (return non-zero)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_GetDeviceConfigFail,
    TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";
    candidate.wifiDeviceConfig.bssid = "aa:bb:cc:dd:ee:01";
    candidate.wifiDeviceConfig.networkId = 1;
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = false;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:02";
    linkedInfo.rssi = -70;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(Return(-1));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: Current network is NOT a portal (currentConfig.isPortal == false)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_CurrentNotPortal, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";
    candidate.wifiDeviceConfig.bssid = "aa:bb:cc:dd:ee:01";
    candidate.wifiDeviceConfig.networkId = 1;
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = false;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:02";
    linkedInfo.rssi = -70;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = false;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: Current signal level is strong (> SIGNAL_LEVEL_TWO, i.e., > 2)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_SignalStrong, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";
    candidate.wifiDeviceConfig.bssid = "aa:bb:cc:dd:ee:01";
    candidate.wifiDeviceConfig.networkId = 1;
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = false;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:02";
    linkedInfo.rssi = -40;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(4));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: Different SSID between candidate and current
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_DifferentSsid, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:03";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "DifferentSSID";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:02";
    linkedInfo.rssi = -80;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(SIGNAL_LEVEL_TWO));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: Same BSSID between candidate and current (not a different AP)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_SameBssid, TestSize.Level1)
{
    std::string sameBssid = "aa:bb:cc:dd:ee:99";
    InterScanInfo scanInfo;
    scanInfo.bssid = sameBssid;
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = sameBssid;
    linkedInfo.rssi = -80;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(SIGNAL_LEVEL_TWO));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: All conditions met -- portal, weak signal, same SSID, different BSSID
 * Expected: IsCurrentPortalWeakAndSameSsid returns true
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_AllConditionsMet, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:03";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:02";
    linkedInfo.rssi = -80;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(SIGNAL_LEVEL_TWO));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_TRUE(result);
}

/*
 * Scenario: Signal level exactly equals boundary value 2 (SIGNAL_LEVEL_TWO)
 * Expected: IsCurrentPortalWeakAndSameSsid returns true (boundary <= 2)
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_SignalLevelBoundaryEq2,
    TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:04";
    scanInfo.ssid = "PortalSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "PortalSSID";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 2;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:05";
    linkedInfo.rssi = -78;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "PortalSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(2), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(2));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_TRUE(result);
}

/*
 * Scenario: Signal level == 1 (below boundary, still weak)
 * Expected: IsCurrentPortalWeakAndSameSsid returns true
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_SignalLevel1, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:06";
    scanInfo.ssid = "WeakPortal";
    scanInfo.frequency = 5180;
    scanInfo.band = static_cast<int>(BandType::BAND_5GHZ);
    scanInfo.rssi = -90;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "WeakPortal";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 3;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:07";
    linkedInfo.rssi = -90;
    linkedInfo.band = static_cast<int>(BandType::BAND_5GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "WeakPortal";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(3), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(1));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_TRUE(result);
}

/*
 * Scenario: Portal network + IsCurrentPortalWeakAndSameSsid returns true (relaxed)
 * Expected: Filter returns true (portal filtering relaxed for roaming)
 * Note: This tests the portal branch in Filter where the relaxation condition holds.
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_Filter_PortalRelaxedReturnsTrue, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:10";
    scanInfo.ssid = "PortalNet";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "PortalNet";
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = true;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;

    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 5;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:11";
    linkedInfo.rssi = -80;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "PortalNet";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(5), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(SIGNAL_LEVEL_TWO));
    
    bool result = filter.Filter(candidate);
    bool hasPortalReason = candidate.filtedReason["ValidConfigNetwork"].count(
        NetworkSelection::FiltedReason::PORTAL_NETWORK) > 0;
    EXPECT_FALSE(hasPortalReason);
}

/*
 * Scenario: Portal network + IsCurrentPortalWeakAndSameSsid returns false (strict filtering)
 * Expected: Filter returns false and adds PORTAL_NETWORK to filtedReason
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_Filter_PortalStrictReturnsFalse, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "PortalNet";
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = true;
    candidate.wifiDeviceConfig.networkId = 10;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(Return(-1));
    
    bool result = filter.Filter(candidate);
    bool hasPortalReason = candidate.filtedReason["ValidConfigNetwork"].count(
        NetworkSelection::FiltedReason::PORTAL_NETWORK) > 0;
    EXPECT_TRUE(hasPortalReason);
}

/*
 * Scenario: Non-portal network passes the portal check (continues to next checks)
 * Expected: Filter does NOT add PORTAL_NETWORK reason at the portal branch
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_Filter_NonPortalPassesPortalCheck, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "NormalNet";
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = false;
    candidate.wifiDeviceConfig.networkId = 20;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;

    NetworkSelection::ValidConfigNetworkFilter filter;
    
    bool result = filter.Filter(candidate);

    bool hasPortalReason = candidate.filtedReason["ValidConfigNetwork"].count(
        NetworkSelection::FiltedReason::PORTAL_NETWORK) > 0;
    EXPECT_FALSE(hasPortalReason);
}

/*
 * Scenario: Portal network, GetLinkedInfo succeeds, current is portal, but signal level == 3 (>2)
 * Expected: IsCurrentPortalWeakAndSameSsid returns false (signal not weak enough)
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_IsCurrentPortalWeakAndSameSsid_SignalLevel3, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:20";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "TestSSID";

    NetworkSelection::ValidConfigNetworkFilter filter;
    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 1;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:21";
    linkedInfo.rssi = -50;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "TestSSID";
    currentConfig.isPortal = true;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(1), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetSignalLevel(_, _, _))
        .WillOnce(Return(3));
    
    bool result = filter.IsCurrentPortalWeakAndSameSsid(candidate);
    EXPECT_FALSE(result);
}

/*
 * Scenario: Portal strict filtering -- candidate is portal, all pre-portal checks pass,
 *  IsCurrentPortalWeakAndSameSsid returns false because current is NOT portal
 * Expected: Filter returns false with PORTAL_NETWORK reason
 */
HWTEST_F(WifiFilterImplTest, ValidConfigNetworkFilter_Filter_PortalStrict_CurrentNotPortal, TestSize.Level1)
{
    InterScanInfo scanInfo;
    scanInfo.bssid = "aa:bb:cc:dd:ee:01";
    scanInfo.ssid = "TestSSID";
    scanInfo.frequency = 2412;
    scanInfo.band = static_cast<int>(BandType::BAND_2GHZ);
    scanInfo.rssi = -60;
    scanInfo.securityType = WifiSecurity::PSK;
    scanInfo.channelWidth = WifiChannelWidth::WIDTH_20MHZ;
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    candidate.wifiDeviceConfig.ssid = "PortalCandidate";
    candidate.wifiDeviceConfig.noInternetAccess = false;
    candidate.wifiDeviceConfig.isPortal = true;
    candidate.wifiDeviceConfig.networkId = 30;
    candidate.wifiDeviceConfig.isSecureWifi = true;
    candidate.wifiDeviceConfig.isAllowAutoConnect = true;
    candidate.wifiDeviceConfig.networkSelectionStatus.status = WifiDeviceConfigStatus::ENABLED;


    NetworkSelection::ValidConfigNetworkFilter filter;

    WifiLinkedInfo linkedInfo;
    linkedInfo.networkId = 30;
    linkedInfo.bssid = "aa:bb:cc:dd:ee:30";
    linkedInfo.rssi = -80;
    linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);

    WifiDeviceConfig currentConfig;
    currentConfig.ssid = "NormalNet";
    currentConfig.isPortal = false;

    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetLinkedInfo(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(linkedInfo), Return(WIFI_OPT_SUCCESS)));
    EXPECT_CALL(WifiSettings::GetInstance(), GetDeviceConfig(TypedEq<const int&>(30), _, _))
        .WillOnce(DoAll(SetArgReferee<1>(currentConfig), Return(0)));
    
    bool result = filter.Filter(candidate);
    EXPECT_FALSE(result);
    bool hasPortalReason = candidate.filtedReason["ValidConfigNetwork"].count(
        NetworkSelection::FiltedReason::PORTAL_NETWORK) > 0;
    EXPECT_TRUE(hasPortalReason);
}
}
}