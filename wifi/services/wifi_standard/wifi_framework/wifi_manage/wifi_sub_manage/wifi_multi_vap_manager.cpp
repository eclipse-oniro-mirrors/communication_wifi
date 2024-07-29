/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ARCH_LITE
#include "wifi_multi_vap_manager.h"
#include "wifi_manager.h"
#include "wifi_service_manager.h"
#include "wifi_config_center.h"
#include "wifi_notification_util.h"
#include "wifi_ap_hal_interface.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_LABEL("WifiMultiVapManager");

namespace OHOS {
namespace Wifi {
bool WifiMultiVapManager::CheckCanConnectDevice()
{
    // to be implemented
    return true;
}

bool WifiMultiVapManager::CheckCanUseP2p()
{
    // to be implemented
    return true;
}

bool WifiMultiVapManager::CheckCanUseSoftAp()
{
    // to be implemented
    return true;
}

bool WifiMultiVapManager::CheckStaConnected()
{
    for (int i = 0; i < STA_INSTANCE_MAX_NUM; ++i) {
        WifiLinkedInfo linkInfo;
        WifiConfigCenter::GetInstance().GetLinkedInfo(linkInfo, i);
        WIFI_LOGI("CheckStaConnected: Instance %{public}d sta connect state is %{public}d", i, linkInfo.connState);
        if (linkInfo.connState == ConnState::CONNECTING || linkInfo.connState == ConnState::AUTHENTICATING
            || linkInfo.connState == ConnState::OBTAINING_IPADDR || linkInfo.connState == ConnState::CONNECTED) {
            return true;
        }
    }

    WIFI_LOGI("CheckStaConnected: Sta is not connected!");
    return false;
}

bool WifiMultiVapManager::CheckP2pConnected()
{
    WifiP2pLinkedInfo p2pLinkedInfo;
    WifiConfigCenter::GetInstance().GetP2pInfo(p2pLinkedInfo);
    if (p2pLinkedInfo.GetConnectState() == P2pConnectedState::P2P_CONNECTED) {
        WIFI_LOGI("CheckP2pConnected: P2p is connected!");
        return true;
    }

    WifiP2pGroupInfo group = WifiConfigCenter::GetInstance().GetCurrentP2pGroupInfo();
    if (group.GetFrequency() > 0) {
        WIFI_LOGI("CheckP2pConnected: P2p is created group!");
        return true;
    }

    IEnhanceService *pEnhanceService = WifiServiceManager::GetInstance().GetEnhanceServiceInst();
    if (pEnhanceService && pEnhanceService->CheckChbaConncted()) {
        WIFI_LOGI("CheckP2pConnected: Chba is connected!");
        return true;
    }

    WIFI_LOGI("CheckP2pConnected: P2p is not connected!");
    return false;
}

bool WifiMultiVapManager::CheckSoftApStarted()
{
    for (int i = 0; i < AP_INSTANCE_MAX_NUM; ++i) {
        int state = WifiConfigCenter::GetInstance().GetHotspotState(i);
        WIFI_LOGI("CheckSoftApStarted: Instance %{public}d ap start state is %{public}d", i, state);
        if (state == static_cast<int>(ApState::AP_STATE_STARTING)
            || state == static_cast<int>(ApState::AP_STATE_STARTED)) {
            return true;
        }
    }

    WIFI_LOGI("CheckSoftApStarted: SoftAp is not Started!");
    return false;
}

void WifiMultiVapManager::ForceStopSoftAp()
{
    for (int i = 0; i < AP_INSTANCE_MAX_NUM; ++i) {
        WifiApHalInterface::GetInstance().StopAp(i);
        if (WifiManager::GetInstance().GetWifiTogglerManager()) {
            WifiManager::GetInstance().GetWifiTogglerManager()->SoftapToggled(0, i);
        }
    }
}

void WifiMultiVapManager::ShowToast()
{
    WifiNotificationUtil::GetInstance().ShowDialog(WifiDialogType::THREE_VAP);
}

}  // namespace Wifi
}  // namespace OHOS
#endif