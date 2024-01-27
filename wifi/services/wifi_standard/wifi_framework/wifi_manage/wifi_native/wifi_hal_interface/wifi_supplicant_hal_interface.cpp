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
#include "wifi_supplicant_hal_interface.h"
#include <mutex>
#include "wifi_log.h"

#undef LOG_TAG
#define LOG_TAG "WifiSupplicantHalInterface"

namespace OHOS {
namespace Wifi {
WifiSupplicantHalInterface &WifiSupplicantHalInterface::GetInstance(void)
{
    static WifiSupplicantHalInterface inst;
    static int initFlag = 0;
    static std::mutex initMutex;
    if (initFlag == 0) {
        std::unique_lock<std::mutex> lock(initMutex);
        if (initFlag == 0) {
#ifdef HDI_WPA_INTERFACE_SUPPORT
            if (inst.InitHdiWpaClient()) {
                initFlag = 1;
            }
#else
            if (inst.InitIdlClient()) {
                initFlag = 1;
            }
#endif
#ifdef HDI_INTERFACE_SUPPORT
            if (inst.InitHdiClient()) {
                initFlag = 1;
            }
#endif
        }
    }
    return inst;
}

WifiErrorNo WifiSupplicantHalInterface::StartSupplicant(void) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    LOGI("call WifiSupplicantHalInterface::%{public}s!", __func__);
    return WIFI_IDL_OPT_FAILED;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqStartSupplicant();
#endif
}

WifiErrorNo WifiSupplicantHalInterface::StopSupplicant(void) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    LOGI("call WifiSupplicantHalInterface::%{public}s!", __func__);
    return WIFI_IDL_OPT_FAILED;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqStopSupplicant();
#endif
}

WifiErrorNo WifiSupplicantHalInterface::ConnectSupplicant(void) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    LOGI("call WifiSupplicantHalInterface::%{public}s!", __func__);
    return WIFI_OPT_NOT_SUPPORTED;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqConnectSupplicant();
#endif
}

WifiErrorNo WifiSupplicantHalInterface::DisconnectSupplicant(void) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    LOGI("call WifiSupplicantHalInterface::%{public}s!", __func__);
    return WIFI_IDL_OPT_FAILED;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqDisconnectSupplicant();
#endif
}

WifiErrorNo WifiSupplicantHalInterface::RequestToSupplicant(const std::string &request) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    LOGI("call WifiSupplicantHalInterface::%{public}s!", __func__);
    return WIFI_IDL_OPT_FAILED;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqRequestToSupplicant(request);
#endif
}

WifiErrorNo WifiSupplicantHalInterface::RegisterSupplicantEventCallback(SupplicantEventCallback &callback)
{
#ifdef HDI_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiClient, WIFI_IDL_OPT_FAILED);
    WifiErrorNo err = mHdiClient->ReqRegisterSupplicantEventCallback(callback);
    if (err == WIFI_IDL_OPT_OK) {
        mCallback = callback;
    }
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    WifiErrorNo err = mIdlClient->ReqRegisterSupplicantEventCallback(callback);
    if (err == WIFI_IDL_OPT_OK) {
        mCallback = callback;
    }
#endif
    return err;
}

WifiErrorNo WifiSupplicantHalInterface::UnRegisterSupplicantEventCallback(void)
{
#ifdef HDI_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiClient, WIFI_IDL_OPT_FAILED);
    WifiErrorNo err = mHdiClient->ReqUnRegisterSupplicantEventCallback();
    mCallback.onScanNotify = nullptr;
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    WifiErrorNo err = mIdlClient->ReqUnRegisterSupplicantEventCallback();
    mCallback.onScanNotify = nullptr;
#endif
    return err;
}

WifiErrorNo WifiSupplicantHalInterface::SetPowerSave(bool enable) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiWpaClient, WIFI_IDL_OPT_FAILED);
    return mHdiWpaClient->ReqSetPowerSave(enable);
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqSetPowerSave(enable);
#endif
}

WifiErrorNo WifiSupplicantHalInterface::WpaSetCountryCode(const std::string &countryCode) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiWpaClient, WIFI_IDL_OPT_FAILED);
    return mHdiWpaClient->ReqWpaSetCountryCode(countryCode);
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqWpaSetCountryCode(countryCode);
#endif
}

WifiErrorNo WifiSupplicantHalInterface::WpaGetCountryCode(std::string &countryCode) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiWpaClient, WIFI_IDL_OPT_FAILED);
    return mHdiWpaClient->ReqWpaGetCountryCode(countryCode);
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqWpaGetCountryCode(countryCode);
#endif
}

const SupplicantEventCallback &WifiSupplicantHalInterface::GetCallbackInst(void) const
{
    return mCallback;
}

WifiErrorNo WifiSupplicantHalInterface::WpaSetSuspendMode(bool mode) const
{
#ifdef HDI_WPA_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiWpaClient, WIFI_IDL_OPT_FAILED);
    return mHdiWpaClient->ReqWpaSetSuspendMode(mode);
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqWpaSetSuspendMode(mode);
#endif
}

WifiErrorNo WifiSupplicantHalInterface::WpaSetPowerMode(bool mode) const
{
#ifdef HDI_INTERFACE_SUPPORT
    CHECK_NULL_AND_RETURN(mHdiClient, WIFI_IDL_OPT_FAILED);
    return mHdiClient->ReqSetPowerModel(mode);
#else
    CHECK_NULL_AND_RETURN(mIdlClient, WIFI_IDL_OPT_FAILED);
    return mIdlClient->ReqWpaSetPowerMode(mode);
#endif
}
}  // namespace Wifi
}  // namespace OHOS
