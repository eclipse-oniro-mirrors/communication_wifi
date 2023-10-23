/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "wifi_protect_manager.h"
#include "wifi_log.h"
#include "wifi_chip_hal_interface.h"
#include "wifi_supplicant_hal_interface.h"
#ifndef OHOS_ARCH_LITE
#include "system_ability_definition.h"
#include "connection_observer_client.h"
#include "app_mgr_client.h"
#include "app_mgr_interface.h"
#include "app_process_data.h"
#include "iservice_registry.h"
#include "app_mgr_constants.h"
#include "wifi_common_util.h"
#endif

#undef LOG_TAG
#define LOG_TAG "OHWIFI_MANAGER_LOCK_MANAGER"

namespace OHOS {
namespace Wifi {
constexpr const int WIFI_PROTECT_APP_MAX_COUNT = 100;
constexpr const char *WIFI_APP_CHANGE_MGR_WORK_THREAD = "WIFI_APP_CHANGE_MGR_WORK_THREAD";
WifiProtectManager::WifiProtectManager()
{
    mWifiConnected = false;
    mScreenOn = false;
    mForceHiPerfMode = false;
    mForceLowLatencyMode = false;
    mCurrentOpMode = WifiProtectMode::WIFI_PROTECT_NO_HELD;
    mFullHighPerfProtectsAcquired = 0;
    mFullHighPerfProtectsReleased = 0;
    mFullLowLatencyProtectsAcquired = 0;
    mFullLowLatencyProtectsReleased = 0;
    mWifiProtects.clear();
#ifndef OHOS_ARCH_LITE
    mAppChangeEventRunner = AppExecFwk::EventRunner::Create(WIFI_APP_CHANGE_MGR_WORK_THREAD);
    if (!mAppChangeEventRunner) {
        LOGE("Create event runner failed.");
        return;
    }
    mAppChangeEventHandler = std::make_shared<AppExecFwk::EventHandler>(mAppChangeEventRunner);
    if (mAppChangeEventHandler) {
        std::function<void()> RegisterAppStateObserverFunc =
                            std::bind(&WifiProtectManager::RegisterAppStateObserver, this);
        mAppChangeEventHandler->PostSyncTask(RegisterAppStateObserverFunc);
    } else {
        LOGE("Create event handler failed.");
    }
    LOGI("Register app state observer successful.");
#endif
}

WifiProtectManager::~WifiProtectManager()
{
#ifndef OHOS_ARCH_LITE
    if (mAppChangeEventRunner) {
        mAppChangeEventRunner->Stop();
        mAppChangeEventRunner.reset();
    }

    if (mAppChangeEventHandler) {
        mAppChangeEventHandler.reset();
    }

    if (mAppStateObserver) {
        mAppStateObserver = nullptr;
    }
#endif
}

WifiProtectManager &WifiProtectManager::GetInstance()
{
    static WifiProtectManager instance;
    return instance;
}

bool WifiProtectManager::IsValidProtectMode(const WifiProtectMode &protectMode)
{
    if (protectMode != WifiProtectMode::WIFI_PROTECT_FULL && 
        protectMode != WifiProtectMode::WIFI_PROTECT_SCAN_ONLY &&
        protectMode != WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF &&
        protectMode != WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY) {
        return false;
    }

    return true;
}

WifiProtectMode WifiProtectManager::GetNearlyProtectMode()
{
#ifndef OHOS_ARCH_LITE
    WifiLinkedInfo linkedInfo;
    WifiSettings::GetInstance().GetLinkedInfo(linkedInfo);
    mWifiConnected = (linkedInfo.connState == ConnState::CONNECTED) ? true : false;

    int screenState = WifiSettings::GetInstance().GetScreenState();
    mScreenOn = (screenState == MODE_STATE_OPEN) ? true : false;
    int foregroudCount = GetFgLowlatyProtectCount();
    LOGI("%{public}s mWifiConnected: %{public}d, mScreenOn: %{public}d,"
        "ForegroundProtectCount: %{public}d, mForceHiPerfMode: %{public}d, mForceLowLatencyMode: %{public}d",
        __func__, mWifiConnected, mScreenOn, foregroudCount, mForceHiPerfMode, mForceLowLatencyMode);
#endif
    /* If Wifi Client is not connected, then all protects are not effective */
    if (!mWifiConnected) {
        return WifiProtectMode::WIFI_PROTECT_NO_HELD;
    }

    /* Check if mode is forced to hi-perf */
    if (mForceHiPerfMode) {
        return WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF;
    }

    /* Check if mode is forced to low-latency */
    if (mForceLowLatencyMode) {
        return WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY;
    }
#ifndef OHOS_ARCH_LITE
    if (mScreenOn && (foregroudCount > 0) &&
        (mFullLowLatencyProtectsAcquired > mFullLowLatencyProtectsReleased)) {
        return WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY;
    }
    if ((mFullHighPerfProtectsAcquired > mFullHighPerfProtectsReleased) &&
        mWifiProtects.size() > 0) {
#else
    if (mFullHighPerfProtectsAcquired > mFullHighPerfProtectsReleased) {
#endif
        return WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF;
    }

    return WifiProtectMode::WIFI_PROTECT_NO_HELD;
}

bool WifiProtectManager::InitWifiProtect(
    const WifiProtectType &protectType,
    const std::string &protectName)
{
    WifiProtect* pProtect = new WifiProtect(protectType,
        WifiProtectMode::WIFI_PROTECT_FULL, protectName);
    mWifiProtects.push_back(pProtect);
    return true;
}

bool WifiProtectManager::GetWifiProtect(
    const WifiProtectMode &protectMode,
    const std::string name)
{
    LOGD("%{public}s mode: %{public}d, bundlename: %{public}s",
        __func__, static_cast<int>(protectMode), name.c_str());
#ifndef OHOS_ARCH_LITE
    if (!IsValidProtectMode(protectMode) || name.empty()) {
        LOGE("Input para protectMode[%{public}d] or name[%{public}s] invalid",
            static_cast<int>(protectMode), name.c_str());
        return false;
    }
    WifiProtectMode curProtectMode;
#endif
    bool isAlreadyExist = false;
    std::unique_lock<std::mutex> lock(mMutex);
    std::vector<WifiProtect *>::iterator itor = mWifiProtects.begin();
    while (itor != mWifiProtects.end()) {
        if ((*itor)->GetName() == name) {
            isAlreadyExist = true;
#ifndef OHOS_ARCH_LITE
            curProtectMode = (*itor)->GetProtectMode();
#endif
            break;
        }
        itor++;
    }

    if (isAlreadyExist) {
#ifndef OHOS_ARCH_LITE
        if (curProtectMode == protectMode) {
            LOGW("attempted to add a protect when already holding one");
            return true;
        } else {
            LOGE("attempted to add a different protect mode to already holding one,"
                "please release holded protect first!");
            return false;
        }
#else
        LOGE("attempted to add a protect when already holding one");
        return false;
#endif
    }
#ifndef OHOS_ARCH_LITE
    if (mWifiProtects.size() >= WIFI_PROTECT_APP_MAX_COUNT) {
        LOGE("Wifi protect app count out of range[%d].", WIFI_PROTECT_APP_MAX_COUNT);
        return false;
    }
#endif
    return AddProtect(protectMode, name);
}

bool WifiProtectManager::IsForegroundApp(const std::string &BundleName)
{
    bool isForegroud = false;
    std::vector<AppExecFwk::AppStateData> fgList;
    if (mAppObject &&
        mAppObject->GetForegroundApplications(fgList) == static_cast<int32_t>(WIFI_OPT_SUCCESS)) {
        std::vector<AppExecFwk::AppStateData>::iterator itor = fgList.begin();
        while (itor != fgList.end()) {
            LOGD("Match foreground bundle name = %{public}s", (*itor).bundleName.c_str());
            if ((*itor).bundleName == BundleName) {
                isForegroud = true;
                break;
            }
            itor++;
        }
    }
    return isForegroud;
}

bool WifiProtectManager::ChangeToPerfMode(bool isEnabled)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mForceHiPerfMode = isEnabled;
    mForceLowLatencyMode = false;
    if (!ChangeWifiPowerMode()) {
        LOGE("Failed to force hi-perf mode, returning to normal mode");
        mForceHiPerfMode = false;
        return false;
    }

    return true;
}
void WifiProtectManager::HandleScreenStateChanged(bool screenOn)
{

    std::unique_lock<std::mutex> lock(mMutex);
    mScreenOn = screenOn;
    LOGD("%{public}s screen is on: %{public}d", __func__, mScreenOn);

#ifndef OHOS_ARCH_LITE
    if (ChangeWifiPowerMode()) {
        LOGE("Failed to update wifi power mode for screen state change");
    }
#endif
}

void WifiProtectManager::UpdateWifiClientConnected(bool isConnected)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mWifiConnected = isConnected;
    LOGD("%{public}s wifi connected: %{public}d", __func__, mWifiConnected);

#ifndef OHOS_ARCH_LITE
    if (ChangeWifiPowerMode()) {
        LOGE("Failed to update wifi power mode for connect state change");
    }
#endif
}

bool WifiProtectManager::AddProtect(
    const WifiProtectMode &protectMode,
    const std::string &name)
{
#ifndef OHOS_ARCH_LITE
    WifiProtect *pProtect = new WifiProtect(name);
    if (pProtect == nullptr) {
        LOGE("Wifi protect pointer is null.");
        return false;
    }
    int state = static_cast<int>(AppExecFwk::ApplicationState::APP_STATE_END);
    if (IsForegroundApp(name)) {
        state = static_cast<int>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    }
    LOGD("%{public}s bundle name: %{public}s state: %{public}d",
        __func__, name.c_str(), state);
    pProtect->SetAppState(state);
#endif
    pProtect->SetProtectMode(protectMode);

    mWifiProtects.push_back(pProtect);
    switch (pProtect->GetProtectMode()) {
        case WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF:
            ++mFullHighPerfProtectsAcquired;
            break;
        case WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY:
            ++mFullLowLatencyProtectsAcquired;
            break;
        default:
            break;
    }
    return ChangeWifiPowerMode();
}

bool WifiProtectManager::PutWifiProtect(const std::string &name)
{
    LOGI("%{public}s enter bundlename: %{public}s", __func__, name.c_str());
    if (name.empty()) {
        LOGE("invalid bundlename: %{public}s", name.c_str());
        return false;
    }
    std::unique_lock<std::mutex> lock(mMutex);
    WifiProtect *pWifiProtect = RemoveProtect(name);
    if (pWifiProtect == nullptr) {
        /* attempting to release a protect that does not exist. */
        return false;
    }
    switch (pWifiProtect->GetProtectMode()) {
        case WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF:
            ++mFullHighPerfProtectsReleased;
            break;
        case WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY:
            ++mFullLowLatencyProtectsReleased;
            break;
        default:
            break;
    }

    /* Recalculate the operating mode */
    bool ret = ChangeWifiPowerMode();
    delete pWifiProtect;
    pWifiProtect = nullptr;
    return ret;
}

WifiProtect *WifiProtectManager::RemoveProtect(const std::string &name)
{
    WifiProtect *pProtect = nullptr;
    std::vector<WifiProtect *>::iterator itor = mWifiProtects.begin();
    while (itor != mWifiProtects.end()) {
        if ((*itor)->GetName() == name) {
            pProtect = *itor;
            itor = mWifiProtects.erase(itor);
            break;
        }
        itor++;
    }
    return pProtect;
}

bool WifiProtectManager::ChangeWifiPowerMode()
{
    WifiProtectMode newProtectMode = GetNearlyProtectMode();
    LOGD("%{public}s currMode: %{public}d, newMode: %{public}d",
        __func__, static_cast<int>(mCurrentOpMode), static_cast<int>(newProtectMode));
    if (newProtectMode == mCurrentOpMode) {
        /* No action is needed */
        LOGI("newProtectMode %{public}d equal to mCurrentOpMode %{public}d, no action is needed",
            static_cast<int>(newProtectMode), static_cast<int>(mCurrentOpMode));
        return true;
    }

    /* Otherwise, we need to change current mode, first reset it to normal */
    switch (mCurrentOpMode) {
        case WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF:
            if (WifiSupplicantHalInterface::GetInstance().SetPowerSave(true) != WIFI_IDL_OPT_OK) {
                LOGE("%{public}s Failed to reset the OpMode from hi-perf to Normal", __func__);
                return false;
            }
            break;
        case WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY:
            if (!SetLowLatencyMode(false)) {
                LOGE("%{public}s Failed to reset the OpMode from low-latency to normal", __func__);
                return false;
            }
            break;
        case WifiProtectMode::WIFI_PROTECT_NO_HELD:
        default:
            /* No action */
            break;
    }

    /* Set the current mode, before we attempt to set the new mode */
    mCurrentOpMode = WifiProtectMode::WIFI_PROTECT_NO_HELD;

    /* Now switch to the new opMode */
    switch (newProtectMode) {
        case WifiProtectMode::WIFI_PROTECT_FULL_HIGH_PERF:
            if (WifiSupplicantHalInterface::GetInstance().SetPowerSave(false) != WIFI_IDL_OPT_OK) {
                LOGE("%{public}s Failed to set the OpMode to hi-perf", __func__);
                return false;
            }
            break;
        case WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY:
            if (!SetLowLatencyMode(true)) {
                LOGE("%{public}s Failed to set the OpMode to low-latency", __func__);
                return false;
            }
            LOGE("%{public}s unspport wifi protect mode WIFI_PROTECT_FULL_LOW_LATENCY", __func__);
            break;
        case WifiProtectMode::WIFI_PROTECT_NO_HELD:
            /* No action */
            break;
        default:
            /* Invalid mode, don't change currentOpMode , and exit with error */
            LOGE("%{public}s Invalid new protect Mode: %{public}d",
                __func__, (int)newProtectMode);
            return false;
    }

    /* Now set the mode to the new value */
    mCurrentOpMode = newProtectMode;
    LOGD("%{public}s protect mode has been set to %{public}d success.",
        __func__, static_cast<int>(mCurrentOpMode));
    return true;
}

bool WifiProtectManager::SetLowLatencyMode(bool enabled)
{
    /* Only set power save mode */
    if (WifiSupplicantHalInterface::GetInstance().SetPowerSave(!enabled) != WIFI_IDL_OPT_OK) {
        LOGE("Failed to set power save mode");
        return false;
    }

    return true;
}
#ifndef OHOS_ARCH_LITE
int WifiProtectManager::GetFgLowlatyProtectCount()
{
    int count = 0;
    std::vector<WifiProtect *>::iterator iter = mWifiProtects.begin();
    while (iter != mWifiProtects.end()) {
        if (static_cast<AppExecFwk::ApplicationState>((*iter)->GetAppState()) ==
            AppExecFwk::ApplicationState::APP_STATE_FOREGROUND &&
            (*iter)->GetProtectMode() == WifiProtectMode::WIFI_PROTECT_FULL_LOW_LATENCY) {
            count += 1;
            LOGD("%{public}s bundlename %{public}s state %{public}d.",
                __func__, (*iter)->GetName().c_str(), (*iter)->GetAppState());
        }
        iter++;
    }
    return count;
}

void WifiProtectManager::RegisterAppStateObserver()
{
    LOGD("%{public}s called", __func__);
    auto appMgrClient = std::make_unique<AppExecFwk::AppMgrClient>();
    mAppStateObserver = new (std::nothrow) AppStateObserver();
    int regAppStatusObsRetry = 0;
    while (appMgrClient->ConnectAppMgrService() != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        LOGE("ConnectAppMgrService fail, try again! retryTimes=%{public}d", ++regAppStatusObsRetry);
    }
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    mAppObject = iface_cast<AppExecFwk::IAppMgr>(systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (mAppObject) {
        int ret = mAppObject->RegisterApplicationStateObserver(mAppStateObserver);
        if (ret == ERR_OK) {
            LOGI("register application state observer success.");
            return;
        }
        LOGE("register application state observer fail, ret = %{public}d", ret);
        return;
    }
    LOGE("get SystemAbilityManager fail");
}

void WifiProtectManager::OnAppDied(const std::string bundlename)
{
    LOGI("Enter %{public}s, remove app bundlename %{public}s.",
        __func__, bundlename.c_str());
    std::unique_lock<std::mutex> lock(mMutex);
    bool needUpdate = false;
    std::vector<WifiProtect *>::iterator iter = mWifiProtects.begin();
    while (iter != mWifiProtects.end()) {
        if ((*iter)->GetName() == bundlename) {
            mWifiProtects.erase(iter);
            needUpdate = true;
            LOGI("%{public}s, remove app bundlename %{public}s.",
                __func__, bundlename.c_str());
            break;
        }
        iter++;
    }
    if (needUpdate) {
        ChangeWifiPowerMode();
    }
}

void WifiProtectManager::OnAppForegroudChanged(const std::string &bundleName, int state)
{
    std::unique_lock<std::mutex> lock(mMutex);
    bool needUpdate = false;
    std::vector<WifiProtect *>::iterator iter = mWifiProtects.begin();
    while (iter != mWifiProtects.end()) {
        if ((*iter)->GetName() == bundleName) {
            (*iter)->SetAppState(state);
            needUpdate = true;
            LOGD("%{public}s, foreground change bundleName %{public}s state %{public}d.",
                __func__, bundleName.c_str(), state);
            break;
        }
        iter++;
    }
    if (needUpdate) {
        ChangeWifiPowerMode();
    }
}

void AppStateObserver::OnAppStarted(const AppExecFwk::AppStateData &appStateData)
{
    LOGD("%{public}s bundleName: %{public}s, uid: %{public}d, state: %{public}d, isFocused: %{public}d",
        __func__, appStateData.bundleName.c_str(), appStateData.uid,
        appStateData.state, appStateData.isFocused);
}

void AppStateObserver::OnAppStopped(const AppExecFwk::AppStateData &appStateData)
{
    LOGI("%{public}s bundleName: %{public}s, uid: %{public}d, state: %{public}d, isFocused: %{public}d",
        __func__, appStateData.bundleName.c_str(), appStateData.uid,
        appStateData.state, appStateData.isFocused);

    if (appStateData.bundleName.empty()) {
        LOGE("App bundle name is empty");
        return;
    }
    WifiProtectManager::GetInstance().OnAppDied(appStateData.bundleName);
    return;
}

void AppStateObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    LOGI("%{public}s bundleName: %{public}s, uid: %{public}d, state: %{public}d, isFocused: %{public}d",
        __func__, appStateData.bundleName.c_str(), appStateData.uid,
        appStateData.state, appStateData.isFocused);

    WifiProtectManager::GetInstance().OnAppForegroudChanged(
        appStateData.bundleName, appStateData.state);
}
#endif
}  // namespace Wifi
}  // namespace OHOS
