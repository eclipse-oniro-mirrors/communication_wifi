/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_EVENT_SUBSCRIBER_MANAGER_H
#define OHOS_WIFI_EVENT_SUBSCRIBER_MANAGER_H

#ifndef OHOS_ARCH_LITE
#include <mutex>
#include <functional>
#include "wifi_errcode.h"
#include "wifi_internal_msg.h"
#include "wifi_system_ability_listerner.h"
#include "common_event_manager.h"
#include "wifi_event_handler.h"

namespace OHOS {
namespace Wifi {
#ifdef HAS_POWERMGR_PART
inline const std::string COMMON_EVENT_POWER_MANAGER_STATE_CHANGED = "usual.event.POWER_MANAGER_STATE_CHANGED";
#endif
const int CAST_ENGINE_SA_ID = 65546;
const int SHARE_SERVICE_ID = 2902;
const int MOUSE_CROSS_SERVICE_ID = 65569;
#ifdef SUPPORT_ClOUD_WIFI_ASSET
inline const std::string COMMON_EVENT_ASSETCLOUD_MANAGER_STATE_CHANGED = "usual.event.ASSET_SYNC_DATA_CHANGED_SA";
const int ASSETID = 6226;
#endif
class CesEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit CesEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CesEventSubscriber();
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData) override;
    void OnReceiveStandbyEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveScreenEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveAirplaneEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveBatteryEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveAppEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveThermalEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveNotificationEvent(const OHOS::EventFwk::CommonEventData &eventData);
    void OnReceiveUserUnlockedEvent(const OHOS::EventFwk::CommonEventData &eventData);
private:
    bool lastSleepState = false;
};

class NotificationEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit NotificationEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    virtual ~NotificationEventSubscriber();
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData) override;
};

#ifdef HAS_POWERMGR_PART
class PowermgrEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit PowermgrEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    virtual ~PowermgrEventSubscriber();
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData) override;
};
#endif
#ifdef SUPPORT_ClOUD_WIFI_ASSET
class AssetEventSubscriber : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit AssetEventSubscriber(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    virtual ~AssetEventSubscriber();
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &eventData) override;
};
#endif
class WifiEventSubscriberManager : public WifiSystemAbilityListener {
public:
    WifiEventSubscriberManager();
    virtual ~WifiEventSubscriberManager();

    void OnSystemAbilityChanged(int systemAbilityId, bool add) override;
    void GetAirplaneModeByDatashare();
    void GetWifiAllowSemiActiveByDatashare();
    bool GetLocationModeByDatashare();
    void DealLocationModeChangeEvent();
    void DealCloneDataChangeEvent();
    void CheckAndStartStaByDatashare();
    bool IsMdmForbidden(void);

private:
    void DelayedAccessDataShare();
    void InitSubscribeListener();
    bool IsDataMgrServiceActive();
    void HandleAppMgrServiceChange(bool add);
    void HandleCommNetConnManagerSysChange(int systemAbilityId, bool add);
#ifdef HAS_MOVEMENT_PART
    void HandleHasMovementPartChange(int systemAbilityId, bool add);
#endif
    void HandleDistributedKvDataServiceChange(bool add);
    void HandleCastServiceChange(bool add);
    void HandleShareServiceChange(bool add);
    void HandleMouseCrossServiceChange(bool add);
    int GetLastStaStateByDatashare();
    void GetCloneDataByDatashare(std::string &cloneData);
    void SetCloneDataByDatashare(const std::string &cloneData);
    void RegisterCloneEvent();
    void UnRegisterCloneEvent();
    void RegisterCesEvent();
#ifdef HAS_POWERMGR_PART
    void RegisterPowermgrEvent();
    void UnRegisterPowermgrEvent();
    std::shared_ptr<PowermgrEventSubscriber> wifiPowermgrEventSubsciber_ = nullptr;
    std::mutex powermgrEventMutex;
#endif
    void UnRegisterCesEvent();
    void RegisterLocationEvent();
    void UnRegisterLocationEvent();
    void RegisterNotificationEvent();
    void UnRegisterNotificationEvent();
    void GetMdmProp();
    void GetChipProp();
    void RegisterMdmPropListener();
    static void MdmPropChangeEvt(const char *key, const char *value, void *context);
#ifdef HAS_MOVEMENT_PART
    void RegisterMovementCallBack();
    void UnRegisterMovementCallBack();
#endif
#ifdef FEATURE_P2P_SUPPORT
    void HandleP2pBusinessChange(int systemAbilityId, bool add);
#endif
#ifdef SUPPORT_ClOUD_WIFI_ASSET
    void RegisterAssetEvent();
    void UnRegisterAssetEvent();
#endif
private:
    std::mutex cloneEventMutex;
    uint32_t cesTimerId{0};
    uint32_t notificationTimerId{0};
    uint32_t accessDatashareTimerId{0};
    std::mutex cesEventMutex;
    std::mutex notificationEventMutex;
    bool isCesEventSubscribered = false;
    std::shared_ptr<CesEventSubscriber> cesEventSubscriber_ = nullptr;
    std::shared_ptr<NotificationEventSubscriber> wifiNotificationSubsciber_ = nullptr;
#ifdef HAS_MOVEMENT_PART
    std::mutex deviceMovementEventMutex;
#endif
    static bool mIsMdmForbidden;
    bool islocationModeObservered = false;
    std::mutex locationEventMutex;
    std::unique_ptr<WifiEventHandler> mWifiEventSubsThread = nullptr;
#ifdef SUPPORT_ClOUD_WIFI_ASSET
    std::shared_ptr<AssetEventSubscriber> wifiAssetrEventSubsciber_ = nullptr;
    std::mutex AssetEventMutex;
    uint32_t assetMgrId{0};
#endif
};

}  // namespace Wifi
}  // namespace OHOS
#endif
#endif // OHOS_WIFI_EVENT_SUBSCRIBER_MANAGER_H
