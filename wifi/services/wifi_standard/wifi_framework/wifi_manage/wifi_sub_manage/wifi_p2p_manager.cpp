/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifdef FEATURE_P2P_SUPPORT
#include "wifi_p2p_manager.h"
#include "wifi_manager.h"
#include "wifi_service_manager.h"
#include "wifi_config_center.h"
#include "wifi_logger.h"
#include "wifi_common_event_helper.h"
#include "wifi_system_timer.h"
#include "wifi_hisysevent.h"
#include "p2p_define.h"
#include "wifi_service_scheduler.h"
#ifdef OHOS_ARCH_LITE
#include "wifi_internal_event_dispatcher_lite.h"
#else
#include "wifi_internal_event_dispatcher.h"
#include "wifi_sa_manager.h"
#endif
#ifdef HDI_CHIP_INTERFACE_SUPPORT
#include "hal_device_manage.h"
#endif

DEFINE_WIFILOG_LABEL("WifiP2pManager");

namespace OHOS {
namespace Wifi {
WifiP2pManager::WifiP2pManager()
{
    WIFI_LOGI("create WifiP2pManager");
    InitP2pCallback();
}

IP2pServiceCallbacks& WifiP2pManager::GetP2pCallback(void)
{
    return mP2pCallback;
}

#ifndef OHOS_ARCH_LITE
static void UnloadP2PSaTimerCallback()
{
    WifiSaLoadManager::GetInstance().UnloadWifiSa(WIFI_P2P_ABILITY_ID);
    WifiManager::GetInstance().GetWifiP2pManager()->StopUnloadP2PSaTimer();
}

void WifiP2pManager::StopUnloadP2PSaTimer(void)
{
    WIFI_LOGI("StopUnloadP2PSaTimer! unloadP2PSaTimerId:%{public}u", unloadP2PSaTimerId);
    std::unique_lock<std::mutex> lock(unloadP2PSaTimerMutex);
    if (unloadP2PSaTimerId == 0) {
        return;
    }
    MiscServices::TimeServiceClient::GetInstance()->StopTimer(unloadP2PSaTimerId);
    MiscServices::TimeServiceClient::GetInstance()->DestroyTimer(unloadP2PSaTimerId);
    unloadP2PSaTimerId = 0;
    return;
}

void WifiP2pManager::StartUnloadP2PSaTimer(void)
{
    WIFI_LOGI("StartUnloadP2PSaTimer! unloadP2PSaTimerId:%{public}u", unloadP2PSaTimerId);
    std::unique_lock<std::mutex> lock(unloadP2PSaTimerMutex);
    if (unloadP2PSaTimerId == 0) {
        std::shared_ptr<WifiSysTimer> wifiSysTimer = std::make_shared<WifiSysTimer>(false, 0, true, false);
        wifiSysTimer->SetCallbackInfo(UnloadP2PSaTimerCallback);
        unloadP2PSaTimerId = MiscServices::TimeServiceClient::GetInstance()->CreateTimer(wifiSysTimer);
        int64_t currentTime = MiscServices::TimeServiceClient::GetInstance()->GetBootTimeMs();
        MiscServices::TimeServiceClient::GetInstance()->StartTimer(unloadP2PSaTimerId,
            currentTime + TIMEOUT_UNLOAD_WIFI_SA);
        WIFI_LOGI("StartUnloadP2PSaTimer success! unloadP2PSaTimerId:%{public}u", unloadP2PSaTimerId);
    }
    return;
}
#endif

void WifiP2pManager::CloseP2pService(void)
{
    WIFI_LOGD("close p2p service");
    WifiServiceManager::GetInstance().UnloadService(WIFI_SERVICE_P2P);
    WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::CLOSED);
    WifiConfigCenter::GetInstance().SetP2pState(static_cast<int>(P2pState::P2P_STATE_CLOSED));
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_P2P_STATE_CHANGE;
    cbMsg.msgData = static_cast<int>(P2pState::P2P_STATE_CLOSED);
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    if (!ifaceName.empty()) {
        DelayedSingleton<HalDeviceManager>::GetInstance()->RemoveP2pIface(ifaceName);
        ifaceName.clear();
        WifiServiceScheduler::GetInstance().ClearP2pIfaceNameMap();
        WifiConfigCenter::GetInstance().SetP2pIfaceName("");
    }
#endif
    WifiOprMidState staState = WifiConfigCenter::GetInstance().GetWifiMidState();
    if (staState == WifiOprMidState::RUNNING || staState == WifiOprMidState::SEMI_ACTIVE) {
        WifiServiceScheduler::GetInstance().AutoStartP2pService(0);
        return;
    }
#ifndef OHOS_ARCH_LITE
    if (WifiConfigCenter::GetInstance().GetAirplaneModeState() == MODE_STATE_OPEN) {
        WIFI_LOGI("airplaneMode not close p2p SA!");
        return;
    }
    StartUnloadP2PSaTimer();
#endif
    WIFI_LOGI("CloseP2pService, current sta state:%{public}d", staState);
    return;
}

void WifiP2pManager::InitP2pCallback(void)
{
    using namespace std::placeholders;
    mP2pCallback.callbackModuleName = "P2pManager";
    mP2pCallback.OnP2pStateChangedEvent = std::bind(&WifiP2pManager::DealP2pStateChanged, this, _1);
    mP2pCallback.OnP2pPeersChangedEvent = std::bind(&WifiP2pManager::DealP2pPeersChanged, this, _1);
    mP2pCallback.OnP2pServicesChangedEvent = std::bind(&WifiP2pManager::DealP2pServiceChanged, this, _1);
    mP2pCallback.OnP2pConnectionChangedEvent = std::bind(&WifiP2pManager::DealP2pConnectionChanged, this, _1);
    mP2pCallback.OnP2pThisDeviceChangedEvent = std::bind(&WifiP2pManager::DealP2pThisDeviceChanged, this, _1);
    mP2pCallback.OnP2pDiscoveryChangedEvent = std::bind(&WifiP2pManager::DealP2pDiscoveryChanged, this, _1);
    mP2pCallback.OnP2pGroupsChangedEvent = std::bind(&WifiP2pManager::DealP2pGroupsChanged, this);
    mP2pCallback.OnP2pActionResultEvent = std::bind(&WifiP2pManager::DealP2pActionResult, this, _1, _2);
    mP2pCallback.OnConfigChangedEvent = std::bind(&WifiP2pManager::DealConfigChanged, this, _1, _2, _3);
    mP2pCallback.OnP2pGcJoinGroupEvent = std::bind(&WifiP2pManager::DealP2pGcJoinGroup, this, _1);
    mP2pCallback.OnP2pGcLeaveGroupEvent = std::bind(&WifiP2pManager::DealP2pGcLeaveGroup, this, _1);
    mP2pCallback.OnP2pPrivatePeersChangedEvent = std::bind(&WifiP2pManager::DealP2pPrivatePeersChanged, this, _1);
    return;
}

void WifiP2pManager::DealP2pStateChanged(P2pState state)
{
    WIFI_LOGI("DealP2pStateChanged, state: %{public}d", static_cast<int>(state));
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_P2P_STATE_CHANGE;
    cbMsg.msgData = static_cast<int>(state);
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    if (state == P2pState::P2P_STATE_IDLE) {
        WifiManager::GetInstance().PushServiceCloseMsg(WifiCloseServiceCode::P2P_SERVICE_CLOSE);
    }
    if (state == P2pState::P2P_STATE_STARTED) {
        WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::OPENING, WifiOprMidState::RUNNING);
        WifiOprMidState staState = WifiConfigCenter::GetInstance().GetWifiMidState();
        WIFI_LOGI("DealP2pStateChanged, current sta state:%{public}d", staState);
        if (staState == WifiOprMidState::CLOSING || staState == WifiOprMidState::CLOSED) {
            WifiServiceScheduler::GetInstance().AutoStopP2pService();
        }
    }
    if (state == P2pState::P2P_STATE_CLOSED) {
        bool ret = WifiConfigCenter::GetInstance().SetP2pMidState(WifiOprMidState::OPENING, WifiOprMidState::CLOSED);
        if (ret) {
            WIFI_LOGE("P2p start failed, stop wifi!");
            WifiServiceScheduler::GetInstance().AutoStopP2pService();
        }
    }
    WifiCommonEventHelper::PublishP2pStateChangedEvent((int)state, "OnP2pStateChanged");
    return;
}

