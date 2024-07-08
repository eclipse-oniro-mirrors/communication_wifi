/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_MANAGER_SERVICE_TEST_H
#define OHOS_WIFI_MANAGER_SERVICE_TEST_H

#include <gtest/gtest.h>
#include "wifi_watchdog_utils.h"

namespace OHOS {
namespace Wifi {
class WifiWatchDogUtilsTest : public testing::Test {
public:
    static void SetupTestCase()
    {
        WifiWatchDogUtilsTest::GetInstance();
    }
    static void TearDownTestCase()
    {
    }
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}

public:
};
}  // namespace Wifi
}  // namespace OHOS
#endif