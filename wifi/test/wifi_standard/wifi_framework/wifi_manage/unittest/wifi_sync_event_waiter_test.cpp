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

#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include "sync_event_waiter.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::Wifi;

class SyncEventWaiterTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        SyncEventWaiter::GetInstance().ClearEvents();
    }

    void TearDown()
    {
        SyncEventWaiter::GetInstance().ClearEvents();
    }
};

HWTEST_F(SyncEventWaiterTest, WaitEventReturnsTrueWhenEventPostedTest, TestSize.Level1)
{
    auto &waiter = SyncEventWaiter::GetInstance();
    waiter.PostEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED);
    EXPECT_TRUE(waiter.WaitEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED, 100));
}

HWTEST_F(SyncEventWaiterTest, WaitEventReturnsFalseWhenEventTimeoutTest, TestSize.Level1)
{
    auto &waiter = SyncEventWaiter::GetInstance();
    EXPECT_FALSE(waiter.WaitEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED, 10));
}

HWTEST_F(SyncEventWaiterTest, ClearEventsDropsPendingEventTest, TestSize.Level1)
{
    auto &waiter = SyncEventWaiter::GetInstance();
    waiter.PostEvent(SyncEventWaiter::EventId::STA_DISCONNECTED);
    waiter.ClearEvents();
    EXPECT_FALSE(waiter.WaitEvent(SyncEventWaiter::EventId::STA_DISCONNECTED, 10));
}

HWTEST_F(SyncEventWaiterTest, WaitEventDiscardsUnexpectedEventTest, TestSize.Level1)
{
    auto &waiter = SyncEventWaiter::GetInstance();
    waiter.PostEvent(SyncEventWaiter::EventId::STA_DISCONNECTED);
    EXPECT_FALSE(waiter.WaitEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED, 10));
    EXPECT_FALSE(waiter.WaitEvent(SyncEventWaiter::EventId::STA_DISCONNECTED, 10));
}

HWTEST_F(SyncEventWaiterTest, PostEventWakesUpWaiterTest, TestSize.Level1)
{
    auto &waiter = SyncEventWaiter::GetInstance();
    std::thread poster([&waiter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        waiter.PostEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED);
    });
    EXPECT_TRUE(waiter.WaitEvent(SyncEventWaiter::EventId::HOTSPOT_STOPPED, 1000));
    poster.join();
}
