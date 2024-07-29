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

#include "ap_started_state.h"
#include <cstring>
#include <string>
#include <typeinfo>
#include <vector>
#include <map>
#include "ap_config_use.h"
#include "ap_macro.h"
#include "ap_monitor.h"
#include "ap_service.h"
#include "ap_state_machine.h"
#include "ap_stations_manager.h"
#include "dhcpd_interface.h"
#include "wifi_ap_hal_interface.h"
#include "wifi_ap_nat_manager.h"
#include "wifi_channel_helper.h"
#include "wifi_config_center.h"
#include "wifi_logger.h"
#include "wifi_common_util.h"
#include "wifi_country_code_manager.h"
#include "wifi_hisysevent.h"
#include "wifi_global_func.h"
#include "wifi_cmd_client.h"
#include "wifi_sta_hal_interface.h"
#include "wifi_randommac_helper.h"
#ifdef HAS_BATTERY_MANAGER_PART
#include "battery_srv_client.h"
#define SET_DUAL_ANTENNAS 45
#endif
DEFINE_WIFILOG_HOTSPOT_LABEL("WifiApStartedState");

namespace OHOS {
namespace Wifi {
const std::string AP_DEFAULT_IP = "192.168.43.1";
const int STA_JOIN_HANDLE_DELAY = 5 * 1000;
ApStartedState::ApStartedState(ApStateMachine &apStateMachine, ApConfigUse &apConfigUse, ApMonitor &apMonitor, int id)
    : State("ApStartedState"),
      m_hotspotConfig(HotspotConfig()),
      m_ApStateMachine(apStateMachine),
      m_ApConfigUse(apConfigUse),
      m_ApMonitor(apMonitor),
      m_id(id)
{
    Init();
}

ApStartedState::~ApStartedState()
{}

void ApStartedState::GoInState()
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    m_ApStateMachine.OnApStateChange(ApState::AP_STATE_STARTING);
    WIFI_LOGI("Instance %{public}d %{public}s  GoInState", m_id, GetStateName().c_str());
    m_ApStateMachine.RegisterEventHandler();
    StartMonitor();
#ifdef SUPPORT_LOCAL_RANDOM_MAC
    SetRandomMac();
#endif
    if (!SetCountry()) {
        m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
        return;
    }
    if (StartAp() == false) {
        WIFI_LOGE("enter ApstartedState is failed.");
        m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
        return;
    }
    WIFI_LOGI("StartAP is ok.");
    if (SetConfig() == false) {
        WIFI_LOGE("wifi_settings.hotspotconfig is error.");
        m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
        return;
    }
    if (!m_ApStateMachine.m_ApStationsManager.EnableAllBlockList()) {
        WIFI_LOGE("Set Blocklist failed.");
    }
    WIFI_LOGE("Singleton version has not nat and use %{public}s.", AP_INTF);

    if (EnableInterfaceNat() == false) {
        m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
        return;
    }
    UpdatePowerMode();
    m_ApStateMachine.OnApStateChange(ApState::AP_STATE_STARTED);
}

void ApStartedState::GoOutState()
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    m_ApStateMachine.OnApStateChange(ApState::AP_STATE_CLOSING);
    DisableInterfaceNat();
    m_ApStateMachine.StopDhcpServer();
    if (!StopAp()) {
        WIFI_LOGE("StopAp not going well.");
    }
    StopMonitor();
    m_ApStateMachine.OnApStateChange(ApState::AP_STATE_IDLE);
    WifiConfigCenter::GetInstance().ClearStationList();
    WifiSettings::GetInstance().GetHotspotConfig(m_hotspotConfig, m_id);
    if (!m_hotspotConfig.GetIpAddress().empty() && m_hotspotConfig.GetIpAddress() != AP_DEFAULT_IP) {
        WIFI_LOGI("reset ip");
        m_hotspotConfig.SetIpAddress(AP_DEFAULT_IP);
        WifiSettings::GetInstance().SetHotspotConfig(m_hotspotConfig, m_id);
        WifiSettings::GetInstance().SyncHotspotConfig();
    }
}

