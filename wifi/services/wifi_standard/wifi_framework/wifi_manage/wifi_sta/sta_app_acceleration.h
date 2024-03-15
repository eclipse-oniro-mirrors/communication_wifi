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

#ifndef OHOS_WIFI_STA_APP_ACCELERATION_H
#define OHOS_WIFI_STA_APP_ACCELERATION_H

#include "wifi_errcode.h"
#include "define.h"
#include "appmgr/app_mgr_interface.h"
#include "sta_service_callback.h"

namespace OHOS {
namespace Wifi {

typedef enum {
    BG_LIMIT_OFF = 0,
    BG_LIMIT_LEVEL_1,
    BG_LIMIT_LEVEL_2,
    BG_LIMIT_LEVEL_3,
    BG_LIMIT_LEVEL_4,
    BG_LIMIT_LEVEL_5,
    BG_LIMIT_LEVEL_6,
    BG_LIMIT_LEVEL_7,
} BgLimitLevel;

typedef enum {
    SET_BG_UID = 0,
    SET_BG_PID,
    SET_FG_UID,
} BgLimitType;

typedef enum {
    BG_LIMIT_CONTROL_ID_GAME = 1,
    BG_LIMIT_CONTROL_ID_STREAM,
    BG_LIMIT_CONTROL_ID_TEMP,
}BgLimitControl;

class StaAppAcceleration {
public:
    explicit StaAppAcceleration(int instId = 0);
    ~StaAppAcceleration();
    static StaAppAcceleration &GetInstance();
    ErrCode InitAppAcceleration();
    void HandleScreenStatusChanged(int screenState);
    void HandleForegroundAppChangedAction(const std::string &bundleName,
        const int uid, const int pid, const int state);

private:
    static void DealStaConnChanged(OperateResState state, const WifiLinkedInfo &info, int instId = 0);
    void SetPmMode(int mode);
    void StartGameBoost(int uid);
    void StopGameBoost(int uid);
    void SetGameBoostMode(int enable, int uid, int type, int limitMode);
    void HighPriorityTransmit(int uid, int protocol, int enable);
    void LimitedSpeed(int controlId, int enable, int limitMode);
    int GetBgLimitMaxMode();
    void SetBgLimitIdList(std::vector<int> idList, int size, int type);
    ErrCode GetAppList(std::vector<AppExecFwk::RunningProcessInfo> &appList, bool getFgAppFlag);
    void StopAllAppAcceleration();

private:
    StaServiceCallback m_staCallback;
    bool gameBoostingFlag;
    std::map<int, int> mBgLimitRecordMap;
};

} // namespace Wifi
} // namespace OHOS
#endif
