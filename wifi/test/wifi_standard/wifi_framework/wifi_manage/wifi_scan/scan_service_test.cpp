/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "scan_service.h"
#include <gtest/gtest.h>
#include "mock_wifi_manager.h"
#include "mock_wifi_config_center.h"
#include "mock_scan_state_machine.h"
#include <log.h>

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
constexpr int FREQ_2_DOT_4_GHZ = 2450;
constexpr int FREQ_5_GHZ = 5200;
constexpr int TWO = 2;
constexpr int FOUR = 4;
constexpr int FAILEDNUM = 6;
constexpr int STANDER = 5;
constexpr int STATUS = 17;
constexpr int MAX_SCAN_CONFIG = 10000;
constexpr int INVAL = 0x0fffff;
constexpr int MAX_THROUGH = -90;
constexpr int TIMES_TAMP = 1000;
static std::string g_errLog;
void ScanServiceCallback(const LogType type, const LogLevel level,
                         const unsigned int domain, const char *tag,
                         const char *msg)
{
    g_errLog = msg;
}
class ScanServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        LOG_SetCallback(ScanServiceCallback);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetHid2dUpperScene(_, _)).Times(AtLeast(0));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetP2pBusinessType(_)).Times(AtLeast(0));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return(""));
        pScanService = std::make_unique<ScanService>();
        pScanService->pScanStateMachine = new MockScanStateMachine();
        pScanService->RegisterScanCallbacks(WifiManager::GetInstance().GetScanCallback());
    }
    virtual void TearDown()
    {
        pScanService.reset();
    }