void ApStartedState::Init()
{
    mProcessFunMap.insert(std::make_pair(ApStatemachineEvent::CMD_FAIL, &ApStartedState::ProcessCmdFail));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_STATION_JOIN, (ProcessFun)&ApStartedState::ProcessCmdStationJoin));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_STATION_LEAVE, (ProcessFun)&ApStartedState::ProcessCmdStationLeave));
    mProcessFunMap.insert(std::make_pair(
        ApStatemachineEvent::CMD_SET_HOTSPOT_CONFIG, (ProcessFun)&ApStartedState::ProcessCmdSetHotspotConfig));
    mProcessFunMap.insert(std::make_pair(
        ApStatemachineEvent::CMD_UPDATE_HOTSPOTCONFIG_RESULT, &ApStartedState::ProcessCmdUpdateConfigResult));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_ADD_BLOCK_LIST, &ApStartedState::ProcessCmdAddBlockList));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_DEL_BLOCK_LIST, &ApStartedState::ProcessCmdDelBlockList));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_STOP_HOTSPOT, &ApStartedState::ProcessCmdStopHotspot));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_DISCONNECT_STATION, &ApStartedState::ProcessCmdDisconnectStation));
    mProcessFunMap.insert(std::make_pair(ApStatemachineEvent::CMD_SET_IDLE_TIMEOUT,
    (ProcessFun)&ApStartedState::ProcessCmdSetHotspotIdleTimeout));
    mProcessFunMap.insert(
        std::make_pair(ApStatemachineEvent::CMD_UPDATE_COUNTRY_CODE, &ApStartedState::ProcessCmdUpdateCountryCode));
    mProcessFunMap.insert(std::make_pair(ApStatemachineEvent::CMD_HOTSPOT_CHANNEL_CHANGED,
    (ProcessFun)&ApStartedState::ProcessCmdHotspotChannelChanged));
    mProcessFunMap.insert(std::make_pair(ApStatemachineEvent::CMD_ASSOCIATED_STATIONS_CHANGED,
    (ProcessFun)&ApStartedState::ProcessCmdAssociatedStaChanged));
}

bool ApStartedState::ExecuteStateMsg(InternalMessagePtr msg)
{
    if (msg == nullptr) {
        WIFI_LOGE("fatal error!");
        return false;
    }
    int msgName = msg->GetMessageName();
    WIFI_LOGI("Instance %{public}d ApStartedState Process msgName:%{public}d.", m_id, msgName);

    auto iter = mProcessFunMap.find(static_cast<ApStatemachineEvent>(msgName));
    if (iter == mProcessFunMap.end()) {
        return NOT_EXECUTED;
    }
    ((*this).*(iter->second))(msg);

    return EXECUTED;
}

bool ApStartedState::UpdatMacAddress(const std::string ssid, KeyMgmt securityType)
{
    HotspotConfig curApConfig;
    WifiSettings::GetInstance().GetHotspotConfig(curApConfig, m_id);

    LOGD("%{public}s: [ssid:%{private}s, securityType:%{public}d] ==> [ssid:%{private}s, securityType:%{public}d]",
        __func__, curApConfig.GetSsid().c_str(), curApConfig.GetSecurityType(),
        ssid.c_str(), securityType);
    if ((curApConfig.GetSsid() != ssid) ||
        (curApConfig.GetSecurityType() != securityType)) {
        std::string macAddress;
        WifiRandomMacHelper::GenerateRandomMacAddress(macAddress);
        if (MacAddress::IsValidMac(macAddress.c_str())) {
            if (WifiApHalInterface::GetInstance().SetConnectMacAddr(
                WifiConfigCenter::GetInstance().GetApIfaceName(), macAddress) != WIFI_HAL_OPT_OK) {
                LOGE("%{public}s: failed to set ap MAC address:%{private}s", __func__, macAddress.c_str());
                return false;
            }
        } else {
            LOGW("%{public}s: macAddress is invalid", __func__);
        }
    }
    return true;
}

bool ApStartedState::SetConfig(HotspotConfig &apConfig)
{
    WIFI_LOGI("set softap config with param, id=%{public}d", m_id);
    m_ApConfigUse.UpdateApChannelConfig(apConfig);
    std::string ifName = WifiConfigCenter::GetInstance().GetApIfaceName();
    if (WifiApHalInterface::GetInstance().SetSoftApConfig(ifName, apConfig, m_id) != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("set hostapd config failed.");
        return false;
    }
#ifdef HAS_BATTERY_MANAGER_PART
    if (PowerMgr::BatterySrvClient::GetInstance().GetCapacity() > SET_DUAL_ANTENNAS) {
        HotspotConfig hotspotConfig;
        WifiSettings::GetInstance().GetHotspotConfig(hotspotConfig, m_id);
        if (hotspotConfig.GetBand() == BandType::BAND_2GHZ) {
            WifiCmdClient::GetInstance().SendCmdToDriver(ifName, CMD_SET_SOFTAP_2G_MSS, CMD_SET_SOFTAP_MIMOMODE);
        }
    }
#endif
    if (WifiApHalInterface::GetInstance().EnableAp(m_id) != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("Enableap failed.");
        return false;
    }

    if (apConfig.GetIpAddress().empty()) {
        WIFI_LOGI("IP is empty, set default ipaddr");
        apConfig.SetIpAddress(AP_DEFAULT_IP);
    }
    WifiSettings::GetInstance().SetHotspotConfig(apConfig, m_id);
    WifiSettings::GetInstance().SyncHotspotConfig();
    WIFI_LOGI("setConfig success");
    return true;
}

