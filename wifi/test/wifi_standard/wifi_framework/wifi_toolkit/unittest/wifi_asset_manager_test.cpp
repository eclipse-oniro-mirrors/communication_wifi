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
#include <gtest/gtest.h>
#include "wifi_asset_manager.h"
 
using namespace testing::ext;
namespace OHOS {
namespace Wifi {
#ifdef SUPPORT_ClOUD_WIFI_ASSET
class WifiAssetManagerTest : public testing::Test {
protected:
    void SetUp() override {
        // Set up code here
    }
 
    void TearDown() override {
        // Tear down code here
    }
};
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetUpdate, testing::ext::TestSize.Level1)
{
    WifiDeviceConfig config;
    int32_t userId = USER_ID_DEFAULT;
    WifiAssetManager::GetInstance().WifiAssetUpdate(config, userId);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetAdd, testing::ext::TestSize.Level1)
{
    WifiDeviceConfig config;
    int32_t userId = USER_ID_DEFAULT;
    bool flagSync = true;
    WifiAssetManager::GetInstance().WifiAssetAdd(config, userId, flagSync);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetQuery, testing::ext::TestSize.Level1)
{
    int32_t userId = USER_ID_DEFAULT;
    WifiAssetManager::GetInstance().WifiAssetQuery(userId);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetRemove, testing::ext::TestSize.Level1)
{
    WifiDeviceConfig config;
    int32_t userId = USER_ID_DEFAULT;
    bool flagSync = true;
    WifiAssetManager::GetInstance().WifiAssetRemove(config, userId, flagSync);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetAddPack, testing::ext::TestSize.Level1)
{
    std::vector<WifiDeviceConfig> mWifiDeviceConfig;
    int32_t userId = USER_ID_DEFAULT;
    bool flagSync = true;
    WifiAssetManager::GetInstance().WifiAssetAddPack(mWifiDeviceConfig, userId, flagSync);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetRemovePack, testing::ext::TestSize.Level1)
{
    std::vector<WifiDeviceConfig> mWifiDeviceConfig;
    int32_t userId = USER_ID_DEFAULT;
    bool flagSync = true;
    WifiAssetManager::GetInstance().WifiAssetRemovePack(mWifiDeviceConfig, userId, flagSync);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetRemoveAll, testing::ext::TestSize.Level1)
{
    int32_t userId = USER_ID_DEFAULT;
    bool flagSync = true;
    WifiAssetManager::GetInstance().WifiAssetRemoveAll(userId, flagSync);
}
 
HWTEST_F(WifiAssetManagerTest, TestWifiAssetUpdatePack, testing::ext::TestSize.Level1)
{
    std::vector<WifiDeviceConfig> mWifiDeviceConfig;
    int32_t userId = USER_ID_DEFAULT;
    WifiAssetManager::GetInstance().WifiAssetUpdatePack(mWifiDeviceConfig, userId);
}
#endif
}  // namespace Wifi
}  // namespace OHOS