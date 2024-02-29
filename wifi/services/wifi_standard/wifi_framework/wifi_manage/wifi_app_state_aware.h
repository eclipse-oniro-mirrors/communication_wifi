/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_APP_STATE_AWARE_H
#define OHOS_WIFI_APP_STATE_AWARE_H

#ifndef OHOS_ARCH_LITE
#include "app_mgr_interface.h"
#include "app_state_data.h"
#include "iremote_object.h"
#include "wifi_event_handler.h"
#include "wifi_errcode.h"
#include "application_state_observer_stub.h"

namespace OHOS {
namespace Wifi {

class AppStateObserver;

class WifiAppStateAware {
public:
    explicit WifiAppStateAware(int instId = 0);
    ~WifiAppStateAware();
    static WifiAppStateAware &GetInstance();
    ErrCode InitAppStateAware();
    void RegisterAppStateObserver();
    void OnForegroundAppChanged(const std::string &bundleName, int uid, int pid,
        const int state, const int mInstId = 0);
    bool IsForegroundApp(std::string &bundleName);
    std::string GetForegroundApp();

private:
    std::mutex mutex_ {};
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ {nullptr};
    std::string foregroundAppBundleName_;
    std::unique_ptr<WifiEventHandler> appChangeEventHandler = nullptr;
    sptr<AppStateObserver> mAppStateObserver {nullptr};
    sptr<AppExecFwk::IAppMgr> mAppObject {nullptr};
};

class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    /**
     * Will be called when the application start.
     *
     * @param appStateData Application state data.
     */
    void OnAppStarted(const AppExecFwk::AppStateData &appStateData) override;

    /**
     * Will be called when the application stop.
     *
     * @param appStateData Application state data.
     */
    void OnAppStopped(const AppExecFwk::AppStateData &appStateData) override;

    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;
};
} // namespace Wifi
} // namespace OHOS
#endif
#endif