bool ApStartedState::SetConfig()
{
    WIFI_LOGI("set softap config, id=%{public}d", m_id);
    if (WifiSettings::GetInstance().GetHotspotConfig(m_hotspotConfig, m_id)) {
        WIFI_LOGE("get config failed");
        return false;
    }
    return SetConfig(m_hotspotConfig);
}

bool ApStartedState::StartAp() const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    std::string ifaceName = WifiConfigCenter::GetInstance().GetApIfaceName();
    WifiErrorNo retCode = WifiApHalInterface::GetInstance().StartAp(m_id, ifaceName);
    if (retCode != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("startAp is failed!");
        return false;
    }
    WriteWifiApStateHiSysEvent(1);
    return true;
}

bool ApStartedState::StopAp() const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    WifiErrorNo retCode = WifiApHalInterface::GetInstance().StopAp(m_id);
    if (retCode != WifiErrorNo::WIFI_HAL_OPT_OK) {
        return false;
    }
    WifiStaHalInterface::GetInstance().SetNetworkInterfaceUpDown(
        WifiConfigCenter::GetInstance().GetApIfaceName(), false);
    WriteWifiApStateHiSysEvent(0);
    return true;
}

void ApStartedState::StartMonitor() const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    m_ApMonitor.StartMonitor();
}

void ApStartedState::StopMonitor() const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    m_ApMonitor.StopMonitor();
}

bool ApStartedState::EnableInterfaceNat() const
{
#ifdef SUPPORT_NAT
    std::string ifaceName = WifiConfigCenter::GetInstance().GetApIfaceName();
    if (!mApNatManager.EnableInterfaceNat(true, ifaceName, ifaceName)) {
        WIFI_LOGE("set nat failed.");
        return false;
    }
#endif
    return true;
}

bool ApStartedState::DisableInterfaceNat() const
{
#ifdef SUPPORT_NAT
    std::string ifaceName = WifiConfigCenter::GetInstance().GetApIfaceName();
    if (!mApNatManager.EnableInterfaceNat(false, ifaceName, ifaceName)) {
        WIFI_LOGE("remove NAT config failed.");
    }
#endif
    return true;
}

void ApStartedState::ProcessCmdFail(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d State Machine message: %{public}d.", m_id, msg->GetMessageName());
    m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
}

void ApStartedState::ProcessCmdStationJoin(InternalMessagePtr msg)
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    StationInfo staInfo;
    if (!msg->GetMessageObj(staInfo)) {
        WIFI_LOGE("failed to get station info.");
        return;
    }
    if (curAssocMacList.find(staInfo.bssid) == curAssocMacList.end()) {
        WIFI_LOGE("sta has removed.");
        return;
    }
    m_ApStateMachine.m_ApStationsManager.StationJoin(staInfo);
}

void ApStartedState::ProcessCmdStationLeave(InternalMessagePtr msg)
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    StationInfo staInfo;
    WriteSoftApAbDisconnectHiSysEvent(AP_ERR_CODE);
    if (msg->GetMessageObj(staInfo)) {
        m_ApStateMachine.m_ApStationsManager.StationLeave(staInfo.bssid);
    } else {
        WIFI_LOGE("failed to get station info.");
    }
}

void ApStartedState::ProcessCmdSetHotspotConfig(InternalMessagePtr msg)
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    std::string ssid = msg->GetStringFromMessage();
    KeyMgmt securityType = static_cast<KeyMgmt>(msg->GetIntFromMessage());
#ifdef SUPPORT_LOCAL_RANDOM_MAC
    UpdatMacAddress(ssid, securityType);
