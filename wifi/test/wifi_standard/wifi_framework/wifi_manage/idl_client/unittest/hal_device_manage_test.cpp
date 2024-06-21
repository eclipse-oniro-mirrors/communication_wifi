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
#ifdef HDI_CHIP_INTERFACE_SUPPORT
#include "hal_device_manage_test.h"

using namespace testing::ext;

namespace OHOS {
namespace Wifi {
void WifiHalDeviceManagerTest::DestoryCallback(std::string &destoryIfaceName, int createIfaceType)
{
    return;
}

HWTEST_F(WifiHalDeviceManagerTest, ScanTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    ScanParams scanParams;
    DelayedSingleton<HalDeviceManager>::GetInstance()->Scan(ifaceName, scanParams);
}

HWTEST_F(WifiHalDeviceManagerTest, StartPnoScanTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    PnoScanParams pnoScanParams;
    DelayedSingleton<HalDeviceManager>::GetInstance()->StartPnoScan(ifaceName, pnoScanParams);
}

HWTEST_F(WifiHalDeviceManagerTest, StopPnoScanTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->StopPnoScan(ifaceName);
}

HWTEST_F(WifiHalDeviceManagerTest, GetScanInfosTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    std::vector<ScanResultsInfo> scanResultsInfo;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetScanInfos(ifaceName, scanResultsInfo);
}

HWTEST_F(WifiHalDeviceManagerTest, GetConnectSignalInfoTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    SignalPollResult signalPollResult;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetConnectSignalInfo(ifaceName, signalPollResult);
}

HWTEST_F(WifiHalDeviceManagerTest, SetPmModeTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int mode = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetPmMode(ifaceName, mode);
}

HWTEST_F(WifiHalDeviceManagerTest, SetDpiMarkRuleTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int uid = 0;
    int protocol = 0;
    int enable = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetDpiMarkRule(ifaceName, uid, protocol, enable);
}

HWTEST_F(WifiHalDeviceManagerTest, SetStaMacAddressTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    std::string mac{"12:34:56:78:90:AB"};
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetStaMacAddress(ifaceName, mac);
}

HWTEST_F(WifiHalDeviceManagerTest, SetNetworkUpDownTest, TestSize.Level1)
{
    std::string ifaceName{"wlan0"};
    bool upDown = true;
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetNetworkUpDown(ifaceName, upDown);
}

HWTEST_F(WifiHalDeviceManagerTest, GetChipsetCategoryTest, TestSize.Level1)
{
    std::string ifaceName{"wlan0"};
    int chipsetCategory = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipsetCategory(ifaceName, chipsetCategory);
}

HWTEST_F(WifiHalDeviceManagerTest, GetChipsetWifiFeatrureCapabilityTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int chipsetFeatrureCapability = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipsetWifiFeatrureCapability(
        ifaceName, chipsetFeatrureCapability);
}

HWTEST_F(WifiHalDeviceManagerTest, GetFrequenciesByBandTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int32_t band = 0;
    std::vector<int> frequencies;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetFrequenciesByBand(ifaceName, band, frequencies);
}

HWTEST_F(WifiHalDeviceManagerTest, SetPowerModelTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int model = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetPowerModel(ifaceName, model);
}

HWTEST_F(WifiHalDeviceManagerTest, GetPowerModelTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    int model = 0;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetPowerModel(ifaceName, model);
}

HWTEST_F(WifiHalDeviceManagerTest, SetWifiCountryCodeTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    std::string code{"AB"};
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetWifiCountryCode(ifaceName, code);
}

HWTEST_F(WifiHalDeviceManagerTest, SetApMacAddressTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    std::string mac{"12:34:56:78:90:AB"};
    DelayedSingleton<HalDeviceManager>::GetInstance()->SetApMacAddress(ifaceName, mac);
}

HWTEST_F(WifiHalDeviceManagerTest, SelectInterfacesToDeleteTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    WifiChipInfo wifiChipInfo;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipInfo(0, wifiChipInfo);
    IfaceType ifaceType = IfaceType::STA;
    std::vector<WifiIfaceInfo> interfacesToBeRemovedFirst;
    DelayedSingleton<HalDeviceManager>::GetInstance()->SelectInterfacesToDelete(
        1, ifaceType, ifaceType, wifiChipInfo.ifaces[ifaceType], interfacesToBeRemovedFirst);
}

HWTEST_F(WifiHalDeviceManagerTest, CreateTheNeedChangeChipModeIfaceDataTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    UsableMode chipMode;
    IfaceCreationData ifaceCreationData;
    WifiChipInfo wifiChipInfo;
    WifiIfaceInfo wifiIfaceInfo;
    wifiChipInfo.ifaces[IfaceType::AP].push_back(wifiIfaceInfo);
    IfaceType createIfaceType = IfaceType::AP;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateTheNeedChangeChipModeIfaceData(
        wifiChipInfo, createIfaceType, chipMode, ifaceCreationData);
    
    IfaceType createIfaceType = IfaceType::STA;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateTheNeedChangeChipModeIfaceData(
        wifiChipInfo, createIfaceType, chipMode, ifaceCreationData);
}

HWTEST_F(WifiHalDeviceManagerTest, CompareIfaceCreationDataTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    WifiIfaceInfo ifaceInfo;
    IfaceCreationData data1;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipInfo(0, data1.chipInfo);
    data1.chipInfo.currentModeIdValid = false;
    data1.interfacesToBeRemovedFirst.push_back(ifaceInfo);
    IfaceCreationData data2;
    DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipInfo(0, data2.chipInfo);
    data2.chipInfo.currentModeIdValid = false;
    data2.interfacesToBeRemovedFirst.push_back(ifaceInfo);
    data2.interfacesToBeRemovedFirst.push_back(ifaceInfo);
    DelayedSingleton<HalDeviceManager>::GetInstance()->CompareIfaceCreationData(data1, data2);

    data1.chipInfo.currentModeIdValid = true;
    data1.chipInfo.currentModeId = 1;
    data1.chipModeId = 2;
    data2.chipInfo.currentModeIdValid = true;
    data2.chipInfo.currentModeId = 1;
    data2.chipModeId = 2;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CompareIfaceCreationData(data1, data2);
}

HWTEST_F(WifiHalDeviceManagerTest, DispatchIfaceDestoryCallbackTest, TestSize.Level1)
{
    IfaceType ifaceType = IfaceType::STA;
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->DispatchIfaceDestoryCallback(
        ifaceName, ifaceType, true, ifaceType);

    ifaceType = IfaceType::AP;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->DispatchIfaceDestoryCallback(
        ifaceName, ifaceType, true, ifaceType);

    ifaceType = IfaceType::P2P;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateP2pIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->DispatchIfaceDestoryCallback(
        ifaceName, ifaceType, true, ifaceType);
}

HWTEST_F(WifiHalDeviceManagerTest, RemoveStaIfaceTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveStaIface(ifaceName);
}

HWTEST_F(WifiHalDeviceManagerTest, RemoveApIfaceTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveApIface(ifaceName);
}

HWTEST_F(WifiHalDeviceManagerTest, RemoveP2pIfaceTest, TestSize.Level1)
{
    std::string ifaceName;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateP2pIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveP2pIface(ifaceName);
}

HWTEST_F(WifiHalDeviceManagerTest, CreateStaIfaceTest, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    HalDeviceManager::g_chipHdiServiceDied = true;
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateStaIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveStaIface(ifaceName);
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiHalDeviceManagerTest, CreateP2pIfaceTest, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateP2pIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveP2pIface(ifaceName);
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiHalDeviceManagerTest, CreateApIfaceTest, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    DelayedSingleton<HalDeviceManager>::GetInstance()->CreateApIface(
        std::bind(WifiHalDeviceManagerTest::DestoryCallback, std::placeholders::_1, std::placeholders::_2), ifaceName);
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveApIface(ifaceName);
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiHalDeviceManagerTest, StartPnoScanTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    PnoScanParams pnoScanParams;

    IChipIfaceTest *data = new IChipIfaceTest;
    sptr<IChipIface> iface = static_cast<IChipIface*>(data);
    DelayedSingleton<HalDeviceManager>::GetInstance()->mIWifiStaIfaces.insert(
        std::pair<std::string, sptr<IChipIface>>(ifaceName, iface));
    DelayedSingleton<HalDeviceManager>::GetInstance()->mIWifiApIfaces.insert(
        std::pair<std::string, sptr<IChipIface>>(ifaceName, iface));
    DelayedSingleton<HalDeviceManager>::GetInstance()->mIWifiP2pIfaces.insert(
        std::pair<std::string, sptr<IChipIface>>(ifaceName, iface));
    
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->StartPnoScan(ifaceName, pnoScanParams);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetPmModeTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int mode = 1;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetPmMode(ifaceName, mode);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, GetChipsetWifiFeatrureCapabilityTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int chipsetFeatrureCapability = 0;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->GetChipsetWifiFeatrureCapability(
        ifaceName, chipsetFeatrureCapability);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetTxPowerTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int model = 0;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetTxPower(ifaceName, model);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetDpiMarkRuleTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int uid = 0;
    int protocol = 0;
    int enable = 0;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetDpiMarkRule(ifaceName, uid, protocol, enable);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetStaMacAddressTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    std::string mac{"12:34:56:78:90:AB"};
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetStaMacAddress(ifaceName, mac);
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiHalDeviceManagerTest, GetPowerModelTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int model = 0;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->GetPowerModel(ifaceName, model);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, GetConnectSignalInfoTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    SignalPollResult signalPollResult;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->GetConnectSignalInfo(ifaceName, signalPollResult);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, GetFrequenciesByBandTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int32_t band = 0;
    std::vector<int> frequencies;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->GetFrequenciesByBand(ifaceName, band, frequencies);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetPowerModeTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    int model = 0;
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetPowerMode(ifaceName, model);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetWifiCountryCodeTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    std::string code{"AB"};
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetWifiCountryCode(ifaceName, code);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, GetIfaceTypeTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    IfaceType ifaceType;
    IChipIfaceTest *data = new IChipIfaceTest;
    sptr<IChipIface> iface = static_cast<IChipIface*>(data);

    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->GetIfaceType(iface, ifaceType);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, StopPnoScanTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->StopPnoScan(ifaceName);
    EXPECT_EQ(result, true);
}

HWTEST_F(WifiHalDeviceManagerTest, SetApMacAddressTest_01, TestSize.Level1)
{
    std::string ifaceName = "Wlan0";
    std::string mac{"12:34:56:78:90:AB"}

    IChipIfaceTest *data = new IChipIfaceTest;
    sptr<IChipIface> iface = static_cast<IChipIface*>(data);
    DelayedSingleton<HalDeviceManager>::GetInstance()->mIWifiApIfaces.insert(
        std::pair<std::string, sptr<IChipIface>>(ifaceName, iface));
    bool result = DelayedSingleton<HalDeviceManager>::GetInstance()->SetApMacAddress(ifaceName, mac);
    EXPECT_EQ(result, false);
}

}  // namespace Wifi
}  // namespace OHOS
#endif