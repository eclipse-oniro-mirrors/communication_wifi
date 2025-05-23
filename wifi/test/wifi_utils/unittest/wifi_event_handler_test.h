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
#ifndef OHOS_WIFI_EVENT_HANDLER_TEST
#define OHOS_WIFI_EVENT_HANDLER_TEST

#include <gtest/gtest.h>
#include "wifi_event_handler.h"

namespace OHOS {
namespace Wifi {
class WifiEventHandlerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    
    virtual void SetUp()
    {
        testEventHandler = std::make_unique<WifiEventHandler>("WifiEventHandlerTest");
        result = 0;
    }
    
    virtual void TearDown()
    {
        if (testEventHandler) {
            testEventHandler.reset();
        }
    }

    static void EventHandlerCallback() { result = 1; }
public:
    static int result;
    std::unique_ptr<WifiEventHandler> testEventHandler = nullptr;
};
}  // namespace Wifi
}  // namespace OHOS
#endif