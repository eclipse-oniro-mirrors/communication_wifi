/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_CONFIG_CENTER_H
#define OHOS_WIFI_CONFIG_CENTER_H

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "wifi_internal_msg.h"
#include "wifi_settings.h"

#define SOFT_BUS_SERVICE_UID 1024
#define CAST_ENGINE_SERVICE_UID 5526
namespace OHOS {
namespace Wifi {
using ChannelsTable = std::map<BandType, std::vector<int32_t>>;

class WifiConfigCenter {
public:
    static WifiConfigCenter &GetInstance();
    ~WifiConfigCenter();

    int Init();

    void SetWifiSelfcureReset(const bool isReset);

    bool GetWifiSelfcureReset() const;

    void SetWifiSelfcureResetEntered(const bool isReset);

    bool GetWifiSelfcureResetEntered() const;

    void SetLastNetworkId(const int networkId);

    int GetLastNetworkId() const;

    void SetSelectedCandidateNetworkId(const int networkId);

    int GetSelectedCandidateNetworkId() const;

    void SetWifiAllowSemiActive(bool isAllowed);

    bool GetWifiAllowSemiActive() const;

    void SetWifiStopState(bool state);

    bool GetWifiStopState() const;

    void SetStaIfaceName(const std::string &ifaceName);

    std::string GetStaIfaceName();

    int GetWifiState(int instId = 0);

    int SetWifiState(int state, int instId = 0);

    WifiDetailState GetWifiDetailState(int instId = 0);

    int SetWifiDetailState(WifiDetailState state, int instId);

    WifiOprMidState GetWifiMidState(int instId = 0);

    bool SetWifiMidState(WifiOprMidState expState, WifiOprMidState state, int instId = 0);

    void SetWifiMidState(WifiOprMidState state, int instId = 0);

    void SetWifiStaCloseTime(int instId = 0);

    double GetWifiStaInterval(int instId = 0);

    bool GetWifiConnectedMode(int instId = 0);

    void SetWifiConnectedMode(bool isContainerConnected, int instId = 0);

    int SetChangeDeviceConfig(ConfigChange value, const WifiDeviceConfig &config);
    
    bool GetChangeDeviceConfig(ConfigChange& value, WifiDeviceConfig &config);

    int GetIpInfo(IpInfo &info, int instId = 0);

    int SaveIpInfo(const IpInfo &info, int instId = 0);

    int GetIpv6Info(IpV6Info &info, int instId = 0);

    int SaveIpV6Info(const IpV6Info &info, int instId = 0);

    std::map<int, WifiLinkedInfo> GetAllWifiLinkedInfo();

    int GetLinkedInfo(WifiLinkedInfo &info, int instId = 0);

    int SaveLinkedInfo(const WifiLinkedInfo &info, int instId = 0);

    void UpdateLinkedChannelWidth(std::string bssid, WifiChannelWidth channelWidth, int instId = 0);

    int SetMacAddress(const std::string &macAddress, int instId = 0);

    int GetMacAddress(std::string &macAddress, int instId = 0);

    void SetUserLastSelectedNetworkId(int networkId, int instId = 0);

    int GetUserLastSelectedNetworkId(int instId = 0);

    time_t GetUserLastSelectedNetworkTimeVal(int instId = 0);

    std::string GetConnectTimeoutBssid(int instId = 0);

    int SetConnectTimeoutBssid(std::string &bssid, int instId = 0);

    void SaveDisconnectedReason(DisconnectedReason discReason, int instId = 0);

    int GetDisconnectedReason(DisconnectedReason &discReason, int instId = 0);

    void InsertWifiCategoryBlackListCache(int blacklistType, const std::string currentBssid,
        const WifiCategoryBlackListInfo wifiBlackListInfo);

    void RemoveWifiCategoryBlackListCache(int blacklistType, const std::string bssid);

    int GetWifiCategoryBlackListCache(int blacklistType,
        std::map<std::string, WifiCategoryBlackListInfo> &blackListCache);

    void UpdateWifiConnectFailListCache(int blacklistType, const std::string bssid,
        const WifiCategoryConnectFailInfo wifi7ConnectFailInfo);

