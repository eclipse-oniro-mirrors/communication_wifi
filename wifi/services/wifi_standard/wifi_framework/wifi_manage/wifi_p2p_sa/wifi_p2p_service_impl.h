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

#ifndef OHOS_WIFI_P2P_SERVICE_IMPL_H
#define OHOS_WIFI_P2P_SERVICE_IMPL_H

#include "wifi_p2p_msg.h"
#include "wifi_errcode.h"
#include "system_ability.h"
#include "wifi_p2p_stub.h"
#include "iremote_object.h"

namespace OHOS {
namespace Wifi {
enum ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
class WifiP2pServiceImpl : public SystemAbility, public WifiP2pStub {
    DECLARE_SYSTEM_ABILITY(WifiP2pServiceImpl);

public:
    WifiP2pServiceImpl();
    virtual ~WifiP2pServiceImpl();

    static sptr<WifiP2pServiceImpl> GetInstance();

    void OnStart() override;
    void OnStop() override;

    /**
     * @Description Enabling the P2P Mode
     *
     * @return ErrCode - operation result
     */
    ErrCode EnableP2p(void) override;

    /**
     * @Description Disable the P2P mode
     *
     * @return ErrCode - operation result
     */
    ErrCode DisableP2p(void) override;

    /**
     * @Description Start Wi-Fi P2P device search
     *
     * @return ErrCode - operation result
     */
    ErrCode DiscoverDevices(void) override;

    /**
     * @Description Stop Wi-Fi P2P device search
     *
     * @return ErrCode - operation result
     */
    ErrCode StopDiscoverDevices(void) override;

    /**
     * @Description Start the search for the Wi-Fi P2P service
     *
     * @return ErrCode - operation result
     */
    ErrCode DiscoverServices(void) override;

    /**
     * @Description Stop the search for the Wi-Fi P2P service
     *
     * @return ErrCode - operation result
     */
    ErrCode StopDiscoverServices(void) override;

    /**
     * @Description request the P2P service
     *
     * @param device - WifiP2pDevice object
     * @param request - WifiP2pServiceRequest object
     * @return ErrCode - operation result
     */
    ErrCode RequestService(const WifiP2pDevice &device, const WifiP2pServiceRequest &request) override;

    /**
     * @Description Register the local P2P service
     *
     * @param srvInfo - WifiP2pServiceInfo object
     * @return ErrCode - operation result
     */
    ErrCode PutLocalP2pService(const WifiP2pServiceInfo &srvInfo) override;

    /**
     * @Description Delete the local P2P service
     *
     * @param srvInfo - WifiP2pServiceInfo object
     * @return ErrCode - operation result
     */
    ErrCode DeleteLocalP2pService(const WifiP2pServiceInfo &srvInfo) override;

    /**
     * @Description Enable Wi-Fi P2P listening
     *
     * @param period - period
     * @param interval - interval
     * @return ErrCode - operation result
     */
    ErrCode StartP2pListen(int period, int interval) override;

    /**
     * @Description Disable Wi-Fi P2P listening
     *
     * @return ErrCode - operation result
     */
    ErrCode StopP2pListen(void) override;

    /**
     * @Description Creating a P2P Group
     *
     * @param config - WifiP2pGroupInfo object
     * @return ErrCode - operation result
     */
    ErrCode CreateGroup(const WifiP2pConfig &config) override;

    /**
     * @Description Remove a P2P Group
     *
     *
     * @return ErrCode - operation result
     */
    ErrCode RemoveGroup(void) override;

    /**
     * @Description Remove a P2P client of current group.
     *
     * @param deviceMac - client deviceMac address
     * @return ErrCode - operation result
     */
    ErrCode RemoveGroupClient(const GcInfo &info) override;

    /**
     * @Description Delete a p2p Group
     *
     * @param group - WifiP2pGroupInfo object
     * @return ErrCode - operation result
     */
    ErrCode DeleteGroup(const WifiP2pGroupInfo &group) override;

    /**
     * @Description P2P connection
     *
     * @param config - WifiP2pConfig object
     * @return ErrCode - operation result
     */
    ErrCode P2pConnect(const WifiP2pConfig &config) override;

    /**
     * @Description Canceling a P2P connection
     *
     * @return ErrCode - operation result
     */
    ErrCode P2pCancelConnect(void) override;

    /**
     * @Description Querying Wi-Fi P2P Connection Information
     *
     * @param linkedInfo - Get the WifiP2pLinkedInfo msg
     * @return ErrCode - operation result
     */
    ErrCode QueryP2pLinkedInfo(WifiP2pLinkedInfo &linkedInfo) override;

