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
#include "wifi_event_subscriber_manager_test.h"
#include "wifi_logger.h"
#include "wifi_manager.h"
#include "mock_wifi_config_center.h"

using namespace testing::ext;
DEFINE_WIFILOG_LABEL("WifiEventSubscriberManagerTest");


namespace OHOS {
namespace Wifi {

constexpr int32_t COMMON_EVENT_SERVICE_ID = 3299;

namespace {
    EventFwk::CommonEventSubscribeInfo BuildSubscribeInfo(const std::string &action)
    {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(action);
        return EventFwk::CommonEventSubscribeInfo(matchingSkills)
    }
}

HWTEST_F(WifiEventSubscriberManagerTest, HandleCesServiceChange_Add_True, TestSize.Level1)
{
    WIFI_LOGE("HandleCesServiceChange_Add_True enter!");
    WifiEventSubscriberManager manager;
    EXPECT_NO_FATAL_FAILURE(manager.HandleCesServiceChange(true));
}

HWTEST_F(WifiEventSubscriberManagerTest, HandleCesServiceChange_Add_False, TestSize.Level1)
{
    WIFI_LOGE("HandleCesServiceChange_Add_False enter!");
    WifiEventSubscriberManager manager;
    EXPECT_NO_FATAL_FAILURE(manager.HandleCesServiceChange(false));
}

HWTEST_F(WifiEventSubscriberManagerTest, OnSystemAbilityChanged_CesSa_Add, TestSize.Level1)
{
    WIFI_LOGE("OnSystemAbilityChanged_CesSa_Add enter!");
    WifiEventSubscriberManager manager;
    EXPECT_NO_FATAL_FAILURE(manager.OnSystemAbilityChanged(COMMON_EVENT_SERVICE_ID, true));
}

HWTEST_F(WifiEventSubscriberManagerTest, OnSystemAbilityChanged_CesSa_Remove, TestSize.Level1)
{
    WIFI_LOGE("OnSystemAbilityChanged_CesSa_Remove enter!");
    WifiEventSubscriberManager manager;
    EXPECT_NO_FATAL_FAILURE(manager.OnSystemAbilityChanged(COMMON_EVENT_SERVICE_ID, false));
}

HWTEST_F(WifiEventSubscriberManagerTest, OnSystemAbilityChanged_OtherSa, TestSize.Level1)
{
    WIFI_LOGE("OnSystemAbilityChanged_OtherSa enter!");
    WifiEventSubscriberManager manager;
    EXPECT_NO_FATAL_FAILURE(manager.OnSystemAbilityChanged(1234, true));
    EXPECT_NO_FATAL_FAILURE(manager.OnSystemAbilityChanged(1234, false));
}

#ifdef HAS_POWERMGR_PART
namespace {
constexpr int POWER_EVENT_CODE_ENTER_FORCESLEEP = 48;
constexpr int POWER_EVENT_CODE_SCREENOFF_HALF_HOUR_FILTER = 224;
constexpr int POWER_EVENT_CODE_UNKNOWN = 0;

EventFwk::CommonEventData BuildPowermgrEvent(const std::string &action, int code)
{
    AAFwk::Want want;
    want.SetAction(action);
    EventFwk::CommonEventData eventData;
    eventData.SetWant(want);
    eventData.SetCode(code);
    return eventData;
}

void InstallTogglerForTest()
{
    WifiManager::GetInstance().GetWifiTogglerManager() = std::make_unique<WifiTogglerManager>();
}

void UninstallTogglerForTest()
{
    WifiManager::GetInstance().GetWifiTogglerManager().reset();
}
}  // namespace

HWTEST_F(WifiEventSubscriberManagerTest, PowermgrEvent_OnReceiveEvent_ForceSleep_TurnOffSoftap, TestSize.Level1)
{
    WIFI_LOGE("PowermgrEvent_OnReceiveEvent_ForceSSleep_TurnOffSoftap enter!");
    InstallTogglerForTest();
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetSoftapToggledState(false)).Times(1);
    PowermgrEventSubscriber subscribe(BuildSubscribeInfo(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED));
    EXPECT_NO_FATAL_FAILURE(subscribe.OnReceiveEvent(
        BuildPowermgrEvent(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED, POWER_EVENT_CODE_ENTER_FORCESLEEP)));
    UninstallTogglerForTest();
}

HWTEST_F(WifiEventSubscriberManagerTest, PowermgrEvent_OnReceiveEvent_ScreenoffHalfHour_TurnOffSoftap, TestSize.Level1)
{
    WIFI_LOGE("PowermgrEvent_OnReceiveEvent_ScreenoffHalfHour_TurnOffSoftap enter!");
    InstallTogglerForTest();
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetSoftapToggledState(false)).Times(1);
    PowermgrEventSubscriber subscribe(BuildSubscribeInfo(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED));
    EXPECT_NO_FATAL_FAILURE(subscribe.OnReceiveEvent(
        BuildPowermgrEvent(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED, POWER_EVENT_CODE_SCREENOFF_HALF_HOUR_FILTER)));
    UninstallTogglerForTest();
}

HWTEST_F(WifiEventSubscriberManagerTest, PowermgrEvent_OnReceiveEvent_NonTargetCode_NoOp_TurnOffSoftap, TestSize.Level1)
{
    WIFI_LOGE("PowermgrEvent_OnReceiveEvent_NonTargetCode_NoOp_TurnOffSoftap enter!");
    InstallTogglerForTest();
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetSoftapToggledState(false)).Times(0);
    PowermgrEventSubscriber subscribe(BuildSubscribeInfo(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED));
    EXPECT_NO_FATAL_FAILURE(subscribe.OnReceiveEvent(
        BuildPowermgrEvent(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED, POWER_EVENT_CODE_UNKNOWN)));
    UninstallTogglerForTest();
}

HWTEST_F(WifiEventSubscriberManagerTest, PowermgrEvent_OnReceiveEvent_UnknownAction_NoOp_TurnOffSoftap, TestSize.Level1)
{
    WIFI_LOGE("PowermgrEvent_OnReceiveEvent_UnknownAction_NoOp_TurnOffSoftap enter!");
    InstallTogglerForTest();
    EXPECT_CALL(WifiConfigCenter::GetInstance(), SetSoftapToggledState(false)).Times(0);
    PowermgrEventSubscriber subscribe(BuildSubscribeInfo("test.unknown.action"));
    EXPECT_NO_FATAL_FAILURE(subscribe.OnReceiveEvent(
        BuildPowermgrEvent("test.unknown.action", POWER_EVENT_CODE_UNKNOWN)));
    UninstallTogglerForTest();
}

HWTEST_F(WifiEventSubscriberManagerTest,
    DISABLED_PowermgrEvent_OnReceiveEvent_EmptyToggler_ShouldNotCrash, TestSize.Level1)
{
    WIFI_LOGE("PowermgrEvent_OnReceiveEvent_EmptyToggler_ShouldNotCrash enter!");
    WifiManager::GetInstance().GetWifiTogglerManager().reset();
    PowermgrEventSubscriber subscribe(BuildSubscribeInfo(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED));
    EXPECT_NO_FATAL_FAILURE(subscribe.OnReceiveEvent(
        BuildPowermgrEvent(COMMON_EVENT_POWER_MANAGER_STATE_CHANGED, POWER_EVENT_CODE_ENTER_FORCESLEEP)));
}
#endif
}  // namespace Wifi
}  // namespace OHOS
