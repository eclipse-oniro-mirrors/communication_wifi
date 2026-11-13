/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "sync_event_waiter.h"
#include <chrono>
#include <string>
#include "wifi_log.h"
#include "wifi_config_center.h"
#include "wifi_service_manager.h"
#include "wifi_manager.h"
#include "wifi_toggler_manager.h"
#include "wifi_ap_msg.h"
#include "wifi_internal_msg.h"
#include "sta_interface.h"
#include "sta_define.h"

DEFINE_WIFILOG_LABEL("SyncEventWaiter");

namespace OHOS {
namespace Wifi {
static const std::string AP_STA_MUTEX_AUTO_JOIN_CONDITION = "ap_sta_connect_exclusion";

SyncEventWaiter& SyncEventWaiter::GetInstance()
{
    static SyncEventWaiter instance;
    return instance;
}

void SyncEventWaiter::PostEvent(EventId event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    eventQueue_.push(event);
    eventCv_.notify_all();
}

void SyncEventWaiter::ClearEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::queue<EventId> empty;
    eventQueue_.swap(empty);
}

bool SyncEventWaiter::WaitEvent(EventId expected, int32_t timeoutMs)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    bool found = false;
    while (!found) {
        if (!eventQueue_.empty()) {
            EventId event = eventQueue_.front();
            eventQueue_.pop();
            if (event == expected) {
                found = true;
                break;
            }
            continue;
        }
        if (!eventCv_.wait_until(lock, deadline, [this] { return !eventQueue_.empty(); })) {
            return false;
        }
    }
    return true;
}

bool SyncEventWaiter::IsStaConnected() const
{
    WifiLinkedInfo linkedInfo;
    WifiConfigCenter::GetInstance().GetLinkedInfo(linkedInfo, INSTID_WLAN0);
    return linkedInfo.connState == ConnState::CONNECTED;
}

bool SyncEventWaiter::IsHotspotClosed() const
{
    return WifiConfigCenter::GetInstance().GetHotspotState(INSTID_WLAN0) ==
        static_cast<int>(ApState::AP_STATE_CLOSED);
}

ErrCode SyncEventWaiter::EnsureStaDisconnectedForHotspot(int apId)
{
    IStaService *staService = WifiServiceManager::GetInstance().GetStaServiceInst(INSTID_WLAN0);
    if (staService == nullptr) {
        WIFI_LOGE("sta service is null, open hotspot aborted, apId=%{public}d", apId);
        return WIFI_OPT_FAILED;
    }

    if (staService->DisableAutoJoin(AP_STA_MUTEX_AUTO_JOIN_CONDITION) != WIFI_OPT_SUCCESS) {
        WIFI_LOGE("disable auto join failed, open hotspot aborted, apId=%{public}d", apId);
        return WIFI_OPT_FAILED;
    }

    ClearEvents();
    if (staService->Disconnect() != WIFI_OPT_SUCCESS) {
        staService->EnableAutoJoin(AP_STA_MUTEX_AUTO_JOIN_CONDITION);
        WIFI_LOGE("disconnect sta failed, open hotspot aborted, apId=%{public}d", apId);
        return WIFI_OPT_FAILED;
    }

    if (!IsStaConnected()) {
        WIFI_LOGI("sta not connected, open hotspot directly, apId=%{public}d", apId);
        return WIFI_OPT_SUCCESS;
    }

    WIFI_LOGI("sta connected, wait sta disconnect, apId=%{public}d, timeout=%{public}d ms", apId, WAIT_TIMEOUT_MS);
    bool disconnected = WaitEvent(EventId::STA_DISCONNECTED, WAIT_TIMEOUT_MS);
    if (!disconnected || IsStaConnected()) {
        staService->EnableAutoJoin(AP_STA_MUTEX_AUTO_JOIN_CONDITION);
        WIFI_LOGE("wait sta disconnect timeout, open hotspot aborted, apId=%{public}d", apId);
        return WIFI_OPT_FAILED;
    }
    WIFI_LOGI("sta disconnected, open hotspot, apId=%{public}d", apId);
    return WIFI_OPT_SUCCESS;
}

void SyncEventWaiter::OnStaConnChanged(int state, int instId)
{
    if (instId != INSTID_WLAN0) {
        return;
    }
    if (state != static_cast<int>(OperateResState::DISCONNECT_DISCONNECTED)) {
        return;
    }
    PostEvent(EventId::STA_DISCONNECTED);
}

ErrCode SyncEventWaiter::EnsureHotspotStoppedForConnect()
{
    if (IsHotspotClosed()) {
        return WIFI_OPT_SUCCESS;
    }
    ClearEvents();
    WIFI_LOGI("user connect while hotspot on, close hotspot first, timeout=%{public}d ms", WAIT_TIMEOUT_MS);
    WifiManager::GetInstance().GetWifiTogglerManager()->SoftapToggled(0, INSTID_WLAN0);
    bool stopped = WaitEvent(EventId::HOTSPOT_STOPPED, WAIT_TIMEOUT_MS);
    if (!stopped || !IsHotspotClosed()) {
        WIFI_LOGE("wait hotspot stop timeout, user connect aborted");
        return WIFI_OPT_FAILED;
    }
    WIFI_LOGI("hotspot stopped, continue user connect");
    return WIFI_OPT_SUCCESS;
}

void SyncEventWaiter::OnHotspotStopped()
{
    PostEvent(EventId::HOTSPOT_STOPPED);
    IStaService *staService = WifiServiceManager::GetInstance().GetStaServiceInst(INSTID_WLAN0);
    if (staService != nullptr && staService->EnableAutoJoin(AP_STA_MUTEX_AUTO_JOIN_CONDITION) == WIFI_OPT_SUCCESS) {
        WIFI_LOGI("enable auto join");
    }
}
} // namespace Wifi
} // namespace OHOS

