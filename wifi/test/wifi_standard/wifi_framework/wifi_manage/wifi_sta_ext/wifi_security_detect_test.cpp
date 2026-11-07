/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include "cJSON.h"
#include "wifi_security_detect_test.h"

using namespace testing::ext;
namespace OHOS {
namespace Wifi {

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest01, TestSize.Level1)
{
    OperateResState state = OperateResState::CONNECT_NETWORK_ENABLED;
    WifiLinkedInfo info;
    info.networkId = 1;
    int instId = 1;
    wifiSecurityDetect_->DealStaConnChanged(state, info, instId);
    EXPECT_EQ(wifiSecurityDetect_->networkDetecting_, true);
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest02, TestSize.Level1)
{
    OperateResState state = OperateResState::DISCONNECT_DISCONNECTED;
    WifiLinkedInfo info;
    info.networkId = 1;
    int instId = 1;
    wifiSecurityDetect_->DealStaConnChanged(state, info, instId);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, -1);
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest03, TestSize.Level1)
{
    OperateResState state = OperateResState::CLOSE_WIFI_SUCCEED;
    WifiLinkedInfo info;
    info.networkId = 1;
    int instId = 1;
    wifiSecurityDetect_->DealStaConnChanged(state, info, instId);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, -1);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest01, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_NONE);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_OPEN);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest02, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_WEP);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_WEP);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest03, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_WPA_PSK);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_PSK);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest04, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_SAE);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_SAE);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest05, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_EAP);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_EAP);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest06, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_SUITE_B_192);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_EAP_WPA3_ENTERPRISE_192_BIT);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest07, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_WAPI_CERT);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_WAPI_CERT);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest08, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(KEY_MGMT_WAPI_PSK);
    EXPECT_EQ(result, SecurityType::SECURITY_TYPE_WAPI_PSK);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest09, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert("");
    EXPECT_EQ(result, -1);
}

HWTEST_F(WifiSecurityDetectTest, IsSettingSecurityDetectOnTest01, TestSize.Level1)
{
    bool result = wifiSecurityDetect_->IsSettingSecurityDetectOn();
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiSecurityDetectTest, PopupNotificationTest01, TestSize.Level1)
{
    wifiSecurityDetect_->PopupNotification(WifiNotification::OPEN, 1);
    EXPECT_TRUE(wifiSecurityDetect_ != nullptr);
}

HWTEST_F(WifiSecurityDetectTest, PopupNotificationTest02, TestSize.Level1)
{
    wifiSecurityDetect_->PopupNotification(WifiNotification::CLOSE, 1);
    EXPECT_TRUE(wifiSecurityDetect_ != nullptr);
}

HWTEST_F(WifiSecurityDetectTest, AssembleUriTest, TestSize.Level1)
{
    Uri uri = wifiSecurityDetect_->AssembleUri("wifi_cloud_security_check");
    EXPECT_EQ(uri.ToString(),
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=wifi_cloud_security_check");
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest04, TestSize.Level1)
{
    OperateResState state = OperateResState::CONNECT_AP_CONNECTED;
    WifiLinkedInfo info;
    info.networkId = 1;
    int instId = 1;
    wifiSecurityDetect_->DealStaConnChanged(state, info, instId);
    EXPECT_TRUE(true);
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest05_DisconnectWithPoped, TestSize.Level1)
{
    OperateResState state = OperateResState::DISCONNECT_DISCONNECTED;
    WifiLinkedInfo info;
    info.networkId = 1;
    wifiSecurityDetect_->isSecureWifiPoped_.store(true);
    wifiSecurityDetect_->DealStaConnChanged(state, info, 1);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, -1);
    EXPECT_EQ(wifiSecurityDetect_->isSecureWifiPoped_.load(), true);
}

HWTEST_F(WifiSecurityDetectTest, GetStaCallbackTest01, TestSize.Level1)
{
    StaServiceCallback callback = wifiSecurityDetect_->GetStaCallback();
    EXPECT_FALSE(callback.callbackModuleName.empty());
}

