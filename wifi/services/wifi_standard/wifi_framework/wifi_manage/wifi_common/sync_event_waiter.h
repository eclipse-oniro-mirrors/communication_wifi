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

#ifndef OHOS_WIFI_SYNC_EVENT_WAITER_H
#define OHOS_WIFI_SYNC_EVENT_WAITER_H

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include "wifi_errcode.h"
#include "wifi_msg.h"

namespace OHOS {
namespace Wifi {
class SyncEventWaiter {
public:
    static constexpr int32_t WAIT_TIMEOUT_MS = 3000;

    enum class EventId {
        STA_DISCONNECTED = 0,
        HOTSPOT_STOPPED,
    };

    static SyncEventWaiter& GetInstance();

    /**
     * @brief  投递事件并唤醒等待方（任意线程可调用）
     * @param None
     * @return None
     */
    void PostEvent(EventId event);

    /**
     * @brief  清空尚未消费的事件（每次开始新的等待前调用，丢弃历史事件）
     * @param None
     * @return None
     */
    void ClearEvents();

    /**
     * @brief  阻塞等待指令事件
     * @param expected 期望的事件
     * @param timeoutMs 超时时间（毫秒）
     * @return true：等到期望事件；false：超时
     */
    bool WaitEvent(EventId expected, int32_t timeoutMs);

    /**
     * @brief  开热点前的互斥准备，禁用自动回连 + 下发断开；STA 已连接时同步等待断开
     * @param apId 热点ID
     * @return WIFI_OPT_SUCCESS：自动回连已禁用（且STA已断开或本就没有连接），可以继续开热点
     * WIFI_OPT_FAILED：禁用自动回连失败，下发断开失败或等待断开超时，本次不开热点
     */
    ErrCode EnsureStaDisconnectedForHotspot(int apId);

    /**
     * @brief  用户发起连接前的准备工作，热点未关闭时，先关闭热点并同步等待其停止
     * @param None
     * @return WIFI_OPT_SUCCESS：热点未开或已停止，可以继续发起连接
     * WIFI_OPT_FAILED：等待超时或热点仍非关闭态，本次连接失败
     */
    ErrCode EnsureHotspotStoppedForConnect();

    void OnHotspotStopped();
    void OnStaConnChanged(int state, int instId);

private:
    SyncEventWaiter() = default;
    ~SyncEventWaiter() = default;
    SyncEventWaiter(const SyncEventWaiter &) = delete;
    SyncEventWaiter &operator=(const SyncEventWaiter &) = delete;

    bool IsStaConnected() const;

    bool IsHotspotClosed() const;

    mutable std::mutex mutex_;
    std::condition_variable eventCv_;
    std::queue<EventId> eventQueue_;
};
} // namespace Wifi
} // namespace OHOS

#endif // OHOS_WIFI_SYNC_EVENT_WAITER_H