    /**
     * @Description Get the Current Group object
     *
     * @param group - the WifiP2pGroupInfo object
     * @return ErrCode - operation result
     */
    ErrCode GetCurrentGroup(WifiP2pGroupInfo &group) override;

    /**
     * @Description Obtains the P2P switch status
     *
     * @param status - the P2P switch status
     * @return ErrCode - operation result
     */
    ErrCode GetP2pEnableStatus(int &status) override;

    /**
     * @Description Obtains the P2P discovery status
     *
     * @param status - the P2P discovery status
     * @return ErrCode
     */
    ErrCode GetP2pDiscoverStatus(int &status) override;

    /**
     * @Description Obtains the P2P connection status
     *
     * @param status - the P2P connection status
     * @return ErrCode - operation result
     */
    ErrCode GetP2pConnectedStatus(int &status) override;

    /**
     * @Description Query the information about the found devices
     *
     * @param devices - Get result vector of WifiP2pDevice
     * @return ErrCode - operation result
     */
    ErrCode QueryP2pDevices(std::vector<WifiP2pDevice> &devices) override;

    /**
     * @Description Query the information about the local device
     *
     * @param devives - Get result of WifiP2pDevice
     * @return ErrCode - operation result
     */
    ErrCode QueryP2pLocalDevice(WifiP2pDevice &device) override;

    /**
     * @Description Query the information about the found groups
     *
     * @param groups - Get result vector of WifiP2pGroupInfo
     * @return ErrCode - operation result
     */
    ErrCode QueryP2pGroups(std::vector<WifiP2pGroupInfo> &groups) override;

    /**
     * @Description Query the service information
     *
     * @param services - Get result vector of Device
     * @return ErrCode - operation result
     */
    ErrCode QueryP2pServices(std::vector<WifiP2pServiceInfo> &services) override;

    /**
     * @Description Register callback function
     *
     * @param callback - IWifiP2pCallback object
     * @return ErrCode - operation result
     */
    ErrCode RegisterCallBack(const sptr<IWifiP2pCallback> &callback, const std::vector<std::string> &event) override;

    /**
     * @Description Get supported feature
     *
     * @param features - return supported feature
     * @return ErrCode - operation result
     */
    ErrCode GetSupportedFeatures(long &features) override;

    /**
     * @Description set the device name
     *
     * @param deviceName - device name
     * @return ErrCode - operation result
     */
    ErrCode SetP2pDeviceName(const std::string &deviceName) override;

    /**
     * @Description set p2p wifi display info
     *
     * @param wfdInfo - wifi display info
     * @return ErrCode - operation result
     */
    ErrCode SetP2pWfdInfo(const WifiP2pWfdInfo &wfdInfo) override;

    /**
     * @Description Request an IP address to the Gc from the IP address pool, used on the GO side.
     *
     * @param gcMac - gc mac address
     * @param ipAddr - applied ip address
     * @return ErrCode - operation result
     */
    ErrCode Hid2dRequestGcIp(const std::string& gcMac, std::string& ipAddr) override;

    /**
     * @Description Increase(+1) hid2d shared link reference counting
     *
     * @return ErrCode - operation result
     */
    ErrCode Hid2dSharedlinkIncrease() override;

    /**
     * @Description Decrease(-1) hid2d shared link reference counting
     *
     * @return ErrCode - operation result
     */
    ErrCode Hid2dSharedlinkDecrease() override;

    /**
     * @Description Create hid2d group, used on the GO side.
     *
     * @param frequency - frequency
     * @param type - frequency type
     * @return ErrCode - operation result
     */
    ErrCode Hid2dCreateGroup(const int frequency, FreqType type) override;

    /**
     * @Description The GC side actively disconnects from the GO, used on the GC side.
     *
     * @param gcIfName - network interface name
     * @return ErrCode - operation result
     */
    ErrCode Hid2dRemoveGcGroup(const std::string& gcIfName) override;

    /**
     * @Description Connect to a specified group using hid2d, used on the GC side.
     *
     * @param config - connection parameters
     * @return ErrCode - operation result
     */
    ErrCode Hid2dConnect(const Hid2dConnectConfig& config) override;

    /**
     * @Description Configuring IP addresses for P2P network interfaces, used on the GC side.
     *
     * @param ifName - network interface name
     * @param ipInfo - IP infos
     * @return ErrCode - operation result
     */
    ErrCode Hid2dConfigIPAddr(const std::string& ifName, const IpAddrInfo& ipInfo) override;

