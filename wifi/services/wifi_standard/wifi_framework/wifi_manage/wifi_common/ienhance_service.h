/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#ifndef OHOS_IENHANCE_SERVICE_H
#define OHOS_IENHANCE_SERVICE_H

#include "wifi_errcode.h"
#include "wifi_scan_control_msg.h"
#include "wifi_msg.h"

namespace OHOS {
namespace Wifi {
class IEnhanceService {
public:
    virtual ~IEnhanceService() = default;
    /**
     * @Description  Enhance service initialization function.
     *
     * @return success: WIFI_OPT_SUCCESS, failed: WIFI_OPT_FAILED
     */
    virtual ErrCode Init() = 0;
    /**
     * @Description  Stopping the Enhance Service.
     *
     * @return success: WIFI_OPT_SUCCESS, failed: WIFI_OPT_FAILED
     */
    virtual ErrCode UnInit() = 0;
    /**
     * @Description  check Scan is allowed.
     *
     * @return true: allowed, false: not allowed
     */
    virtual bool AllowScanBySchedStrategy() = 0;
    /**
     * @Description  Set EnhanceService Param.
     *
     * @return success: WIFI_OPT_SUCCESS, failed: WIFI_OPT_FAILED
     */
    virtual ErrCode SetEnhanceParam(int64_t availableTime) = 0;

    /**
     * @Description Install Paket Filter Program
     *
     * @param ipAddr - ip address
     * @param netMaskLen - net mask length
     * @param macAddr - mac address
     * @param macLen - mac address length
     * @param screenState - screen state
     * @return success: WIFI_OPT_SUCCESS, failed: WIFI_OPT_FAILED
     */
    virtual ErrCode InstallFilterProgram(
        unsigned int ipAddr, int netMaskLen, const unsigned char *macAddr, int macLen, int screenState) = 0;

    /**
     * @Description Get wifi category
     *
     * @param infoElems - info elems
     * @param chipsetCategory - chipset category
     * @param chipsetFeatrureCapability - chipset featrure capability
     * @return 1: DEFAULT, 2: WIFI6, 3: WIFI6_PLUS
     */
    virtual WifiCategory GetWifiCategory(
        std::vector<WifiInfoElem> infoElems, int chipsetCategory, int chipsetFeatrureCapability) = 0;

    /**
     * @Description set low tx power
     *
     * @param wifiLowPowerParam - wifi low power param
     * @return ErrCode - operation result
     */
    virtual ErrCode SetLowTxPower(const WifiLowPowerParam wifiLowPowerParam) = 0;
    
    /**
     * @Description Check Chba conncted
     *
     * @return true: conncted, false: not conncted
     */
    virtual bool CheckChbaConncted() = 0;

    /**
     * @Description Is external scan allowed.
     *
     * @param scanDeviceInfo - scan device info
     * @return true: allowed, false: not allowed
     */
    virtual bool IsScanAllowed(WifiScanDeviceInfo &scanDeviceInfo) = 0;

    /**
     * @Description selfcure for multi dhcp server.
     *
     * @param cmd - add、get size、clear
     * @param ipInfo - ip information
     * @param retSize - get dhcp offer size
     * @return ErrCode - operation result
     */
    virtual ErrCode DealDhcpOfferResult(const OperationCmd &cmd, const IpInfo &ipInfo, uint32_t &retSize) = 0;

    /**
     * @Description selfcure for multi dhcp server.
     *
     * @param isChanged - is gateway changed situation
     * @return ErrCode - operation result
     */
    virtual ErrCode IsGatewayChanged(bool &isChanged) = 0;

    /**
     * @Description selfcure for multi dhcp server.
     *
     * @param isMultiDhcpServer - true、false
     * @param startSelfcure - true、false
     * @param ipInfo - get ipinfo
     * @return ErrCode - operation result
     */
    virtual ErrCode GetStaticIpConfig(const bool &isMultiDhcpServer, const bool &startSelfcure, IpInfo &ipInfo) = 0;

    /**
     * @Description Is Wide Bandwidth Supported.
     *
     * @return true: support, false: not support
     */
    virtual bool IsWideBandwidthSupported() = 0;
};
}  // namespace Wifi
}  // namespace OHOS
#endif