#endif
    m_hotspotConfig.SetSsid(ssid);
    m_hotspotConfig.SetPreSharedKey(msg->GetStringFromMessage());
    m_hotspotConfig.SetSecurityType(securityType);
    m_hotspotConfig.SetBand(static_cast<BandType>(msg->GetIntFromMessage()));
    m_hotspotConfig.SetChannel(msg->GetIntFromMessage());
    m_hotspotConfig.SetBandWidth(msg->GetIntFromMessage());
    m_hotspotConfig.SetMaxConn(msg->GetIntFromMessage());
    m_hotspotConfig.SetIpAddress(msg->GetStringFromMessage());
    m_hotspotConfig.SetLeaseTime(msg->GetIntFromMessage());
    if (SetConfig(m_hotspotConfig)) {
        WIFI_LOGI("SetSoftApConfig success.");
    } else {
        WIFI_LOGE("SetSoftApConfig failed.");
    }
}

void ApStartedState::ProcessCmdUpdateConfigResult(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    if (msg->GetParam1() == 1) {
        WIFI_LOGI("Hot update HotspotConfig succeeded.");
        if (WifiSettings::GetInstance().SetHotspotConfig(m_hotspotConfig, m_id)) {
            WIFI_LOGE("set apConfig to settings failed.");
        }
#ifndef WIFI_DHCP_DISABLED
        m_ApStateMachine.StopDhcpServer();
        m_ApStateMachine.StartDhcpServer(m_hotspotConfig.GetIpAddress(), m_hotspotConfig.GetLeaseTime());
#endif
    } else {
        m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
    }
}

void ApStartedState::ProcessCmdAddBlockList(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    StationInfo staInfo;
    staInfo.deviceName = msg->GetStringFromMessage();
    staInfo.bssid = msg->GetStringFromMessage();
    staInfo.ipAddr = msg->GetStringFromMessage();
    WIFI_LOGI("staInfo:%{private}s, %{public}s, %{public}s.",
        staInfo.deviceName.c_str(), MacAnonymize(staInfo.bssid).c_str(), IpAnonymize(staInfo.ipAddr).c_str());
    m_ApStateMachine.m_ApStationsManager.AddBlockList(staInfo);
}

void ApStartedState::ProcessCmdDelBlockList(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    StationInfo staInfo;
    staInfo.deviceName = msg->GetStringFromMessage();
    staInfo.bssid = msg->GetStringFromMessage();
    staInfo.ipAddr = msg->GetStringFromMessage();
    WIFI_LOGI("staInfo:%{private}s, %{public}s, %{public}s.", staInfo.deviceName.c_str(),
        MacAnonymize(staInfo.bssid).c_str(), IpAnonymize(staInfo.ipAddr).c_str());
    m_ApStateMachine.m_ApStationsManager.DelBlockList(staInfo);
}

void ApStartedState::ProcessCmdStopHotspot(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d Disable hotspot: %{public}d.", m_id, msg->GetMessageName());
    m_ApStateMachine.SwitchState(&m_ApStateMachine.m_ApIdleState);
}

void ApStartedState::ProcessCmdDisconnectStation(InternalMessagePtr msg) const
{
    WIFI_LOGI("Instance %{public}d %{public}s", m_id, __func__);
    StationInfo staInfo;
    staInfo.deviceName = msg->GetStringFromMessage();
    staInfo.bssid = msg->GetStringFromMessage();
    staInfo.ipAddr = msg->GetStringFromMessage();
    m_ApStateMachine.m_ApStationsManager.DisConnectStation(staInfo);
}

void ApStartedState::ProcessCmdUpdateCountryCode(InternalMessagePtr msg) const
{
    std::string wifiCountryCode = msg->GetStringFromMessage();
    if (wifiCountryCode.empty() ||
        strncasecmp(wifiCountryCode.c_str(), m_wifiCountryCode.c_str(), WIFI_COUNTRY_CODE_LEN) == 0) {
        WIFI_LOGI("wifi country code is same or empty, code=%{public}s", wifiCountryCode.c_str());
        return;
    }
    WifiErrorNo ret = WifiApHalInterface::GetInstance().SetWifiCountryCode(
        WifiConfigCenter::GetInstance().GetApIfaceName(), wifiCountryCode);
    if (ret == WifiErrorNo::WIFI_HAL_OPT_OK) {
        m_wifiCountryCode = wifiCountryCode;
        WIFI_LOGI("update wifi country code success, wifiCountryCode=%{public}s", wifiCountryCode.c_str());
        WifiChannelHelper::GetInstance().UpdateValidChannels(
            WifiConfigCenter::GetInstance().GetApIfaceName(), m_id);
        return;
    }
    WIFI_LOGE("update wifi country code fail, wifiCountryCode=%{public}s, ret=%{public}d",
        wifiCountryCode.c_str(), ret);
}

