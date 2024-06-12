/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include "wifi_hdi_wpa_client.h"
#include "wifi_hdi_wpa_callback.h"
#include "wifi_error_no.h"

using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
class WifiHdiWpaClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}

public:
    std::unique_ptr<WifiHdiWpaClient> wifiHdiWpaClient;
};

HWTEST_F(WifiHdiWpaClientTest, StartWifi, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->StartWifi("wlan");
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqConnect, TestSize.Level1)
{
    int networkId = 123;
    WifiErrorNo result = wifiHdiWpaClient->ReqConnect(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, StopWifi, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->StopWifi();
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, ReqReconnect, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqReconnect();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqReassociate, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqReassociate();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqDisconnect, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqDisconnect();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, GetStaCapabilities, TestSize.Level1)
{
    unsigned int capabilities = 0;
    WifiErrorNo result = wifiHdiWpaClient->GetStaCapabilities(capabilities);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
    EXPECT_EQ(capabilities, 0);
}

HWTEST_F(WifiHdiWpaClientTest, GetStaDeviceMacAddress, TestSize.Level1)
{
    std::string macAddress = "00:11:22:33:44:55";
    std::string result;
    WifiErrorNo error = wifiHdiWpaClient->GetStaDeviceMacAddress(result);
    EXPECT_EQ(error, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, QueryScanInfos, TestSize.Level1)
{
    int size = WIFI_IDL_GET_MAX_SCAN_INFO;
    ScanInfo *results = (ScanInfo *)malloc(sizeof(ScanInfo) * size);
    for (int i = 0; i < size; ++i) {
        results[i].freq = 2412;
        results[i].siglv = -50;
        results[i].timestamp = 1234567890;
        results[i].channelWidth = 20;
        results[i].centerFrequency0 = 2412;
        results[i].centerFrequency1 = 0;
        results[i].isVhtInfoExist = false;
        results[i].isHtInfoExist = false;
        results[i].isHeInfoExist = false;
        results[i].isErpExist = false;
        results[i].maxRates = 54;
        results[i].extMaxRates = 0;
        results[i].ieSize = 0;
        results[i].infoElems = NULL;
        results[i].isHiLinkNetwork = false;
    }
    std::vector<InterScanInfo> scanInfos;
    WifiErrorNo result = wifiHdiWpaClient->QueryScanInfos(scanInfos);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    free(results);
}

HWTEST_F(WifiHdiWpaClientTest, ReqStartPnoScan, TestSize.Level1)
{
    WifiPnoScanParam scanParam;
    WifiErrorNo result = wifiHdiWpaClient->ReqStartPnoScan(scanParam);
    EXPECT_EQ(result, WIFI_IDL_OPT_NOT_SUPPORT);
}

HWTEST_F(WifiHdiWpaClientTest, ReqStopPnoScan, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqStopPnoScan();
    EXPECT_EQ(result, WIFI_IDL_OPT_NOT_SUPPORT);
}

HWTEST_F(WifiHdiWpaClientTest, RemoveDevice, TestSize.Level1)
{
    int networkId = 123;
    WifiErrorNo result = wifiHdiWpaClient->RemoveDevice(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiHdiWpaClient->RemoveDevice(-1);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
}

HWTEST_F(WifiHdiWpaClientTest, GetNextNetworkId, TestSize.Level1)
{
    int networkId;
    WifiErrorNo result = wifiHdiWpaClient->GetNextNetworkId(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    EXPECT_EQ(networkId, 0);
}

HWTEST_F(WifiHdiWpaClientTest, ClearDeviceConfig, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ClearDeviceConfig();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqEnableNetwork, TestSize.Level1)
{
    int networkId = 123;
    WifiErrorNo result = wifiHdiWpaClient->ReqEnableNetwork(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqDisableNetwork, TestSize.Level1)
{
    int networkId = 123;
    WifiErrorNo result = wifiHdiWpaClient->ReqDisableNetwork(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, SetDeviceConfig, TestSize.Level1)
{
    int networkId = 123;
    WifiIdlDeviceConfig config;
    config.ssid = "TestSSID";
    config.psk = "TestPassword";
    config.keyMgmt = "WPA-PSK";
    config.priority = 1;

    SetNetworkConfig expectedConfig[DEVICE_CONFIG_END_POS];
    memcpy_s(expectedConfig, sizeof(expectedConfig), 0, sizeof(expectedConfig));
    WifiErrorNo result = wifiHdiWpaClient->SetDeviceConfig(networkId, config);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, SetBssid, TestSize.Level1)
{
    int networkId = 123;
    std::string bssid = "00:11:22:33:44:55";

    SetNetworkConfig expectedConfig;
    memset_s(&expectedConfig, sizeof(expectedConfig), 0, sizeof(expectedConfig));

    WifiErrorNo result = wifiHdiWpaClient->SetBssid(networkId, bssid);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, SaveDeviceConfig, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->SaveDeviceConfig();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqRegisterStaEventCallbackTEST, TestSize.Level1)
{
    WifiEventCallback *wifiEventCallbackMock = new WifiEventCallback();
    WifiErrorNo result = wifiHdiWpaClient->ReqRegisterStaEventCallback(*wifiEventCallbackMock);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    delete wifiEventCallbackMock;
}

HWTEST_F(WifiHdiWpaClientTest, ReqStartWpsPbcModeTEST, TestSize.Level1)
{
    WifiIdlWpsConfig config;
    config.anyFlag = true;
    config.multiAp = false;
    config.bssid = "00:11:22:33:44:55";
    WifiWpsParam expectedParam;
    memset_s(&expectedParam, sizeof(expectedParam), 0, sizeof(expectedParam));
    expectedParam.anyFlag = true;
    expectedParam.multiAp = false;
    strncpy_s(expectedParam.bssid, sizeof(expectedParam.bssid), config.bssid.c_str(), sizeof(expectedParam.bssid) - 1);
    WifiErrorNo result = wifiHdiWpaClient->ReqStartWpsPbcMode(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqStartWpsPinModeTEST, TestSize.Level1)
{
    WifiIdlWpsConfig config;
    config.anyFlag = true;
    config.multiAp = false;
    config.bssid = "00:11:22:33:44:55";
    config.pinCode = "12345678";
    WifiWpsParam expectedParam;
    memset_s(&expectedParam, sizeof(expectedParam), 0, sizeof(expectedParam));
    expectedParam.anyFlag = config.anyFlag;
    expectedParam.multiAp = config.multiAp;
    strncpy_s(expectedParam.bssid, sizeof(expectedParam.bssid), config.bssid.c_str(), sizeof(expectedParam.bssid) - 1);
    strncpy_s(expectedParam.pinCode, sizeof(expectedParam.pinCode), config.pinCode.c_str(),
        sizeof(expectedParam.pinCode) - 1);
    int pinCode;
    WifiErrorNo result = wifiHdiWpaClient->ReqStartWpsPinMode(config, pinCode);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqStopWpsTEST, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqStopWps();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqGetRoamingCapabilitiesTEST, TestSize.Level1)
{
    OHOS::Wifi::WifiIdlRoamCapability capability;
    WifiErrorNo result = wifiHdiWpaClient->ReqGetRoamingCapabilities(capability);
    EXPECT_EQ(result, WIFI_IDL_OPT_NOT_SUPPORT);
}

HWTEST_F(WifiHdiWpaClientTest, ReqSetRoamConfigTEST, TestSize.Level1)
{
    WifiIdlRoamConfig config;
    WifiErrorNo result = wifiHdiWpaClient->ReqSetRoamConfig(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_NOT_SUPPORT);
}

HWTEST_F(WifiHdiWpaClientTest, ReqGetConnectSignalInfoTEST, TestSize.Level1)
{
    std::string endBssid = "00:11:22:33:44:55";
    WifiWpaSignalInfo info;
    WifiErrorNo result = wifiHdiWpaClient->ReqGetConnectSignalInfo(endBssid, info);
    EXPECT_EQ(result, WIFI_IDL_OPT_NOT_SUPPORT);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaAutoConnectTEST, TestSize.Level1)
{
    int enable = 1;
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaAutoConnect(enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaBlocklistClearTEST, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaBlocklistClear();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqSetPowerSave, TestSize.Level1)
{
    bool enable = true;
    WifiErrorNo result = wifiHdiWpaClient->ReqSetPowerSave(enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaSetCountryCodeTEST, TestSize.Level1)
{
    std::string countryCode = "US";
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaSetCountryCode(countryCode);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaGetCountryCodeTEST, TestSize.Level1)
{
    std::string expectedCountryCode = "US";
    std::string countryCode;
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaGetCountryCode(countryCode);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaSetSuspendModeTEST, TestSize.Level1)
{
    bool mode = true;
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaSetSuspendMode(mode);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqWpaShellCmd, TestSize.Level1)
{
    std::string ifName = "wlan0";
    std::string cmd = "iw wlan0 scan";
    WifiErrorNo result = wifiHdiWpaClient->ReqWpaShellCmd(ifName, cmd);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, GetNetworkListTEST, TestSize.Level1)
{
    std::vector<WifiWpaNetworkInfo> networkList;
    WifiErrorNo result = wifiHdiWpaClient->GetNetworkList(networkList);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, GetDeviceConfigTEST, TestSize.Level1)
{
    WifiIdlGetDeviceConfig config;
    config.networkId = 123;
    config.param = "param";
    WifiErrorNo result = wifiHdiWpaClient->GetDeviceConfig(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, StartAp, TestSize.Level1)
{
    int id = 123;
    std::string ifaceName = "wlan0";
    WifiErrorNo result = wifiHdiWpaClient->StartAp(id, ifaceName);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, StopAp, TestSize.Level1)
{
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->StopAp(id);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, SetSoftApConfigTEST, TestSize.Level1)
{
    HotspotConfig config;
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->SetSoftApConfig(config, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, RegisterApEventTEST, TestSize.Level1)
{
    IWifiApMonitorEventCallback callback;
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->RegisterApEvent(callback, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
}

HWTEST_F(WifiHdiWpaClientTest, GetStationListTEST, TestSize.Level1)
{
    std::vector<std::string> result;
    WifiErrorNo res = wifiHdiWpaClient->GetStationList(result);
    EXPECT_EQ(res, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, AddBlockByMacTEST, TestSize.Level1)
{
    std::string mac = "00:11:22:33:44:55";
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->AddBlockByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    mac = "";
    wifiHdiWpaClient->AddBlockByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, DelBlockByMacTEST, TestSize.Level1)
{
    std::string mac = "00:11:22:33:44:55";
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->DelBlockByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    mac = "";
    wifiHdiWpaClient->DelBlockByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, RemoveStationTEST, TestSize.Level1)
{
    std::string mac = "00:11:22:33:44:55";
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->RemoveStation(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    mac = "";
    wifiHdiWpaClient->RemoveStation(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqDisconnectStaByMacTEST, TestSize.Level1)
{
    std::string mac = "00:11:22:33:44:55";
    int id = 123;
    WifiErrorNo result = wifiHdiWpaClient->ReqDisconnectStaByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    mac = "";
    wifiHdiWpaClient->ReqDisconnectStaByMac(mac, id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pStart, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pStart("wlan");
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pStop, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pStop();
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetSsidPostfixName, TestSize.Level1)
{
    std::string postfixName = "postfix";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetSsidPostfixName(postfixName);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetDeviceName, TestSize.Level1)
{
    std::string name = "DeviceName";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetDeviceName(name);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWpsDeviceType, TestSize.Level1)
{
    std::string type = "type";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWpsDeviceType(type);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWpsSecondaryDeviceType, TestSize.Level1)
{
    std::string type = "WPS_TYPE";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWpsSecondaryDeviceType(type);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWpsConfigMethods, TestSize.Level1)
{
    std::string config = "12345678";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWpsConfigMethods(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pGetDeviceAddress, TestSize.Level1)
{
    std::string result;
    WifiErrorNo ret = wifiHdiWpaClient->ReqP2pGetDeviceAddress(result);
    EXPECT_EQ(ret, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pFlush, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pFlush();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pFlushService, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pFlushService();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSaveConfig, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSaveConfig();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pRegisterCallback, TestSize.Level1)
{
    P2pHalCallback callbacks;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pRegisterCallback(callbacks);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetupWpsPbc, TestSize.Level1)
{
    std::string groupInterface = "p2p0";
    std::string bssid = "00:11:22:33:44:55";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetupWpsPbc(groupInterface, bssid);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetupWpsPinTest, TestSize.Level1)
{
    std::string groupInterface = "wlan0";
    std::string address = "00:11:22:33:44:55";
    std::string pin = "12345678";
    std::string result;
    WifiErrorNo ret = wifiHdiWpaClient->ReqP2pSetupWpsPin(groupInterface, address, pin, result);
    EXPECT_EQ(ret, WIFI_IDL_OPT_FAILED);
    pin = "1234";
    ret = wifiHdiWpaClient->ReqP2pSetupWpsPin(groupInterface, address, pin, result);
    EXPECT_EQ(ret, WIFI_IDL_OPT_INVALID_PARAM);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pRemoveNetwork, TestSize.Level1)
{
    int networkId = 123;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pRemoveNetwork(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pListNetworks, TestSize.Level1)
{
    std::map<int, WifiP2pGroupInfo> mapGroups;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pListNetworks(mapGroups);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pListNetworks_Error, TestSize.Level1)
{
    std::map<int, WifiP2pGroupInfo> mapGroups;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pListNetworks(mapGroups);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    EXPECT_TRUE(mapGroups.empty());
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetGroupMaxIdle, TestSize.Level1)
{
    std::string groupInterface = "p2p0";
    size_t time = 300;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetGroupMaxIdle(groupInterface, time);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetPowerSave_Enable, TestSize.Level1)
{
    std::string groupInterface = "p2p0";
    bool enable = true;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetPowerSave(groupInterface, enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetPowerSave_Disable, TestSize.Level1)
{
    std::string groupInterface = "p2p0";
    bool enable = false;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetPowerSave(groupInterface, enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWfdEnable_Enable, TestSize.Level1)
{
    bool enable = true;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWfdEnable(enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWfdEnable_Disable, TestSize.Level1)
{
    bool enable = false;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWfdEnable(enable);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetWfdDeviceConfig, TestSize.Level1)
{
    std::string config = "config";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetWfdDeviceConfig(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pStartFind, TestSize.Level1)
{
    size_t timeout = 5000;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pStartFind(timeout);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pStopFind, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pStopFind();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetExtListen, TestSize.Level1)
{
    bool enable = true;
    size_t period = 100;
    size_t interval = 200;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    period = 0;
    result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    period = 65536;
    result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    interval = 0;
    result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    interval = 65536;
    result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    period = 1;
    interval = 0;
    result = wifiHdiWpaClient->ReqP2pSetExtListen(enable, period, interval);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetListenChannel, TestSize.Level1)
{
    size_t channel = 6;
    unsigned char regClass = 81;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetListenChannel(channel, regClass);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pConnect_WithValidParams_ReturnsOptOk, TestSize.Level1)
{
    std::string temp = "";
    WifiP2pConfigInternal config;
    config.SetDeviceAddress("00:11:22:33:44:55");
    config.SetGroupOwnerIntent(7);
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pConnect(config, false, temp);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pCancelConnect, TestSize.Level1)
{
    WifiHdiWpaClient wifiClient;
    WifiErrorNo result = wifiClient.ReqP2pCancelConnect();
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pProvisionDiscovery, TestSize.Level1)
{
    WifiP2pConfigInternal config;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pProvisionDiscovery(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pAddGroupTest, TestSize.Level1)
{
    int id = 1;
    int fre = 15;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pAddGroup(true, id, fre);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pRemoveGroupTest, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pRemoveGroup("p2p0");
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pRemoveGroupNonExistentTest, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pRemoveGroup("p2p1");
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pInviteTest, TestSize.Level1)
{
    WifiP2pGroupInfo groupInfo;
    std::string deviceAddress = "00:11:22:33:44:55";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pInvite(groupInfo, deviceAddress);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReinvokeTest01, TestSize.Level1)
{
    int networkId = 1;
    std::string deviceAddr = "00:11:22:33:44:55";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReinvoke(networkId, deviceAddr);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReinvokeTest02, TestSize.Level1)
{
    int networkId = -1;
    std::string deviceAddr = "00:11:22:33:44:55";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReinvoke(networkId, deviceAddr);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReinvokeTest03, TestSize.Level1)
{
    int networkId = 1;
    std::string deviceAddr = "00:11:22:33:44";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReinvoke(networkId, deviceAddr);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pGetGroupCapabilityTest01, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    uint32_t capability;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pGetGroupCapability(deviceAddress, capability);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pGetGroupCapabilityTest02, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44";
    uint32_t capability;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pGetGroupCapability(deviceAddress, capability);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pAddServiceTest, TestSize.Level1)
{
    WifiP2pServiceInfo serviceInfo;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pAddService(serviceInfo);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pRemoveServiceTest, TestSize.Level1)
{
    WifiP2pServiceInfo info;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pRemoveService(info);
    EXPECT_EQ(result, WIFI_IDL_OPT_OK);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReqServiceDiscoveryTest01, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    std::vector<unsigned char> tlvs = { 0x01, 0x02, 0x03 };
    std::string reqID;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReqServiceDiscovery(deviceAddress, tlvs, reqID);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    EXPECT_TRUE(reqID.empty());
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReqServiceDiscoveryTest02, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44";
    std::vector<unsigned char> tlvs = { 0x01, 0x02, 0x03 };
    std::string reqID;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReqServiceDiscovery(deviceAddress, tlvs, reqID);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    EXPECT_TRUE(reqID.empty());
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pReqServiceDiscoveryTest03, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    std::vector<unsigned char> tlvs;
    std::string reqID;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pReqServiceDiscovery(deviceAddress, tlvs, reqID);
    EXPECT_EQ(result, WIFI_IDL_OPT_INVALID_PARAM);
    EXPECT_TRUE(reqID.empty());
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pCancelServiceDiscoveryTest, TestSize.Level1)
{
    std::string id = "12345";
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pCancelServiceDiscovery(id);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiHdiWpaClient->ReqP2pCancelServiceDiscovery("");
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetRandomMacTest, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetRandomMac(true);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiHdiWpaClient->ReqP2pSetRandomMac(false);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiHdiWpaClient->ReqP2pSetRandomMac(2);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetMiracastTypeTest, TestSize.Level1)
{
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetMiracastType(1);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqSetPersistentReconnectTest, TestSize.Level1)
{
    WifiHdiWpaClient wifiClient;
    WifiErrorNo result = wifiClient.ReqSetPersistentReconnect(1);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiClient.ReqSetPersistentReconnect(0);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    result = wifiClient.ReqSetPersistentReconnect(2);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqRespServiceDiscoveryTest, TestSize.Level1)
{
    WifiP2pDevice device;
    int frequency = 2412;
    int dialogToken = 1;
    std::vector<unsigned char> tlvs = { 0x01, 0x02, 0x03 };
    WifiErrorNo result = wifiHdiWpaClient->ReqRespServiceDiscovery(device, frequency, dialogToken, tlvs);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    tlvs.empty();
    result = wifiHdiWpaClient->ReqRespServiceDiscovery(device, frequency, dialogToken, tlvs);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqSetServiceDiscoveryExternalTest, TestSize.Level1)
{
    WifiHdiWpaClient client;
    WifiErrorNo result = client.ReqSetServiceDiscoveryExternal(true);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqGetP2pPeerTest01, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44:55";
    WifiP2pDevice device;
    WifiErrorNo result = wifiHdiWpaClient->ReqGetP2pPeer(deviceAddress, device);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqGetP2pPeerTest02, TestSize.Level1)
{
    std::string deviceAddress = "00:11:22:33:44";
    WifiP2pDevice device;
    WifiErrorNo result = wifiHdiWpaClient->ReqGetP2pPeer(deviceAddress, device);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pGetSupportFrequenciesTest, TestSize.Level1)
{
    std::vector<int> frequencies;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pGetSupportFrequencies(1, frequencies);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    EXPECT_TRUE(frequencies.empty());
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pSetGroupConfigTest, TestSize.Level1)
{
    int networkId = 1;
    IdlP2pGroupConfig config;
    config.ssid = "TestSSID";
    config.bssid = "00:11:22:33:44:55";
    config.psk = "TestPassword";
    config.proto = "WPA2";
    config.keyMgmt = "WPA-PSK";
    config.pairwise = "CCMP";
    config.authAlg = "OPEN";
    config.mode = 1;
    config.disabled = 0;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pSetGroupConfig(networkId, config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, PushP2pGroupConfigStringTest, TestSize.Level1)
{
    P2pGroupConfig config;
    std::string str = "test";
    int result = wifiHdiWpaClient->PushP2pGroupConfigString(&config, GROUP_CONFIG_SSID, str);
    EXPECT_EQ(result, 1);
    str = "";
    result = wifiHdiWpaClient->PushP2pGroupConfigString(&config, GROUP_CONFIG_SSID, str);
    EXPECT_EQ(result, 0);
}

HWTEST_F(WifiHdiWpaClientTest, PushP2pGroupConfigIntTest, TestSize.Level1)
{
    P2pGroupConfig pConfig;
    int expectedValue = 123;
    int result = wifiHdiWpaClient->PushP2pGroupConfigInt(&pConfig, GROUP_CONFIG_MODE, expectedValue);
    EXPECT_EQ(result, 1);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pGetGroupConfigTest, TestSize.Level1)
{
    int networkId = 1;
    IdlP2pGroupConfig config;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pGetGroupConfig(networkId, config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pAddNetworkTest, TestSize.Level1)
{
    int networkId = -1;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pAddNetwork(networkId);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
    EXPECT_EQ(networkId, -1);
}

HWTEST_F(WifiHdiWpaClientTest, ReqP2pHid2dConnect_Success, TestSize.Level1)
{
    Hid2dConnectConfig config;
    WifiErrorNo result = wifiHdiWpaClient->ReqP2pHid2dConnect(config);
    EXPECT_EQ(result, WIFI_IDL_OPT_FAILED);
}
} // namespace Wifi
} // namespace OHOS