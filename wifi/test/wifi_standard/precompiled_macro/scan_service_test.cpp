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
#include "Mock/mock_wifi_manager.h"
#include "mock_wifi_config_center.h"
#include "Mock/mock_wifi_settings.h"
#include "Mock/mock_scan_state_machine.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

constexpr int TWO = 2;
constexpr int FOUR = 4;
constexpr int STATUS = 17;

namespace OHOS {
namespace Wifi {

class ScanServiceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
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
 
    void SystemScanByIntervalSuccess()
    {
        int expScanCount = 1;
        int interval = 1;
        const int constTest = 2;
        int count = constTest;
        EXPECT_EQ(pScanService->SystemScanByInterval(expScanCount, interval, count), true);
    }
    void SetEnhanceServiceTest()
    {
        IEnhanceService* enhanceService = nullptr;
        pScanService->SetEnhanceService(enhanceService);
    }

    void StopPnoScanTest()
    {
        pScanService->isPnoScanBegined = true;
        pScanService->StopPnoScan();
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
};

HWTEST_F(ScanServiceTest, SystemScanByIntervalSuccess, TestSize.Level1)
{
    SystemScanByIntervalSuccess();
}

HWTEST_F(ScanServiceTest, SetEnhanceServiceTest, TestSize.Level1)
{
    SetEnhanceServiceTest();
}

HWTEST_F(ScanServiceTest, GetScanControlInfoSuccess, TestSize.Level1)
{
    GetScanControlInfoSuccess();
}

HWTEST_F(ScanServiceTest, GetScanControlInfoFail, TestSize.Level1)
{
    GetScanControlInfoFail();
}

HWTEST_F(ScanServiceTest, StopPnoScanTest, TestSize.Level1)
{
    StopPnoScanTest();
}

HWTEST_F(ScanServiceTest, AllowExternScanSuccess, TestSize.Level1)
{
    AllowExternScanSuccess();
}

HWTEST_F(ScanServiceTest, AllowExternScanFail1, TestSize.Level1)
{
    AllowExternScanFail1();
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
}  // namespace Wifi
}  // namespace OHOS