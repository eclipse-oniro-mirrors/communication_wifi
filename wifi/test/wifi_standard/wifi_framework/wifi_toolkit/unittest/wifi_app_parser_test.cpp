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

#include "wifi_app_parser.h"
#include <gtest/gtest.h>
#include <string>
#include "cJSON.h"
#include "wifi_logger.h"

using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiAppParserTest");

namespace {
constexpr const char *MONITOR_APP_JSON = "{"
    "\"Version\": 1.0,"
    "\"GameInfo\": [{\"gameName\": \"gameApp\"},"
    "  {\"gameName\": \"rttGame\", \"mGameRtt\": 50, \"mGameRttGap\": 10}],"
    "\"AppWhiteList\": [\"whiteListApp\"],"
    "\"AppBlackList\": [\"blackListApp\"],"
    "\"MultiLinkBlackList\": [\"multiLinkApp\"],"
    "\"ChariotApp\": [\"chariotApp\"],"
    "\"HiGameExcludeList\": [\"hiGameApp\"]}";

constexpr const char *NETWORK_CONTROL_APP_JSON = "{"
    "\"Version\": 1.0,"
    "\"KeyAppForegroundList\": [\"keyFgApp\"],"
    "\"KeyBackgroundLimitListApp\": [\"keyBgApp\"],"
    "\"GameBackgroundLimitListApp\": [\"gameBgApp\"],"
    "\"RssGameList\": [\"rssGameApp\"],"
    "\"AppLiveStream\": [\"liveApp\"],"
    "\"AsyncDelayTime\": 100}";

constexpr const char *PACKAGE_FILTER_JSON = "{"
    "\"Version\": 1.0,"
    "\"ScanControlPackages\": {\"scan_thermal_filter\": [\"thermalPkg\"]},"
    "\"CandidateFilterPackages\": [\"candidatePkg\"],"
    "\"CorePackages\": {\"SETTINGS\": \"settingsPkg\","
    "  \"BROWSER_BUNDLE\": \"browserPkg\"},"
    "\"AclAuthPackages\": [{\"name\": \"aclPkg\", \"appid\": \"456\"}],"
    "\"ScanLimitPackages\": [{\"name\": \"scanLimitPkg\"}],"
    "\"LandscapeSwitchLimitList\": [\"landscapePkg\"]}";

cJSON *ParseJsonString(const char *jsonStr)
{
    return cJSON_Parse(jsonStr);
}
} // namespace

class AppParserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}

protected:
    AppParser &parser = AppParser::GetInstance();
};

HWTEST_F(AppParserTest, ParseMonitorAppConfig_NullRoot, TestSize.Level1)
{
    WIFI_LOGI("ParseMonitorAppConfig_NullRoot enter");
    EXPECT_FALSE(parser.ParseMonitorAppConfig(nullptr));
}

HWTEST_F(AppParserTest, ParseMonitorAppConfig_UnknownKey, TestSize.Level1)
{
    WIFI_LOGI("ParseMonitorAppConfig_UnknownKey enter");
    cJSON *root = ParseJsonString("{\"Version\": 1.0, \"UnknownKey\": [\"app\"]}");
    ASSERT_NE(root, nullptr);
    /* unknown keys are skipped: parsing still succeeds (same as old xml logic) */
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);
}

HWTEST_F(AppParserTest, ParseMonitorAppConfig_Success, TestSize.Level1)
{
    WIFI_LOGI("ParseMonitorAppConfig_Success enter");
    cJSON *root = ParseJsonString(MONITOR_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);

    EXPECT_TRUE(parser.IsLowLatencyApp("gameApp"));
    EXPECT_TRUE(parser.IsLowLatencyApp("rttGame"));
    EXPECT_TRUE(parser.IsWhiteListApp("whiteListApp"));
    EXPECT_TRUE(parser.IsBlackListApp("blackListApp"));
    EXPECT_TRUE(parser.IsMultiLinkApp("multiLinkApp"));
    EXPECT_TRUE(parser.IsChariotApp("chariotApp"));
    EXPECT_TRUE(parser.IsHiGameExcludeApp("hiGameApp"));
}