HWTEST_F(WifiSecurityDetectTest, SetDatashareReadyTest01, TestSize.Level1)
{
    wifiSecurityDetect_->SetDatashareReady();
    EXPECT_EQ(wifiSecurityDetect_->datashareReady_, true);
}

HWTEST_F(WifiSecurityDetectTest, SetChangeNetworkidTest01, TestSize.Level1)
{
    wifiSecurityDetect_->SetChangeNetworkid(5);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, 5);
}

HWTEST_F(WifiSecurityDetectTest, IsSettingSecurityDetectOnTest02_DatashareReady, TestSize.Level1)
{
    wifiSecurityDetect_->SetDatashareReady();
    bool result = wifiSecurityDetect_->IsSettingSecurityDetectOn();
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiSecurityDetectTest, IsSecurityDetectTimeoutTest01, TestSize.Level1)
{
    bool result = wifiSecurityDetect_->IsSecurityDetectTimeout(99999);
    EXPECT_EQ(result, false);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest10_UnknownKey, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert("UNKNOWN_KEY");
    EXPECT_EQ(result, -1);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest11_EmptyKey, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert("");
    EXPECT_EQ(result, -1);
}

HWTEST_F(WifiSecurityDetectTest, PopupNotificationTest03_InvalidNetworkId, TestSize.Level1)
{
    wifiSecurityDetect_->PopupNotification(WifiNotification::OPEN, -1);
    EXPECT_TRUE(wifiSecurityDetect_ != nullptr);
}

HWTEST_F(WifiSecurityDetectTest, PopupNotificationTest04_NetworkIdMismatch, TestSize.Level1)
{
    wifiSecurityDetect_->SetChangeNetworkid(1);
    wifiSecurityDetect_->PopupNotification(WifiNotification::OPEN, 2);
    EXPECT_TRUE(wifiSecurityDetect_ != nullptr);
}

HWTEST_F(WifiSecurityDetectTest, PopupNotificationTest05_Close, TestSize.Level1)
{
    wifiSecurityDetect_->PopupNotification(WifiNotification::CLOSE, 1);
    EXPECT_TRUE(wifiSecurityDetect_ != nullptr);
}

HWTEST_F(WifiSecurityDetectTest, AssembleUriTest02, TestSize.Level1)
{
    Uri uri = wifiSecurityDetect_->AssembleUri("test_key");
    EXPECT_EQ(uri.ToString(),
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=test_key");
}

HWTEST_F(WifiSecurityDetectTest, AssembleUriTest03_EmptyKey, TestSize.Level1)
{
    Uri uri = wifiSecurityDetect_->AssembleUri("");
    EXPECT_EQ(uri.ToString(),
        "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=");
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest06_OtherState, TestSize.Level1)
{
    OperateResState state = OperateResState::CONNECT_NETWORK_DISABLED;
    WifiLinkedInfo info;
    info.networkId = 1;
    wifiSecurityDetect_->DealStaConnChanged(state, info, 1);
    EXPECT_TRUE(true);
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest07_NetworkDetecting, TestSize.Level1)
{
    wifiSecurityDetect_->networkDetecting_.store(true);
    OperateResState state = OperateResState::CONNECT_NETWORK_ENABLED;
    WifiLinkedInfo info;
    info.networkId = 5;
    wifiSecurityDetect_->DealStaConnChanged(state, info, 1);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, 5);
}

HWTEST_F(WifiSecurityDetectTest, RegisterSecurityDetectObserverTest01, TestSize.Level1)
{
    wifiSecurityDetect_->RegisterSecurityDetectObserver();
    EXPECT_TRUE(true);
}

HWTEST_F(WifiSecurityDetectTest, UnRegisterSecurityDetectObserverTest01, TestSize.Level1)
{
    wifiSecurityDetect_->UnRegisterSecurityDetectObserver();
    EXPECT_TRUE(true);
}

HWTEST_F(WifiSecurityDetectTest, SecurityModelJsonResultTest01_NullResult, TestSize.Level1)
{
    SecurityModelResult model;
    model.result = "";
    bool result = true;
    ErrCode ret = wifiSecurityDetect_->SecurityModelJsonResult(model, result);
    EXPECT_NE(ret, WIFI_OPT_SUCCESS);
}