    /**
     * @Description Clear IP address when the P2P connection is disconnected, used on the GC side.
     *
     * @param ifName - network interface name
     * @return ErrCode - operation result
     */
    ErrCode Hid2dReleaseIPAddr(const std::string& ifName) override;

     /**
     * @Description Obtain the recommended channel and bandwidth for link setup
     *
     * @param request - request data
     * @param response - response result
     * @return ErrCode - operation result
     */
    ErrCode Hid2dGetRecommendChannel(const RecommendChannelRequest& request,
        RecommendChannelResponse& response) override;

    /**
     * @Description get 5G channel list
     *
     * @param vecChannelList - result for channel list
     * @return ErrCode - operation result
     */
    ErrCode Hid2dGetChannelListFor5G(std::vector<int>& vecChannelList) override;

    /**
     * @Description get the self wifi configuration information
     *
     * @param cfgType - configuration type
     * @param cfgData - the queried data of wifi configuration
     * @param getDatValidLen - the valid data length in the array `cfgData`
     * @return ErrCode - operation result
     */
    ErrCode Hid2dGetSelfWifiCfgInfo(SelfCfgType cfgType,
        char cfgData[CFG_DATA_MAX_BYTES], int* getDatValidLen) override;

    /**
     * @Description set the peer wifi configuration information
     *
     * @param cfgType - configuration type
     * @param cfgData - the wifi configuration data to be set
     * @param setDataValidLen - the valid data length in the array `cfgData`
     * @return ErrCode - operation result
     */
    ErrCode Hid2dSetPeerWifiCfgInfo(PeerCfgType cfgType,
        char cfgData[CFG_DATA_MAX_BYTES], int setDataValidLen) override;

    /**
     * @Description Set the scene of upper layer
     *
     * @param ifName - interface name
     * @param scene - scene
     * @return ErrCode - operate result
     */
    ErrCode Hid2dSetUpperScene(const std::string& ifName, const Hid2dUpperScene& scene) override;

    /**
     * @Description Monitor the wifi configuration change
     *
     * @return ErrCode - operate result
     */
    ErrCode MonitorCfgChange(void) override;

    /**
     * @Description dump p2p information
     *
     * @param fd - file descriptor
     * @param args - dump arguments
     * @return ErrCode - operation result
     */
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    /**
     * @Description Check whether service is died.
     *
     * @return bool - true: service is died, false: service is not died.
     */
    bool IsRemoteDied(void) override;

    /**
     * @Description Discover p2p device
     *
     * @return ErrCode - operate result
     */
    ErrCode DiscoverPeers(int32_t channelid) override;

    /**
     * @Description Disable random mac
     *
     * @return ErrCode - operate result
     */
    ErrCode DisableRandomMac(int setmode) override;

    /**
     * @Description Check can use P2P
     *
     * @return ErrCode - operation result
     */
    ErrCode CheckCanUseP2p() override;

    /**
     * @Description Hid2d Is Wide Bandwidth Supported
     *
     * @param isSupport - is support 160M
     * @return ErrCode - operation result
     */
    ErrCode Hid2dIsWideBandwidthSupported(bool &isSupport) override;

    /**
     * @Description Set miracast sink config
     *
     * @param config - miracast config
     * @return ErrCode - operation result
     */
    ErrCode SetMiracastSinkConfig(const std::string& config) override;

    /**
     * @Description Get support channels for band
     *
     * @param channels - support channels
     * @param band - channel band
     * @return ErrCode - operation result
     */
    ErrCode GetSupportedChanForBand(std::vector<int> &channels, int band) override;
private:
    bool Init();
    ErrCode CheckCanEnableP2p(void);
    // forceEnable: true - force enable P2pService if p2pservice is not running, false - check the service status
    bool IsP2pServiceRunning(bool forceEnable = true);
    bool IsCallingAllowed();
    static void SaBasicDump(std::string& result);

private:
    static sptr<WifiP2pServiceImpl> instance;
    static std::mutex instanceLock;
    static std::mutex g_p2pMutex;
    bool mPublishFlag;
    ServiceRunningState mState;
    static constexpr int WIFI_P2P_PASSPHRASE_MIN_LEN = 8;
    static constexpr int WIFI_P2P_PASSPHRASE_MAX_LEN = 63;
};
}  // namespace Wifi
}  // namespace OHOS
#endif