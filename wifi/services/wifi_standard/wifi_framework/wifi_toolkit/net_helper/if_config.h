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
#ifndef OHOS_WIFI_IF_CONFIG_H
#define OHOS_WIFI_IF_CONFIG_H

#include <memory>
#include "wifi_log.h"

#ifdef OHOS_ARCH_LITE
#include "dhcp_c_api.h"
#endif

namespace OHOS {
namespace Wifi {
typedef enum IpType {
    IPTYPE_IPV4,
    IPTYPE_IPV6,
    IPTYPE_MIX,
    IPTYPE_BUTT,
} IpType;

class IfConfig {
public:
    IfConfig();
    ~IfConfig();

    static IfConfig &GetInstance();

    void FlushIpAddr(const std::string &ifName, const int &ipType);

    void AddIpAddr(const std::string &ifName, const std::string &ipAddr, const std::string &mask, const int &ipType);

    void SetProxy(bool isAuto, const std::string &proxy, const std::string &port, const std::string &noProxys,
        const std::string &pac);

    bool ExecCommand(const std::vector<std::string> &vecCommandArg);

    bool GetIpAddr(const std::string& ifName, std::string& ipAddr);

#ifdef OHOS_ARCH_LITE
    /**
     * @Description : Set the If Addr object
     *
     * @param dhcpInfo - dhcp information[in]
     * @param ipType - ip type[in]
     * @return int
     */
    int SetIfDnsAndRoute(const DhcpResult *dhcpInfo, int ipType, int instId = 0);

    void SetNetDns(const std::string &ifName, const std::string &dns1, const std::string &dns2);

    void AddIfRoute(const std::string &ifName, const std::string &ipAddr, const std::string &mask,
        const std::string &gateWay, const int &ipType);

    void AddIpv4Route(
        const std::string &ifName, const std::string &ipAddr, const std::string &mask, const std::string &gateWay);

    void AddIpv6Route(
        const std::string &ifName, const std::string &ipAddr, const std::string &mask, const std::string &gateWay);
#endif

private:
    /**
     * @Description : Check if the interface name is valid
     */
    bool CheckIfaceValid(const std::string& ifname);
};
}  // namespace Wifi
}  // namespace OHOS
#endif