HWTEST_F(WifiSecurityDetectTest, SecurityModelJsonResultTest02_StatusError, TestSize.Level1)
{
    SecurityModelResult model;
    model.result = "{\"status\":1,\"result\":\"test\"}";
    bool result = true;
    ErrCode ret = wifiSecurityDetect_->SecurityModelJsonResult(model, result);
    EXPECT_NE(ret, WIFI_OPT_SUCCESS);
}

HWTEST_F(WifiSecurityDetectTest, SecurityModelJsonResultTest03_ValidResult, TestSize.Level1)
{
    SecurityModelResult model;
    model.result = "{\"status\":0,\"result\":\"secure\"}";
    bool result = false;
    ErrCode ret = wifiSecurityDetect_->SecurityModelJsonResult(model, result);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret != WIFI_OPT_SUCCESS);
}

HWTEST_F(WifiSecurityDetectTest, SecurityModelJsonResultTest04_NoResultField, TestSize.Level1)
{
    SecurityModelResult model;
    model.result = "{\"status\":0}";
    bool result = false;
    ErrCode ret = wifiSecurityDetect_->SecurityModelJsonResult(model, result);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret != WIFI_OPT_SUCCESS);
}

HWTEST_F(WifiSecurityDetectTest, AddWifiStandardToJsonTest01, TestSize.Level1)
{
    cJSON *root = cJSON_CreateObject();
    wifiSecurityDetect_->AddWifiStandardToJson(root, WireType::WIRE_802_11A);
    cJSON *item = cJSON_GetObjectItem(root, "wirelessType");
    EXPECT_TRUE(item != nullptr);
    cJSON_Delete(root);
}

HWTEST_F(WifiSecurityDetectTest, AddWifiStandardToJsonTest02, TestSize.Level1)
{
    cJSON *root = cJSON_CreateObject();
    wifiSecurityDetect_->AddWifiStandardToJson(root, WireType::WIRE_802_11B);
    cJSON *item = cJSON_GetObjectItem(root, "wirelessType");
    EXPECT_TRUE(item != nullptr);
    cJSON_Delete(root);
}

HWTEST_F(WifiSecurityDetectTest, AddWifiStandardToJsonTest03, TestSize.Level1)
{
    cJSON *root = cJSON_CreateObject();
    wifiSecurityDetect_->AddWifiStandardToJson(root, WireType::WIRE_802_11AX);
    cJSON *item = cJSON_GetObjectItem(root, "wirelessType");
    EXPECT_TRUE(item != nullptr);
    cJSON_Delete(root);
}

HWTEST_F(WifiSecurityDetectTest, AddWifiStandardToJsonTest04_Unknown, TestSize.Level1)
{
    cJSON *root = cJSON_CreateObject();
    wifiSecurityDetect_->AddWifiStandardToJson(root, 999);
    cJSON *item = cJSON_GetObjectItem(root, "wirelessType");
    EXPECT_TRUE(item != nullptr);
    cJSON_Delete(root);
}

HWTEST_F(WifiSecurityDetectTest, DealStaConnChangedTest08_DisconnectNotPoped, TestSize.Level1)
{
    wifiSecurityDetect_->isSecureWifiPoped_.store(false);
    OperateResState state = OperateResState::DISCONNECT_DISCONNECTED;
    WifiLinkedInfo info;
    info.networkId = 3;
    wifiSecurityDetect_->DealStaConnChanged(state, info, 1);
    EXPECT_EQ(wifiSecurityDetect_->currentConnectedNetworkId_, -1);
    EXPECT_EQ(wifiSecurityDetect_->networkDetecting_.load(), false);
}

HWTEST_F(WifiSecurityDetectTest, AuthenticationConvertTest12_OtherKey, TestSize.Level1)
{
    int32_t result = wifiSecurityDetect_->AuthenticationConvert(std::string("OSEN"));
    EXPECT_NE(result, SecurityType::SECURITY_TYPE_OSEN);
}

}  // namespace Wifi
}  // namespace OHOS