    void RemoveWifiConnectFailListCache(const std::string bssid);

    int GetWifiConnectFailListCache(
        std::map<std::string, WifiCategoryConnectFailInfo> &connectFailCache);

    bool EnableNetwork(int networkId, bool disableOthers, int instId = 0);

    WifiOprMidState GetScanMidState(int instId = 0);

    bool SetScanMidState(WifiOprMidState expState, WifiOprMidState state, int instId = 0);

    void SetScanMidState(WifiOprMidState state, int instId = 0);

    WifiOprMidState GetWifiScanOnlyMidState(int instId = 0);

    bool SetWifiScanOnlyMidState(WifiOprMidState expState, WifiOprMidState state, int instId = 0);

    void SetWifiScanOnlyMidState(WifiOprMidState state, int instId = 0);

    int GetScanControlInfo(ScanControlInfo &info, int instId = 0);

    int SetScanControlInfo(const ScanControlInfo &info, int instId = 0);

    void RecordWifiCategory(const std::string bssid, WifiCategory category);

    void CleanWifiCategoryRecord();

    void SetAbnormalApps(const std::vector<std::string> &abnormalAppList);

    int GetAbnormalApps(std::vector<std::string> &abnormalAppList);

    int SaveScanInfoList(const std::vector<WifiScanInfo> &results);

    int ClearScanInfoList();

    int GetScanInfoList(std::vector<WifiScanInfo> &results);

    int SetWifiLinkedStandardAndMaxSpeed(WifiLinkedInfo &linkInfo);

    bool CheckScanOnlyAvailable(int instId = 0);

    std::string GetConnectedBssid(int instId = 0);

    void SetSoftapToggledState(bool state);

    bool GetSoftapToggledState() const;

    int SetHotspotIdleTimeout(int time);

    int GetHotspotIdleTimeout() const;

    void SetApIfaceName(const std::string &ifaceName);

    std::string GetApIfaceName();

    WifiOprMidState GetApMidState(int id = 0);

    bool SetApMidState(WifiOprMidState expState, WifiOprMidState state, int id = 0);

    void SetApMidState(WifiOprMidState state, int id = 0);

    int GetHotspotState(int id = 0);

    int SetHotspotState(int state, int id = 0);

    int SetPowerModel(const PowerModel& model, int id = 0);

    int GetPowerModel(PowerModel& model, int id = 0);

    int GetStationList(std::vector<StationInfo> &results, int id = 0);

    int ManageStation(const StationInfo &info, int mode, int id = 0);

    int ClearStationList(int id = 0);

    void SetP2pIfaceName(const std::string &ifaceName);

    std::string GetP2pIfaceName();

    int SetHid2dUpperScene(int uid, const Hid2dUpperScene &scene);

    int GetHid2dUpperScene(int uid, Hid2dUpperScene &scene);

    void ClearLocalHid2dInfo(int uid = 0);

    int SetP2pEnhanceState(int state);

    int GetP2pEnhanceState();

    WifiOprMidState GetP2pMidState();

    bool SetP2pMidState(WifiOprMidState expState, WifiOprMidState state);

    void SetP2pMidState(WifiOprMidState state);

    int SetP2pState(int state);

    int GetP2pState();

    int SetP2pDiscoverState(int state);

    int GetP2pDiscoverState();

    int SetP2pBusinessType(const P2pBusinessType &type);

    int GetP2pBusinessType(P2pBusinessType &type);

    int SaveP2pCreatorUid(int uid);

    int GetP2pCreatorUid();

    void SetExplicitGroup(bool isExplicit);

    bool IsExplicitGroup(void);

    int SaveP2pInfo(WifiP2pLinkedInfo &linkedInfo);

    int GetP2pInfo(WifiP2pLinkedInfo &linkedInfo);

    void SetCurrentP2pGroupInfo(const WifiP2pGroupInfo &group);

    WifiP2pGroupInfo GetCurrentP2pGroupInfo();

    void SetCoexSupport(bool isSupport);

