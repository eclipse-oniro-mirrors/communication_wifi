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
#ifndef OHOS_MOCK_RPT_INTERFACE_H
#define OHOS_MOCK_RPT_INTERFACE_H

#include <gmock/gmock.h>
#include "rpt_interface.h"

namespace OHOS {
namespace Wifi {
class MockRptInterface : public RptInterface {
public:
    MOCK_METHOD0(IsRptRunning, bool());
    MOCK_METHOD1(GetStationList, ErrCode(std::vector<StationInfo> &result));
    MOCK_METHOD0(GetRptIfaceName, std::string());
    MOCK_METHOD1(AddBlock, void(const std::string &mac));
    MOCK_METHOD1(DelBlock, void(const std::string &mac));
#ifdef FEATURE_WIFI_MDM_RESTRICTED_SUPPORT
    MOCK_METHOD1(DeauthMdmNotAllowedSta, ErrCode(const std::vector<StationInfo> &stations));
#endif
    MOCK_METHOD2(OnP2pActionResult, void(P2pActionCallback action, ErrCode code));
    MOCK_METHOD1(OnP2pConnectionChanged, void(P2pConnectedState p2pConnState));
    MOCK_METHOD1(OnStationJoin, void(std::string mac));
    MOCK_METHOD1(OnStationLeave, void(std::string mac));
};
} // namespace Wifi
} // namespace OHOS
#endif