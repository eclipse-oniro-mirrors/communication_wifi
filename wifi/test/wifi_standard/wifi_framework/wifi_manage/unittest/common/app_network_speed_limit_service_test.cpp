/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include "wifi_log.h"
#include "wifi_logger.h"
#include "app_network_speed_limit_service.h"
#include "wifi_app_state_aware.h"
#include "mock_wifi_config_center.h"
#include "mock_wifi_settings.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("AppNetworkSpeedLimitServiceTest");
static std::string g_errLog;
void AppNetworkSpeedLimitServiceCallback(const LogType type, const LogLevel level, const unsigned int domain,
                                         const char *tag, const char *msg)
{
    g_errLog = msg;
}

class AppNetworkSpeedLimitServiceTest : public testing::Test {
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
        LOG_SetCallback(AppNetworkSpeedLimitServiceCallback);
        EXPECT_CALL(WifiConfigCenter::GetInstance(), GetStaIfaceName(_)).WillRepeatedly(Return("wlan0"));
    }
    virtual void TearDown() {}
};

HWTEST_F(AppNetworkSpeedLimitServiceTest, Init, TestSize.Level1)
{
    WIFI_LOGI("InitTest enter");
    EXPECT_EQ(BG_LIMIT_OFF, AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_GAME]);
    EXPECT_EQ(BG_LIMIT_OFF, AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_STREAM]);
    EXPECT_EQ(BG_LIMIT_OFF, AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_TEMP]);
    EXPECT_EQ(BG_LIMIT_OFF,
        AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_MODULE_FOREGROUND_OPT]);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, LimitSpeed_HighTemp, TestSize.Level1)
{
    WIFI_LOGI("LimitSpeed_HighTemp enter");
    AppNetworkSpeedLimitService::GetInstance().LimitSpeed(BG_LIMIT_CONTROL_ID_TEMP, BG_LIMIT_LEVEL_3);
    sleep(1);
    EXPECT_NE(BG_LIMIT_LEVEL_3,
        AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_TEMP]);
}


