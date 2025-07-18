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
#ifndef OHOS_I_WIFI_DEVICE_CALLBACK_H
#define OHOS_I_WIFI_DEVICE_CALLBACK_H

#include <string>
#ifndef OHOS_ARCH_LITE
#include <string_ex.h>
#include <iremote_stub.h>
#include "message_parcel.h"
#include "message_option.h"
#endif
#include "wifi_msg.h"
#include "define.h"


namespace OHOS {
namespace Wifi {
#ifdef OHOS_ARCH_LITE
class IWifiDeviceCallBack {
#else
class IWifiDeviceCallBack : public IRemoteBroker {
#endif
public:
    /**
     * @Description Deal wifi state change message
     *
     * @param state - Wifi State
     */
    virtual void OnWifiStateChanged(int state) = 0;

    /**
     * @Description Deal wifi connection state change message
     *
     * @param state - Wifi connect state
     * @param info - WifiLinkedInfo object
     */
    virtual void OnWifiConnectionChanged(int state, const WifiLinkedInfo &info) = 0;

    /**
     * @Description Deal wifi rssi change message
     *
     * @param rssi - rssi
     */
    virtual void OnWifiRssiChanged(int rssi) = 0;

    /**
     * @Description Deal wps state change message
     *
     * @param state - wps state
     * @param pinCode - when wps pin mode, open wps successfully, return the pin code
     */
    virtual void OnWifiWpsStateChanged(int state, const std::string &pinCode) = 0;

    /**
     * @Description Deal stream change message
     *
     * @param direction - stream direction
     */
    virtual void OnStreamChanged(int direction) = 0;

    /**
     * @Description Deal device config change message
     *
     * @param ConfigChange - change type of config
     */
    virtual void OnDeviceConfigChanged(ConfigChange value) = 0;

    /**
     * @Description Deal candidate approval status change message
     *
     * @param status - user approval status
     */
    virtual void OnCandidateApprovalStatusChanged(CandidateApprovalStatus status) {}

#ifndef OHOS_ARCH_LITE
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.wifi.IWifiDeviceCallBack");

    std::string name = "default";
#endif
};
}  // namespace Wifi
}  // namespace OHOS
#endif