HWTEST_F(AppParserTest, ParseMonitorAppConfig_InvalidItem, TestSize.Level1)
{
    WIFI_LOGI("ParseMonitorAppConfig_InvalidItem enter");
    cJSON *root = ParseJsonString("{\"AppWhiteList\": [\"goodApp\", 123],"
        "\"GameInfo\": [{\"gameName\": \"validGame\"}, {\"mGameRtt\": 50}]}");
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);

    EXPECT_TRUE(parser.IsWhiteListApp("goodApp"));
    EXPECT_TRUE(parser.IsLowLatencyApp("validGame"));
    /* entry without gameName is skipped */
    EXPECT_FALSE(parser.IsLowLatencyApp("nonameGame"));
}

HWTEST_F(AppParserTest, ParseMonitorAppConfig_SectionsReplaced, TestSize.Level1)
{
    WIFI_LOGI("ParseMonitorAppConfig_SectionsReplaced enter");
    cJSON *root = ParseJsonString(MONITOR_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);
    EXPECT_TRUE(parser.IsWhiteListApp("whiteListApp"));

    /* parse a config without AppWhiteList: the section is wiped and refilled */
    cJSON *newRoot = ParseJsonString("{\"AppBlackList\": [\"newBlackApp\"]}");
    ASSERT_NE(newRoot, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(newRoot));
    cJSON_Delete(newRoot);

    EXPECT_FALSE(parser.IsWhiteListApp("whiteListApp"));
    EXPECT_FALSE(parser.IsLowLatencyApp("gameApp"));
    EXPECT_TRUE(parser.IsBlackListApp("newBlackApp"));
}

HWTEST_F(AppParserTest, IsLowLatencyApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsLowLatencyApp_False enter");
    EXPECT_FALSE(parser.IsLowLatencyApp("other"));
}

HWTEST_F(AppParserTest, IsWhiteListApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsWhiteListApp_False enter");
    EXPECT_FALSE(parser.IsWhiteListApp("other"));
}

HWTEST_F(AppParserTest, IsBlackListApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsBlackListApp_False enter");
    EXPECT_FALSE(parser.IsBlackListApp("other"));
}

HWTEST_F(AppParserTest, IsChariotApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsChariotApp_False enter");
    EXPECT_FALSE(parser.IsChariotApp("other"));
}

HWTEST_F(AppParserTest, IsMultiLinkApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsMultiLinkApp_False enter");
    EXPECT_FALSE(parser.IsMultiLinkApp("other"));
}

HWTEST_F(AppParserTest, IsHiGameExcludeApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsHiGameExcludeApp_False enter");
    EXPECT_FALSE(parser.IsHiGameExcludeApp("other"));
}

HWTEST_F(AppParserTest, IsOverGameRtt_WithRtt, TestSize.Level1)
{
    WIFI_LOGI("IsOverGameRtt_WithRtt enter");
    cJSON *root = ParseJsonString(MONITOR_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);

    EXPECT_TRUE(parser.IsOverGameRtt("rttGame", 60));
    EXPECT_FALSE(parser.IsOverGameRtt("rttGame", 40));
}

HWTEST_F(AppParserTest, IsOverGameRtt_NotInGameInfo, TestSize.Level1)
{
    WIFI_LOGI("IsOverGameRtt_NotInGameInfo enter");
    cJSON *root = ParseJsonString(MONITOR_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);

    /* gameApp has no mGameRtt: only in low latency list, not in game info */
    EXPECT_FALSE(parser.IsOverGameRtt("gameApp", 100));
    EXPECT_FALSE(parser.IsOverGameRtt("nonexistent", 100));
}

