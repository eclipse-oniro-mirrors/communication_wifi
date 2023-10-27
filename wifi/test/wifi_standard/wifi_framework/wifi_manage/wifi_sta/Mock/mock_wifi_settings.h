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
#ifndef OHOS_MOCK_WIFISETTINGS_H
#define OHOS_MOCK_WIFISETTINGS_H

#include "wifi_ap_msg.h"
#include "wifi_msg.h"
#include <gmock/gmock.h>
#include "wifi_internal_msg.h"

namespace OHOS {
namespace Wifi {
using ChannelsTable = std::map<BandType, std::vector<int32_t>>;
class MockWifiSettings {
public:
    virtual ~MockWifiSettings() = default;
    virtual int SetWifiState(int state, int instId = 0) = 0;
    virtual int AddDeviceConfig(const WifiDeviceConfig &config) = 0;
    virtual int RemoveDevice(int networkId) = 0;
    virtual void ClearDeviceConfig() = 0;
    virtual int GetDeviceConfig(std::vector<WifiDeviceConfig> &results) = 0;
    virtual int GetDeviceConfig(const int &networkId, WifiDeviceConfig &config) = 0;
    virtual int GetDeviceConfig(const std::string &ssid, const std::string &keymgmt, WifiDeviceConfig &config) = 0;
    virtual int GetDeviceConfig(const std::string &index, const int &indexType, WifiDeviceConfig &config) = 0;
    virtual int SetDeviceState(int networkId, int state, bool bSetOther = false) = 0;
    virtual int SyncDeviceConfig() = 0;
    virtual int ReloadDeviceConfig() = 0;
    virtual int GetIpInfo(IpInfo &info, int instId = 0) = 0;
    virtual int SaveIpInfo(const IpInfo &info, int instId = 0) = 0;
    virtual int GetLinkedInfo(WifiLinkedInfo &info, int instId = 0) = 0;
    virtual int SaveLinkedInfo(const WifiLinkedInfo &info, int instId = 0) = 0;
    virtual int SetMacAddress(const std::string &macAddress, int instId = 0) = 0;
    virtual int GetMacAddress(std::string &macAddress, int instId = 0) = 0;
    virtual int SetCountryCode(const std::string &countryCode) = 0;
    virtual int GetCountryCode(std::string &countryCode) = 0;
    virtual int GetSignalLevel(const int &rssi, const int &band) = 0;
    virtual bool EnableNetwork(int networkId, bool disableOthers, int instId = 0) = 0;
    virtual void SetUserLastSelectedNetworkId(int networkId, int instId = 0) = 0;
    virtual int GetUserLastSelectedNetworkId(int instId = 0) = 0;
    virtual time_t GetUserLastSelectedNetworkTimeVal(int instId = 0) = 0;
    virtual int GetDhcpIpType() = 0;
    virtual int SetDhcpIpType(int dhcpIpType) = 0;
    virtual int SetWhetherToAllowNetworkSwitchover(bool bSwitch) = 0;
    virtual bool GetWhetherToAllowNetworkSwitchover() = 0;
    virtual int GetSavedDeviceAppraisalPriority() = 0;
    virtual int GetExternDeviceAppraisalPriority() = 0;
    virtual int GetScoretacticsScoreSlope() = 0;
    virtual int GetScoretacticsInitScore() = 0;
    virtual int GetScoretacticsSameBssidScore() = 0;
    virtual int GetScoretacticsSameNetworkScore() = 0;
    virtual int GetScoretacticsFrequency5GHzScore() = 0;
    virtual int GetScoretacticsLastSelectionScore() = 0;
    virtual int GetScoretacticsSecurityScore() = 0;
    virtual std::string GetStrDnsBak() = 0;
    virtual int GetScanInfoList(std::vector<WifiScanInfo> &results, int instId = 0) = 0;
    virtual std::string GetConnectTimeoutBssid(int instId = 0) = 0;
    virtual int SetConnectTimeoutBssid(std::string &bssid, int instId = 0) = 0;
    virtual int SetDeviceAfterConnect(int networkId) = 0;
    virtual int GetCandidateConfig(const int uid, const int &networkId, WifiDeviceConfig &config) = 0;
    virtual int GetAllCandidateConfig(const int uid, std::vector<WifiDeviceConfig> &configs) = 0;
    virtual int GetValidChannels(ChannelsTable &channelsInfo) = 0;
    virtual int GetWifiState(int instId = 0) = 0;
    virtual int SetDeviceConnFailedCount(const std::string &index, const int &indexType, int count) = 0;
    virtual int IncreaseDeviceConnFailedCount(const std::string &index, const int &indexType, int count) = 0;
    virtual int SaveIpV6Info(const IpV6Info &info, int instId = 0) = 0;
    virtual int GetIpv6Info(IpV6Info &info, int instId = 0) = 0;
    virtual int SetRealMacAddress(const std::string &macAddress) = 0;
    virtual int GetRealMacAddress(std::string &macAddress) = 0;
    virtual int GetScoretacticsNormalScore() = 0;
    virtual int SetWifiLinkedStandardAndMaxSpeed(WifiLinkedInfo &linkInfo, int instId = 0) = 0;
};

class WifiSettings : public MockWifiSettings {
public:
    WifiSettings() = default;
    ~WifiSettings() = default;
    static WifiSettings &GetInstance(void);

