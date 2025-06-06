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
#ifndef OHOS_AP_INTERFACE_H
#define OHOS_AP_INTERFACE_H
#include "ap_config_use.h"
#include "ap_monitor.h"
#include "ap_stations_manager.h"
#include "ap_service.h"
#include "ap_state_machine.h"

namespace OHOS {
namespace Wifi {
class ApInterface : public IApService {
public:
    /**
     * @Description  construction method.
     * @param None
     * @return None
     */
    explicit ApInterface(int id = 0);
    /**
     * @Description  destructor method.
     * @param None
     * @return None
     */
    ~ApInterface();

public:
    /**
     * @Description  Enable hotspot.
     * @param None
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode EnableHotspot() override;

    /**
     * @Description  Disable hotspot.
     * @param None
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode DisableHotspot() override;

    /**
     * @Description  Add block list from station information.
     * @param stationInfo - station information.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode AddBlockList(const StationInfo &stationInfo) override;

    /**
     * @Description  Del block list from station information.
     * @param stationInfo - station information.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode DelBlockList(const StationInfo &stationInfo) override;

    /**
     * @Description  Set hotspot configure.
     * @param hotspotConfig - hotspot configure.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode SetHotspotConfig(const HotspotConfig &hotspotConfig) override;

    /**
     * @Description Set the idel timeout of Hotspot
     *
     * @param time -input time,
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode SetHotspotIdleTimeout(int time) override;

    /**
     * @Description  Disconnect Station connect from station information.
     * @param stationInfo - station information.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode DisconnetStation(const StationInfo &stationInfo) override;

    /**
     * @Description Get the Station List object.
     *
     * @param result - current connected station info
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode GetStationList(std::vector<StationInfo> &result) override;

    /**
     * @Description  Sets the callback function for the state machine.
     * @param callbacks - callbacks list.
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode RegisterApServiceCallbacks(const IApServiceCallbacks &callbacks) override;

    /**
     * @Description Get valid bands.
     *
     * @param bands - return valid bands
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode GetValidBands(std::vector<BandType> &bands) override;

    /**
     * @Description Get valid channels.
     *
     * @param band - input band
     * @param validchannel - band's valid channel
     * @return ErrCode - success: WIFI_OPT_SUCCESS    failed: ERROR_CODE
     */
    virtual ErrCode GetValidChannels(BandType band, std::vector<int32_t> &validChannel) override;

    /**
     * @Description Get supported power model list
     *
     * @param setPowerModelList - supported power model list
     * @return ErrCode - operation result
     */
    virtual ErrCode GetSupportedPowerModel(std::set<PowerModel>& setPowerModelList) override;

    /**
     * @Description Get power model
     *
     * @param model - current power model
     * @return ErrCode - operation result
     */
    virtual ErrCode GetPowerModel(PowerModel& model) override;

    /**
     * @Description Get supported power model list
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    virtual ErrCode SetPowerModel(const PowerModel& model) override;

    /**
     * @Description get hotspot mode
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    virtual ErrCode GetHotspotMode(HotspotMode &mode) override;

    /**
     * @Description set hotspot mode
     *
     * @param model - the model to be set
     * @return ErrCode - operation result
     */
    virtual ErrCode SetHotspotMode(const HotspotMode &mode) override;

    /**
     * @Description Deal On Net Capabilities Changed
     *
     * @param apStatus
     */
    void OnNetCapabilitiesChanged(const int apStatus) override;
#ifndef OHOS_ARCH_LITE
    /**
     * @Description Set EnhanceService to Ap service
     *
     * @param enhanceService IEnhanceService object
     */
    void SetEnhanceService(IEnhanceService* enhanceService) override;
#endif
private:
    ApRootState m_ApRootState;
    ApStartedState m_ApStartedState;
    ApIdleState m_ApIdleState;

    ApMonitor m_ApMonitor;
    ApStateMachine m_ApStateMachine;
    ApService m_ApService;

    ApStationsManager m_ApStationsManager;
    ApConfigUse m_ApConfigUse;
};
}  // namespace Wifi
}  // namespace OHOS

#endif