void ApStartedState::UpdatePowerMode() const
{
    WIFI_LOGI("UpdatePowerMode.");
    int model = -1;
    if (WifiApHalInterface::GetInstance().GetPowerModel(
        WifiConfigCenter::GetInstance().GetApIfaceName(), model) != WIFI_HAL_OPT_OK) {
        LOGE("GetPowerModel() failed!");
        return;
    }
    LOGI("SetPowerModel(): %{public}d.", model);
    WifiConfigCenter::GetInstance().SetPowerModel(PowerModel(model));
}

void ApStartedState::ProcessCmdSetHotspotIdleTimeout(InternalMessagePtr msg)
{
    int mTimeoutDelay = msg->GetIntFromMessage();
    WIFI_LOGI("Set hotspot idle time is %{public}d", mTimeoutDelay);
    if (mTimeoutDelay == WifiConfigCenter::GetInstance().GetHotspotIdleTimeout()) {
        return;
    }
    WifiConfigCenter::GetInstance().SetHotspotIdleTimeout(mTimeoutDelay);
}

void ApStartedState::SetRandomMac() const
{
    std::string macAddress;
    WifiRandomMacHelper::GenerateRandomMacAddress(macAddress);
    if (MacAddress::IsValidMac(macAddress.c_str())) {
        if (WifiApHalInterface::GetInstance().SetConnectMacAddr(
            WifiConfigCenter::GetInstance().GetApIfaceName(), macAddress) != WIFI_HAL_OPT_OK) {
            LOGE("%{public}s: failed to set ap MAC address:%{private}s", __func__, macAddress.c_str());
        }
    } else {
        LOGE("%{public}s: macAddress is invalid", __func__);
    }
}

bool ApStartedState::SetCountry()
{
    std::string countryCode;
    WifiCountryCodeManager::GetInstance().GetWifiCountryCode(countryCode);
    if (countryCode.empty() || !IsValidCountryCode(countryCode) ||
        WifiApHalInterface::GetInstance().SetWifiCountryCode(WifiConfigCenter::GetInstance().GetApIfaceName(),
            countryCode) != WifiErrorNo::WIFI_HAL_OPT_OK) {
        WIFI_LOGE("set countryCode=%{public}s failed", countryCode.c_str());
        return false;
    }
    m_wifiCountryCode = std::move(countryCode);
    return true;
}

void ApStartedState::ProcessCmdHotspotChannelChanged(InternalMessage &msg)
{
    int freq = msg.GetParam1();
    int channel = TransformFrequencyIntoChannel(freq);
    WIFI_LOGI("%{public}s: update channel to %{public}d", __func__, channel);
    WifiSettings::GetInstance().GetHotspotConfig(m_hotspotConfig, m_id);
    m_hotspotConfig.SetChannel(channel);
    WifiSettings::GetInstance().SetHotspotConfig(m_hotspotConfig, m_id);
}

void ApStartedState::ProcessCmdAssociatedStaChanged(InternalMessage &msg)
{
    int event = msg.GetParam1();
    StationInfo staInfo;
    if (!msg.GetMessageObj(staInfo)) {
        WIFI_LOGE("%{public}s:failed to get station info.", __func__);
        return;
    }
    if (staInfo.bssid.empty()) {
        WIFI_LOGE("%{public}s:bssid is empty.", __func__);
        return;
    }
    WIFI_LOGI("%{public}s: associated station event: %{public}d", __func__, event);
    if (event == HAL_CBK_CMD_STA_JOIN) {
        curAssocMacList.insert(staInfo.bssid);
        m_ApStateMachine.MessageExecutedLater(static_cast<int>(ApStatemachineEvent::CMD_STATION_JOIN),
            staInfo, STA_JOIN_HANDLE_DELAY);
        return;
    }
    if (event == HAL_CBK_CMD_STA_LEAVE) {
        if (curAssocMacList.find(staInfo.bssid) != curAssocMacList.end()) {
            curAssocMacList.erase(staInfo.bssid);
            WIFI_LOGI("%{public}s: delete station in curAssocMacList", __func__);
        }
        m_ApStateMachine.SendMessage(static_cast<int>(ApStatemachineEvent::CMD_STATION_LEAVE), staInfo);
    }
    return;
}

}  // namespace Wifi
}  // namespace OHOS
