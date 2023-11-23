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
#ifndef OHOS_WIFI_HAL_WPA_COMMON_TEST_H
#define OHOS_WIFI_HAL_WPA_COMMON_TEST_H

#include <gtest/gtest.h>
#include "server.h"
#include "wifi_p2p_hal.h"
#include "wifi_wpa_hal.h"
#include "wifi_common_hal.h"

namespace OHOS {
namespace Wifi {
class WifiHalWpaCommonTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp()
    {}
    virtual void TearDown()
    {}

};
}  // namespace Wifi
}  // namespace OHOS
#endif