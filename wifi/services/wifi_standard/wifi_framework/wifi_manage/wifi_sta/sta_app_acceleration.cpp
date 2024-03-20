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

#include "sta_app_acceleration.h"
#include "wifi_logger.h"
#include "wifi_sta_hal_interface.h"
#include "wifi_common_util.h"
#include "wifi_app_parser.h"
#include "wifi_settings.h"
#include "app_mgr_client.h"
#include "app_network_speed_limit_service.h"

namespace OHOS {
namespace Wifi {

DEFINE_WIFILOG_LABEL("StaAppAcceleration");
const std::string CLASS_NAME = "StaAppAcceleration";

constexpr const int POWER_SAVE_ENABLE = 3;
constexpr const int POWER_SAVE_DISABLE = 4;
constexpr const int GAME_BOOST_ENABLE = 1;
constexpr const int GAME_BOOST_DISABLE = 0;
constexpr const int BOOST_UDP_TYPE = 17;

StaAppAcceleration::StaAppAcceleration(int instId) : gameBoostingFlag(false)
{}

StaAppAcceleration::~StaAppAcceleration()
{}

StaAppAcceleration &StaAppAcceleration::GetInstance()
{
    static StaAppAcceleration gStaAppAcceleration;
    return gStaAppAcceleration;
}

ErrCode StaAppAcceleration::InitAppAcceleration()
{
    m_staCallback.callbackModuleName = CLASS_NAME;
    m_staCallback.OnStaConnChanged = DealStaConnChanged;

    return WIFI_OPT_SUCCESS;
}

void StaAppAcceleration::DealStaConnChanged(OperateResState state, const WifiLinkedInfo &info, int instId)
{
    if (state == OperateResState::DISCONNECT_DISCONNECTED) {
        StaAppAcceleration::GetInstance().StopAllAppAcceleration();
    }
}

void StaAppAcceleration::HandleScreenStatusChanged(int screenState)
{
    WIFI_LOGI("Enter HandleScreenStatusChanged.\n");

    if (screenState == MODE_STATE_OPEN) {
        SetPmMode(POWER_SAVE_DISABLE);
    } else if (screenState == MODE_STATE_CLOSE) {
        SetPmMode(POWER_SAVE_ENABLE);
    } else {
        WIFI_LOGI("mode not handle.\n");
    }
}
void StaAppAcceleration::HandleForegroundAppChangedAction(const std::string &bundleName,
    int uid, int pid, const int state)
{
    if (state == static_cast<int>(AppExecFwk::AppProcessState::APP_STATE_FOREGROUND)) {
        if (AppParser::GetInstance().IsLowLatencyApp(bundleName)) {
            WIFI_LOGI("target app on the foreground.");
            StartGameBoost(uid);
        } else {
            StopGameBoost(uid);
        }
    }
    return;
}

void StaAppAcceleration::SetPmMode(int mode)
{
    if (mode != POWER_SAVE_DISABLE && POWER_SAVE_ENABLE) {
        WIFI_LOGI("Unsupported mode %{public}d.", mode);
        return;
    }

    WifiLinkedInfo linkedInfo;
    WifiSettings::GetInstance().GetLinkedInfo(linkedInfo);
    WifiErrorNo ret = WifiStaHalInterface::GetInstance().SetPmMode(linkedInfo.frequency, mode);
    if (ret != 0) {
        WIFI_LOGE("SetPmMode failed, ret = %{public}d.", ret);
        return;
    }
}

void StaAppAcceleration::StartGameBoost(int uid)
{
    if (!gameBoostingFlag) {
        WIFI_LOGI("start game boost.\n");
        SetGameBoostMode(GAME_BOOST_ENABLE, uid, BOOST_UDP_TYPE, BG_LIMIT_LEVEL_3);
        gameBoostingFlag = true;
        return;
    } else {
        WIFI_LOGE("game boost has started, not handle.\n");
        return;
    }
}

void StaAppAcceleration::StopGameBoost(int uid)
{
    if (gameBoostingFlag) {
        SetGameBoostMode(GAME_BOOST_DISABLE, uid, BOOST_UDP_TYPE, BG_LIMIT_OFF);
        gameBoostingFlag = false;
    }
}

void StaAppAcceleration::SetGameBoostMode(int enable, int uid, int type, int limitMode)
{
    HighPriorityTransmit(uid, type, enable);
    AppNetworkSpeedLimitService::GetInstance().LimitSpeed(BG_LIMIT_CONTROL_ID_GAME, limitMode);
}

void StaAppAcceleration::HighPriorityTransmit(int uid, int protocol, int enable)
{
    WIFI_LOGI("Enter HighPriorityTransmit.\n");
    WifiErrorNo ret = WifiStaHalInterface::GetInstance().SetDpiMarkRule(uid, protocol, enable);
    if (ret != 0) {
        WIFI_LOGE("HighPriorityTransmit failed, ret = %{public}d.", ret);
        return;
    }
}

void StaAppAcceleration::StopAllAppAcceleration()
{
    WIFI_LOGI("Wifi disconnected, stop game boost.\n");
    SetPmMode(POWER_SAVE_ENABLE);
    HighPriorityTransmit(UNKNOWN_UID, BOOST_UDP_TYPE, GAME_BOOST_DISABLE);
    AppNetworkSpeedLimitService::GetInstance().LimitSpeed(BG_LIMIT_CONTROL_ID_GAME, BG_LIMIT_OFF);
}

} // namespace Wifi
} // namespace OHOS