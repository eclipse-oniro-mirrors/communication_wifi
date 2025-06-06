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
#ifndef OHOS_AP_SERVICE_H
#define OHOS_AP_SERVICE_H

#include "ap_define.h"
#include "wifi_internal_msg.h"
#include "i_ap_service.h"
#include "i_ap_service_callbacks.h"
#include "i_wifi_country_code_change_listener.h"
#include "ap_started_state.h"
#include "ap_network_monitor.h"
#ifndef OHOS_ARCH_LITE
#include "ienhance_service.h"
#endif
namespace OHOS {
namespace Wifi {
class ApStateMachine;
class ApService {
    FRIEND_GTEST(ApService);

public:
    /**
     * @Description  construction method.
     * @param None
     * @return None
     */
    explicit ApService(ApStateMachine &apStateMachine, ApStartedState &apStartedState, int id = 0);

    /**
     * @Description  destructor method.
     * @param None
     * @return None
     */
    ~ApService();
    DISALLOW_COPY_AND_ASSIGN(ApService)

    /**
     * @Description  open hotspot.
     * @param None
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode EnableHotspot();

    /**
     * @Description  close hotspot.
     * @param None
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode DisableHotspot() const;

    /**
     * @Description  set ap config.
     * @param cfg - ap config
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode SetHotspotConfig(const HotspotConfig &cfg) const;

    /**
     * @Description Set the idel timeout of Hotspot
     *
     * @param time -input time,
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode SetHotspotIdleTimeout(int time) const;

    /**
     * @Description  add block list
     * @param stationInfo - sta infos
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode AddBlockList(const StationInfo &stationInfo) const;

    /**
     * @Description  delete block list.
     * @param stationInfo - sta infos
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode DelBlockList(const StationInfo &stationInfo) const;

    /**
     * @Description  Disconnect a specified STA.
     * @param stationInfo - sta infos
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode DisconnetStation(const StationInfo &stationInfo) const;

    /**
     * @Description Get the Station List object.
     *
     * @param result - current connected station info
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode GetStationList(std::vector<StationInfo> &result) const;

    /**
     * @Description Get valid bands.
     *
     * @param bands - return valid bands
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode GetValidBands(std::vector<BandType> &bands);

    /**
     * @Description Get valid channels.
     *
     * @param band - input band
     * @param validchannel - band's valid channel
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode GetValidChannels(BandType band, std::vector<int32_t> &validChannel);

    /**
     * @Description Sets the callback function for the state machine.
     *
     * @param callbacks - callbacks list.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    ErrCode RegisterApServiceCallbacks(const IApServiceCallbacks &callbacks);

    /**
     * @Description Get supported power model list
     *
     * @param setPowerModelList - supported power model list
     * @return ErrCode - operation result
     */
    ErrCode GetSupportedPowerModel(std::set<PowerModel>& setPowerModelList);

    /**
     * @Description Get power model
     *
     * @param model - current power model
     * @return ErrCode - operation result
     */
    ErrCode GetPowerModel(PowerModel& model);

    /**
     * @Description Get supported power model list
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    ErrCode SetPowerModel(const PowerModel& model);

    /**
     * @Description get hotspot mode
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    ErrCode GetHotspotMode(HotspotMode &mode);

    /**
     * @Description set hotspot mode
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    ErrCode SetHotspotMode(const HotspotMode &mode);
    void HandleNetCapabilitiesChanged(const int apStatus);
#ifndef OHOS_ARCH_LITE
    /**
     * @Description Set EnhanceService to Ap service
     *
     * @param enhanceService IEnhanceService object
     */
    void SetEnhanceService(IEnhanceService* enhanceService);
#endif
private:
    class WifiCountryCodeChangeObserver : public IWifiCountryCodeChangeListener {
    public:
        WifiCountryCodeChangeObserver(const std::string &name, StateMachine &stateMachineObj)
            : IWifiCountryCodeChangeListener(name, stateMachineObj) {}
        ~WifiCountryCodeChangeObserver() override = default;
        ErrCode OnWifiCountryCodeChanged(const std::string &wifiCountryCode) override;
        std::string GetListenerModuleName() override;
    };
    ApStateMachine &m_ApStateMachine;
    ApStartedState &apStartedState_;
    int m_id;
    std::shared_ptr<IWifiCountryCodeChangeListener> m_apObserver;
#ifndef OHOS_ARCH_LITE
    IEnhanceService *enhanceService_ = nullptr;
#endif
};
}  // namespace Wifi
}  // namespace OHOS

#endif