    bool GetCoexSupport() const;

    void SetScreenState(const int &state);

    int GetScreenState() const;

    void SetThermalLevel(const int &level);

    int GetThermalLevel() const;

    void SetPowerIdelState(const int &state);

    int GetPowerIdelState() const;

    void SetGnssFixState(const int &state);

    int GetGnssFixState() const;

    void SetScanGenieState(const int &state);

    int GetScanGenieState() const;

    bool SetWifiStateOnAirplaneChanged(const int &state);

    int GetAirplaneModeState() const;

    int GetWifiToggledEnable();

    void SetWifiToggledState(int state);

    void SetPowerSavingModeState(const int &state);

    int GetPowerSavingModeState() const;

    void SetFreezeModeState(int state);

    int GetFreezeModeState() const;

    void SetNoChargerPlugModeState(int state);

    int GetNoChargerPlugModeState() const;

    void SetThreadStatusFlag(bool state);

    bool GetThreadStatusFlag(void) const;

    uint64_t GetThreadStartTime(void) const;

    bool StoreWifiMacAddrPairInfo(WifiMacAddrInfoType type, const std::string &realMacAddr,
        const std::string &randomAddr);

    std::string GetRandomMacAddr(WifiMacAddrInfoType type, std::string bssid);

    int RemoveMacAddrPairs(WifiMacAddrInfoType type, const WifiMacAddrInfo &macAddrInfo);

    std::string GetMacAddrPairs(WifiMacAddrInfoType type, const WifiMacAddrInfo &macAddrInfo);

    void ClearMacAddrPairs(WifiMacAddrInfoType type);

    std::set<int> GetAllWifiLinkedNetworkId();

private:
    WifiConfigCenter();
    bool HasWifiActive();
    void UpdateLinkedInfo(int instId = 0);
    void InitScanControlForbidList();
    void InitScanControlIntervalList();
    void SetPersistWifiState(int state);
    int GetPersistWifiState();
    std::string GetPairMacAddress(std::map<WifiMacAddrInfo, std::string>& macAddrInfoMap,
        const WifiMacAddrInfo &macAddrInfo);
    WifiMacAddrErrCode InsertMacAddrPairs(std::map<WifiMacAddrInfo, std::string>& macAddrInfoMap,
        const WifiMacAddrInfo &macAddrInfo, std::string& randomMacAddr);
    void DelMacAddrPairs(std::map<WifiMacAddrInfo, std::string>& macAddrInfoMap, const WifiMacAddrInfo &macAddrInfo);
    void RemoveMacAddrPairInfo(WifiMacAddrInfoType type, std::string bssid);
    WifiMacAddrErrCode AddMacAddrPairs(WifiMacAddrInfoType type, const WifiMacAddrInfo &macAddrInfo,
        std::string randomMacAddr);

private:
    // STA
    std::mutex mStaMutex;
    std::atomic<bool> mWifiSelfcureReset {false};
    std::atomic<bool> mWifiSelfcureResetEntered {false};
    std::atomic<int> mLastNetworkId {INVALID_NETWORK_ID};
    std::atomic<int> mSelectedCandidateNetworkId {INVALID_NETWORK_ID};
    std::atomic<bool> mWifiAllowSemiActive {false};
    std::atomic<bool> mWifiStoping {false};
    std::string mStaIfaceName {"wlan0"};
    std::map<int, std::atomic<int>> mWifiState;
    std::map<int, WifiDetailState> mWifiDetailState;
    std::map<int, std::atomic<WifiOprMidState>> mStaMidState;
    std::map<int, std::chrono::steady_clock::time_point> mWifiCloseTime;
    std::map<int, std::atomic<bool>> mIsAncoConnected;
    std::pair<int, WifiDeviceConfig> mLastRemoveDeviceConfig;
    std::map<int, IpInfo> mWifiIpInfo;
    std::map<int, IpV6Info> mWifiIpV6Info;
    std::map<int, WifiLinkedInfo> mWifiLinkedInfo;
    std::map<int, std::string> mMacAddress;
    std::map<int, int> mLastSelectedNetworkId;
    std::map<int, time_t> mLastSelectedTimeVal;
    std::map<int, std::pair<std::string, int>> mBssidToTimeoutTime;
    std::map<int, DisconnectedReason> mLastDiscReason;
    std::map<int, std::map<std::string, WifiCategoryBlackListInfo>> mWifiCategoryBlackListCache;
    std::map<std::string, WifiCategoryConnectFailInfo> mWifiConnectFailCache;