public:
    std::unique_ptr<ScanService> pScanService;
    std::vector<TrustListPolicy> refVecTrustList;
    MovingFreezePolicy defaultValue;

    void InitScanServiceSuccess1()
    {
        std::vector<int32_t> band_2G_channel = { 1, 2, 3, 4, 5, 6, 7 };
        std::vector<int32_t> band_5G_channel = { 149, 168, 169 };
        ChannelsTable temp = { { BandType::BAND_2GHZ, band_2G_channel }, { BandType::BAND_5GHZ, band_5G_channel } };
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScanControlInfo(_, _)).Times(AtLeast(1));
        EXPECT_CALL(WifiManager::GetInstance(), DealScanOpenRes(_)).Times(AtLeast(0));
        pScanService->InitScanService(WifiManager::GetInstance().GetScanCallback());
    }

    void InitScanServiceSuccess2()
    {
        std::vector<int32_t> band_2G_channel = { 1, 2, 3, 4, 5, 6, 7 };
        std::vector<int32_t> band_5G_channel = { 149, 168, 169 };
        ChannelsTable temp = { { BandType::BAND_2GHZ, band_2G_channel }, { BandType::BAND_5GHZ, band_5G_channel } };
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScanControlInfo(_, _)).Times(AtLeast(1));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScreenState()).Times(AtLeast(1));
        EXPECT_CALL(WifiManager::GetInstance(), DealScanOpenRes(_)).Times(AtLeast(0));
        pScanService->InitScanService(WifiManager::GetInstance().GetScanCallback());
    }

    void UnInitScanServiceSuccess()
    {
        if (pScanService->InitScanService(WifiManager::GetInstance().GetScanCallback()) == true) {
            pScanService->UnInitScanService();
        }
    }

    void HandleScanStatusReportSuccess1()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanOpenRes(_)).Times(AtLeast(1));
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = SCAN_STARTED_STATUS;
        pScanService->HandleScanStatusReport(scanStatusReport);
        pScanService->pScanStateMachine = nullptr;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportSuccess2()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanCloseRes(_)).Times(AtLeast(1));
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = SCAN_FINISHED_STATUS;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportSuccess3()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _)).Times(AtLeast(0));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).Times(AtLeast(0));
        EXPECT_CALL(WifiManager::GetInstance(), DealScanInfoNotify(_, _)).Times(AtLeast(1));
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = COMMON_SCAN_SUCCESS;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportSuccess4()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _)).Times(AtLeast(0));
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = COMMON_SCAN_FAILED;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportSuccess5()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanInfoNotify(_, _)).Times(AtLeast(1));
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = PNO_SCAN_INFO;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportSuccess6()
    {
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = PNO_SCAN_FAILED;
        pScanService->HandleScanStatusReport(scanStatusReport);
        pScanService->pScanStateMachine = nullptr;
        pScanService->HandleScanStatusReport(scanStatusReport);
        EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
    }

    void HandleScanStatusReportSuccess7()
    {
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = SCAN_INNER_EVENT;
        pScanService->HandleScanStatusReport(scanStatusReport);
    }

    void HandleScanStatusReportFail()
    {
        ScanStatusReport scanStatusReport;
        scanStatusReport.status = SCAN_STATUS_INVALID;
        pScanService->HandleScanStatusReport(scanStatusReport);
        EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
    }

    void HandleInnerEventReportSuccess1()
    {
        ScanInnerEventType innerEvent;
        innerEvent = SYSTEM_SCAN_TIMER;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleInnerEventReport(innerEvent);
    }

    void HandleInnerEventReportSuccess2()
    {
        ScanInnerEventType innerEvent;
        innerEvent = DISCONNECTED_SCAN_TIMER;
        pScanService->HandleInnerEventReport(innerEvent);
    }

    void HandleInnerEventReportSuccess3()
    {
        ScanInnerEventType innerEvent;
        innerEvent = RESTART_PNO_SCAN_TIMER;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleInnerEventReport(innerEvent);
    }

    void HandleInnerEventReportFail()
    {
        ScanInnerEventType innerEvent;
        innerEvent = SCAN_INNER_EVENT_INVALID;
        pScanService->HandleInnerEventReport(innerEvent);
    }

    void ScanSuccess()
    {
        pScanService->scanStartedFlag = false;
        EXPECT_TRUE(pScanService->Scan(ScanType::SCAN_TYPE_NATIVE_EXTERN) == WIFI_OPT_FAILED);
    }

    void ScanFail()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetThermalLevel()).WillRepeatedly(Return(FOUR));
        pScanService->scanStartedFlag = true;
        EXPECT_TRUE(pScanService->Scan(ScanType::SCAN_TYPE_EXTERN) == WIFI_OPT_SUCCESS);
        EXPECT_TRUE(pScanService->Scan(ScanType::SCAN_TYPE_NATIVE_EXTERN) == WIFI_OPT_SUCCESS);
        pScanService->pScanStateMachine = nullptr;
        EXPECT_TRUE(pScanService->Scan(ScanType::SCAN_TYPE_EXTERN) == WIFI_OPT_FAILED);
    }

    void ScanWithParamSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetThermalLevel()).WillRepeatedly(Return(1));
        pScanService->scanStartedFlag = true;
        WifiScanParams params;
        params.band = SCAN_BAND_BOTH_WITH_DFS;
        EXPECT_EQ(WIFI_OPT_SUCCESS, pScanService->ScanWithParam(params, ScanType::SCAN_TYPE_NATIVE_EXTERN));
    }

    void ScanWithParamFail1()
    {
        pScanService->scanStartedFlag = false;
        WifiScanParams params;
        params.band = SCAN_BAND_BOTH_WITH_DFS;
        EXPECT_EQ(WIFI_OPT_FAILED, pScanService->ScanWithParam(params, ScanType::SCAN_TYPE_NATIVE_EXTERN));
    }

    void ScanWithParamFail2()
    {
        pScanService->scanStartedFlag = true;
        WifiScanParams params;
        params.band = -1;
        EXPECT_EQ(WIFI_OPT_FAILED, pScanService->ScanWithParam(params, ScanType::SCAN_TYPE_NATIVE_EXTERN));
    }

    void ScanWithParamFail3()
    {
        pScanService->scanStartedFlag = true;
        WifiScanParams params;
        params.band = SCAN_BAND_UNSPECIFIED;
        EXPECT_EQ(WIFI_OPT_FAILED, pScanService->ScanWithParam(params, ScanType::SCAN_TYPE_NATIVE_EXTERN));
    }

    void ScanWithParamFail4()
    {
        pScanService->scanStartedFlag = true;
        WifiScanParams params;
        params.band = SCAN_BAND_UNSPECIFIED;
        EXPECT_EQ(WIFI_OPT_FAILED, pScanService->ScanWithParam(params, ScanType::SCAN_TYPE_NATIVE_EXTERN));
    }

    void SingleScanSuccess1()
    {
        ScanConfig scanConfig;
        scanConfig.scanBand = SCAN_BAND_BOTH_WITH_DFS;
        scanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        scanConfig.scanStyle = SCAN_TYPE_HIGH_ACCURACY;
        EXPECT_EQ(true, pScanService->SingleScan(scanConfig));
    }

    void SingleScanSuccess2()
    {
        ScanConfig scanConfig;
        scanConfig.scanBand = SCAN_BAND_UNSPECIFIED;
        scanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        scanConfig.scanStyle = SCAN_TYPE_HIGH_ACCURACY;
        scanConfig.scanFreqs.push_back(0);
        EXPECT_EQ(true, pScanService->SingleScan(scanConfig));
    }

    void SingleScanSuccess3()
    {
        ScanConfig scanConfig;
        scanConfig.scanBand = SCAN_BAND_24_GHZ;
        scanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        scanConfig.scanStyle = SCAN_TYPE_HIGH_ACCURACY;
        EXPECT_EQ(true, pScanService->SingleScan(scanConfig));
    }

    void SingleScanFail1()
    {
        ScanConfig scanConfig;
        scanConfig.scanBand = SCAN_BAND_UNSPECIFIED;
        scanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        scanConfig.scanStyle = SCAN_TYPE_HIGH_ACCURACY;
        EXPECT_EQ(false, pScanService->SingleScan(scanConfig));
    }

    void SingleScanFail2()
    {
        ScanConfig scanConfig;
        scanConfig.scanBand = static_cast<ScanBandType>(-1);
        scanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        scanConfig.scanStyle = SCAN_TYPE_HIGH_ACCURACY;
        EXPECT_EQ(false, pScanService->SingleScan(scanConfig));
    }

    void GetBandFreqsSuccess1()
    {
        ScanBandType band = SCAN_BAND_24_GHZ;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsSuccess2()
    {
        ScanBandType band = SCAN_BAND_5_GHZ;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsSuccess3()
    {
        ScanBandType band = SCAN_BAND_BOTH;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsSuccess4()
    {
        ScanBandType band = SCAN_BAND_5_GHZ_DFS_ONLY;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsSuccess5()
    {
        ScanBandType band = SCAN_BAND_5_GHZ_WITH_DFS;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsSuccess6()
    {
        ScanBandType band = SCAN_BAND_BOTH_WITH_DFS;
        std::vector<int> freqs;
        EXPECT_EQ(true, pScanService->GetBandFreqs(band, freqs));
    }

    void GetBandFreqsFail()
    {
        ScanBandType band = SCAN_BAND_UNSPECIFIED;
        std::vector<int> freqs;
        EXPECT_EQ(false, pScanService->GetBandFreqs(band, freqs));
    }

    void AddScanMessageBodySuccess()
    {
        InternalMessagePtr msg = std::make_shared<InternalMessage>();
        InterScanConfig interConfig;
        interConfig.hiddenNetworkSsid.push_back("hmwifi");
        interConfig.scanFreqs.push_back(FREQ_2_DOT_4_GHZ);
        EXPECT_EQ(true, pScanService->AddScanMessageBody(msg, interConfig));
    }

    void AddScanMessageBodyFail()
    {
        InterScanConfig interConfig;
        EXPECT_EQ(false, pScanService->AddScanMessageBody(nullptr, interConfig));
    }

    void StoreRequestScanConfigSuccess()
    {
        ScanConfig scanConfig;
        InterScanConfig interConfig;
        pScanService->StoreRequestScanConfig(scanConfig, interConfig);
    }

    void StoreRequestScanConfigFail()
    {
        ScanConfig scanConfig;
        InterScanConfig interConfig;
        pScanService->scanConfigMap.clear();
        pScanService->scanConfigStoreIndex = MAX_SCAN_CONFIG;
        EXPECT_TRUE(pScanService->StoreRequestScanConfig(scanConfig, interConfig) == 0);
    }

    void HandleCommonScanFailedSuccess1()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _)).Times(AtLeast(0));
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_CONNECTING);
        std::vector<int> requestIndexList;
        pScanService->HandleCommonScanFailed(requestIndexList);
    }

    void HandleCommonScanFailedSuccess2()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _)).Times(AtLeast(0));
        pScanService->staStatus = static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED);
        std::vector<int> requestIndexList;
        requestIndexList.push_back(0);
        pScanService->HandleCommonScanFailed(requestIndexList);
    }

    void HandleCommonScanFailedSuccess3()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _));
        pScanService->staStatus = static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED);
        StoreScanConfig storeScanConfig;
        storeScanConfig.fullScanFlag = true;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        std::vector<int> requestIndexList;
        requestIndexList.push_back(0);
        pScanService->HandleCommonScanFailed(requestIndexList);
    }

    void HandleCommonScanFailedSuccess4()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanFinished(_, _)).Times(AtLeast(0));
        pScanService->staStatus = static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED);
        StoreScanConfig storeScanConfig;
        storeScanConfig.fullScanFlag = false;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        std::vector<int> requestIndexList;
        requestIndexList.push_back(0);
        pScanService->HandleCommonScanFailed(requestIndexList);
    }

    void HandleCommonScanInfoSuccess1()
    {
        std::vector<int> requestIndexList;
        requestIndexList.push_back(0);
        std::vector<InterScanInfo> scanInfoList;
        pScanService->HandleCommonScanInfo(requestIndexList, scanInfoList);
    }

    void HandleCommonScanInfoSuccess2()
    {
        ON_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillByDefault(Return(0));
        StoreScanConfig storeScanConfig0;
        storeScanConfig0.fullScanFlag = true;
        StoreScanConfig storeScanConfig1;
        storeScanConfig1.fullScanFlag = true;
        pScanService->scanConfigMap.emplace(0, storeScanConfig0);
        pScanService->scanConfigMap.emplace(1, storeScanConfig1);
        std::vector<int> requestIndexList { 0, 1 };
        std::vector<InterScanInfo> scanInfoList;
        pScanService->HandleCommonScanInfo(requestIndexList, scanInfoList);
    }

    void HandleCommonScanInfoSuccess3()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).Times(AtLeast(0));
        StoreScanConfig storeScanConfig;
        storeScanConfig.fullScanFlag = false;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        std::vector<int> requestIndexList { 0 };
        std::vector<InterScanInfo> scanInfoList;
        pScanService->HandleCommonScanInfo(requestIndexList, scanInfoList);
    }

    void StoreFullScanInfoSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillRepeatedly(Return(0));
        StoreScanConfig scanConfig;
        std::vector<InterScanInfo> scanInfoList { InterScanInfo() };
        EXPECT_EQ(true, pScanService->StoreFullScanInfo(scanConfig, scanInfoList));
    }

    void StoreFullScanInfoFail()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillRepeatedly(Return(-1));
        StoreScanConfig scanConfig;
        std::vector<InterScanInfo> scanInfoList { InterScanInfo() };
        EXPECT_EQ(true, pScanService->StoreFullScanInfo(scanConfig, scanInfoList));
    }

    void StoreUserScanInfoSuccess1()
    {
        StoreScanConfig scanConfig;
        std::vector<InterScanInfo> scanInfoList { InterScanInfo() };
        EXPECT_EQ(true, pScanService->StoreUserScanInfo(scanConfig, scanInfoList));
    }

    void StoreUserScanInfoSuccess2()
    {
        StoreScanConfig scanConfig;
        scanConfig.scanFreqs.push_back(FREQ_2_DOT_4_GHZ);
        std::vector<InterScanInfo> scanInfoList;
        InterScanInfo interScanInfo;
        interScanInfo.timestamp = 1;
        scanInfoList.push_back(interScanInfo);
        EXPECT_EQ(false, pScanService->StoreUserScanInfo(scanConfig, scanInfoList));
    }

    void ReportScanInfosSuccess()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanInfoNotify(_, _));
        std::vector<InterScanInfo> scanInfoList;
        pScanService->ReportScanInfos(scanInfoList);
    }

    void BeginPnoScanSuccess1()
    {
        pScanService->isPnoScanBegined = false;
        std::vector<WifiDeviceConfig> results;
        WifiDeviceConfig cfg;
        cfg.isPasspoint = false;
        cfg.isEphemeral = false;
        results.push_back(cfg);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillRepeatedly(Return(0));
        pScanService->BeginPnoScan();
    }

    void BeginPnoScanFail1()
    {
        pScanService->isPnoScanBegined = false;
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillRepeatedly(Return(0));
        EXPECT_EQ(false, pScanService->BeginPnoScan());
    }

    void BeginPnoScanFail2()
    {
        pScanService->isPnoScanBegined = false;
        pScanService->staStatus = static_cast<int>(OperateResState::OPEN_WIFI_OPENING);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SaveScanInfoList(_)).WillRepeatedly(Return(0));
        EXPECT_EQ(false, pScanService->BeginPnoScan());
    }

    void BeginPnoScanFail3()
    {
        pScanService->isPnoScanBegined = true;
        EXPECT_EQ(false, pScanService->BeginPnoScan());
    }

    void PnoScanSuccess()
    {
        PnoScanConfig pnoScanConfig;
        InterScanConfig interScanConfig;
        EXPECT_EQ(true, pScanService->PnoScan(pnoScanConfig, interScanConfig));
    }

    void PnoScanFail()
    {
        PnoScanConfig pnoScanConfig;
        InterScanConfig interScanConfig;
        pScanService->pScanStateMachine = nullptr;
        EXPECT_EQ(false, pScanService->PnoScan(pnoScanConfig, interScanConfig));
    }

    void AddPnoScanMessageBodySuccess()
    {
        InternalMessagePtr interMessage = std::make_shared<InternalMessage>();
        PnoScanConfig pnoScanConfig;
        EXPECT_EQ(true, pScanService->AddPnoScanMessageBody(interMessage, pnoScanConfig));
    }

    void AddPnoScanMessageBodyFail()
    {
        PnoScanConfig pnoScanConfig;
        EXPECT_EQ(false, pScanService->AddPnoScanMessageBody(nullptr, pnoScanConfig));
    }

    void HandlePnoScanInfoSuccess()
    {
        EXPECT_CALL(WifiManager::GetInstance(), DealScanInfoNotify(_, _));
        std::vector<InterScanInfo> scanInfoList;
        InterScanInfo interScanInfo;
        interScanInfo.timestamp = TIMES_TAMP;
        scanInfoList.push_back(interScanInfo);
        pScanService->pnoScanStartTime = 0;
        pScanService->HandlePnoScanInfo(scanInfoList);
    }

    void EndPnoScanSuccess()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->isPnoScanBegined = true;
        pScanService->EndPnoScan();
    }

    void EndPnoScanFail()
    {
        pScanService->EndPnoScan();
    }

    void EndPnoScanFail2()
    {
        pScanService->isPnoScanBegined = true;
        pScanService->pScanStateMachine = nullptr;
        pScanService->EndPnoScan();
    }

    void HandleScreenStatusChangedSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return(""));
        pScanService->HandleScreenStatusChanged();
    }

    void HandleStaStatusChangedSuccess1()
    {
        int status = static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return(""));
        pScanService->HandleStaStatusChanged(status);
    }

    void HandleStaStatusChangedSuccess2()
    {
        int status = static_cast<int>(OperateResState::CONNECT_AP_CONNECTED);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return(""));
        pScanService->HandleStaStatusChanged(status);
    }

    void HandleStaStatusChangedFail()
    {
        int status = static_cast<int>(OperateResState::OPEN_WIFI_FAILED);
        pScanService->HandleStaStatusChanged(status);
    }

    void HandleCustomStatusChangedSuccess1()
    {
        int customScene = 0;
        int customSceneStatus = MODE_STATE_CLOSE;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleCustomStatusChanged(customScene, customSceneStatus);
    }

    void HandleCustomStatusChangedSuccess2()
    {
        int customScene = 0;
        int customSceneStatus = MODE_STATE_OPEN;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleCustomStatusChanged(customScene, customSceneStatus);
    }

    void SystemScanProcessSuccess1()
    {
        ScanIntervalMode mode;
        mode.scanScene = SCAN_SCENE_ALL;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = false;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScreenState()).WillRepeatedly(Return(1));
        pScanService->SystemScanProcess(true);
    }

    void SystemScanProcessSuccess2()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->SystemScanProcess(true);
    }

    void SystemScanProcessSuccess3()
    {
        ScanIntervalMode mode;
        mode.scanScene = SCAN_SCENE_MAX;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = false;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScreenState()).WillRepeatedly(Return(1));
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->SystemScanProcess(true);
    }

    void SystemSingleScanProcessSuccess()
    {
        pScanService->SystemSingleScanProcess();
    }

    void GetRelatedFreqsSuccess()
    {
        int lastStaFreq = 0;
        int p2pFreq = 0;
        int p2pEnhanceFreq = 0;
        pScanService->GetRelatedFreqs(lastStaFreq, p2pFreq, p2pEnhanceFreq);
    }

    void StopSystemScanSuccess()
    {
        pScanService->StopSystemScan();
        pScanService->pScanStateMachine = nullptr;
        pScanService->StopSystemScan();
    }

    void StartSystemTimerScanFail1()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetThermalLevel()).WillRepeatedly(Return(FOUR));
        pScanService->staStatus = 0;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->StartSystemTimerScan(true);
    }

    void StartSystemTimerScanFail2()
    {
        pScanService->lastSystemScanTime = 1;
        pScanService->systemScanIntervalMode.scanIntervalMode.interval = 1;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->StartSystemTimerScan(true);
    }

    void StartSystemTimerScanFail3()
    {
        pScanService->lastSystemScanTime = 1;
        pScanService->systemScanIntervalMode.scanIntervalMode.interval = INVAL;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->StartSystemTimerScan(false);
    }

    void StartSystemTimerScanFail4()
    {
        pScanService->lastSystemScanTime = 1;
        pScanService->systemScanIntervalMode.scanIntervalMode.interval = 1;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->StartSystemTimerScan(false);
    }

    void StartSystemTimerScanSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return(""));
        pScanService->lastSystemScanTime = 0;
        pScanService->systemScanIntervalMode.scanIntervalMode.interval = MAX_SCAN_CONFIG;
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->StartSystemTimerScan(false);
    }

    void HandleSystemScanTimeoutSuccess()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleSystemScanTimeout();
        EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
    }

    void DisconnectedTimerScanSuccess()
    {
        pScanService->DisconnectedTimerScan();
        pScanService->pScanStateMachine = nullptr;
        pScanService->DisconnectedTimerScan();
    }

    void HandleDisconnectedScanTimeoutSuccess()
    {
        pScanService->HandleDisconnectedScanTimeout();
        pScanService->staStatus = 0;
        pScanService->HandleDisconnectedScanTimeout();
    }

    void HandleDisconnectedScanTimeoutFail1()
    {
        pScanService->pScanStateMachine = nullptr;
        pScanService->HandleDisconnectedScanTimeout();
    }

    void HandleDisconnectedScanTimeoutFail2()
    {
        pScanService->scanStartedFlag = false;
        pScanService->HandleDisconnectedScanTimeout();
    }

    void RestartPnoScanTimeOutSuccess()
    {
        pScanService->RestartPnoScanTimeOut();
    }

    void RestartPnoScanTimeOutFail()
    {
        pScanService->pnoScanFailedNum = FAILEDNUM;
        pScanService->RestartPnoScanTimeOut();
    }

    void GetScanControlInfoSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScanControlInfo(_, _)).WillRepeatedly(Return(0));
        pScanService->GetScanControlInfo();
    }

    void GetScanControlInfoFail()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetScanControlInfo(_, _)).WillRepeatedly(Return(-1));
        pScanService->GetScanControlInfo();
    }
    
    void AllowExternScanSuccess()
    {
        pScanService->AllowExternScan();
    }

    void AllowExternScanFail1()
    {
        int staScene = 0;
        StoreScanConfig cfg;
        pScanService->scanConfigMap.emplace(staScene, cfg);

        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode forbidMode;
        forbidMode.scanScene = SCAN_SCENE_SCANNING;
        forbidMode.scanMode = scanMode;
        pScanService->scanControlInfo.scanForbidList.push_back(forbidMode);

        pScanService->AllowExternScan();
    }

    void AllowExternScanFail2()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppRunningState())
            .WillRepeatedly(Return(ScanMode::SYS_FOREGROUND_SCAN));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetThermalLevel()).WillRepeatedly(Return(FOUR));
        EXPECT_EQ(pScanService->AllowExternScan(), WIFI_OPT_SUCCESS);
    }

    void AllowExternScanFail3()
    {
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode forbidMode;
        forbidMode.scanScene = SCAN_SCENE_CONNECTED;
        forbidMode.scanMode = scanMode;
        forbidMode.forbidTime = 0;
        forbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(forbidMode);
        pScanService->staStatus = STATUS;
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppRunningState())
            .WillRepeatedly(Return(ScanMode::SYS_FOREGROUND_SCAN));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetThermalLevel()).WillRepeatedly(Return(FOUR));
        EXPECT_EQ(pScanService->AllowExternScan(), WIFI_OPT_SUCCESS);
    }

    void AllowExternScanFail4()
    {
        pScanService->disableScanFlag = true;
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetThermalLevel(TWO)).Times(AtLeast(0));
        EXPECT_EQ(pScanService->AllowExternScan(), WIFI_OPT_FAILED);
    }

    void AllowSystemTimerScanSuccess()
    {
        ScanIntervalMode mode;
        mode.scanScene = SCAN_SCENE_ALL;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = false;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->AllowSystemTimerScan();
    }

    void AllowSystemTimerScanFail1()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->AllowSystemTimerScan();
    }

    void AllowSystemTimerScanFail3()
    {
        pScanService->staStatus = FREQ_2_DOT_4_GHZ;
        EXPECT_EQ(pScanService->AllowSystemTimerScan(), WIFI_OPT_FAILED);
    }

    void AllowSystemTimerScanFail5()
    {
        const int staScene = 17;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        ScanMode scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        ScanForbidMode forbidMode;
        forbidMode.scanScene = static_cast<int>(OperateResState::CONNECT_AP_CONNECTED);
        forbidMode.scanMode = scanMode;
        forbidMode.forbidTime = 0;
        forbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(forbidMode);
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->scanTrustMode = false;
        EXPECT_EQ(pScanService->AllowSystemTimerScan(), WIFI_OPT_FAILED);
    }

    void AllowPnoScanSuccess()
    {
        ScanIntervalMode mode;
        mode.scanScene = SCAN_SCENE_ALL;
        mode.scanMode = ScanMode::PNO_SCAN;
        mode.isSingle = false;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->AllowPnoScan();
    }

    void GetStaSceneSuccess1()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_AP_CONNECTED);
        EXPECT_EQ(SCAN_SCENE_CONNECTED, pScanService->GetStaScene());
    }

    void GetStaSceneSuccess2()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED);
        EXPECT_EQ(SCAN_SCENE_DISCONNCTED, pScanService->GetStaScene());
    }

    void GetStaSceneSuccess3()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_CONNECTING);
        EXPECT_EQ(SCAN_SCENE_CONNECTING, pScanService->GetStaScene());
    }

    void GetStaSceneSuccess4()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_OBTAINING_IP);
        EXPECT_EQ(SCAN_SCENE_OBTAINING_IP, pScanService->GetStaScene());
    }

    void GetStaSceneSuccess5()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_ASSOCIATING);
        EXPECT_EQ(SCAN_SCENE_ASSOCIATING, pScanService->GetStaScene());
    }

    void GetStaSceneSuccess6()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_ASSOCIATED);
        EXPECT_EQ(SCAN_SCENE_ASSOCIATED, pScanService->GetStaScene());
    }

    void GetStaSceneFail()
    {
        pScanService->staStatus = static_cast<int>(OperateResState::OPEN_WIFI_FAILED);
        EXPECT_EQ(SCAN_SCENE_MAX, pScanService->GetStaScene());
    }

    void IsExternScanningSuccess()
    {
        StoreScanConfig storeScanConfig;
        storeScanConfig.scanType = ScanType::SCAN_TYPE_EXTERN;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        EXPECT_EQ(true, pScanService->IsExternScanning());
    }

    void IsScanningWithParamTest()
    {
        StoreScanConfig storeScanConfig;
        storeScanConfig.scanningWithParamFlag = true;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        EXPECT_EQ(true, pScanService->IsScanningWithParam());

        pScanService->scanConfigMap.clear();
        EXPECT_EQ(false, pScanService->IsScanningWithParam());
    }

    void IsExternScanningFail()
    {
        StoreScanConfig storeScanConfig;
        storeScanConfig.scanType = ScanType::SCAN_TYPE_SYSTEMTIMER;
        pScanService->scanConfigMap.emplace(0, storeScanConfig);
        EXPECT_EQ(false, pScanService->IsExternScanning());
    }

    void GetAllowBandFreqsControlInfoSuccess()
    {
        std::vector<int> freqs;
        freqs.push_back(FREQ_2_DOT_4_GHZ);
        ScanBandType scanBand = SCAN_BAND_24_GHZ;
        ScanForbidMode scanForbidMode, forbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_ALL;
        scanForbidMode.scanMode = ScanMode::BAND_24GHZ_SCAN;
        forbidMode.scanScene = SCAN_SCENE_ALL;
        forbidMode.scanMode = ScanMode::BAND_5GHZ_SCAN;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        pScanService->scanControlInfo.scanForbidList.push_back(forbidMode);
        pScanService->GetAllowBandFreqsControlInfo(scanBand, freqs);
    }

    void GetAllowBandFreqsControlInfoSuccess1()
    {
        std::vector<int> freqs;
        freqs.push_back(FREQ_2_DOT_4_GHZ);
        time_t now = time(nullptr);
        ScanBandType scanBand = SCAN_BAND_24_GHZ;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_CONNECTED;
        scanForbidMode.scanMode = ScanMode::BAND_24GHZ_SCAN;
        scanForbidMode.forbidTime = 0;
        scanForbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        pScanService->customSceneTimeMap.emplace(SCAN_SCENE_CONNECTED, now);
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_AP_CONNECTED);
        pScanService->GetAllowBandFreqsControlInfo(scanBand, freqs);
    }

    void GetAllowBandFreqsControlInfoSuccess2()
    {
        std::vector<int> freqs;
        freqs.push_back(FREQ_5_GHZ);
        time_t now = time(nullptr);
        ScanBandType scanBand = SCAN_BAND_5_GHZ;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_CONNECTED;
        scanForbidMode.scanMode = ScanMode::BAND_5GHZ_SCAN;
        scanForbidMode.forbidTime = 0;
        scanForbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        pScanService->customSceneTimeMap.emplace(SCAN_SCENE_CONNECTED, now);
        pScanService->staStatus = static_cast<int>(OperateResState::CONNECT_AP_CONNECTED);
        pScanService->GetAllowBandFreqsControlInfo(scanBand, freqs);
    }

    void ConvertBandNotAllow24GSuccess1()
    {
        ScanBandType scanBand = SCAN_BAND_24_GHZ;
        EXPECT_EQ(SCAN_BAND_UNSPECIFIED, pScanService->ConvertBandNotAllow24G(scanBand));
    }

    void ConvertBandNotAllow24GSuccess2()
    {
        ScanBandType scanBand = SCAN_BAND_BOTH;
        EXPECT_EQ(SCAN_BAND_5_GHZ, pScanService->ConvertBandNotAllow24G(scanBand));
    }

    void ConvertBandNotAllow24GSuccess3()
    {
        ScanBandType scanBand = SCAN_BAND_BOTH_WITH_DFS;
        EXPECT_EQ(SCAN_BAND_5_GHZ_WITH_DFS, pScanService->ConvertBandNotAllow24G(scanBand));
    }

    void ConvertBandNotAllow24GSuccess4()
    {
        ScanBandType scanBand = SCAN_BAND_5_GHZ;
        EXPECT_EQ(SCAN_BAND_5_GHZ, pScanService->ConvertBandNotAllow24G(scanBand));
    }

    void ConvertBandNotAllow24GFail()
    {
        ScanBandType scanBand = SCAN_BAND_UNSPECIFIED;
        EXPECT_EQ(SCAN_BAND_UNSPECIFIED, pScanService->ConvertBandNotAllow24G(scanBand));
    }

    void ConvertBandNotAllow5GSuccess()
    {
        ScanBandType scanBand = SCAN_BAND_24_GHZ;
        EXPECT_EQ(SCAN_BAND_24_GHZ, pScanService->ConvertBandNotAllow5G(scanBand));
    }

    void ConvertBandNotAllow5GFail()
    {
        ScanBandType scanBand = SCAN_BAND_5_GHZ;
        EXPECT_EQ(SCAN_BAND_UNSPECIFIED, pScanService->ConvertBandNotAllow5G(scanBand));
    }

    void Delete24GhzFreqsSuccess()
    {
        std::vector<int> freqs;
        freqs.push_back(FREQ_2_DOT_4_GHZ);
        pScanService->Delete24GhzFreqs(freqs);
    }

    void Delete5GhzFreqsSuccess()
    {
        std::vector<int> freqs;
        freqs.push_back(FREQ_5_GHZ);
        pScanService->Delete5GhzFreqs(freqs);
    }

    void GetSavedNetworkSsidListSuccess()
    {
        std::vector<std::string> savedNetworkSsid;
        EXPECT_EQ(true, pScanService->GetSavedNetworkSsidList(savedNetworkSsid));
    }

    void GetSavedNetworkSsidListFail()
    {
        std::vector<std::string> savedNetworkSsid;
        EXPECT_EQ(true, pScanService->GetSavedNetworkSsidList(savedNetworkSsid));
    }

    void GetHiddenNetworkSsidListSuccess1()
    {
        std::vector<std::string> hiddenNetworkSsid;
        EXPECT_EQ(true, pScanService->GetHiddenNetworkSsidList(hiddenNetworkSsid));
    }

    void GetHiddenNetworkSsidListSuccess2()
    {
        std::vector<WifiDeviceConfig> deviceConfigs;
        WifiDeviceConfig cfg;
        cfg.hiddenSSID = true;
        deviceConfigs.push_back(cfg);
        std::vector<std::string> hiddenNetworkSsid;
        EXPECT_EQ(true, pScanService->GetHiddenNetworkSsidList(hiddenNetworkSsid));
    }

    void GetHiddenNetworkSsidListFail()
    {
        std::vector<std::string> hiddenNetworkSsid;
        EXPECT_EQ(true, pScanService->GetHiddenNetworkSsidList(hiddenNetworkSsid));
    }

    void SetStaCurrentTimeSuccess()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetScreenState(TWO));
        pScanService->ClearScanControlValue();
        pScanService->SetStaCurrentTime();
    }

    void AllowScanDuringScanningSuccess()
    {
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        EXPECT_EQ(pScanService->AllowScanDuringScanning(scanMode), true);
    }

    void AllowScanDuringScanningFail()
    {
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode forbidMode;
        forbidMode.scanScene = SCAN_SCENE_SCANNING;
        forbidMode.scanMode = scanMode;
        pScanService->scanControlInfo.scanForbidList.push_back(forbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringScanning(scanMode), false);
    }

    void AllowScanDuringStaSceneSuccess1()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 1;
        scanForbidMode.forbidCount = 1;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringStaScene(staScene, scanMode), false);
    }

    void AllowScanDuringStaSceneFail1()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 0;
        scanForbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringStaScene(staScene, scanMode), false);
    }

    void AllowScanDuringStaSceneFail2()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 1;
        scanForbidMode.forbidCount = 1;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringStaScene(staScene, scanMode), false);
    }

    void AllowScanDuringStaSceneFail3()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 1;
        scanForbidMode.forbidCount = 1;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringStaScene(staScene, scanMode), false);
    }

    void AllowScanDuringCustomSceneSuccess()
    {
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), true);
    }

    void AllowScanDuringCustomSceneSuccess1()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        pScanService->SetScanTrustMode();
        pScanService->scanTrustSceneIds.emplace(0);
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), true);
    }

    void AllowScanDuringCustomSceneSuccess2()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        pScanService->SetScanTrustMode();
        pScanService->scanTrustSceneIds.emplace(1);
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), true);
    }

    void AllowScanDuringCustomSceneFail1()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 0;
        scanForbidMode.forbidCount = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), false);
    }

    void AllowScanDuringCustomSceneFail2()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 1;
        scanForbidMode.forbidCount = 1;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), false);
    }

    void AllowScanDuringCustomSceneFail3()
    {
        const int staScene = 0;
        ScanMode scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        time_t now = time(nullptr);
        if (now < 0) {
            return;
        }
        pScanService->customSceneTimeMap.emplace(staScene, now);
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = staScene;
        scanForbidMode.scanMode = scanMode;
        scanForbidMode.forbidTime = 1;
        scanForbidMode.forbidCount = 1;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        EXPECT_EQ(pScanService->AllowScanDuringCustomScene(scanMode), false);
    }

    void AllowExternScanByIntervalModeFail1()
    {
        pScanService->SetScanTrustMode();
        pScanService->AddScanTrustSceneId(0);
        EXPECT_EQ(pScanService->AllowExternScanByIntervalMode(0, 0, ScanMode::SYSTEM_TIMER_SCAN), true);
    }

    void AllowExternScanByIntervalModeFail2()
    {
        ScanIntervalMode mode;
        mode.scanScene = 0;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = true;
        mode.interval = 1;
        mode.count = 0;
        mode.intervalMode = IntervalMode::INTERVAL_FIXED;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->scanTrustMode = false;
        EXPECT_EQ(pScanService->AllowExternScanByIntervalMode(0, 0, ScanMode::SYSTEM_TIMER_SCAN), false);
    }

    void AllowExternScanByIntervalModeFail3()
    {
        ScanIntervalMode mode;
        mode.scanScene = 0;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = false;
        mode.interval = 1;
        mode.count = 0;
        mode.intervalMode = IntervalMode::INTERVAL_FIXED;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->scanTrustMode = false;
        EXPECT_EQ(pScanService->AllowExternScanByIntervalMode(0, 0, ScanMode::SYSTEM_TIMER_SCAN), false);
    }

    void AllowExternScanByIntervalModeSuccess()
    {
        ScanIntervalMode mode;
        mode.scanScene = 1;
        mode.scanMode = ScanMode::SYSTEM_TIMER_SCAN;
        mode.isSingle = false;
        pScanService->scanControlInfo.scanIntervalList.push_back(mode);
        pScanService->scanTrustMode = false;
        EXPECT_EQ(pScanService->AllowExternScanByIntervalMode(0, 0, ScanMode::SYSTEM_TIMER_SCAN), true);
    }

    void AllowExternScanByIntervalModeSuccess1()
    {
        pScanService->scanTrustMode = false;
        pScanService->scanTrustSceneIds.emplace(0);
        EXPECT_EQ(pScanService->AllowExternScanByIntervalMode(0, 0, ScanMode::SYSTEM_TIMER_SCAN), true);
    }

    void PnoScanByIntervalSuccess1()
    {
        int fixedScanCount = 0;
        time_t fixedScanTime = 0;
        int interval = 0;
        int count = 0;
        EXPECT_EQ(pScanService->PnoScanByInterval(fixedScanCount, fixedScanTime, interval, count), true);
    }

    void PnoScanByIntervalSuccess2()
    {
        int fixedScanCount = 1;
        time_t fixedScanTime = time(nullptr) - 1;
        int interval = 0;
        int count = 0;
        EXPECT_EQ(pScanService->PnoScanByInterval(fixedScanCount, fixedScanTime, interval, count), true);
    }

    void PnoScanByIntervalSuccess3()
    {
        int fixedScanCount = 1;
        time_t fixedScanTime = time(nullptr) + 1;
        int interval = 1;
        int count = 1;
        EXPECT_EQ(pScanService->PnoScanByInterval(fixedScanCount, fixedScanTime, interval, count), true);
    }

    void PnoScanByIntervalFail1()
    {
        int fixedScanCount = 2;
        time_t fixedScanTime = time(nullptr) + 1;
        int interval = 1;
        int count = 1;
        EXPECT_EQ(pScanService->PnoScanByInterval(fixedScanCount, fixedScanTime, interval, count), false);
    }

    void SystemScanByIntervalSuccess()
    {
        int expScanCount = 1;
        int interval = 1;
        const int constTest = 2;
        int count = constTest;
        EXPECT_EQ(pScanService->SystemScanByInterval(expScanCount, interval, count), true);
    }

    void ExternScanByIntervalSuccess1()
    {
        SingleAppForbid singleAppForbid;
        singleAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;
        EXPECT_EQ(pScanService->ExternScanByInterval(0, singleAppForbid), true);
    }

    void ExternScanByIntervalSuccess2()
    {
        SingleAppForbid singleAppForbid;
        singleAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_EXP;
        EXPECT_EQ(pScanService->ExternScanByInterval(0, singleAppForbid), true);
    }

    void ExternScanByIntervalSuccess3()
    {
        SingleAppForbid singleAppForbid;
        singleAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_CONTINUE;
        EXPECT_EQ(pScanService->ExternScanByInterval(0, singleAppForbid), true);
    }

    void ExternScanByIntervalSuccess4()
    {
        SingleAppForbid singleAppForbid;
        singleAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_BLOCKLIST;
        EXPECT_EQ(pScanService->ExternScanByInterval(0, singleAppForbid), true);
    }

    void ExternScanByIntervalSuccess5()
    {
        SingleAppForbid singleAppForbid;
        singleAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_MAX;
        EXPECT_EQ(pScanService->ExternScanByInterval(0, singleAppForbid), true);
    }

    void AllowSingleAppScanByIntervalSuccess1()
    {
        int appId = 0;
        ScanIntervalMode scanIntervalMode;
        EXPECT_EQ(pScanService->AllowSingleAppScanByInterval(appId, scanIntervalMode), true);
    }

    void AllowSingleAppScanByIntervalSuccess2()
    {
        int appId = 0;
        SingleAppForbid sAppForbid;
        sAppForbid.appID = appId;
        sAppForbid.scanIntervalMode.scanScene = SCAN_SCENE_ALL;
        sAppForbid.scanIntervalMode.scanMode = ScanMode::SCAN_MODE_MAX;
        pScanService->appForbidList.push_back(sAppForbid);
        ScanIntervalMode scanIntervalMode = sAppForbid.scanIntervalMode;
        EXPECT_EQ(pScanService->AllowSingleAppScanByInterval(appId, scanIntervalMode), true);
    }

    void AllowSingleAppScanByIntervalFail1()
    {
        int appId = 0;
        SingleAppForbid sAppForbid;
        sAppForbid.appID = appId;
        sAppForbid.scanIntervalMode.scanScene = SCAN_SCENE_ALL;
        sAppForbid.scanIntervalMode.scanMode = ScanMode::SCAN_MODE_MAX;

        sAppForbid.fixedScanCount = 1;
        sAppForbid.fixedCurrentTime = time(nullptr) - 1;
        const int intervalOrCount = 2;
        sAppForbid.scanIntervalMode.interval = intervalOrCount;
        sAppForbid.scanIntervalMode.count = 1;
        sAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;

        pScanService->appForbidList.push_back(sAppForbid);

        ScanIntervalMode scanIntervalMode = sAppForbid.scanIntervalMode;
        EXPECT_EQ(pScanService->AllowSingleAppScanByInterval(appId, scanIntervalMode), false);
    }

    void AllowFullAppScanByIntervalSuccess1()
    {
        int appId = 0;
        ScanIntervalMode scanIntervalMode;
        EXPECT_EQ(pScanService->AllowFullAppScanByInterval(appId, scanIntervalMode), true);
    }

    void AllowFullAppScanByIntervalSuccess2()
    {
        int appId = 0;
        SingleAppForbid sAppForbid;
        sAppForbid.appID = appId;
        sAppForbid.scanIntervalMode.scanScene = SCAN_SCENE_ALL;
        sAppForbid.scanIntervalMode.scanMode = ScanMode::SCAN_MODE_MAX;
        pScanService->fullAppForbidList.push_back(sAppForbid);
        ScanIntervalMode scanIntervalMode = sAppForbid.scanIntervalMode;
        EXPECT_EQ(pScanService->AllowFullAppScanByInterval(appId, scanIntervalMode), true);
    }

    void AllowFullAppScanByIntervalFail1()
    {
        int appId = 0;
        SingleAppForbid sAppForbid;
        sAppForbid.appID = appId;
        sAppForbid.scanIntervalMode.scanScene = SCAN_SCENE_ALL;
        sAppForbid.scanIntervalMode.scanMode = ScanMode::SCAN_MODE_MAX;

        sAppForbid.fixedScanCount = 1;
        sAppForbid.fixedCurrentTime = time(nullptr) - 1;
        const int intervalOrCount = 2;
        sAppForbid.scanIntervalMode.interval = intervalOrCount;
        sAppForbid.scanIntervalMode.count = 1;
        sAppForbid.scanIntervalMode.intervalMode = IntervalMode::INTERVAL_FIXED;

        pScanService->fullAppForbidList.push_back(sAppForbid);

        ScanIntervalMode scanIntervalMode = sAppForbid.scanIntervalMode;
        EXPECT_EQ(pScanService->AllowFullAppScanByInterval(appId, scanIntervalMode), false);
    }

    void AllowScanByIntervalFixedSuccess1()
    {
        int fixedScanCount = 0;
        time_t fixedScanTime = 0;
        int interval = 0;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalFixed(fixedScanCount, fixedScanTime, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalFixedSuccess2()
    {
        int fixedScanCount = 1;
        time_t fixedScanTime = 0;
        int interval = 0;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalFixed(fixedScanCount, fixedScanTime, interval, count);
        EXPECT_EQ(rlt, true);
        fixedScanTime = time(nullptr);
        interval = 0;
        count = 1;
        rlt = pScanService->AllowScanByIntervalFixed(fixedScanCount, fixedScanTime, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalFixedSuccess3()
    {
        int fixedScanCount = 1;
        time_t fixedScanTime = time(nullptr) - 1;
        const int intervalOrCount = 2;
        int interval = intervalOrCount;
        int count = intervalOrCount;
        bool rlt = pScanService->AllowScanByIntervalFixed(fixedScanCount, fixedScanTime, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalFixedFail1()
    {
        int fixedScanCount = 1;
        time_t fixedScanTime = time(nullptr) - 1;
        const int intervalOrCount = 2;
        int interval = intervalOrCount;
        int count = 1;
        bool rlt = pScanService->AllowScanByIntervalFixed(fixedScanCount, fixedScanTime, interval, count);
        EXPECT_EQ(rlt, false);
    }

    void AllowScanByIntervalExpSuccess()
    {
        int expScanCount = 1;
        int interval = 1;
        const int conutTest = 2;
        int count = conutTest;
        EXPECT_EQ(pScanService->AllowScanByIntervalExp(expScanCount, interval, count), true);
    }

    void AllowScanByIntervalContinueSuccess1()
    {
        time_t continueScanTime = 0;
        int lessThanIntervalCount = 0;
        int interval = 0;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalContinue(continueScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalContinueSuccess2()
    {
        const int timeTest = 2;
        time_t continueScanTime = time(nullptr) - timeTest;
        const int intervalTest = 5;
        int interval = intervalTest;
        int lessThanIntervalCount = 0;
        const int countTest = 2;
        int count = countTest;
        bool rlt = pScanService->AllowScanByIntervalContinue(continueScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalContinueSuccess3()
    {
        const int timeTest = 5;
        time_t continueScanTime = time(nullptr) - timeTest;
        int lessThanIntervalCount = 0;
        const int intervalTest = 1;
        int interval = intervalTest;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalContinue(continueScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalContinueFail1()
    {
        const int timeTest = 2;
        time_t continueScanTime = time(nullptr) - timeTest;
        const int intervalTest = 5;
        int interval = intervalTest;
        int lessThanIntervalCount = 0;
        const int countTest = 1;
        int count = countTest;
        bool rlt = pScanService->AllowScanByIntervalContinue(continueScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, false);
    }

    void AllowScanByIntervalBlocklistSuccess1()
    {
        int appId = 0;
        const int timeTest = 2;
        time_t blockListScanTime = time(nullptr) - timeTest;
        int lessThanIntervalCount = 0;
        int interval = 0;
        int count = 0;
        pScanService->scanBlocklist.push_back(appId);
        pScanService->scanBlocklist.push_back(1);
        bool rlt = pScanService->AllowScanByIntervalBlocklist(
            appId, blockListScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalBlocklistSuccess2()
    {
        int appId = 0;
        const int timeTest = 2;
        time_t blockListScanTime = 0;
        int lessThanIntervalCount = 0;
        int interval = time(nullptr) + timeTest;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalBlocklist(
            appId, blockListScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalBlocklistSuccess3()
    {
        int appId = 0;
        const int timeTest = 5;
        time_t blockListScanTime = time(nullptr) - timeTest;
        int lessThanIntervalCount = 0;
        const int intervalTest = 10;
        int interval = intervalTest;
        const int countTest = 2;
        int count = countTest;
        bool rlt = pScanService->AllowScanByIntervalBlocklist(
            appId, blockListScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, true);
    }

    void AllowScanByIntervalBlocklistFail1()
    {
        int appId = 0;
        const int timeTest = 5;
        time_t blockListScanTime = time(nullptr) - timeTest;
        int lessThanIntervalCount = 0;
        const int intervalTest = 10;
        int interval = intervalTest;
        int count = 0;
        pScanService->scanBlocklist.push_back(appId);
        bool rlt = pScanService->AllowScanByIntervalBlocklist(
            appId, blockListScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, false);
    }

    void AllowScanByIntervalBlocklistFail2()
    {
        int appId = 0;
        const int timeTest = 5;
        time_t blockListScanTime = time(nullptr) - timeTest;
        int lessThanIntervalCount = 0;
        const int intervalTest = 10;
        int interval = intervalTest;
        int count = 0;
        bool rlt = pScanService->AllowScanByIntervalBlocklist(
            appId, blockListScanTime, lessThanIntervalCount, interval, count);
        EXPECT_EQ(rlt, false);
    }

    void HandleMovingFreezeChangedTest()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppRunningState()).Times(AtLeast(0));
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetFreezeModeState()).Times(AtLeast(0));
        std::map<int, time_t> sceneMap;
        pScanService->HandleGetCustomSceneState(sceneMap);
        pScanService->HandleMovingFreezeChanged();
    }

    void HandleAutoConnectStateChangedTest()
    {
        pScanService->HandleAutoConnectStateChanged(true);
    }

    void HandleNetworkQualityChangedTest()
    {
        int status = static_cast<int>(OperateResState::CONNECT_NETWORK_ENABLED);
        pScanService->staStatus = static_cast<int>(OperateResState::CLOSE_WIFI_FAILED);
        pScanService->HandleNetworkQualityChanged(status);
        status = static_cast<int>(OperateResState::CONNECT_NETWORK_DISABLED);
        pScanService->HandleNetworkQualityChanged(status);
    }

    void WifiMaxThroughputTest()
    {
        int channelUtilization = 10;
        std::vector<int> freqVector = {11, 51, -1, 69, 138, 92, 184, 366, 618, 0};
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(0, false, WifiChannelWidth::WIDTH_160MHZ, 0, 0, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(1, false, WifiChannelWidth::WIDTH_160MHZ, MAX_THROUGH, 1, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(FOUR, false, WifiChannelWidth::WIDTH_20MHZ, 0, 0, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(FOUR, false, WifiChannelWidth::WIDTH_40MHZ, 0, 0, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(STANDER, false, WifiChannelWidth::WIDTH_20MHZ, 0, 0, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(STANDER, false, WifiChannelWidth::WIDTH_40MHZ, 0, 0, channelUtilization)) != 0);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(STANDER, false, WifiChannelWidth::WIDTH_80MHZ, 0, 0, channelUtilization)) != 0);
            WifiMaxThroughput(STANDER, false, WifiChannelWidth::WIDTH_INVALID, 0, 0, channelUtilization);
        EXPECT_TRUE(count(freqVector.begin(), freqVector.end(),
            WifiMaxThroughput(FAILEDNUM, true, WifiChannelWidth::WIDTH_20MHZ, 0, 0, channelUtilization))!= 0);
            WifiMaxThroughput(FAILEDNUM, true, WifiChannelWidth::WIDTH_40MHZ, 0, 0, channelUtilization);
            WifiMaxThroughput(FAILEDNUM, true, WifiChannelWidth::WIDTH_80MHZ, 0, 0, channelUtilization);
            WifiMaxThroughput(FAILEDNUM, true, WifiChannelWidth::WIDTH_INVALID, 0, 0, channelUtilization);
    }

    void RecordScanLimitInfoTest()
    {
        WifiScanDeviceInfo info;
        info.packageName = "123.test";
        pScanService->RecordScanLimitInfo(info, ScanLimitType::WIFI_DISABLE);
        EXPECT_EQ(info.GetScanInitiatorName(), "123.test");
    }

    void IsPackageInTrustListTest()
    {
        EXPECT_TRUE(pScanService->IsPackageInTrustList("123456|", 0, "123456") == true);
        EXPECT_TRUE(pScanService->IsPackageInTrustList("123456|", 0, "1234") == false);
    }

    void IsAppInFilterListTest()
    {
        std::vector<PackageInfo> packageFilter;
        PackageInfo packageInfo;
        packageInfo.name = "com.test.test";
        packageFilter.push_back(packageInfo);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return("com.test.test"));
        EXPECT_TRUE(pScanService->IsAppInFilterList(packageFilter) == false);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetAppPackageName()).WillRepeatedly(Return("com.test.test1"));
        EXPECT_TRUE(pScanService->IsAppInFilterList(packageFilter) == false);
    }

    void SetScanTrustModeTest()
    {
        pScanService->SetScanTrustMode();
        EXPECT_EQ(pScanService->scanTrustMode, true);
    }

    void ResetToNonTrustModeTest()
    {
        pScanService->ResetToNonTrustMode();
        EXPECT_EQ(pScanService->scanTrustMode, false);
    }

    void AllowScanByMovingFreezeTest1()
    {
        pScanService->scanTrustMode = true;
        pScanService->scanTrustSceneIds.insert(-1);
        EXPECT_TRUE(pScanService->AllowScanByMovingFreeze(ScanMode::SYSTEM_TIMER_SCAN) == true);
    }

    void AllowScanByMovingFreezeTest2()
    {
        pScanService->lastFreezeState = true;
        pScanService->isAbsFreezeScaned = false;
        EXPECT_FALSE(pScanService->AllowScanByMovingFreeze(ScanMode::SYSTEM_TIMER_SCAN) == false);
        pScanService->isAbsFreezeScaned = true;
        EXPECT_FALSE(pScanService->AllowScanByMovingFreeze(ScanMode::SYSTEM_TIMER_SCAN) == false);
    }

    void AllowScanByHid2dStateTest()
    {
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetP2pBusinessType(P2pBusinessType::P2P_TYPE_HID2D));
        EXPECT_TRUE(pScanService->AllowScanByHid2dState() == true);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), SetP2pBusinessType(P2pBusinessType::P2P_TYPE_CLASSIC));
        EXPECT_TRUE(pScanService->AllowScanByHid2dState() == true);
    }

    void AllowCustomSceneCheckTest1()
    {
        std::map<int, time_t> customIter;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_SCANNING;
        scanForbidMode.scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        time_t now = time(nullptr);
        customIter.emplace(SCAN_SCENE_SCANNING, now);
        auto customIters = customIter.begin();
        EXPECT_TRUE(pScanService->AllowCustomSceneCheck(customIters, ScanMode::SYSTEM_TIMER_SCAN) == true);
    }

    void AllowCustomSceneCheckTest2()
    {
        std::map<int, time_t> customIter;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_SCANNING;
        scanForbidMode.scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        time_t now = time(nullptr);
        customIter.emplace(SCAN_SCENE_SCANNING, now);
        auto customIters = customIter.begin();
        EXPECT_TRUE(pScanService->AllowCustomSceneCheck(customIters, ScanMode::SYSTEM_TIMER_SCAN) == true);
    }

    void AllowCustomSceneCheckTest3()
    {
        std::map<int, time_t> customIter;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_SCANNING;
        scanForbidMode.scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        scanForbidMode.forbidCount = -1;
        scanForbidMode.forbidTime = 0;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        time_t now = time(nullptr);
        customIter.emplace(SCAN_SCENE_SCANNING, now);
        auto customIters = customIter.begin();
        EXPECT_TRUE(pScanService->AllowCustomSceneCheck(customIters, ScanMode::SYS_FOREGROUND_SCAN) == true);
    }

    void AllowCustomSceneCheckTest4()
    {
        std::map<int, time_t> customIter;
        ScanForbidMode scanForbidMode;
        scanForbidMode.scanScene = SCAN_SCENE_SCANNING;
        scanForbidMode.scanMode = ScanMode::SYS_FOREGROUND_SCAN;
        scanForbidMode.forbidCount = 1;
        scanForbidMode.forbidTime = STATUS;
        pScanService->scanControlInfo.scanForbidList.push_back(scanForbidMode);
        time_t now = time(nullptr);
        customIter.emplace(SCAN_SCENE_SCANNING, now);
        pScanService->customSceneForbidCount = STATUS;
        auto customIters = customIter.begin();
        EXPECT_TRUE(pScanService->AllowCustomSceneCheck(customIters, ScanMode::SYS_FOREGROUND_SCAN) == false);
    }

    void GetScanControlInfoTest()
    {
        pScanService->GetScanControlInfo();
    }

    void ClearScanTrustSceneIdsTest()
    {
        pScanService->ClearScanTrustSceneIds();
    }

    void ApplyTrustListPolicyTest()
    {
        ScanType scanType = ScanType::SCAN_TYPE_EXTERN;
        pScanService->ApplyTrustListPolicy(scanType);
    }

    void SetNetworkInterfaceUpDownTest()
    {
        pScanService->SetNetworkInterfaceUpDown(false);
    }

    void SystemScanConnectedPolicyTest()
    {
        int interval = 0;
        pScanService->SystemScanConnectedPolicy(interval);
    }

    void SystemScanDisconnectedPolicyTest()
    {
        int interval = 0;
        int count = 0;
        pScanService->SystemScanDisconnectedPolicy(interval, count);
    }

    void OnWifiCountryCodeChangedTest()
    {
        std::string countryCode = "CN";
        if (pScanService->InitScanService(WifiManager::GetInstance().GetScanCallback()) == true) {
            EXPECT_EQ(ErrCode::WIFI_OPT_SUCCESS, pScanService->m_scanObserver->OnWifiCountryCodeChanged(countryCode));
        }
    }
};

HWTEST_F(ScanServiceTest, InitScanServiceSuccess1, TestSize.Level1)
{
    InitScanServiceSuccess1();
}

HWTEST_F(ScanServiceTest, InitScanServiceSuccess2, TestSize.Level1)
{
    InitScanServiceSuccess2();
}

HWTEST_F(ScanServiceTest, UnInitScanServiceSuccess, TestSize.Level1)
{
    UnInitScanServiceSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess1, TestSize.Level1)
{
    HandleScanStatusReportSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess2, TestSize.Level1)
{
    HandleScanStatusReportSuccess2();
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess3, TestSize.Level1)
{
    HandleScanStatusReportSuccess3();
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess4, TestSize.Level1)
{
    HandleScanStatusReportSuccess4();
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess5, TestSize.Level1)
{
    HandleScanStatusReportSuccess5();
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess6, TestSize.Level1)
{
    HandleScanStatusReportSuccess6();
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportSuccess7, TestSize.Level1)
{
    HandleScanStatusReportSuccess7();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleScanStatusReportFail, TestSize.Level1)
{
    HandleScanStatusReportFail();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleInnerEventReportSuccess1, TestSize.Level1)
{
    HandleInnerEventReportSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleInnerEventReportSuccess2, TestSize.Level1)
{
    HandleInnerEventReportSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleInnerEventReportSuccess3, TestSize.Level1)
{
    HandleInnerEventReportSuccess3();
}

HWTEST_F(ScanServiceTest, HandleInnerEventReportFail, TestSize.Level1)
{
    HandleInnerEventReportFail();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, ScanSuccess, TestSize.Level1)
{
    ScanSuccess();
}

HWTEST_F(ScanServiceTest, ScanFail, TestSize.Level1)
{
    ScanFail();
}

HWTEST_F(ScanServiceTest, ScanWithParamSuccess, TestSize.Level1)
{
    ScanWithParamSuccess();
}

HWTEST_F(ScanServiceTest, ScanWithParamFail1, TestSize.Level1)
{
    ScanWithParamFail1();
}

HWTEST_F(ScanServiceTest, ScanWithParamFail2, TestSize.Level1)
{
    ScanWithParamFail2();
}

HWTEST_F(ScanServiceTest, ScanWithParamFail3, TestSize.Level1)
{
    ScanWithParamFail3();
}

HWTEST_F(ScanServiceTest, ScanWithParamFail4, TestSize.Level1)
{
    ScanWithParamFail4();
}

HWTEST_F(ScanServiceTest, SingleScanSuccess1, TestSize.Level1)
{
    SingleScanSuccess1();
}

HWTEST_F(ScanServiceTest, SingleScanSuccess2, TestSize.Level1)
{
    SingleScanSuccess2();
}

HWTEST_F(ScanServiceTest, SingleScanFail1, TestSize.Level1)
{
    SingleScanFail1();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess1, TestSize.Level1)
{
    GetBandFreqsSuccess1();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess2, TestSize.Level1)
{
    GetBandFreqsSuccess2();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess3, TestSize.Level1)
{
    GetBandFreqsSuccess3();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess4, TestSize.Level1)
{
    GetBandFreqsSuccess4();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess5, TestSize.Level1)
{
    GetBandFreqsSuccess5();
}

HWTEST_F(ScanServiceTest, GetBandFreqsSuccess6, TestSize.Level1)
{
    GetBandFreqsSuccess6();
}

HWTEST_F(ScanServiceTest, GetBandFreqsFail, TestSize.Level1)
{
    GetBandFreqsFail();
}

HWTEST_F(ScanServiceTest, AddScanMessageBodySuccess, TestSize.Level1)
{
    AddScanMessageBodySuccess();
}

HWTEST_F(ScanServiceTest, AddScanMessageBodyFail, TestSize.Level1)
{
    AddScanMessageBodyFail();
}

HWTEST_F(ScanServiceTest, StoreRequestScanConfigSuccess, TestSize.Level1)
{
    StoreRequestScanConfigSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: StoreRequestScanConfigFail
 * @tc.desc: StoreRequestScanConfig()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, StoreRequestScanConfigFail, TestSize.Level1)
{
    StoreRequestScanConfigFail();
}

HWTEST_F(ScanServiceTest, HandleCommonScanFailedSuccess1, TestSize.Level1)
{
    HandleCommonScanFailedSuccess1();
}

HWTEST_F(ScanServiceTest, HandleCommonScanFailedSuccess2, TestSize.Level1)
{
    HandleCommonScanFailedSuccess2();
}

HWTEST_F(ScanServiceTest, HandleCommonScanFailedSuccess3, TestSize.Level1)
{
    HandleCommonScanFailedSuccess3();
}

HWTEST_F(ScanServiceTest, HandleCommonScanFailedSuccess4, TestSize.Level1)
{
    HandleCommonScanFailedSuccess4();
}

HWTEST_F(ScanServiceTest, HandleCommonScanInfoSuccess1, TestSize.Level1)
{
    HandleCommonScanInfoSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleCommonScanInfoSuccess2, TestSize.Level1)
{
    HandleCommonScanInfoSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleCommonScanInfoSuccess3, TestSize.Level1)
{
    HandleCommonScanInfoSuccess3();
}

HWTEST_F(ScanServiceTest, StoreFullScanInfoSuccess, TestSize.Level1)
{
    StoreFullScanInfoSuccess();
}

HWTEST_F(ScanServiceTest, StoreFullScanInfoFail, TestSize.Level1)
{
    StoreFullScanInfoFail();
}

HWTEST_F(ScanServiceTest, StoreUserScanInfoSuccess1, TestSize.Level1)
{
    StoreUserScanInfoSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, StoreUserScanInfoSuccess2, TestSize.Level1)
{
    StoreUserScanInfoSuccess2();
}

HWTEST_F(ScanServiceTest, ReportScanInfosSuccess, TestSize.Level1)
{
    ReportScanInfosSuccess();
}

HWTEST_F(ScanServiceTest, BeginPnoScanSuccess1, TestSize.Level1)
{
    BeginPnoScanSuccess1();
}

HWTEST_F(ScanServiceTest, BeginPnoScanFail1, TestSize.Level1)
{
    BeginPnoScanFail1();
}

HWTEST_F(ScanServiceTest, BeginPnoScanFail2, TestSize.Level1)
{
    BeginPnoScanFail2();
}

HWTEST_F(ScanServiceTest, BeginPnoScanFail3, TestSize.Level1)
{
    BeginPnoScanFail3();
}

HWTEST_F(ScanServiceTest, PnoScanSuccess, TestSize.Level1)
{
    PnoScanSuccess();
}

HWTEST_F(ScanServiceTest, AddPnoScanMessageBodySuccess, TestSize.Level1)
{
    AddPnoScanMessageBodySuccess();
}

HWTEST_F(ScanServiceTest, AddPnoScanMessageBodyFail, TestSize.Level1)
{
    AddPnoScanMessageBodyFail();
}

HWTEST_F(ScanServiceTest, HandlePnoScanInfoSuccess, TestSize.Level1)
{
    HandlePnoScanInfoSuccess();
}

HWTEST_F(ScanServiceTest, EndPnoScanSuccess, TestSize.Level1)
{
    EndPnoScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, EndPnoScanFail, TestSize.Level1)
{
    EndPnoScanFail();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, EndPnoScanFail2, TestSize.Level1)
{
    EndPnoScanFail2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleScreenStatusChangedSuccess, TestSize.Level1)
{
    HandleScreenStatusChangedSuccess();
}

HWTEST_F(ScanServiceTest, HandleStaStatusChangedSuccess1, TestSize.Level1)
{
    HandleStaStatusChangedSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleStaStatusChangedSuccess2, TestSize.Level1)
{
    HandleStaStatusChangedSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleStaStatusChangedFail, TestSize.Level1)
{
    HandleStaStatusChangedFail();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleCustomStatusChangedSuccess1, TestSize.Level1)
{
    HandleCustomStatusChangedSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleCustomStatusChangedSuccess2, TestSize.Level1)
{
    HandleCustomStatusChangedSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, SystemScanProcessSuccess1, TestSize.Level1)
{
    SystemScanProcessSuccess1();
}

HWTEST_F(ScanServiceTest, SystemScanProcessSuccess2, TestSize.Level1)
{
    SystemScanProcessSuccess2();
}
/**
 * @tc.name: SystemScanProcessSuccess3
 * @tc.desc: SystemScanProcessSuccess()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, SystemScanProcessSuccess3, TestSize.Level1)
{
    SystemScanProcessSuccess3();
}

HWTEST_F(ScanServiceTest, SystemSingleScanProcessSuccess, TestSize.Level1)
{
    SystemSingleScanProcessSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, GetRelatedFreqsSuccess, TestSize.Level1)
{
    GetRelatedFreqsSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, StopSystemScanSuccess, TestSize.Level1)
{
    StopSystemScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, StartSystemTimerScanFail1, TestSize.Level1)
{
    StartSystemTimerScanFail1();
}
/**
 * @tc.name: StartSystemTimerScanFail2
 * @tc.desc: StartSystemTimerScan()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, StartSystemTimerScanFail2, TestSize.Level1)
{
    StartSystemTimerScanFail2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: StartSystemTimerScanFail3
 * @tc.desc: StartSystemTimerScan()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, StartSystemTimerScanFail3, TestSize.Level1)
{
    StartSystemTimerScanFail3();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: StartSystemTimerScanFail4
 * @tc.desc: StartSystemTimerScan()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, StartSystemTimerScanFail4, TestSize.Level1)
{
    StartSystemTimerScanFail4();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleSystemScanTimeoutSuccess, TestSize.Level1)
{
    HandleSystemScanTimeoutSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, DisconnectedTimerScanSuccess, TestSize.Level1)
{
    DisconnectedTimerScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleDisconnectedScanTimeoutSuccess, TestSize.Level1)
{
    HandleDisconnectedScanTimeoutSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: HandleDisconnectedScanTimeoutFail1
 * @tc.desc: HandleDisconnectedScanTimeout()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, HandleDisconnectedScanTimeoutFail1, TestSize.Level1)
{
    HandleDisconnectedScanTimeoutFail1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: HandleDisconnectedScanTimeoutFail2
 * @tc.desc: HandleDisconnectedScanTimeout()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, HandleDisconnectedScanTimeoutFail2, TestSize.Level1)
{
    HandleDisconnectedScanTimeoutFail2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, RestartPnoScanTimeOutSuccess, TestSize.Level1)
{
    RestartPnoScanTimeOutSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, RestartPnoScanTimeOutFail, TestSize.Level1)
{
    RestartPnoScanTimeOutFail();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, GetScanControlInfoSuccess, TestSize.Level1)
{
    GetScanControlInfoSuccess();
}

HWTEST_F(ScanServiceTest, GetScanControlInfoFail, TestSize.Level1)
{
    GetScanControlInfoFail();
}

HWTEST_F(ScanServiceTest, AllowExternScanSuccess, TestSize.Level1)
{
    AllowExternScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, AllowExternScanFail1, TestSize.Level1)
{
    AllowExternScanFail1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, AllowExternScanFail2, TestSize.Level1)
{
    AllowExternScanFail2();
}

HWTEST_F(ScanServiceTest, AllowExternScanFail3, TestSize.Level1)
{
    AllowExternScanFail3();
}

HWTEST_F(ScanServiceTest, AllowExternScanFail4, TestSize.Level1)
{
    AllowExternScanFail4();
}

HWTEST_F(ScanServiceTest, AllowSystemTimerScanSuccess, TestSize.Level1)
{
    AllowSystemTimerScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, AllowSystemTimerScanFail1, TestSize.Level1)
{
    AllowSystemTimerScanFail1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, AllowSystemTimerScanFail3, TestSize.Level1)
{
    AllowSystemTimerScanFail3();
}

HWTEST_F(ScanServiceTest, AllowSystemTimerScanFail5, TestSize.Level1)
{
    AllowSystemTimerScanFail5();
}

HWTEST_F(ScanServiceTest, AllowPnoScanSuccess, TestSize.Level1)
{
    AllowPnoScanSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess1, TestSize.Level1)
{
    GetStaSceneSuccess1();
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess2, TestSize.Level1)
{
    GetStaSceneSuccess2();
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess3, TestSize.Level1)
{
    GetStaSceneSuccess3();
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess4, TestSize.Level1)
{
    GetStaSceneSuccess4();
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess5, TestSize.Level1)
{
    GetStaSceneSuccess5();
}

HWTEST_F(ScanServiceTest, GetStaSceneSuccess6, TestSize.Level1)
{
    GetStaSceneSuccess6();
}

HWTEST_F(ScanServiceTest, GetStaSceneFail, TestSize.Level1)
{
    GetStaSceneFail();
}

HWTEST_F(ScanServiceTest, IsExternScanningSuccess, TestSize.Level1)
{
    IsExternScanningSuccess();
}

HWTEST_F(ScanServiceTest, IsExternScanningFail, TestSize.Level1)
{
    IsExternScanningFail();
}

HWTEST_F(ScanServiceTest, GetAllowBandFreqsControlInfoSuccess, TestSize.Level1)
{
    GetAllowBandFreqsControlInfoSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow24GSuccess1, TestSize.Level1)
{
    ConvertBandNotAllow24GSuccess1();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow24GSuccess2, TestSize.Level1)
{
    ConvertBandNotAllow24GSuccess2();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow24GSuccess3, TestSize.Level1)
{
    ConvertBandNotAllow24GSuccess3();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow24GSuccess4, TestSize.Level1)
{
    ConvertBandNotAllow24GSuccess4();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow24GFail, TestSize.Level1)
{
    ConvertBandNotAllow24GFail();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow5GSuccess, TestSize.Level1)
{
    ConvertBandNotAllow5GSuccess();
}

HWTEST_F(ScanServiceTest, ConvertBandNotAllow5GFail, TestSize.Level1)
{
    ConvertBandNotAllow5GFail();
}

HWTEST_F(ScanServiceTest, Delete24GhzFreqsSuccess, TestSize.Level1)
{
    Delete24GhzFreqsSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, Delete5GhzFreqsSuccess, TestSize.Level1)
{
    Delete5GhzFreqsSuccess();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, GetSavedNetworkSsidListSuccess, TestSize.Level1)
{
    GetSavedNetworkSsidListSuccess();
}

HWTEST_F(ScanServiceTest, GetSavedNetworkSsidListFail, TestSize.Level1)
{
    GetSavedNetworkSsidListFail();
}

HWTEST_F(ScanServiceTest, GetHiddenNetworkSsidListSuccess, TestSize.Level1)
{
    GetHiddenNetworkSsidListSuccess1();
}

HWTEST_F(ScanServiceTest, GetHiddenNetworkSsidListSuccess2, TestSize.Level1)
{
    GetHiddenNetworkSsidListSuccess2();
}

HWTEST_F(ScanServiceTest, GetHiddenNetworkSsidListFail, TestSize.Level1)
{
    GetHiddenNetworkSsidListFail();
}

HWTEST_F(ScanServiceTest, SetStaCurrentTimeSuccess, TestSize.Level1)
{
    SetStaCurrentTimeSuccess();
}

HWTEST_F(ScanServiceTest, AllowScanDuringScanningSuccess, TestSize.Level1)
{
    AllowScanDuringScanningSuccess();
}

HWTEST_F(ScanServiceTest, AllowScanDuringScanningFail, TestSize.Level1)
{
    AllowScanDuringScanningFail();
}

HWTEST_F(ScanServiceTest, AllowScanDuringStaSceneSuccess, TestSize.Level1)
{
    AllowScanDuringStaSceneSuccess1();
}

HWTEST_F(ScanServiceTest, AllowScanDuringStaSceneFail1, TestSize.Level1)
{
    AllowScanDuringStaSceneFail1();
}

HWTEST_F(ScanServiceTest, AllowScanDuringStaSceneFail2, TestSize.Level1)
{
    AllowScanDuringStaSceneFail2();
}

HWTEST_F(ScanServiceTest, AllowScanDuringStaSceneFail3, TestSize.Level1)
{
    AllowScanDuringStaSceneFail3();
}

HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneSuccess, TestSize.Level1)
{
    AllowScanDuringCustomSceneSuccess();
}

HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneFail1, TestSize.Level1)
{
    AllowScanDuringCustomSceneFail1();
}

HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneFail2, TestSize.Level1)
{
    AllowScanDuringCustomSceneFail2();
}

HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneFail3, TestSize.Level1)
{
    AllowScanDuringCustomSceneFail3();
}
/**
 * @tc.name: AllowExternScanByIntervalMode_001
 * @tc.desc: AllowExternScanByIntervalMode()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowExternScanByIntervalMode_001, TestSize.Level1)
{
    AllowExternScanByIntervalModeFail1();
}
/**
 * @tc.name: AllowExternScanByIntervalMode_002
 * @tc.desc: AllowExternScanByIntervalMode()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowExternScanByIntervalMode_002, TestSize.Level1)
{
    AllowExternScanByIntervalModeFail2();
}
/**
 * @tc.name: AllowExternScanByIntervalMode_003
 * @tc.desc: AllowExternScanByIntervalMode()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowExternScanByIntervalMode_003, TestSize.Level1)
{
    AllowExternScanByIntervalModeFail3();
}
/**
 * @tc.name: AllowExternScanByIntervalMode_004
 * @tc.desc: AllowExternScanByIntervalMode()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowExternScanByIntervalMode_004, TestSize.Level1)
{
    AllowExternScanByIntervalModeSuccess();
}
/**
 * @tc.name: AllowExternScanByIntervalMode_005
 * @tc.desc: AllowExternScanByIntervalMode()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowExternScanByIntervalMode_005, TestSize.Level1)
{
    AllowExternScanByIntervalModeSuccess1();
}

HWTEST_F(ScanServiceTest, PnoScanByIntervalSuccess1, TestSize.Level1)
{
    PnoScanByIntervalSuccess1();
}

HWTEST_F(ScanServiceTest, PnoScanByIntervalSuccess2, TestSize.Level1)
{
    PnoScanByIntervalSuccess2();
}

HWTEST_F(ScanServiceTest, PnoScanByIntervalSuccess3, TestSize.Level1)
{
    PnoScanByIntervalSuccess3();
}

HWTEST_F(ScanServiceTest, PnoScanByIntervalFail1, TestSize.Level1)
{
    PnoScanByIntervalFail1();
}

HWTEST_F(ScanServiceTest, SystemScanByIntervalSuccess, TestSize.Level1)
{
    SystemScanByIntervalSuccess();
}

HWTEST_F(ScanServiceTest, ExternScanByIntervalSuccess1, TestSize.Level1)
{
    ExternScanByIntervalSuccess1();
}

HWTEST_F(ScanServiceTest, ExternScanByIntervalSuccess2, TestSize.Level1)
{
    ExternScanByIntervalSuccess2();
}

HWTEST_F(ScanServiceTest, ExternScanByIntervalSuccess3, TestSize.Level1)
{
    ExternScanByIntervalSuccess3();
}

HWTEST_F(ScanServiceTest, ExternScanByIntervalSuccess4, TestSize.Level1)
{
    ExternScanByIntervalSuccess4();
}

HWTEST_F(ScanServiceTest, ExternScanByIntervalSuccess5, TestSize.Level1)
{
    ExternScanByIntervalSuccess5();
}

HWTEST_F(ScanServiceTest, AllowSingleAppScanByIntervalSuccess1, TestSize.Level1)
{
    AllowSingleAppScanByIntervalSuccess1();
}

HWTEST_F(ScanServiceTest, AllowSingleAppScanByIntervalSuccess2, TestSize.Level1)
{
    AllowSingleAppScanByIntervalSuccess2();
}

HWTEST_F(ScanServiceTest, AllowSingleAppScanByIntervalFail1, TestSize.Level1)
{
    AllowSingleAppScanByIntervalFail1();
}

HWTEST_F(ScanServiceTest, AllowFullAppScanByIntervalSuccess1, TestSize.Level1)
{
    AllowFullAppScanByIntervalSuccess1();
}

HWTEST_F(ScanServiceTest, AllowFullAppScanByIntervalSuccess2, TestSize.Level1)
{
    AllowFullAppScanByIntervalSuccess2();
}

HWTEST_F(ScanServiceTest, AllowFullAppScanByIntervalFail1, TestSize.Level1)
{
    AllowFullAppScanByIntervalFail1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalFixedSuccess1, TestSize.Level1)
{
    AllowScanByIntervalFixedSuccess1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalFixedSuccess2, TestSize.Level1)
{
    AllowScanByIntervalFixedSuccess2();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalFixedSuccess3, TestSize.Level1)
{
    AllowScanByIntervalFixedSuccess3();
}

HWTEST_F(ScanServiceTest, AllowScanByHid2dStateTest, TestSize.Level1)
{
    AllowScanByHid2dStateTest();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalFixedFail1, TestSize.Level1)
{
    AllowScanByIntervalFixedFail1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalExpSuccess, TestSize.Level1)
{
    AllowScanByIntervalExpSuccess();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalContinueSuccess1, TestSize.Level1)
{
    AllowScanByIntervalContinueSuccess1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalContinueSuccess2, TestSize.Level1)
{
    AllowScanByIntervalContinueSuccess2();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalContinueSuccess3, TestSize.Level1)
{
    AllowScanByIntervalContinueSuccess3();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalContinueFail1, TestSize.Level1)
{
    AllowScanByIntervalContinueFail1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalBlocklistSuccess1, TestSize.Level1)
{
    AllowScanByIntervalBlocklistSuccess1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalBlocklistSuccess2, TestSize.Level1)
{
    AllowScanByIntervalBlocklistSuccess2();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalBlocklistSuccess3, TestSize.Level1)
{
    AllowScanByIntervalBlocklistSuccess3();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalBlocklistFail1, TestSize.Level1)
{
    AllowScanByIntervalBlocklistFail1();
}

HWTEST_F(ScanServiceTest, AllowScanByIntervalBlocklistFail2, TestSize.Level1)
{
    AllowScanByIntervalBlocklistFail2();
}

HWTEST_F(ScanServiceTest, SingleScanFail2, TestSize.Level1)
{
    SingleScanFail2();
}

HWTEST_F(ScanServiceTest, PnoScanFail, TestSize.Level1)
{
    PnoScanFail();
}

HWTEST_F(ScanServiceTest, HandleMovingFreezeChangedTest, TestSize.Level1)
{
    HandleMovingFreezeChangedTest();
}

HWTEST_F(ScanServiceTest, HandleAutoConnectStateChangedTest, TestSize.Level1)
{
    HandleAutoConnectStateChangedTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, HandleNetworkQualityChangedTest, TestSize.Level1)
{
    HandleNetworkQualityChangedTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: WifiMaxThroughputTest
 * @tc.desc: WifiMaxThroughputTest
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, WifiMaxThroughputTest, TestSize.Level1)
{
    WifiMaxThroughputTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: RecordScanLimitInfoTest
 * @tc.desc: RecordScanLimitInfoTest()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, RecordScanLimitInfoTest, TestSize.Level1)
{
    RecordScanLimitInfoTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: IsPackageInTrustListTest
 * @tc.desc: IsPackageInTrustList()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, IsPackageInTrustListTest, TestSize.Level1)
{
    IsPackageInTrustListTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: IsAppInFilterListTest
 * @tc.desc: IsAppInFilterList()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, IsAppInFilterListTest, TestSize.Level1)
{
    IsAppInFilterListTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowScanByMovingFreeze_001
 * @tc.desc: AllowScanByMovingFreeze()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowScanByMovingFreeze_001, TestSize.Level1)
{
    AllowScanByMovingFreezeTest1();
}
/**
 * @tc.name: AllowScanByMovingFreeze_002
 * @tc.desc: AllowScanByMovingFreeze()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowScanByMovingFreeze_002, TestSize.Level1)
{
    AllowScanByMovingFreezeTest2();
}

HWTEST_F(ScanServiceTest, SetScanTrustMode_001, TestSize.Level1)
{
    SetScanTrustModeTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, ResetToNonTrustMode_001, TestSize.Level1)
{
    ResetToNonTrustModeTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, IsScanningWithParam_001, TestSize.Level1)
{
    IsScanningWithParamTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowCustomSceneCheck_001
 * @tc.desc: AllowCustomSceneCheck()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowCustomSceneCheck_001, TestSize.Level1)
{
    AllowCustomSceneCheckTest1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowCustomSceneCheck_002
 * @tc.desc: AllowCustomSceneCheck()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowCustomSceneCheck_002, TestSize.Level1)
{
    AllowCustomSceneCheckTest2();
}
/**
 * @tc.name: AllowCustomSceneCheck_003
 * @tc.desc: AllowCustomSceneCheck()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowCustomSceneCheck_003, TestSize.Level1)
{
    AllowCustomSceneCheckTest3();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowCustomSceneCheck_004
 * @tc.desc: AllowCustomSceneCheck()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowCustomSceneCheck_004, TestSize.Level1)
{
    AllowCustomSceneCheckTest4();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

/**
 * @tc.name: GetAllowBandFreqsControlInfoSuccess1
 * @tc.desc: GetAllowBandFreqsControlInfo()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, GetAllowBandFreqsControlInfoSuccess1, TestSize.Level1)
{
    GetAllowBandFreqsControlInfoSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: GetAllowBandFreqsControlInfoSuccess2
 * @tc.desc: GetAllowBandFreqsControlInfo()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, GetAllowBandFreqsControlInfoSuccess2, TestSize.Level1)
{
    GetAllowBandFreqsControlInfoSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowScanDuringCustomSceneSuccess2
 * @tc.desc: AllowScanDuringCustomScene()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneSuccess1, TestSize.Level1)
{
    AllowScanDuringCustomSceneSuccess1();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}
/**
 * @tc.name: AllowScanDuringCustomSceneSuccess2
 * @tc.desc: AllowScanDuringCustomScene()
 * @tc.type: FUNC
 * @tc.require: issue
*/
HWTEST_F(ScanServiceTest, AllowScanDuringCustomSceneSuccess2, TestSize.Level1)
{
    AllowScanDuringCustomSceneSuccess2();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, GetScanControlInfoTest, TestSize.Level1)
{
    GetScanControlInfoTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, ClearScanTrustSceneIdsTest, TestSize.Level1)
{
    ClearScanTrustSceneIdsTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, ApplyTrustListPolicyTest, TestSize.Level1)
{
    ApplyTrustListPolicyTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, SetNetworkInterfaceUpDownTest, TestSize.Level1)
{
    SetNetworkInterfaceUpDownTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, SystemScanConnectedPolicyTest, TestSize.Level1)
{
    SystemScanConnectedPolicyTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, SystemScanDisconnectedPolicyTest, TestSize.Level1)
{
    SystemScanDisconnectedPolicyTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(ScanServiceTest, OnWifiCountryCodeChangedTest, TestSize.Level1)
{
    OnWifiCountryCodeChangedTest();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

} // namespace Wifi
} // namespace OHOS
