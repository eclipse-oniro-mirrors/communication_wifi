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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ap_service.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"
#include "mock_pendant.h"
#include "operator_overload.h"
#include "mock_wifi_ap_hal_interface.h"
#include "i_ap_service_callbacks.h"
#include "wifi_logger.h"

using namespace OHOS;
using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

DEFINE_WIFILOG_LABEL("ApServiceTest");

namespace OHOS {
namespace Wifi {
const StationInfo staInfo = {
    "test_deviceName",
    "AA:BB:CC:DD:EE:FF",
    1,
    "127.0.0.1",
};

constexpr int FREQUENCY2G = 2412;

class ApService_test : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        const int SLEEP_TIME = 20;
        pMockPendant = new MockPendant();
        pApService = new ApService(pMockPendant->GetMockApStateMachine(),
            pMockPendant->GetMockApStartedState());
        int testMaxConn = 10;
        int channel = 6;
        apInfo.SetSsid(std::string("TEST"));
        apInfo.SetPreSharedKey(std::string("123456789"));
        apInfo.SetSecurityType(KeyMgmt::WPA2_PSK);
        apInfo.SetBand(BandType::BAND_2GHZ);
        apInfo.SetChannel(channel);
        apInfo.SetMaxConn(testMaxConn);
        usleep(SLEEP_TIME);
    }
    virtual void TearDown()
    {
        delete pApService;
        pApService = nullptr;
        delete pMockPendant;
        pMockPendant = nullptr;
    }

public:
    ErrCode WarpRegisterApServiceCallbacks(const IApServiceCallbacks &callbacks)
    {
        return pApService->RegisterApServiceCallbacks(callbacks);
    }

public:
    MockPendant *pMockPendant;
    ApService *pApService;
    HotspotConfig apInfo;
};
/* EnableHotspot */
HWTEST_F(ApService_test, EnableHotspot_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->EnableHotspot());
}
/* DisableHotspot */
HWTEST_F(ApService_test, DisableHotspotSUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    pApService->EnableHotspot();
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->DisableHotspot());
}
/* SetHotspotConfig */
HWTEST_F(ApService_test, SetHotspotConfig_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->SetHotspotConfig(apInfo));
}
/* AddBlockList */
HWTEST_F(ApService_test, AddBlockList_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->AddBlockList(staInfo));
}
/* DelBlockList */
HWTEST_F(ApService_test, DelBlockList_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->DelBlockList(staInfo));
}
/* DisconnetStation */
HWTEST_F(ApService_test, DisconnetStation_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->DisconnetStation(staInfo));
}

/* GetValidBands */
HWTEST_F(ApService_test, GetValidBands_SUCCESS, TestSize.Level1)
{
    std::vector<BandType> vecSta;

    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidBands(vecSta));
}
HWTEST_F(ApService_test, GetValidBands_FAILED, TestSize.Level1)
{
    std::vector<BandType> vecSta;

    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidBands(vecSta));
}

/* GetValidChannels */
HWTEST_F(ApService_test, GetValidChannels_SUCCESS, TestSize.Level1)
{
    std::vector<int32_t> vecChannels;
    std::vector<int32_t> band_2G_channel = { 1, 2, 3, 4, 5, 6, 7 };
    std::vector<int32_t> band_5G_channel = { 149, 168, 169 };
    ChannelsTable temp = { { BandType::BAND_2GHZ, band_2G_channel }, { BandType::BAND_5GHZ, band_5G_channel } };
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidChannels(BandType::BAND_2GHZ, vecChannels));
    vecChannels.clear();

    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidChannels(BandType::BAND_5GHZ, vecChannels));
    vecChannels.clear();

    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidChannels(BandType::BAND_5GHZ, vecChannels));
}

/* RegisterApServiceCallbacks */
HWTEST_F(ApService_test, RegisterApServiceCallbacks_SUCCESS, TestSize.Level1)
{
    EXPECT_CALL(WifiApHalInterface::GetInstance(), RegisterApEvent(_, 0))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    IApServiceCallbacks callbacks;
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, WarpRegisterApServiceCallbacks(callbacks));
}

HWTEST_F(ApService_test, GetStationListSuccess1, TestSize.Level1)
{
    std::vector<StationInfo> result;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetStationList(_, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetStationList(result));
}

HWTEST_F(ApService_test, GetStationListFailed, TestSize.Level1)
{
    std::vector<StationInfo> result;
    StationInfo info;
    info.deviceName = "TV";
    info.bssid = "AA:BB:CC:DD:EE:FF",
    info.ipAddr = "127.0.0.1",
    result.push_back(info);
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetStationList(_, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetStationList(result));
}