HWTEST_F(AppParserTest, IsOverGameLowRttThresh_WithRttGap, TestSize.Level1)
{
    WIFI_LOGI("IsOverGameLowRttThresh_WithRttGap enter");
    cJSON *root = ParseJsonString(MONITOR_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseMonitorAppConfig(root));
    cJSON_Delete(root);

    /* rtt(50) - rttGap(10) = 40 */
    EXPECT_TRUE(parser.IsOverGameLowRttThresh("rttGame", 45));
    EXPECT_FALSE(parser.IsOverGameLowRttThresh("rttGame", 35));
    EXPECT_FALSE(parser.IsOverGameLowRttThresh("nonexistent", 100));
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_NullRoot, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_NullRoot enter");
    EXPECT_FALSE(parser.ParseNetworkControlAppConfig(nullptr));
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_UnknownKey, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_UnknownKey enter");
    cJSON *root = ParseJsonString("{\"Version\": 1.0, \"UnknownKey\": [\"app\"]}");
    ASSERT_NE(root, nullptr);
    /* unknown keys are skipped: parsing still succeeds (same as old xml logic) */
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(root));
    cJSON_Delete(root);
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_Success, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_Success enter");
    cJSON *root = ParseJsonString(NETWORK_CONTROL_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(root));
    cJSON_Delete(root);

    EXPECT_TRUE(parser.IsKeyForegroundApp("keyFgApp"));
    EXPECT_TRUE(parser.IsKeyBackgroundLimitApp("keyBgApp"));
    EXPECT_TRUE(parser.IsGameBackgroundLimitApp("gameBgApp"));
    EXPECT_TRUE(parser.IsRssGameApp("rssGameApp"));
    EXPECT_TRUE(parser.IsLiveStreamApp("liveApp"));
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_DelayTimeNumber, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_DelayTimeNumber enter");
    cJSON *root = ParseJsonString(NETWORK_CONTROL_APP_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(root));
    cJSON_Delete(root);

    EXPECT_EQ(parser.GetAsyncLimitSpeedDelayTime(), "100");
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_DelayTimeString, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_DelayTimeString enter");
    cJSON *root = ParseJsonString("{\"KeyAppForegroundList\": [\"keyFgApp\"],"
        "\"AsyncDelayTime\": \"200\"}");
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(root));
    cJSON_Delete(root);

    EXPECT_EQ(parser.GetAsyncLimitSpeedDelayTime(), "200");
}

HWTEST_F(AppParserTest, ParseNetworkControlAppConfig_DelayTimeKeptWhenAbsent, TestSize.Level1)
{
    WIFI_LOGI("ParseNetworkControlAppConfig_DelayTimeKeptWhenAbsent enter");
    cJSON *root = ParseJsonString("{\"AsyncDelayTime\": \"300\"}");
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(root));
    cJSON_Delete(root);
    EXPECT_EQ(parser.GetAsyncLimitSpeedDelayTime(), "300");

    /* AsyncDelayTime key absent: previous value is kept */
    cJSON *newRoot = ParseJsonString("{\"KeyAppForegroundList\": [\"keyFgApp\"]}");
    ASSERT_NE(newRoot, nullptr);
    EXPECT_TRUE(parser.ParseNetworkControlAppConfig(newRoot));
    cJSON_Delete(newRoot);

    EXPECT_EQ(parser.GetAsyncLimitSpeedDelayTime(), "300");
}

HWTEST_F(AppParserTest, IsKeyForegroundApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsKeyForegroundApp_False enter");
    EXPECT_FALSE(parser.IsKeyForegroundApp("other"));
}

HWTEST_F(AppParserTest, IsKeyBackgroundLimitApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsKeyBackgroundLimitApp_False enter");
    EXPECT_FALSE(parser.IsKeyBackgroundLimitApp("other"));
}

HWTEST_F(AppParserTest, IsLiveStreamApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsLiveStreamApp_False enter");
    EXPECT_FALSE(parser.IsLiveStreamApp("other"));
}

HWTEST_F(AppParserTest, IsGameBackgroundLimitApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsGameBackgroundLimitApp_False enter");
    EXPECT_FALSE(parser.IsGameBackgroundLimitApp("other"));
}