    MOCK_METHOD2(SetWifiState, int(int state, int));
    MOCK_METHOD1(AddDeviceConfig, int(const WifiDeviceConfig &config));
    MOCK_METHOD1(RemoveDevice, int(int networkId));
    MOCK_METHOD0(ClearDeviceConfig, void());
    MOCK_METHOD1(GetDeviceConfig, int(std::vector<WifiDeviceConfig> &results));
    MOCK_METHOD2(GetDeviceConfig, int(const int &networkId, WifiDeviceConfig &config));
    MOCK_METHOD3(GetDeviceConfig, int(const std::string &ssid, const std::string &keymgmt, WifiDeviceConfig &config));
    MOCK_METHOD3(GetDeviceConfig, int(const std::string &index, const int &indexType, WifiDeviceConfig &config));
    MOCK_METHOD3(SetDeviceState, int(int networkId, int state, bool bSetOther));
    MOCK_METHOD0(SyncDeviceConfig, int());
    MOCK_METHOD0(ReloadDeviceConfig, int());
    MOCK_METHOD2(GetIpInfo, int(IpInfo &info, int));
    MOCK_METHOD2(SaveIpInfo, int(const IpInfo &info, int));
    MOCK_METHOD2(GetLinkedInfo, int(WifiLinkedInfo &info, int));
    MOCK_METHOD2(SaveLinkedInfo, int(const WifiLinkedInfo &info, int));
    MOCK_METHOD2(SetMacAddress, int(const std::string &macAddress, int));
    MOCK_METHOD2(GetMacAddress, int(std::string &macAddress, int));
    MOCK_METHOD1(SetCountryCode, int(const std::string &countryCode));
    MOCK_METHOD1(GetCountryCode, int(std::string &countryCode));
    MOCK_METHOD2(GetSignalLevel, int(const int &rssi, const int &band));
    MOCK_METHOD3(EnableNetwork, bool(int networkId, bool disableOthers, int));
    MOCK_METHOD2(SetUserLastSelectedNetworkId, void(int networkId, int));
    MOCK_METHOD1(GetUserLastSelectedNetworkId, int(int));
    MOCK_METHOD1(GetUserLastSelectedNetworkTimeVal, time_t(int));
    MOCK_METHOD0(GetDhcpIpType, int());
    MOCK_METHOD1(SetDhcpIpType, int(int dhcpIpType));
    MOCK_METHOD1(SetWhetherToAllowNetworkSwitchover, int(bool bSwitch));
    MOCK_METHOD0(GetWhetherToAllowNetworkSwitchover, bool());
    MOCK_METHOD0(GetSavedDeviceAppraisalPriority, int());
    MOCK_METHOD0(GetExternDeviceAppraisalPriority, int());
    MOCK_METHOD0(GetScoretacticsScoreSlope, int());
    MOCK_METHOD0(GetScoretacticsInitScore, int());
    MOCK_METHOD0(GetScoretacticsSameBssidScore, int());
    MOCK_METHOD0(GetScoretacticsSameNetworkScore, int());
    MOCK_METHOD0(GetScoretacticsFrequency5GHzScore, int());
    MOCK_METHOD0(GetScoretacticsLastSelectionScore, int());
    MOCK_METHOD0(GetScoretacticsSecurityScore, int());
    MOCK_METHOD0(GetStrDnsBak, std::string());
    MOCK_METHOD2(GetScanInfoList, int(std::vector<WifiScanInfo> &results, int));
    MOCK_METHOD1(GetConnectTimeoutBssid, std::string(int));
    MOCK_METHOD2(SetConnectTimeoutBssid, int(std::string &bssid, int));
    MOCK_METHOD1(SetDeviceAfterConnect, int(int networkId));
    MOCK_METHOD3(GetCandidateConfig, int(const int uid, const int &networkId, WifiDeviceConfig &config));
    MOCK_METHOD2(GetAllCandidateConfig, int(const int uid, std::vector<WifiDeviceConfig> &configs));
    MOCK_METHOD1(GetValidChannels, int(ChannelsTable &channelsInfo));
    MOCK_METHOD1(GetWifiState, int(int));
    MOCK_METHOD3(SetDeviceConnFailedCount, int(const std::string &index, const int &indexType, int count));
    MOCK_METHOD3(IncreaseDeviceConnFailedCount, int(const std::string &index, const int &indexType, int count));
    MOCK_METHOD2(SaveIpV6Info, int(const IpV6Info &info, int));
    MOCK_METHOD2(GetIpv6Info, int(IpV6Info &info, int));
    MOCK_METHOD1(SetRealMacAddress, int(const std::string &macAddress));
    MOCK_METHOD1(GetRealMacAddress, int(std::string &macAddress));
    MOCK_METHOD0(GetScoretacticsNormalScore, int());
    MOCK_METHOD2(SetWifiLinkedStandardAndMaxSpeed, int(WifiLinkedInfo &linkInfo, int));
};
}  // namespace OHOS
}  // namespace Wifi

#endif