HWTEST_F(ApService_test, GetValidBandsSuccess, TestSize.Level1)
{
    std::vector<BandType> vecSta;
    std::vector<int> allowed2GFreq;
    allowed2GFreq.push_back(FREQUENCY2G);
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetApIfaceName()).WillRepeatedly(Return("wifitest"));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, Eq(allowed2GFreq)))
        .WillOnce(DoAll(SetArgReferee<2>(allowed2GFreq), Return(WifiErrorNo::WIFI_HAL_OPT_OK)));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidBands(vecSta));
}

HWTEST_F(ApService_test, GetValidBandsFailed, TestSize.Level1)
{
    std::vector<BandType> vecSta;
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetApIfaceName()).WillRepeatedly(Return("wifitest"));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidBands(vecSta));
}

HWTEST_F(ApService_test, GetValidChannelsSuccess2, TestSize.Level1)
{
    std::vector<int32_t> vecChannels;
    std::vector<int32_t> allowed2GFreq = { 2412, 2417, 2422, 2427, 2432, 2437, 2442 };
    std::vector<int32_t> allowed5GFreq = { 5745, 5765, 5785 };
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetApIfaceName()).WillRepeatedly(Return("wifitest"));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, Eq(allowed2GFreq)))
        .WillOnce(DoAll(SetArgReferee<2>(allowed2GFreq), Return(WifiErrorNo::WIFI_HAL_OPT_OK)));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(allowed5GFreq), Return(WifiErrorNo::WIFI_HAL_OPT_OK)));
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetValidChannels(BandType::BAND_2GHZ, vecChannels));
}

HWTEST_F(ApService_test, GetValidChannelsFail2, TestSize.Level1)
{
    std::vector<int32_t> vecChannels;
    std::vector<int32_t> allowed2GFreq = { 2412, 2417, 2422, 2427, 2432, 2437, 2442 };
    std::vector<int32_t> allowed5GFreq = { 5745, 5765, 5785 };
    EXPECT_CALL(WifiConfigCenter::GetInstance(), GetApIfaceName()).WillRepeatedly(Return("wifitest"));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, Eq(allowed2GFreq)))
        .WillOnce(DoAll(SetArgReferee<2>(allowed2GFreq), Return(WifiErrorNo::WIFI_HAL_OPT_OK)));
    EXPECT_CALL(WifiApHalInterface::GetInstance(), GetFrequenciesByBand(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(allowed5GFreq), Return(WifiErrorNo::WIFI_HAL_OPT_OK)));
    EXPECT_EQ(ErrCode::WIFI_OPT_INVALID_PARAM, pApService->GetValidChannels(BandType::BAND_ANY, vecChannels));
}

HWTEST_F(ApService_test, GetSupportedPowerModelSuccess, TestSize.Level1)
{
    std::set<PowerModel> setPowerModelList;
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->GetSupportedPowerModel(setPowerModelList));
}

HWTEST_F(ApService_test, SetPowerModelSuccess, TestSize.Level1)
{
    PowerModel model = PowerModel::SLEEPING;
    EXPECT_CALL(WifiApHalInterface::GetInstance(), SetPowerModel(_, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_OK));
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetPowerModel(_, _)).WillOnce(Return(ErrCode::WIFI_OPT_SUCCESS));
    pApService->SetPowerModel(model);
}

HWTEST_F(ApService_test, SetPowerModelFailed, TestSize.Level1)
{
    PowerModel model = PowerModel::SLEEPING;
    EXPECT_CALL(WifiApHalInterface::GetInstance(), SetPowerModel(_, _))
        .WillRepeatedly(Return(WifiErrorNo::WIFI_HAL_OPT_FAILED));
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetPowerModel(_, _)).WillRepeatedly(Return(ErrCode::WIFI_OPT_SUCCESS));
    EXPECT_EQ(ErrCode::WIFI_OPT_FAILED, pApService->SetPowerModel(model));
}

HWTEST_F(ApService_test, OnWifiCountryCodeChangedSuccess, TestSize.Level1)
{
    WIFI_LOGI("OnWifiCountryCodeChangedSuccess enter");
    std::string countryCode = "CN";
    pApService->EnableHotspot();
    EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pApService->m_apObserver->OnWifiCountryCodeChanged(countryCode));
}

HWTEST_F(ApService_test, GetPowerModelTest, TestSize.Level1)
{
    WIFI_LOGI("GetPowerModelTest enter");
    HotspotMode mode = HotspotMode::NONE;
    pApService->SetHotspotMode(HotspotMode::SOFTAP);
    pApService->GetHotspotMode(mode);
    EXPECT_EQ(mode, HotspotMode::SOFTAP);
}

HWTEST_F(ApService_test, SetHotspotModeTest, TestSize.Level1)
{
    WIFI_LOGI("SetHotspotModeTest enter");
    HotspotMode mode = HotspotMode::NONE;
    pApService->SetHotspotMode(HotspotMode::RPT);
    pApService->GetHotspotMode(mode);
    EXPECT_EQ(mode, HotspotMode::RPT);
}
} // namespace Wifi
} // namespace OHOS