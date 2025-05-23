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
#include "ap_stations_manager.h"
#include <unistd.h>
#include "ap_service.h"
#include "wifi_log.h"
#include "wifi_config_center.h"
#include "wifi_ap_hal_interface.h"
#include "ap_state_machine.h"
#include "wifi_logger.h"
#include "wifi_common_util.h"

DEFINE_WIFILOG_HOTSPOT_LABEL("WifiApStationsManager");

namespace OHOS {
namespace Wifi {
ApStationsManager::ApStationsManager(int id) : m_id(id)
{}

ApStationsManager::~ApStationsManager()
{}

#ifdef HDI_CHIP_INTERFACE_SUPPORT
static WifiErrorNo SetMacFilter(std::string mac, int mid)
{
    std::vector<StationInfo> blockList;
    WifiSettings::GetInstance().GetBlockList(blockList, mid);

    std::vector<std::string> blockMacs;
    for (auto& sta : blockList) {
        if (mac != sta.bssid) {
            blockMacs.push_back(sta.bssid);
        }
    }

    std::string ifname = WifiConfigCenter::GetInstance().GetApIfaceName();
    WIFI_LOGI("SetMacFilter size:%{public}d", static_cast<int>(blockMacs.size()));
    return WifiApHalInterface::GetInstance().SetSoftApBlockList(ifname, blockMacs);
}
#endif

bool ApStationsManager::AddBlockList(const StationInfo &staInfo) const
{
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    WifiApHalInterface::GetInstance().DisAssociateSta(WifiConfigCenter::GetInstance().GetApIfaceName(), staInfo.bssid);
    return SetMacFilter("", m_id);
#else
    if (WifiApHalInterface::GetInstance().AddBlockByMac(staInfo.bssid, m_id) != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("Instance is %{public}d failed to add block.", m_id);
        return false;
    }
    return true;
#endif
}

bool ApStationsManager::DelBlockList(const StationInfo &staInfo) const
{
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    return SetMacFilter(staInfo.bssid, m_id);
#else
    if (WifiApHalInterface::GetInstance().DelBlockByMac(staInfo.bssid, m_id) != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("Instance is %{public}d failed to del block.", m_id);
        return false;
    }
    return true;
#endif
}

bool ApStationsManager::AddAssociationStation(const StationInfo &staInfo) const
{
    if (WifiConfigCenter::GetInstance().ManageStation(staInfo, MODE_ADD, m_id)) {
        WIFI_LOGE("Instance is %{public}d failed add station.", m_id);
        return false;
    }
    return true;
}

bool ApStationsManager::DelAssociationStation(const StationInfo &staInfo) const
{
    if (WifiConfigCenter::GetInstance().ManageStation(staInfo, MODE_DEL, m_id)) {
        WIFI_LOGE("Instance is %{public}d failed to del station.", m_id);
        return false;
    }
    return true;
}

bool ApStationsManager::EnableAllBlockList() const
{
#ifdef HDI_CHIP_INTERFACE_SUPPORT
    return SetMacFilter("", m_id);
#else
    std::vector<StationInfo> results;
    if (WifiSettings::GetInstance().GetBlockList(results, m_id)) {
        WIFI_LOGE("Instance is %{public}d failed to get blocklist.", m_id);
        return false;
    }
    std::string mac;
    bool ret = true;
    for (std::vector<StationInfo>::iterator iter = results.begin(); iter != results.end(); iter++) {
        if (WifiApHalInterface::GetInstance().AddBlockByMac(iter->bssid, m_id) != WifiErrorNo::WIFI_HAL_OPT_OK) {
            WIFI_LOGE("Instance is %{public}d failed to add block bssid is:%{public}s.", m_id,
                MacAnonymize(iter->bssid).c_str());
            ret = false;
        }
    }
    return ret;
#endif
}

void ApStationsManager::StationLeave(const std::string &mac) const
{
    StationInfo staInfo;
    std::vector<StationInfo> results;
    if (WifiConfigCenter::GetInstance().GetStationList(results, m_id)) {
        WIFI_LOGE("Instance is %{public}d failed to GetStationList.", m_id);
        return;
    }
    auto it = results.begin();
    for (; it != results.end(); ++it) {
        if (it->bssid == mac) {
            staInfo = *it;
            if (!DelAssociationStation(staInfo)) {
                WIFI_LOGE("Instance is %{public}d del AssociationStation failed.", m_id);
                return;
            }
            break;
        }
    }
    if (m_stationChangeCallback) {
        m_stationChangeCallback(staInfo, ApStatemachineEvent::CMD_STATION_LEAVE);
    }
    return;
}

void ApStationsManager::StationJoin(const StationInfo &staInfo) const
{
    StationInfo staInfoTemp = staInfo;
    std::vector<StationInfo> results;
    if (WifiConfigCenter::GetInstance().GetStationList(results, m_id)) {
        WIFI_LOGE("Instance is %{public}d failed to GetStationList.", m_id);
        return;
    }
    auto it = results.begin();
    for (; it != results.end(); ++it) {
        if (it->bssid == staInfo.bssid) {
            if (staInfo.deviceName == OHOS::Wifi::GETTING_INFO && staInfo.ipAddr == OHOS::Wifi::GETTING_INFO) {
                staInfoTemp = *it;
            }
            break;
        }
    }

    if (!AddAssociationStation(staInfoTemp)) {
        WIFI_LOGE("Instance is %{public}d addAssociationStation failed.", m_id);
        return;
    }

    if (it == results.end() || it->ipAddr != staInfo.ipAddr) {
        if (m_stationChangeCallback) {
            m_stationChangeCallback(staInfoTemp, ApStatemachineEvent::CMD_STATION_JOIN);
        }
    }
    return;
}

bool ApStationsManager::DisConnectStation(const StationInfo &staInfo) const
{
    std::string mac = staInfo.bssid;
    int ret = static_cast<int>(WifiApHalInterface::GetInstance().DisconnectStaByMac(mac, m_id));
    if (ret != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("Instance is %{public}d failed to DisConnectStation staInfo bssid:%{public}s,address:%{public}s, name:%{private}s.",
            m_id,
            MacAnonymize(staInfo.bssid).c_str(),
            IpAnonymize(staInfo.ipAddr).c_str(),
            staInfo.deviceName.c_str());
        return false;
    }
    return true;
}

std::vector<std::string> ApStationsManager::GetAllConnectedStations() const
{
    std::vector<std::string> staMacList;
    if (WifiApHalInterface::GetInstance().GetStationList(staMacList, m_id) == WifiErrorNo::WIFI_HAL_OPT_OK) {
        for (size_t i = 0; i < staMacList.size(); ++i) {
            WIFI_LOGD("Instance is %{public}d staMacList[%{public}zu]:%{private}s.", m_id, i, staMacList[i].c_str());
        }
    }
    return staMacList;
}

void ApStationsManager::RegisterEventHandler(std::function<void(const StationInfo &, ApStatemachineEvent)> callback)
{
    m_stationChangeCallback = callback;
}
}  // namespace Wifi
}  // namespace OHOS