void WifiP2pManager::DealP2pPeersChanged(const std::vector<WifiP2pDevice> &vPeers)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_PEER_CHANGE;
    cbMsg.device = vPeers;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    WifiCommonEventHelper::PublishP2pPeersStateChangedEvent(vPeers.size(), "OnP2pPeersChanged");
    return;
}

void WifiP2pManager::DealP2pPrivatePeersChanged(const std::string &privateInfo)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_PRIVATE_PEER_CHANGE;
    cbMsg.privateWfdInfo = privateInfo;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealP2pServiceChanged(const std::vector<WifiP2pServiceInfo> &vServices)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_SERVICE_CHANGE;
    cbMsg.serviceInfo = vServices;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealP2pConnectionChanged(const WifiP2pLinkedInfo &info)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_CONNECT_CHANGE;
    cbMsg.p2pInfo = info;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    WifiCommonEventHelper::PublishP2pConnStateEvent((int)info.GetConnectState(), "OnP2pConnectStateChanged");
    WifiP2pGroupInfo group;
    IP2pService *pService = WifiServiceManager::GetInstance().GetP2pServiceInst();
    if (pService == nullptr) {
        WIFI_LOGE("Get P2P service failed!");
        return;
    }
    ErrCode errCode = pService->GetCurrentGroup(group);
    if (errCode != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("Get current group info failed!");
        return;
    }
    WriteWifiP2pStateHiSysEvent(group.GetInterface(), (int32_t)info.IsGroupOwner(), (int32_t)info.GetConnectState());
    if (info.GetConnectState() == P2pConnectedState::P2P_CONNECTED) {
        WriteP2pKpiCountHiSysEvent(static_cast<int>(P2P_CHR_EVENT::CONN_SUC_CNT));
    }
    return;
}