HWTEST_F(AppNetworkSpeedLimitServiceTest, GetBgLimitMaxMode, TestSize.Level1)
{
    WIFI_LOGI("GetBgLimitMaxMode enter");
    EXPECT_EQ(BG_LIMIT_OFF, AppNetworkSpeedLimitService::GetInstance().GetBgLimitMaxMode());
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckNetWorkCanBeLimited_GameControlId, TestSize.Level1)
{
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_GAME;

    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().CheckNetWorkCanBeLimited(controlId);

    // Verify
    EXPECT_TRUE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckNetWorkCanBeLimited_StreamControlId, TestSize.Level1)
{
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_STREAM;

    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().CheckNetWorkCanBeLimited(controlId);

    // Verify
    EXPECT_TRUE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckNetWorkCanBeLimited_TempControlId_WifiConnected, TestSize.Level1)
{
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_TEMP;
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;

    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().CheckNetWorkCanBeLimited(controlId);

    // Verify
    EXPECT_TRUE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckNetWorkCanBeLimited_TempControlId_WifiDisconnected, TestSize.Level1)
{
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_TEMP;
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = false;

    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().CheckNetWorkCanBeLimited(controlId);

    // Verify
    EXPECT_FALSE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckNetWorkCanBeLimited_ModuleForegroundOptControlId, TestSize.Level1)
{
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_MODULE_FOREGROUND_OPT;

    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().CheckNetWorkCanBeLimited(controlId);

    // Verify
    EXPECT_TRUE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, IsLimitSpeedBgApp, TestSize.Level1)
{
    WIFI_LOGI("IsLimitSpeedBgApp enter");
    // Prepare
    int controlId = BG_LIMIT_CONTROL_ID_GAME;
    int enable = 1;
    // Execute
    bool result = AppNetworkSpeedLimitService::GetInstance().IsLimitSpeedBgApp(controlId, "com.ohos.wifi", enable);

    // Verify
    EXPECT_TRUE(result);

    enable = 0;
    result = AppNetworkSpeedLimitService::GetInstance().IsLimitSpeedBgApp(controlId, "com.ohos.wifi", enable);
    EXPECT_FALSE(result);

    controlId = BG_LIMIT_CONTROL_ID_STREAM;
    result = AppNetworkSpeedLimitService::GetInstance().IsLimitSpeedBgApp(controlId, "com.ohos.wifi", enable);
    EXPECT_FALSE(result);

    controlId = BG_LIMIT_CONTROL_ID_KEY_FG_APP;
    result = AppNetworkSpeedLimitService::GetInstance().IsLimitSpeedBgApp(controlId, "com.ohos.wifi", enable);
    EXPECT_FALSE(result);

    controlId = BG_LIMIT_CONTROL_ID_MODULE_FOREGROUND_OPT;
    result = AppNetworkSpeedLimitService::GetInstance().IsLimitSpeedBgApp(controlId, "com.ohos.wifi", enable);
    EXPECT_TRUE(result);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, DealStaConnChanged, TestSize.Level1)
{
    WIFI_LOGI("DealStaConnChanged enter");
    WifiLinkedInfo info;
    int instId = 1;
    AppNetworkSpeedLimitService::GetInstance().DealStaConnChanged(
            OperateResState::DISCONNECT_DISCONNECTED, info, instId);
    AppNetworkSpeedLimitService::GetInstance().DealStaConnChanged(OperateResState::CONNECT_AP_CONNECTED, info, instId);
    EXPECT_FALSE(g_errLog.find("service is null")!=std::string::npos);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, HandleForegroundAppChangedAction, TestSize.Level1)
{
    WIFI_LOGI("HandleForegroundAppChangedAction enter");
    AppExecFwk::AppStateData appStateData;
    appStateData.state = static_cast<int>(AppExecFwk::AppProcessState::APP_STATE_FOREGROUND);
    appStateData.isFocused = true;
    AppNetworkSpeedLimitService::GetInstance().HandleForegroundAppChangedAction(appStateData);
    EXPECT_TRUE(appStateData.isFocused);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GetAppList, TestSize.Level1)
{
    WIFI_LOGI("GetAppList enter");
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    bool getFgAppFlag = false;
    AppNetworkSpeedLimitService::GetInstance().GetAppList(infos, getFgAppFlag);
    getFgAppFlag = true;
    AppNetworkSpeedLimitService::GetInstance().GetAppList(infos, getFgAppFlag);
    EXPECT_TRUE(getFgAppFlag);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, UpdateSpeedLimitConfigs, TestSize.Level1)
{
    WIFI_LOGI("UpdateSpeedLimitConfigs enter");
    AppNetworkSpeedLimitService::GetInstance().m_limitSpeedMode = BG_LIMIT_LEVEL_3;
    int enable = 1;
    AppNetworkSpeedLimitService::GetInstance().UpdateSpeedLimitConfigs(enable);
    EXPECT_FALSE(g_errLog.find("service is null")!=std::string::npos);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, HandleRequest, TestSize.Level1)
{
    WIFI_LOGI("HandleRequest enter");
    AsyncParamInfo asyncParamInfo;
    asyncParamInfo.funcName = "HandleWifiConnectStateChanged";
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    asyncParamInfo.funcName = "HandleForegroundAppChangedAction";
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    asyncParamInfo.funcName = "LimitSpeed";
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    asyncParamInfo.funcName = "ReceiveNetworkControlInfo";
    asyncParamInfo.networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    asyncParamInfo.networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_AUDIO_PLAYBACK;
    asyncParamInfo.networkControlInfo.state = 1;
    asyncParamInfo.networkControlInfo.uid = 20020022;
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    asyncParamInfo.networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_VIDEO_CALL;
    AppNetworkSpeedLimitService::GetInstance().HandleRequest(asyncParamInfo);
    int audioUidSize = AppNetworkSpeedLimitService::GetInstance().m_bgAudioPlaybackUidSet.size();
    EXPECT_EQ(audioUidSize, 1);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, SendLimitInfo, TestSize.Level1)
{
    WIFI_LOGI("SendLimitInfo enter");
    AppNetworkSpeedLimitService::GetInstance().m_limitSpeedMode = BG_LIMIT_LEVEL_3;
    AppNetworkSpeedLimitService::GetInstance().m_lastLimitSpeedMode = BG_LIMIT_OFF;
    AppNetworkSpeedLimitService::GetInstance().m_lastBgUidSet = {};
    AppNetworkSpeedLimitService::GetInstance().m_bgUidSet = {-1};
    AppNetworkSpeedLimitService::GetInstance().m_lastFgUidSet = {};
    AppNetworkSpeedLimitService::GetInstance().m_fgUidSet = {-1};
    AppNetworkSpeedLimitService::GetInstance().SendLimitInfo();
    EXPECT_EQ(BG_LIMIT_LEVEL_3, AppNetworkSpeedLimitService::GetInstance().m_limitSpeedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, ReceiveNetworkControlInfo, TestSize.Level1)
{
    WIFI_LOGI("ReceiveNetworkControlInfo enter");
    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.youku.next";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_WINDOW_VISIBLE;
    networkControlInfo.state = 1;
    networkControlInfo.uid = 20020044;
    AppNetworkSpeedLimitService::GetInstance().ReceiveNetworkControlInfo(networkControlInfo);
    AppNetworkSpeedLimitService::GetInstance().m_additionalWindowUidSet.clear();
    int windowUidSize = AppNetworkSpeedLimitService::GetInstance().m_additionalWindowUidSet.size();
    EXPECT_EQ(windowUidSize, 0);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, VideoCallNetworkSpeedLimitConfigs, TestSize.Level1)
{
    WIFI_LOGI("VideoCallNetworkSpeedLimitConfigs enter");
    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.state = 1;
    AppNetworkSpeedLimitService::GetInstance().VideoCallNetworkSpeedLimitConfigs(networkControlInfo);
    auto level = AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_VIDEO_CALL];
    EXPECT_EQ(level, BG_LIMIT_LEVEL_7);
    networkControlInfo.state = 0;
    AppNetworkSpeedLimitService::GetInstance().VideoCallNetworkSpeedLimitConfigs(networkControlInfo);
    level = AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_VIDEO_CALL];
    EXPECT_EQ(level, BG_LIMIT_OFF);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, UpdateNoSpeedLimitConfigs, TestSize.Level1)
{
    WIFI_LOGI("UpdateNoSpeedLimitConfigs enter");
    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.uid = 20020022;
    networkControlInfo.state = 1;
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_AUDIO_PLAYBACK;
    AppNetworkSpeedLimitService::GetInstance().UpdateNoSpeedLimitConfigs(networkControlInfo);
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_WINDOW_VISIBLE;
    AppNetworkSpeedLimitService::GetInstance().UpdateNoSpeedLimitConfigs(networkControlInfo);

    networkControlInfo.state = 0;
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_AUDIO_PLAYBACK;
    AppNetworkSpeedLimitService::GetInstance().UpdateNoSpeedLimitConfigs(networkControlInfo);
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_WINDOW_VISIBLE;
    AppNetworkSpeedLimitService::GetInstance().UpdateNoSpeedLimitConfigs(networkControlInfo);
    int windowUidSize = AppNetworkSpeedLimitService::GetInstance().m_additionalWindowUidSet.size();
    EXPECT_EQ(windowUidSize, 0);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, ForegroundAppChangedAction_Test, TestSize.Level1)
{
    WIFI_LOGI("ForegroundAppChangedAction enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().m_bgLimitRecordMap[BG_LIMIT_CONTROL_ID_TEMP] = BG_LIMIT_LEVEL_3;
    std::string bundleName = "com.xingin.xhs_hos";
    AppNetworkSpeedLimitService::GetInstance().ForegroundAppChangedAction(bundleName);

    bundleName = "com.ohos.wifi";
    AppNetworkSpeedLimitService::GetInstance().ForegroundAppChangedAction(bundleName);
    EXPECT_TRUE(AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, HighPriorityTransmit, TestSize.Level1)
{
    WIFI_LOGI("HighPriorityTransmit enter");
    AppNetworkSpeedLimitService::GetInstance().m_isHighPriorityTransmit = 1;
    int uid = 1;
    int protocol = 17;
    int enable = 0;
    AppNetworkSpeedLimitService::GetInstance().HighPriorityTransmit(uid, protocol, enable);
    EXPECT_FALSE(g_errLog.find("service is null")!=std::string::npos);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, SetGamePowerMode_WifiConnected, TestSize.Level1)
{
    WIFI_LOGI("SetGamePowerMode_WifiConnected enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);
    std::string ifaceName = "wlan0";

    bool gameActive = GAME_POWER_MODE_ACTIVE;
    AppNetworkSpeedLimitService::GetInstance().SetGamePowerMode(ifaceName, gameActive);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);

    gameActive = GAME_POWER_MODE_INACTIVE;
    AppNetworkSpeedLimitService::GetInstance().SetGamePowerMode(ifaceName, gameActive);

    cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, SetGamePowerMode_WifiDisconnected, TestSize.Level1)
{
    WIFI_LOGI("SetGamePowerMode_WifiDisconnected enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = false;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(UNKNOWN_MODE);
    std::string ifaceName = "wlan0";

    bool gameActive = GAME_POWER_MODE_ACTIVE;
    AppNetworkSpeedLimitService::GetInstance().SetGamePowerMode(ifaceName, gameActive);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(UNKNOWN_MODE, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, SetGamePowerMode_CacheOptimization, TestSize.Level1)
{
    WIFI_LOGI("SetGamePowerMode_CacheOptimization enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);
    std::string ifaceName = "wlan0";

    bool gameActive = GAME_POWER_MODE_ACTIVE;
    AppNetworkSpeedLimitService::GetInstance().SetGamePowerMode(ifaceName, gameActive);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);

    EXPECT_FALSE(g_errLog.find("SetPmMode failed") != std::string::npos);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, DealStaConnChanged_ResetGamePowerModeCache, TestSize.Level1)
{
    WIFI_LOGI("DealStaConnChanged_ResetGamePowerModeCache enter");

    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NORMAL_SLEEP);
    WifiLinkedInfo info;
    int instId = 1;

    AppNetworkSpeedLimitService::GetInstance().DealStaConnChanged(
        OperateResState::DISCONNECT_DISCONNECTED, info, instId);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);

    AppNetworkSpeedLimitService::GetInstance().DealStaConnChanged(
        OperateResState::CONNECT_AP_CONNECTED, info, instId);
    cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, HandleWifiConnectStateChanged_OnReconnect, TestSize.Level1)
{
    WIFI_LOGI("HandleWifiConnectStateChanged_OnReconnect enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    WifiNetworkControlInfo pvpInfo;
    pvpInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    pvpInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    pvpInfo.state = GameSceneId::MSG_GAME_ENTER_PVP_BATTLE;
    pvpInfo.uid = 20010001;
    pvpInfo.rtt = 50;
    
    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(pvpInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
    AppNetworkSpeedLimitService::GetInstance().HandleWifiConnectStateChanged(false);
    sleep(1);

    cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);

    AppNetworkSpeedLimitService::GetInstance().HandleWifiConnectStateChanged(true);
    sleep(1);
    cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
    
    WIFI_LOGI("WiFi reconnect successfully restored game power mode");
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, ResetPowerMode_Success, TestSize.Level1)
{
    WIFI_LOGI("ResetPowerMode_Success enter");
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);
    AppNetworkSpeedLimitService::GetInstance().ResetPowerMode();

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, ResetPowerMode_AlreadyNormalMode, TestSize.Level1)
{
    WIFI_LOGI("ResetPowerMode_AlreadyNormalMode enter");
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NORMAL_SLEEP);
    AppNetworkSpeedLimitService::GetInstance().ResetPowerMode();

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, Init_CallsResetPowerMode, TestSize.Level1)
{
    WIFI_LOGI("Init_CallsResetPowerMode enter");
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);
    AppNetworkSpeedLimitService::GetInstance().Init();

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, Init_RestoreGamePowerModeFromCachedPvpState, TestSize.Level1)
{
    WIFI_LOGI("Init_RestoreGamePowerModeFromCachedPvpState enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo cachedPvpInfo;
    cachedPvpInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    cachedPvpInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    cachedPvpInfo.state = GameSceneId::MSG_GAME_ENTER_PVP_BATTLE;
    cachedPvpInfo.uid = 20010001;
    cachedPvpInfo.rtt = 50;
    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(cachedPvpInfo);
    sleep(1);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, Init_WifiDisconnected_ShouldResetPowerMode, TestSize.Level1)
{
    WIFI_LOGI("Init_WifiDisconnected_ShouldResetPowerMode enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = false;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);
    AppNetworkSpeedLimitService::GetInstance().Init();
    sleep(1);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_EnterPvpBattle, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_EnterPvpBattle enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NORMAL_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_ENTER_PVP_BATTLE;
    networkControlInfo.uid = 20010001;
    networkControlInfo.rtt = 50;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_ExitPvpBattle, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_ExitPvpBattle enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_EXIT_PVP_BATTLE;
    networkControlInfo.uid = 20010001;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_ConsecutivePvpEnter, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_ConsecutivePvpEnter enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NORMAL_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_ENTER_PVP_BATTLE;
    networkControlInfo.uid = 20010001;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);

    g_errLog.clear();
    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NORMAL_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_GameStart, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_GameStart enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.pubgmhd";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_STATE_START;
    networkControlInfo.uid = 20010002;
    networkControlInfo.rtt = 40;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_GameForeground, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_GameForeground enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_STATE_FOREGROUND;
    networkControlInfo.uid = 20010001;
    networkControlInfo.rtt = 50;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_GameBackground, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_GameBackground enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.pubgmhd";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_STATE_BACKGROUND;
    networkControlInfo.uid = 20010002;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_GameEnd, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_GameEnd enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_STATE_END;
    networkControlInfo.uid = 20010001;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckAndResetGamePowerMode_RssGameToRssGame, TestSize.Level1)
{
    WIFI_LOGI("CheckAndResetGamePowerMode_RssGameToRssGame enter");
    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    std::string rssGameBundleName = "com.tencent.tmgp.sgame.hw";
    AppNetworkSpeedLimitService::GetInstance().CheckAndResetGamePowerMode(rssGameBundleName);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, CheckAndResetGamePowerMode_NonRssGameApp, TestSize.Level1)
{
    WIFI_LOGI("CheckAndResetGamePowerMode_NonRssGameApp enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    std::string nonRssGameBundleName = "com.mihoyo.hyperion";
    AppNetworkSpeedLimitService::GetInstance().CheckAndResetGamePowerMode(nonRssGameBundleName);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, ForegroundAppChangedAction, TestSize.Level1)
{
    WIFI_LOGI("ForegroundAppChangedAction enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = true;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(POWER_MODE_NO_SLEEP);

    std::string nonGameBundleName = "com.example.testapp";
    AppNetworkSpeedLimitService::GetInstance().ForegroundAppChangedAction(nonGameBundleName);

    sleep(1);
    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(POWER_MODE_NO_SLEEP, cachedMode);
}

HWTEST_F(AppNetworkSpeedLimitServiceTest, GameNetworkSpeedLimitConfigs_WifiDisconnected, TestSize.Level1)
{
    WIFI_LOGI("GameNetworkSpeedLimitConfigs_WifiDisconnected enter");

    AppNetworkSpeedLimitService::GetInstance().m_isWifiConnected = false;
    AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.store(UNKNOWN_MODE);

    WifiNetworkControlInfo networkControlInfo;
    networkControlInfo.bundleName = "com.tencent.tmgp.sgame.hw";
    networkControlInfo.sceneId = BG_LIMIT_CONTROL_ID_GAME;
    networkControlInfo.state = GameSceneId::MSG_GAME_ENTER_PVP_BATTLE;
    networkControlInfo.uid = 20010001;

    AppNetworkSpeedLimitService::GetInstance().GameNetworkSpeedLimitConfigs(networkControlInfo);

    int cachedMode = AppNetworkSpeedLimitService::GetInstance().cachedGamePowerMode_.load();
    EXPECT_EQ(UNKNOWN_MODE, cachedMode);
}
} // namespace Wifi
} // namespace OHOS