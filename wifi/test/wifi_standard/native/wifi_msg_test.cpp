/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "wifi_msg.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Ref;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
constexpr int INVALID_VALUE = -1;
constexpr int FOUR = 4;
constexpr int FIVE = 5;
constexpr int NINE = 9;
class WifiMsgTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

HWTEST_F(WifiMsgTest, Phase2MethodToStrTest, TestSize.Level1)
{
    std::string eap = "TTLS";
    int method = INVALID_VALUE;
    EXPECT_TRUE(WifiEapConfig::Phase2MethodToStr(eap, method) == "auth=NONE");
    method = NINE;
    EXPECT_TRUE(WifiEapConfig::Phase2MethodToStr(eap, method) == "auth=NONE");
    method = FOUR;
    EXPECT_TRUE(WifiEapConfig::Phase2MethodToStr(eap, method) == "autheap=GTC");
    method = FIVE;
    EXPECT_TRUE(WifiEapConfig::Phase2MethodToStr(eap, method) == "auth=SIM");
}

HWTEST_F(WifiMsgTest, Phase2MethodFromStrTest, TestSize.Level1)
{
    std::string str = "NONE";
    EXPECT_TRUE(WifiEapConfig::Phase2MethodFromStr(str) == Phase2Method::NONE);
    str = "auth=PAP";
    EXPECT_TRUE(WifiEapConfig::Phase2MethodFromStr(str) == Phase2Method::PAP);
    str = "auth=NONE";
    EXPECT_TRUE(WifiEapConfig::Phase2MethodFromStr(str) == Phase2Method::NONE);
    str = "autheap=PAP";
    EXPECT_TRUE(WifiEapConfig::Phase2MethodFromStr(str) == Phase2Method::PAP);
}

/* ========== WifiEnhanceConfigType：增强配置类型枚举 ========== */

/**
 * @tc.name  : WifiEnhanceConfigType_ShouldContainAllExpectedValues
 * @tc.desc  : WifiEnhanceConfigType 枚举包含全部预期枚举值（含新增 CAMPUS_WIFI_STATE），
 *             编译期可见性校验，确保跨仓桥接 GetWifiEnhanceConfig 取值正确
 */
HWTEST_F(WifiMsgTest, WifiEnhanceConfigType_ShouldContainAllExpectedValues, TestSize.Level1)
{
    EXPECT_NE(static_cast<int>(WifiEnhanceConfigType::FAST_RECONNECT),
        static_cast<int>(WifiEnhanceConfigType::ROAM_STATE));
    EXPECT_NE(static_cast<int>(WifiEnhanceConfigType::ROAM_STATE),
        static_cast<int>(WifiEnhanceConfigType::CAMPUS_WIFI_STATE));
}

/**
 * @tc.name  : WifiEnhanceConfigType_CampusWifiState_ShouldBeDistinct
 * @tc.desc  : 新增的 CAMPUS_WIFI_STATE 枚举值与其他枚举值均不同，
 *             保证 GetWifiEnhanceConfig 分支选择唯一
 */
HWTEST_F(WifiMsgTest, WifiEnhanceConfigType_CampusWifiState_ShouldBeDistinct, TestSize.Level1)
{
    int campusValue = static_cast<int>(WifiEnhanceConfigType::CAMPUS_WIFI_STATE);
    EXPECT_NE(campusValue, static_cast<int>(WifiEnhanceConfigType::FAST_RECONNECT));
    EXPECT_NE(campusValue, static_cast<int>(WifiEnhanceConfigType::ROAM_STATE));
}
} // namespace Wifi
} // namespace OHOS