HWTEST_F(AppParserTest, IsRssGameApp_False, TestSize.Level1)
{
    WIFI_LOGI("IsRssGameApp_False enter");
    EXPECT_FALSE(parser.IsRssGameApp("other"));
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_NullRoot, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_NullRoot enter");
    EXPECT_FALSE(parser.ParsePackageFilterConfig(nullptr));
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_UnknownKey, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_UnknownKey enter");
    cJSON *root = ParseJsonString("{\"Version\": 1.0, \"UnknownKey\": [{\"name\": \"app\"}]}");
    ASSERT_NE(root, nullptr);
    /* unknown keys are skipped: parsing still succeeds (same as old xml logic) */
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_Success, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_Success enter");
    cJSON *root = ParseJsonString(PACKAGE_FILTER_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);

    EXPECT_EQ(parser.GetPackageName("CandidateFilterPackages"), "candidatePkg");
    EXPECT_EQ(parser.GetPackageName("scan_thermal_filter"), "thermalPkg");
    /* each section is parsed according to its fixed form */
    EXPECT_EQ(parser.GetPackageName("SETTINGS"), "settingsPkg");
    EXPECT_EQ(parser.GetPackageName("BROWSER_BUNDLE"), "browserPkg");
    EXPECT_EQ(parser.GetPackageName("NoSuchTag"), "");
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_GetPackageInfoByName, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_GetPackageInfoByName enter");
    cJSON *root = ParseJsonString(PACKAGE_FILTER_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);

    std::vector<PackageInfo> packageInfo;
    EXPECT_EQ(parser.GetPackageInfoByName("AclAuthPackages", packageInfo), 0);
    ASSERT_EQ(packageInfo.size(), 1);
    EXPECT_EQ(packageInfo[0].name, "aclPkg");
    EXPECT_EQ(packageInfo[0].appid, "456");

    /* single string entry is normalized to a one-element list */
    packageInfo.clear();
    EXPECT_EQ(parser.GetPackageInfoByName("SETTINGS", packageInfo), 0);
    ASSERT_EQ(packageInfo.size(), 1);
    EXPECT_EQ(packageInfo[0].name, "settingsPkg");

    EXPECT_EQ(parser.GetPackageInfoByName("NoSuchName", packageInfo), -1);
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_GetPackageInfoMap, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_GetPackageInfoMap enter");
    cJSON *root = ParseJsonString(PACKAGE_FILTER_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);

    std::map<std::string, std::vector<PackageInfo>> packageInfoMap;
    EXPECT_EQ(parser.GetPackageInfoMap(packageInfoMap), 0);
    EXPECT_NE(packageInfoMap.find("scan_thermal_filter"), packageInfoMap.end());
    EXPECT_NE(packageInfoMap.find("CandidateFilterPackages"), packageInfoMap.end());
    EXPECT_NE(packageInfoMap.find("SETTINGS"), packageInfoMap.end());
    EXPECT_NE(packageInfoMap.find("LandscapeSwitchLimitList"), packageInfoMap.end());
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_InvalidItem, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_InvalidItem enter");
    cJSON *root = ParseJsonString("{\"ScanLimitPackages\": "
        "[{\"name\": \"validPkg\"}, {\"appid\": \"noName\"}],"
        "\"LandscapeSwitchLimitList\": [\"validNamePkg\", \"\"]}");
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);

    std::vector<PackageInfo> packageInfo;
    EXPECT_EQ(parser.GetPackageInfoByName("ScanLimitPackages", packageInfo), 0);
    /* object without name is skipped */
    ASSERT_EQ(packageInfo.size(), 1);
    EXPECT_EQ(packageInfo[0].name, "validPkg");

    packageInfo.clear();
    EXPECT_EQ(parser.GetPackageInfoByName("LandscapeSwitchLimitList", packageInfo), 0);
    /* empty name string is skipped */
    ASSERT_EQ(packageInfo.size(), 1);
    EXPECT_EQ(packageInfo[0].name, "validNamePkg");
}

HWTEST_F(AppParserTest, ParsePackageFilterConfig_MapReplaced, TestSize.Level1)
{
    WIFI_LOGI("ParsePackageFilterConfig_MapReplaced enter");
    cJSON *root = ParseJsonString(PACKAGE_FILTER_JSON);
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(root));
    cJSON_Delete(root);
    EXPECT_EQ(parser.GetPackageName("CandidateFilterPackages"), "candidatePkg");

    /* parse a new filter config: the whole map is replaced */
    cJSON *newRoot = ParseJsonString("{\"ScanLimitPackages\": [{\"name\": \"newPkg\"}]}");
    ASSERT_NE(newRoot, nullptr);
    EXPECT_TRUE(parser.ParsePackageFilterConfig(newRoot));
    cJSON_Delete(newRoot);

    EXPECT_EQ(parser.GetPackageName("CandidateFilterPackages"), "");
    EXPECT_EQ(parser.GetPackageName("ScanLimitPackages"), "newPkg");
}
} // namespace Wifi
} // namespace OHOS
