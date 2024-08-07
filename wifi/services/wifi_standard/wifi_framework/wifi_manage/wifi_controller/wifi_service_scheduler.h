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

#ifndef OHOS_WIFI_SERVICE_SCHEDULER_H
#define OHOS_WIFI_SERVICE_SCHEDULER_H

#include <string>
#include "wifi_logger.h"
#include "wifi_errcode.h"
#include "sta_service_callback.h"
#include "iscan_service_callbacks.h"
#ifdef FEATURE_AP_SUPPORT
#include "i_ap_service_callbacks.h"
#endif
#ifdef FEATURE_P2P_SUPPORT
#include "ip2p_service_callbacks.h"
#endif
#include "wifi_internal_msg.h"
#include "wifi_controller_define.h"
#include "wifi_service_manager.h"
#include "state.h"

namespace OHOS {
namespace Wifi {
class WifiServiceScheduler {
public:
    static WifiServiceScheduler &GetInstance();
    explicit WifiServiceScheduler();
    ~WifiServiceScheduler();
    ErrCode AutoStartStaService(int instId, std::string &staIfName);
    ErrCode AutoStopStaService(int instId);
    ErrCode AutoStartScanOnly(int instId, std::string &staIfName);
    ErrCode AutoStopScanOnly(int instId, bool setIfaceDown);
    ErrCode AutoStartSemiStaService(int instId, std::string &staIfName);
    ErrCode AutoStartP2pService(int instId);
    ErrCode AutoStopP2pService();
    ErrCode AutoStartApService(int instId, std::string &softApIfName);
    ErrCode AutoStopApService(int instId);
    void DispatchWifiOpenRes(OperateResState state, int instId);
    void DispatchWifiSemiActiveRes(OperateResState state, int instId);
    void DispatchWifiCloseRes(OperateResState state, int instId);
    void ClearStaIfaceNameMap(int instId);
    void ClearP2pIfaceNameMap(int instId);
    void ClearSoftApIfaceNameMap(int instId)
private:
    ErrCode PreStartWifi(int instId, std::string &staIfName);
    ErrCode PostStartWifi(int instId);
    ErrCode InitStaService(IStaService *pService);
    
#ifdef FEATURE_P2P_SUPPORT
    ErrCode InitP2pService();
#endif
#ifdef FEATURE_SELF_CURE_SUPPORT
    ErrCode StartSelfCureService(int instId);
#endif
    ErrCode TryToStartApService(int instId);
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    void StaIfaceDestoryCallback(std::string &destoryIfaceName, int createIfaceType);
    void P2pIfaceDestoryCallback(std::string &destoryIfaceName, int createIfaceType);
    void SoftApIfaceDestoryCallback(std::string &destoryIfaceName, int createIfaceType);
    void OnRssiReportCallback(int index, int antRssi);
    std::map<int, std::string> staIfaceNameMap;
    std::map<int, std::string> p2pIfaceNameMap;
    std::map<int, std::string> softApIfaceNameMap;
#endif
#ifdef FEATURE_P2P_SUPPORT
    IP2pServiceCallbacks mP2pCallback;
#endif
    std::mutex mutex;
    std::mutex staIfaceNameMapMutex;
    std::mutex p2pIfaceNameMapMutex;
    std::mutex softApIfaceNameMapMutex;
};
}
}
#endif