    // SCAN
    std::mutex mScanMutex;
    std::map<int, std::atomic<WifiOprMidState>> mScanMidState;
    std::map<int, std::atomic<WifiOprMidState>> mScanOnlyMidState;
    std::map<int, ScanControlInfo> mScanControlInfo;
    std::map<std::string, WifiCategory> mWifiCategoryRecord;
    std::vector<std::string> mAbnormalAppList;
    std::vector<WifiScanInfo> mWifiScanInfoList;

    // AP
    std::mutex mApMutex;
    std::atomic<bool> mSoftapToggled {false};
    std::atomic<int> mHotspotIdleTimeout {HOTSPOT_IDLE_TIMEOUT_INTERVAL_MS};
    std::string mApIfaceName {"wlan0"};
    std::map<int, std::atomic<WifiOprMidState>> mApMidState;
    std::map <int, std::atomic<int>> mHotspotState;
    std::map<int, PowerModel> powerModel;
    std::map<std::string, StationInfo> mConnectStationInfo;

    // P2P
    std::mutex mP2pMutex;
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    std::string mP2pIfaceName {"p2p0"};
#else
    std::string mP2pIfaceName {"p2p-dev-wlan0"};
#endif
    std::map<int, Hid2dUpperScene> mHid2dUpperScenePair;
    std::atomic<WifiOprMidState> mP2pMidState {WifiOprMidState::CLOSED};
    std::atomic<int> mP2pState {static_cast<int>(P2pState::P2P_STATE_CLOSED)};
    std::atomic<int> mP2pEnhanceState {0};
    std::atomic<int> mP2pDiscoverState {0};
    std::atomic<P2pBusinessType> mP2pBusinessType {P2pBusinessType::INVALID};
    std::atomic<int> mP2pCreatorUid {-1};
    std::atomic<bool> mExplicitGroup {false};
    WifiP2pLinkedInfo mWifiP2pInfo;
    WifiP2pGroupInfo m_P2pGroupInfo;

    // COMMON
    std::atomic<bool> mIsSupportCoex {false};
    std::atomic<int> mScreenState {MODE_STATE_DEFAULT};
    std::atomic<int> mThermalLevel {static_cast<int>(ThermalLevel::NORMAL)};
    std::atomic<int> mPowerIdelState {MODE_STATE_CLOSE};
    std::atomic<int> mGnssFixState {MODE_STATE_CLOSE};
    std::atomic<int> mScanGenieState {MODE_STATE_OPEN};
    std::atomic<int> mAirplaneModeState {MODE_STATE_CLOSE};
    std::atomic<int> mPersistWifiState {WIFI_STATE_DISABLED};
    std::atomic<int> mPowerSavingModeState {MODE_STATE_CLOSE};
    std::atomic<int> mFreezeModeState {MODE_STATE_CLOSE};
    std::atomic<int> mNoChargerPlugModeState {MODE_STATE_CLOSE};
    std::atomic<bool> mThreadStatusFlag_ {false};
    std::atomic<uint64_t> mThreadStartTime {0};
    std::mutex mMacAddrPairMutex;
    std::map<WifiMacAddrInfo, std::string> mWifiScanMacAddrPair;
    std::map<WifiMacAddrInfo, std::string> mHotspotMacAddrPair;
    std::map<WifiMacAddrInfo, std::string> mP2pDeviceMacAddrPair;
    std::map<WifiMacAddrInfo, std::string> mP2pGroupsInfoMacAddrPair;
    std::map<WifiMacAddrInfo, std::string> mP2pCurrentgroupMacAddrPair;
};
} // namespace Wifi
} // namespace OHOS
#endif
