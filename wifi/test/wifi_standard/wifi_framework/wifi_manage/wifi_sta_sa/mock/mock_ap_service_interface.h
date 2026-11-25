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
#ifndef OHOS_MOCK_AP_SERVICE_INTERFACE_H
#define OHOS_MOCK_AP_SERVICE_INTERFACE_H

#include <gmock/gmock.h>
#include "i_ap_service.h"

namespace OHOS {
namespace Wifi {
/* 定义在 mock_wifi_service_manager.cpp；测试通过该声明注入 IApService 实例 */
extern IApService *g_utApService;

class MockApServiceInterface : public IApService {
public:
    MOCK_METHOD0(EnableHotspot, ErrCode());
    MOCK_METHOD0(DisableHotspot, ErrCode());
    MOCK_METHOD1(AddBlockList, ErrCode(const StationInfo &stationInfo));
    MOCK_METHOD1(DelBlockList, ErrCode(const StationInfo &stationInfo));
    MOCK_METHOD1(SetHotspotConfig, ErrCode(const HotspotConfig &hotspotConfig));
    MOCK_METHOD1(DisconnetStation, ErrCode(const StationInfo &stationInfo));
    MOCK_METHOD1(GetStationList, ErrCode(std::vector<StationInfo> &result));
    MOCK_METHOD1(GetSupportedPowerModel, ErrCode(std::set<PowerModel> &setPowerModelList));
    MOCK_METHOD1(GetPowerModel, ErrCode(PowerModel &model));
    MOCK_METHOD1(SetPowerModel, ErrCode(const PowerModel &model));
    MOCK_METHOD1(SetHotspotIdleTimeout, ErrCode(int time));
    MOCK_METHOD1(OnNetCapabilitiesChanged, void(const int apStatus));
#ifndef OHOS_ARCH_LITE
    MOCK_METHOD1(SetEnhanceService, void(IEnhanceService *enhanceService));
#endif
    MOCK_METHOD1(RegisterApServiceCallbacks, ErrCode(const IApServiceCallbacks &callbacks));
    /* IApService 中这两个非纯虚函数没有默认实现，必须 override，否则 vtable 链接失败 */
    MOCK_METHOD1(GetHotspotMode, ErrCode(HotspotMode &mode));
    MOCK_METHOD1(SetHotspotMode, ErrCode(const HotspotMode &mode));
#ifdef FEATURE_WIFI_MDM_RESTRICTED_SUPPORT
    MOCK_METHOD1(DeauthMdmNotAllowedSta, ErrCode(const std::vector<StationInfo> &stations));
#endif
};
} // namespace Wifi
} // namespace OHOS
#endif