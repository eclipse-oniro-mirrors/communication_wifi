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

#ifndef OHOS_WIFI_HOTSPOT_SERVICE_H
#define OHOS_WIFI_HOTSPOT_SERVICE_H

#include <map>
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"
#include "i_wifi_hotspot.h"

namespace OHOS {
namespace Wifi {
class WifiHotspotStub : public IRemoteStub<IWifiHotspot> {
public:
    using handleFunc = std::function<void
        (uint32_t, MessageParcel &, MessageParcel &, MessageOption &)>;
    using HandleFuncMap = std::map<int, handleFunc>;

    WifiHotspotStub();
    explicit WifiHotspotStub(int id);

    virtual ~WifiHotspotStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void InitHandleMap(void);
    void OnIsHotspotActive(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetApStateWifi(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetHotspotConfig(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetApConfigWifi(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetApIdleTimeout(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetStationList(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnDisassociateSta(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnEnableWifiAp(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnDisableWifiAp(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnAddBlockList(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnDelBlockList(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetBlockLists(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetValidBands(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetValidChannels(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnRegisterCallBack(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetSupportedFeatures(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetSupportedPowerModel(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetPowerModel(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnSetPowerModel(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnIsHotspotDualBandSupported(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnIsOpenSoftApAllowed(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetApIfaceName(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    bool CheckHotspot160MParam(BandType band, int bandwidth, int channel);
    bool CheckHostspot160MCountryCode();
    void OnEnableLocalOnlyHotspot(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnDisableLocalOnlyHotspot(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetHotspotMode(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    void OnGetLocalOnlyHotspotConfig(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
private:
    HandleFuncMap handleFuncMap;
    std::mutex deathRecipientMutex;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    bool mSingleCallback;
protected:
    int m_id;
};
}  // namespace Wifi
}  // namespace OHOS
#endif