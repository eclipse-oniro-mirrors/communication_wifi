/*
 * Copyright (C) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_WIFI_PRO_INTERFACE_H
#define OHOS_WIFI_WIFI_PRO_INTERFACE_H

#include "iwifi_pro_service.h"
#include "define.h"
#include "wifi_pro_common.h"

namespace OHOS {
namespace Wifi {
class WifiProService;
class WifiProInterface : public IWifiProService {
    FRIEND_GTEST(WifiProInterface);
public:
    explicit WifiProInterface(int32_t instId = 0);
    ~WifiProInterface() override;

    /**
     * @Description  self cure service initialization function.
     *
     * @return success: WIFI_OPT_SUCCESS, failed: WIFI_OPT_FAILED
     */
    ErrCode InitWifiProService() override;

    /**
     * @Description Get register sta callback
     *
     * @return StaServiceCallback - sta callback
     */
    StaServiceCallback GetStaCallback() const override;

    /**
     * @Description deal scan results
     *
     * @return results - scan results
     */
    void DealScanResult(const std::vector<InterScanInfo> &results) override;

    /**
     * @Description deal qoe slow
     */
    void DealQoeSlowResult() override;
private:
    /**
     * @Description deal sta connection change
     *
     * @param state - OperateResState
     * @param info -  const WifiLinkedInfo
     */
    void DealStaConnChanged(OperateResState state, const WifiLinkedInfo &linkedInfo, int32_t instId = 0);

    /**
     * @Description rssi level changed
     *
     * @param rssi
     */
    void DealRssiLevelChanged(int32_t rssi, int32_t instId = 0);
private:
    std::mutex mutex_;
    std::shared_ptr<WifiProService> pWifiProService_ { nullptr };
    int32_t instId_ { 0 };
    StaServiceCallback staCallback_;
    void InitCallback();
};
}  // namespace Wifi
}  // namespace OHOS
#endif