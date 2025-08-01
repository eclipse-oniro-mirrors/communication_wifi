/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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


#ifndef OHOS_WIFI_NETWORK_SELECTION_MANAGER_H
#define OHOS_WIFI_NETWORK_SELECTION_MANAGER_H

#include <mutex>
#include "network_selection.h"
#include "network_selector_factory.h"

namespace OHOS::Wifi {
struct NetworkSelectionResult {
    InterScanInfo interScanInfo;
    WifiDeviceConfig wifiDeviceConfig;
};

class NetworkSelectionManager {
public:
    NetworkSelectionManager();

    /**
     * the function to select network with ssid
     *
     * @param deviceConfig Ap device config information
     * @param autoSelectBssid Candidate network bssid
     */
    void SelectNetworkWithSsid(WifiDeviceConfig& deviceConfig, std::string& autoSelectBssid);

    /**
     * Convert WifiScanInfo to InterScanInfo
     *
     * @param wifiScanInfo wifiScanInfo
     * @param interScanInfo interScanInfo
     */
    void ConvertScanInfo(WifiScanInfo &wifiScanInfo, InterScanInfo &interScanInfo);

    /**
     * the function to select network.
     *
     * @param networkSelectionResult Network selection result
     * @param type the type of networkSelection
     * @param scanInfos scanInfos
     * @return whether network selection is successful.
     */
    bool SelectNetwork(NetworkSelectionResult &networkSelectionResult,
                       NetworkSelectType type,
                       const std::vector<InterScanInfo> &scanInfos);
private:
    std::unique_ptr<NetworkSelectorFactory> pNetworkSelectorFactory = nullptr;

    /**
     * get the saved deviceConfig associated with scanInfo
     *
     * @param networkCandidates  Candidate network
     * @param scanInfos scanInfos
     */
    static void GetAllDeviceConfigs(std::vector<NetworkSelection::NetworkCandidate> &networkCandidates,
                                    const std::vector<InterScanInfo> &scanInfos);

    /**
     * Try nominator the candidate network.
     *
     * @param networkCandidates candidate networks
     * @param networkSelector networkSelector
     */
    static void TryNominate(std::vector<NetworkSelection::NetworkCandidate> &networkCandidates,
                            const std::unique_ptr<NetworkSelection::INetworkSelector> &networkSelector);
    
    /**
     * get saved configs for chr
     *
     * @param networkCandidates candidate networks
     */
    std::string GetSavedNetInfoForChr(
        std::vector<NetworkSelection::NetworkCandidate> &networkCandidates, bool &isSavedNetEmpty);
    /**
     * get filtered reason for chr
     *
     * @param networkCandidates candidate networks
     */
    std::string GetFilteredReasonForChr(
        std::vector<NetworkSelection::NetworkCandidate> &networkCandidates);
    
    /**
     * get selected network info for chr
     *
     * @param networkCandidate best network candidate
     */
    std::string GetSelectedInfoForChr(NetworkSelection::NetworkCandidate *networkCandidate);
    
    /**
     * is outdoor filter
     *
     * @param networkCandidate best network candidate
     */
    bool IsOutdoorFilter(NetworkSelection::NetworkCandidate *networkCandidate);

private:
    std::mutex rssiCntMutex_;
    std::map<std::string, int> rssiCntMap_;
};
}
#endif
