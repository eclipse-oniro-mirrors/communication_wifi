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

#include <gtest/gtest.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>

static bool CliAcceptable(int result)
{
    return result == 0 || WEXITSTATUS(result) == 1;
}

class WifiCliTest : public testing::Test {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(WifiCliTest, TestHelpCommand)
{
    int result = system("ohos-wifiManager sta-enable --help > /dev/null 2>&1");
    EXPECT_EQ(result, 0);
}

TEST_F(WifiCliTest, TestStaEnableCommand)
{
    int result = system("ohos-wifiManager sta-enable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestStaDisableCommand)
{
    int result = system("ohos-wifiManager sta-disable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestScanStartCommand)
{
    int result = system("ohos-wifiManager scan-start > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestScanListCommand)
{
    int result = system("ohos-wifiManager scan-list > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestUnknownCommand)
{
    int result = system("ohos-wifiManager unknown-cmd > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestNoCommand)
{
    int result = system("ohos-wifiManager > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotEnable)
{
    int result = system("ohos-wifiManager hotspot-enable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotDisable)
{
    int result = system("ohos-wifiManager hotspot-disable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigGet)
{
    int result = system("ohos-wifiManager hotspot-config-get > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSet)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--ssid TestHotspot --preSharedKey 12345678 > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotBlocklistList)
{
    int result = system("ohos-wifiManager hotspot-blocklist > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotOldSyntaxRejected)
{
    int result = system("ohos-wifiManager hotspot able on > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotHelp)
{
    int result = system("ohos-wifiManager --help > /dev/null 2>&1");
    EXPECT_EQ(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetNoFields)
{
    int result = system("ohos-wifiManager hotspot-config-set > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetInvalidSsidLength)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--ssid 012345678901234567890123456789012 > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetShortPassword)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--preSharedKey 1234567 > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetInvalidBand)
{
    int result = system("ohos-wifiManager hotspot-config-set --band 3.5G > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetBand6G)
{
    int result = system("ohos-wifiManager hotspot-config-set --band 6G > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetBand60G)
{
    int result = system("ohos-wifiManager hotspot-config-set --band 60G > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetUnknownOption)
{
    int result = system("ohos-wifiManager hotspot-config-set --ssidx Test > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetMissingValue)
{
    int result = system("ohos-wifiManager hotspot-config-set --ssid > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotUnknownAction)
{
    int result = system("ohos-wifiManager hotspot-config-update > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotBlocklistAddRemoved)
{
    int result = system("ohos-wifiManager hotspot-blocklist-add "
        "--name TestDevice --mac AA:BB:CC:DD:EE:FF > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotStationsListRemoved)
{
    int result = system("ohos-wifiManager hotspot-stations > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotEnableIdempotent)
{
    int result1 = system("ohos-wifiManager hotspot-enable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result1));
    int result2 = system("ohos-wifiManager hotspot-enable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result2));
}

TEST_F(WifiCliTest, TestHotspotDisableIdempotent)
{
    int result1 = system("ohos-wifiManager hotspot-disable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result1));
    int result2 = system("ohos-wifiManager hotspot-disable > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result2));
}

TEST_F(WifiCliTest, TestHotspotConfigSetSsid32)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--ssid 01234567890123456789012345678901 > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSetPassword63)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--preSharedKey 123456789012345678901234567890123456789012345678901234567890123 > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSetPassword64)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--preSharedKey 1234567890123456789012345678901234567890123456789012345678901234 > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetEmptySsid)
{
    int result = system("ohos-wifiManager hotspot-config-set --ssid \"\" > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetBandValid2G)
{
    int result = system("ohos-wifiManager hotspot-config-set --band 2.4G > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSetBandValid5G)
{
    int support = system("ohos-wifiManager hotspot-config-get 2>/dev/null | grep -q '\"5G\"'");
    if (support != 0) {
        GTEST_SKIP();
        return;
    }
    int result = system("ohos-wifiManager hotspot-config-set --band 5G > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSetBandAnyRejected)
{
    int result = system("ohos-wifiManager hotspot-config-set --band ANY > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetBandLowercase)
{
    int result = system("ohos-wifiManager hotspot-config-set --band 2.4g > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetDuplicateSsid)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--ssid TestDupA --ssid TestDupB > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

TEST_F(WifiCliTest, TestHotspotConfigSetPreSharedKeyMissingValue)
{
    int result = system("ohos-wifiManager hotspot-config-set --preSharedKey > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetBandMissingValue)
{
    int result = system("ohos-wifiManager hotspot-config-set --band > /dev/null 2>&1");
    EXPECT_NE(result, 0);
}

TEST_F(WifiCliTest, TestHotspotConfigSetAllFields)
{
    int result = system("ohos-wifiManager hotspot-config-set "
        "--ssid TestAllFields --preSharedKey 12345678 --band 2.4G > /dev/null 2>&1");
    EXPECT_TRUE(CliAcceptable(result));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