void WifiP2pManager::DealP2pThisDeviceChanged(const WifiP2pDevice &info)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_THIS_DEVICE_CHANGE;
    cbMsg.p2pDevice = info;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    WifiCommonEventHelper::PublishP2pCurrentDeviceStateChangedEvent(
        (int)info.GetP2pDeviceStatus(), "OnP2pThisDeviceChanged");
    return;
}

void WifiP2pManager::DealP2pDiscoveryChanged(bool bState)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_DISCOVERY_CHANGE;
    cbMsg.msgData = static_cast<int>(bState);
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealP2pGroupsChanged() __attribute__((no_sanitize("cfi")))
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_PERSISTENT_GROUPS_CHANGE;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    WifiCommonEventHelper::PublishP2pGroupStateChangedEvent(0, "OnP2pGroupStateChanged");
    return;
}

void WifiP2pManager::DealP2pActionResult(P2pActionCallback action, ErrCode code)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_P2P_ACTION_RESULT;
    cbMsg.p2pAction = action;
    cbMsg.msgData = static_cast<int>(code);
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealP2pGcJoinGroup(const GcInfo &info)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_P2P_GC_JOIN_GROUP;
    cbMsg.gcInfo = info;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealP2pGcLeaveGroup(const GcInfo &info)
{
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_P2P_GC_LEAVE_GROUP;
    cbMsg.gcInfo = info;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::DealConfigChanged(CfgType type, char* data, int dataLen)
{
    if (data == nullptr || dataLen <= 0) {
        return;
    }
    WifiEventCallbackMsg cbMsg;
    cbMsg.msgCode = WIFI_CBK_MSG_CFG_CHANGE;
    CfgInfo* cfgInfoPtr = new (std::nothrow) CfgInfo();
    if (cfgInfoPtr == nullptr) {
        WIFI_LOGE("DealConfigChanged: new CfgInfo failed");
        return;
    }
    cfgInfoPtr->type = type;
    char* cfgData = new (std::nothrow) char[dataLen];
    if (cfgData == nullptr) {
        WIFI_LOGE("DealConfigChanged: new data failed");
        delete cfgInfoPtr;
        return;
    }
    if (memcpy_s(cfgData, dataLen, data, dataLen) != EOK) {
        WIFI_LOGE("DealConfigChanged: memcpy_s failed");
        delete cfgInfoPtr;
        delete[] cfgData;
        return;
    }
    cfgInfoPtr->data = cfgData;
    cfgInfoPtr->dataLen = dataLen;
    cbMsg.cfgInfo = cfgInfoPtr;
    WifiInternalEventDispatcher::GetInstance().AddBroadCastMsg(cbMsg);
    return;
}

void WifiP2pManager::SetP2pIfName(const std::string &p2pIfName)
{
    ifaceName = p2pIfName;
}
}  // namespace Wifi
}  // namespace OHOS
#endif