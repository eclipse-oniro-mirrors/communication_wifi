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

#include <cstdio>
#include <chrono>
#include <random>
#include "sta_state_machine.h"
#include "if_config.h"
#include "ip_tools.h"
#include "log_helper.h"
#include "mac_address.h"
#include "sta_monitor.h"
#include "wifi_chip_capability.h"
#include "wifi_common_util.h"
#include "wifi_logger.h"
#include "wifi_settings.h"
#include "wifi_sta_hal_interface.h"
#include "wifi_supplicant_hal_interface.h"
#include "wifi_hisysevent.h"
#include "wifi_config_center.h"
#include "wifi_hisysevent.h"
#ifndef OHOS_ARCH_LITE
#include "ability_manager_client.h"
#include "wifi_net_observer.h"
#include <dlfcn.h>
#endif // OHOS_ARCH_LITE

#ifndef OHOS_WIFI_STA_TEST
#else
#include "mock_dhcp_service.h"
#endif
namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("StaStateMachine");
#define PBC_ANY_BSSID "any"
#define FIRST_DNS "8.8.8.8"
#define SECOND_DNS "180.76.76.76"
#define PORTAL_ACTION "ohos.want.action.viewData"
#define PORTAL_ENTITY "entity.system.browsable"
#define PORTAL_CHECK_TIME (10 * 60)
#define PORTAL_MILLSECOND  1000
#define WPA3_BLACKMAP_MAX_NUM 20
#define WPA3_BLACKMAP_RSSI_THRESHOLD (-70)
#define WPA3_CONNECT_FAIL_COUNT_THRESHOLD 2
#define WPA_CB_ASSOCIATING 3
#define WPA_CB_CONNECTED 1
#define WPA_CB_ASSOCIATED 4
#define TRANSFORMATION_TO_MBPS 10
#define DEFAULT_NUM_ARP_PINGS 3
#define MAX_ARP_CHECK_TIME 300
#define NETWORK 1
#define NO_NETWORK 0
#define WPA_DEFAULT_NETWORKID 0

StaStateMachine::StaStateMachine(int instId)
    : StateMachine("StaStateMachine"),
      lastNetworkId(INVALID_NETWORK_ID),
      operationalMode(STA_CONNECT_MODE),
      targetNetworkId(INVALID_NETWORK_ID),
      pinCode(0),
      wpsState(SetupMethod::INVALID),
      lastSignalLevel(-1),
      targetRoamBssid(WPA_BSSID_ANY),
      currentTpType(IPTYPE_IPV4),
      isWpsConnect(IsWpsConnected::WPS_INVALID),
      getIpSucNum(0),
      getIpFailNum(0),
      enableSignalPoll(true),
      isRoam(false),
      netNoWorkNum(0),
      networkStatusHistoryInserted(false),
      pDhcpResultNotify(nullptr),
      pNetcheck(nullptr),
      pRootState(nullptr),
      pInitState(nullptr),
      pWpaStartingState(nullptr),
      pWpaStartedState(nullptr),
      pWpaStoppingState(nullptr),
      pLinkState(nullptr),
      pSeparatingState(nullptr),
      pSeparatedState(nullptr),
      pApLinkedState(nullptr),
      pWpsState(nullptr),
      pGetIpState(nullptr),
      pLinkedState(nullptr),
      pApRoamingState(nullptr),
      m_netState(NETWORK_UNKNOWN),
      m_instId(instId)
{
}

StaStateMachine::~StaStateMachine()
{
    WIFI_LOGI("~StaStateMachine");
    StopHandlerThread();
    ParsePointer(pRootState);
    ParsePointer(pInitState);
    ParsePointer(pWpaStartingState);
    ParsePointer(pWpaStartedState);
    ParsePointer(pWpaStoppingState);
    ParsePointer(pLinkState);
    ParsePointer(pSeparatingState);
    ParsePointer(pSeparatedState);
    ParsePointer(pApLinkedState);
    ParsePointer(pWpsState);
    ParsePointer(pGetIpState);
    ParsePointer(pLinkedState);
    ParsePointer(pApRoamingState);
    std::string ifname = IF_NAME + std::to_string(m_instId);
    if (currentTpType == IPTYPE_IPV4) {
        StopDhcpClient(ifname.c_str(), false);
    } else {
        StopDhcpClient(ifname.c_str(), true);
    }
    ParsePointer(pDhcpResultNotify);
    ParsePointer(pNetcheck);
}

/* ---------------------------Initialization functions------------------------------ */
ErrCode StaStateMachine::InitStaStateMachine()
{
    WIFI_LOGI("Enter InitStaStateMachine.\n");
    if (!InitialStateMachine()) {
        WIFI_LOGE("Initial StateMachine failed.\n");
        return WIFI_OPT_FAILED;
    }

    if (InitStaStates() == WIFI_OPT_FAILED) {
        return WIFI_OPT_FAILED;
    }
    BuildStateTree();
    SetFirstState(pInitState);
    StartStateMachine();
    InitStaSMHandleMap();
    pNetcheck = new (std::nothrow)
        StaNetworkCheck(std::bind(&StaStateMachine::NetDetectionProcess, this,
            std::placeholders::_1, std::placeholders::_2),
            std::bind(&StaStateMachine::HandleArpCheckResult, this, std::placeholders::_1),
            std::bind(&StaStateMachine::HandleDnsCheckResult, this, std::placeholders::_1), m_instId);
    if (pNetcheck == nullptr) {
        WIFI_LOGE("pNetcheck is null\n");
        return WIFI_OPT_FAILED;
    }
    pNetcheck->InitNetCheckThread();
#ifndef OHOS_ARCH_LITE
    NetSupplierInfo = std::make_unique<NetManagerStandard::NetSupplierInfo>().release();
    m_NetWorkState = sptr<NetStateObserver>(new NetStateObserver());
    m_NetWorkState->SetNetStateCallback(
        std::bind(&StaStateMachine::NetStateObserverCallback, this, std::placeholders::_1));
#endif
    return WIFI_OPT_SUCCESS;
}

ErrCode StaStateMachine::InitStaStates()
{
    WIFI_LOGE("Enter InitStaStates\n");
    int tmpErrNumber;
    pRootState = new (std::nothrow)RootState();
    tmpErrNumber = JudgmentEmpty(pRootState);
    pInitState = new (std::nothrow)InitState(this);
    tmpErrNumber += JudgmentEmpty(pInitState);
    pWpaStartingState = new (std::nothrow)WpaStartingState(this);
    tmpErrNumber += JudgmentEmpty(pWpaStartingState);
    pWpaStartedState = new (std::nothrow)WpaStartedState(this);
    tmpErrNumber += JudgmentEmpty(pWpaStartedState);
    pWpaStoppingState = new (std::nothrow)WpaStoppingState(this);
    tmpErrNumber += JudgmentEmpty(pWpaStoppingState);
    pLinkState = new (std::nothrow)LinkState(this);
    tmpErrNumber += JudgmentEmpty(pLinkState);
    pSeparatingState = new (std::nothrow)SeparatingState();
    tmpErrNumber += JudgmentEmpty(pSeparatingState);
    pSeparatedState = new (std::nothrow)SeparatedState(this);
    tmpErrNumber += JudgmentEmpty(pSeparatedState);
    pApLinkedState = new (std::nothrow)ApLinkedState(this);
    tmpErrNumber += JudgmentEmpty(pApLinkedState);
    pWpsState = new (std::nothrow)StaWpsState(this);
    tmpErrNumber += JudgmentEmpty(pWpsState);
    pGetIpState = new (std::nothrow)GetIpState(this);
    tmpErrNumber += JudgmentEmpty(pGetIpState);
    pLinkedState = new (std::nothrow)LinkedState(this);
    tmpErrNumber += JudgmentEmpty(pLinkedState);
    pApRoamingState = new (std::nothrow)ApRoamingState(this);
    tmpErrNumber += JudgmentEmpty(pApRoamingState);
    pDhcpResultNotify = new (std::nothrow)DhcpResultNotify();
    tmpErrNumber += JudgmentEmpty(pDhcpResultNotify);
    if (tmpErrNumber != 0) {
        WIFI_LOGE("InitStaStates some one state is null\n");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

void StaStateMachine::InitWifiLinkedInfo()
{
    linkedInfo.networkId = INVALID_NETWORK_ID;
    linkedInfo.ssid = "";
    linkedInfo.bssid = "";
    linkedInfo.macAddress = "";
    linkedInfo.macType = 0;
    linkedInfo.rxLinkSpeed = 0;
    linkedInfo.txLinkSpeed = 0;
    linkedInfo.rssi = 0;
    linkedInfo.band = 0;
    linkedInfo.frequency = 0;
    linkedInfo.linkSpeed = 0;
    linkedInfo.ipAddress = 0;
    linkedInfo.connState = ConnState::DISCONNECTED;
    linkedInfo.ifHiddenSSID = false;
    linkedInfo.chload = 0;
    linkedInfo.snr = 0;
    linkedInfo.isDataRestricted = 0;
    linkedInfo.platformType = "";
    linkedInfo.portalUrl = "";
    linkedInfo.detailedState = DetailedState::DISCONNECTED;
    linkedInfo.channelWidth = WifiChannelWidth::WIDTH_INVALID;
    linkedInfo.lastPacketDirection = 0;
    linkedInfo.lastRxPackets = 0;
    linkedInfo.lastTxPackets = 0;
    linkedInfo.retryedConnCount = 0;
    linkedInfo.isAncoConnected = 0;
}

void StaStateMachine::InitLastWifiLinkedInfo()
{
    lastLinkedInfo.networkId = INVALID_NETWORK_ID;
    lastLinkedInfo.ssid = "";
    lastLinkedInfo.bssid = "";
    lastLinkedInfo.macAddress = "";
    linkedInfo.macType = 0;
    lastLinkedInfo.rxLinkSpeed = 0;
    lastLinkedInfo.txLinkSpeed = 0;
    lastLinkedInfo.rssi = 0;
    lastLinkedInfo.band = 0;
    lastLinkedInfo.frequency = 0;
    lastLinkedInfo.linkSpeed = 0;
    lastLinkedInfo.ipAddress = 0;
    lastLinkedInfo.connState = ConnState::DISCONNECTED;
    lastLinkedInfo.ifHiddenSSID = false;
    lastLinkedInfo.chload = 0;
    lastLinkedInfo.snr = 0;
    linkedInfo.isDataRestricted = 0;
    linkedInfo.platformType = "";
    linkedInfo.portalUrl = "";
    lastLinkedInfo.lastPacketDirection = 0;
    lastLinkedInfo.lastRxPackets = 0;
    lastLinkedInfo.lastTxPackets = 0;
    lastLinkedInfo.detailedState = DetailedState::DISCONNECTED;
    linkedInfo.retryedConnCount = 0;
}

void StaStateMachine::BuildStateTree()
{
    StatePlus(pRootState, nullptr);
    StatePlus(pInitState, pRootState);
    StatePlus(pWpaStartingState, pRootState);
    StatePlus(pWpaStartedState, pRootState);
    StatePlus(pLinkState, pWpaStartedState);
    StatePlus(pSeparatingState, pLinkState);
    StatePlus(pSeparatedState, pLinkState);
    StatePlus(pApLinkedState, pLinkState);
    StatePlus(pGetIpState, pApLinkedState);
    StatePlus(pLinkedState, pApLinkedState);
    StatePlus(pApRoamingState, pApLinkedState);
    StatePlus(pWpsState, pLinkState);
    StatePlus(pWpaStoppingState, pRootState);
}

void StaStateMachine::RegisterStaServiceCallback(const StaServiceCallback &callback)
{
    WIFI_LOGI("RegisterStaServiceCallback, callback module name: %{public}s", callback.callbackModuleName.c_str());
    m_staCallback.insert_or_assign(callback.callbackModuleName, callback);
}

void StaStateMachine::InvokeOnStaOpenRes(OperateResState state)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnStaOpenRes != nullptr) {
            callBackItem.second.OnStaOpenRes(state, m_instId);
        }
    }
}

void StaStateMachine::InvokeOnStaCloseRes(OperateResState state)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnStaCloseRes != nullptr) {
            callBackItem.second.OnStaCloseRes(state, m_instId);
        }
    }
}

void StaStateMachine::InvokeOnStaConnChanged(OperateResState state, const WifiLinkedInfo &info)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnStaConnChanged != nullptr) {
            callBackItem.second.OnStaConnChanged(state, info, m_instId);
        }
    }
}

void StaStateMachine::InvokeOnWpsChanged(WpsStartState state, const int code)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnWpsChanged != nullptr) {
            callBackItem.second.OnWpsChanged(state, code, m_instId);
        }
    }
}

void StaStateMachine::InvokeOnStaStreamChanged(StreamDirection direction)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnStaStreamChanged != nullptr) {
            callBackItem.second.OnStaStreamChanged(direction, m_instId);
        }
    }
}

void StaStateMachine::InvokeOnStaRssiLevelChanged(int level)
{
    for (const auto &callBackItem : m_staCallback) {
        if (callBackItem.second.OnStaRssiLevelChanged != nullptr) {
            callBackItem.second.OnStaRssiLevelChanged(level, m_instId);
        }
    }
}

/* --------------------------- state machine root state ------------------------------ */
StaStateMachine::RootState::RootState() : State("RootState")
{}

StaStateMachine::RootState::~RootState()
{}

void StaStateMachine::RootState::GoInState()
{
    WIFI_LOGI("RootState GoInState function.");
    return;
}

void StaStateMachine::RootState::GoOutState()
{
    WIFI_LOGI("RootState GoOutState function.");
    return;
}

bool StaStateMachine::RootState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    WIFI_LOGI("RootState-msgCode=%{public}d is received.\n", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_UPDATE_COUNTRY_CODE: {
#ifndef OHOS_ARCH_LITE
            ret = EXECUTED;
            std::string wifiCountryCode = msg->GetStringFromMessage();
            if (wifiCountryCode.empty()) {
                break;
            }
            WifiErrorNo result = WifiSupplicantHalInterface::GetInstance().WpaSetCountryCode(wifiCountryCode);
            if (result == WifiErrorNo::WIFI_IDL_OPT_OK) {
                WIFI_LOGI("update wifi country code sucess, wifiCountryCode=%{public}s", wifiCountryCode.c_str());
                break;
            }
            WIFI_LOGE("update wifi country code fail, wifiCountryCode=%{public}s, ret=%{public}d",
                wifiCountryCode.c_str(), result);
#endif
            break;
        }
        default:
            WIFI_LOGI("RootState-msgCode=%{public}d not handled.\n", msg->GetMessageName());
            break;
    }
    return ret;
}

/* --------------------------- state machine Init State ------------------------------ */
StaStateMachine::InitState::InitState(StaStateMachine *staStateMachine)
    : State("InitState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::InitState::~InitState()
{}

void StaStateMachine::InitState::GoInState()
{
    WIFI_LOGI("InitState GoInState function.");
    return;
}

void StaStateMachine::InitState::GoOutState()
{
    WIFI_LOGI("InitState GoOutState function.");
    return;
}

bool StaStateMachine::InitState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    WIFI_LOGI("InitState-msgCode=%{public}d is received.\n", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_ENABLE_WIFI: {
            ret = EXECUTED;
            pStaStateMachine->operationalMode = msg->GetParam1();
            pStaStateMachine->StartWifiProcess();
            break;
        }

        case WIFI_SVR_CMD_STA_OPERATIONAL_MODE:
            break;

        default:
            WIFI_LOGI("InitState-msgCode=%d not handled.\n", msg->GetMessageName());
            break;
    }
    return ret;
}

ErrCode StaStateMachine::ConvertDeviceCfg(const WifiDeviceConfig &config) const
{
    LOGI("Enter ConvertDeviceCfg.\n");
    WifiIdlDeviceConfig idlConfig;
    idlConfig.ssid = config.ssid;
    idlConfig.bssid = config.bssid;
    idlConfig.psk = config.preSharedKey;
    idlConfig.keyMgmt = config.keyMgmt;
    idlConfig.priority = config.priority;
    idlConfig.scanSsid = config.hiddenSSID ? 1 : 0;
    idlConfig.eap = config.wifiEapConfig.eap;
    idlConfig.identity = config.wifiEapConfig.identity;
    idlConfig.password = config.wifiEapConfig.password;
    idlConfig.clientCert = config.wifiEapConfig.clientCert;
    idlConfig.privateKey = config.wifiEapConfig.privateKey;
    idlConfig.phase2Method = static_cast<int>(config.wifiEapConfig.phase2Method);
    idlConfig.wepKeyIdx = config.wepTxKeyIndex;
    if (strcmp(config.keyMgmt.c_str(), "WEP") == 0) {
        /* for wep */
        idlConfig.authAlgorithms = 0x02;
    }

    if (IsWpa3Transition(config.ssid)) {
        if (IsInWpa3BlackMap(config.ssid)) {
            idlConfig.keyMgmt = KEY_MGMT_WPA_PSK;
        } else {
            idlConfig.keyMgmt = KEY_MGMT_SAE;
        }
        idlConfig.isRequirePmf = false;
    }

    if (config.keyMgmt.find("SAE") != std::string::npos) {
        idlConfig.isRequirePmf = true;
    }

    if (idlConfig.keyMgmt.find("SAE") != std::string::npos) {
        idlConfig.allowedProtocols = 0x02; // RSN
        idlConfig.allowedPairwiseCiphers = 0x2c; // CCMP|GCMP|GCMP-256
        idlConfig.allowedGroupCiphers = 0x2c; // CCMP|GCMP|GCMP-256
    }

    for (int i = 0; i < MAX_WEPKEYS_SIZE; i++) {
        idlConfig.wepKeys[i] = config.wepKeys[i];
    }

    if (WifiStaHalInterface::GetInstance().SetDeviceConfig(WPA_DEFAULT_NETWORKID, idlConfig) != WIFI_IDL_OPT_OK) {
        LOGE("ConvertDeviceCfg SetDeviceConfig failed!");
        return WIFI_OPT_FAILED;
    }
    return WIFI_OPT_SUCCESS;
}

void StaStateMachine::StartWifiProcess()
{
    WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::ENABLING), m_instId);
    InvokeOnStaOpenRes(OperateResState::OPEN_WIFI_OPENING);
    int res;
    if (WifiOprMidState::RUNNING == WifiConfigCenter::GetInstance().GetWifiScanOnlyMidState(m_instId)) {
        res = static_cast<int>(WIFI_IDL_OPT_OK);
    } else {
        res = WifiStaHalInterface::GetInstance().StartWifi();
    }
    
    if (res == static_cast<int>(WIFI_IDL_OPT_OK)) {
        WIFI_LOGI("Start wifi successfully!");
        if (WifiStaHalInterface::GetInstance().WpaAutoConnect(false) != WIFI_IDL_OPT_OK) {
            WIFI_LOGI("The automatic Wpa connection is disabled failed.");
        }

        /* callback the InterfaceService that wifi is enabled successfully. */
        WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::ENABLED), m_instId);
        InvokeOnStaOpenRes(OperateResState::OPEN_WIFI_SUCCEED);
        /* Sets the MAC address of WifiSettings. */
        std::string mac;
        if ((WifiStaHalInterface::GetInstance().GetStaDeviceMacAddress(mac)) == WIFI_IDL_OPT_OK) {
            WifiSettings::GetInstance().SetMacAddress(mac, m_instId);
            std::string realMacAddress;
            WifiSettings::GetInstance().GetRealMacAddress(realMacAddress, m_instId);
            if (realMacAddress.empty()) {
                WifiSettings::GetInstance().SetRealMacAddress(mac, m_instId);
            }
        } else {
            WIFI_LOGI("GetStaDeviceMacAddress failed!");
        }
#ifdef SUPPORT_LOCAL_RANDOM_MAC
        std::string macAddress;
        WifiSettings::GetInstance().GenerateRandomMacAddress(macAddress);
        if (MacAddress::IsValidMac(macAddress.c_str())) {
            if (WifiStaHalInterface::GetInstance().SetConnectMacAddr(macAddress) != WIFI_IDL_OPT_OK) {
                LOGE("%{public}s: failed to set sta MAC address:%{private}s", __func__, macAddress.c_str());
            }
            WifiSettings::GetInstance().SetMacAddress(macAddress, m_instId);
        } else {
            LOGE("%{public}s: macAddress is invalid", __func__);
        }
#endif
#ifndef OHOS_ARCH_LITE
        WIFI_LOGI("Register netsupplier");
        WifiNetAgent::GetInstance().OnStaMachineWifiStart();
        m_NetWorkState->StartNetStateObserver();
#endif
        /* Initialize Connection Information. */
        InitWifiLinkedInfo();
        InitLastWifiLinkedInfo();
        WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
#ifndef OHOS_ARCH_LITE
        ChipCapability::GetInstance().InitializeChipCapability();
#endif
        /* The current state of StaStateMachine transfers to SeparatedState after
         * enable supplicant.
         */
        SwitchState(pSeparatedState);
    } else {
        /* Notify the InterfaceService that wifi is failed to enable wifi. */
        LOGE("StartWifi failed, and errcode is %d.", res);
        WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::DISABLED), m_instId);
        WifiSettings::GetInstance().SetUserLastSelectedNetworkId(INVALID_NETWORK_ID, m_instId);
        InvokeOnStaOpenRes(OperateResState::OPEN_WIFI_FAILED);
    }
}

/* --------------------------- state machine WpaStarting State ------------------------------ */
StaStateMachine::WpaStartingState::WpaStartingState(StaStateMachine *staStateMachine)
    : State("WpaStartingState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::WpaStartingState::~WpaStartingState()
{}

void StaStateMachine::WpaStartingState::InitWpsSettings()
{
    WIFI_LOGI("WpaStartingState InitWpsSettings function.");
    return;
}

void StaStateMachine::WpaStartingState::GoInState()
{
    WIFI_LOGI("WpaStartingState GoInState function.");
    return;
}

void StaStateMachine::WpaStartingState::GoOutState()
{
    LOGI("WpaStartingState GoOutState function.");
    return;
}

bool StaStateMachine::WpaStartingState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_SUP_CONNECTION_EVENT: {
            ret = EXECUTED;
            pStaStateMachine->SwitchState(pStaStateMachine->pWpaStartedState);
            break;
        }
        default:
            break;
    }
    return ret;
}

/* --------------------------- state machine WpaStarted State ------------------------------ */
StaStateMachine::WpaStartedState::WpaStartedState(StaStateMachine *staStateMachine)
    : State("WpaStartedState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::WpaStartedState::~WpaStartedState()
{}

void StaStateMachine::WpaStartedState::GoInState()
{
    WIFI_LOGI("WpaStartedState GoInState function.");
    if (pStaStateMachine->operationalMode == STA_CONNECT_MODE) {
        pStaStateMachine->SwitchState(pStaStateMachine->pSeparatedState);
    } else if (pStaStateMachine->operationalMode == STA_DISABLED_MODE) {
        pStaStateMachine->SwitchState(pStaStateMachine->pWpaStoppingState);
    }
    return;
}
void StaStateMachine::WpaStartedState::GoOutState()
{
    WIFI_LOGI("WpaStartedState GoOutState function.");
    return;
}

bool StaStateMachine::WpaStartedState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        LOGI("msg is nullptr");
        return false;
    }

    WIFI_LOGI("WpaStartedState ExecuteStateMsg-msgCode:%{public}d.\n", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_DISABLE_WIFI: {
            ret = EXECUTED;
            pStaStateMachine->StopWifiProcess();
            break;
        }

        default:
            break;
    }
    return ret;
}

void StaStateMachine::StopWifiProcess()
{
    WIFI_LOGI("Enter StaStateMachine::StopWifiProcess.\n");
#ifndef OHOS_ARCH_LITE
    WifiNetAgent::GetInstance().UnregisterNetSupplier();
    m_NetWorkState->StopNetStateObserver();
#endif
    WIFI_LOGI("Stop wifi is in process...\n");
    WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::DISABLING), m_instId);
    InvokeOnStaCloseRes(OperateResState::CLOSE_WIFI_CLOSING);
    StopTimer(static_cast<int>(CMD_SIGNAL_POLL));
    WIFI_LOGI("StopTimer CMD_START_RENEWAL_TIMEOUT StopWifiProcess");
    StopTimer(static_cast<int>(CMD_START_RENEWAL_TIMEOUT));
    std::string ifname = IF_NAME + std::to_string(m_instId);
    if (currentTpType == IPTYPE_IPV4) {
        StopDhcpClient(ifname.c_str(), false);
    } else {
        StopDhcpClient(ifname.c_str(), true);
    }
    isRoam = false;
    WifiSettings::GetInstance().SetMacAddress("", m_instId);

    IpInfo ipInfo;
    WifiSettings::GetInstance().SaveIpInfo(ipInfo, m_instId);
    IpV6Info ipV6Info;
    WifiSettings::GetInstance().SaveIpV6Info(ipV6Info, m_instId);
#ifdef OHOS_ARCH_LITE
    IfConfig::GetInstance().FlushIpAddr(IF_NAME + std::to_string(m_instId), IPTYPE_IPV4);
#endif

    ConnState curConnState = linkedInfo.connState;
    WIFI_LOGI("current connect state is %{public}d\n", curConnState);
    std::string ssid = linkedInfo.ssid;
    /* clear connection information. */
    InitWifiLinkedInfo();
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    if (curConnState == ConnState::CONNECTED) {
        /* Callback result to InterfaceService. */
        linkedInfo.ssid = ssid;
        InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
        linkedInfo.ssid = "";
    }

    if (WifiConfigCenter::GetInstance().GetWifiScanOnlyMidState(m_instId) == WifiOprMidState::RUNNING) {
        WifiErrorNo disconnectRet = WifiStaHalInterface::GetInstance().Disconnect();
        /* Callback result to InterfaceService. */
        WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::DISABLED), m_instId);
        InvokeOnStaCloseRes(OperateResState::CLOSE_WIFI_SUCCEED);
        WIFI_LOGI("Stop WifiProcess successfully! disconnectRet:%{public}d", disconnectRet);
        /* The current state of StaStateMachine transfers to InitState. */
        SwitchState(pInitState);
    } else if (WifiStaHalInterface::GetInstance().StopWifi() == WIFI_IDL_OPT_OK) {
        /* Callback result to InterfaceService. */
        WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::DISABLED), m_instId);
        InvokeOnStaCloseRes(OperateResState::CLOSE_WIFI_SUCCEED);
        WIFI_LOGI("Stop WifiProcess successfully!");
        /* The current state of StaStateMachine transfers to InitState. */
        SwitchState(pInitState);
    } else {
        WIFI_LOGE("StopWifiProcess failed.");
        WifiSettings::GetInstance().SetWifiState(static_cast<int>(WifiState::UNKNOWN), m_instId);
        InvokeOnStaCloseRes(OperateResState::CLOSE_WIFI_FAILED);
    }

    WifiSettings::GetInstance().SetUserLastSelectedNetworkId(INVALID_NETWORK_ID, m_instId);
}

/* --------------------------- state machine WpaStopping State ------------------------------ */
StaStateMachine::WpaStoppingState::WpaStoppingState(StaStateMachine *staStateMachine)
    : State("WpaStoppingState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::WpaStoppingState::~WpaStoppingState()
{}

void StaStateMachine::WpaStoppingState::GoInState()
{
    WIFI_LOGI("WpaStoppingState GoInState function.");
    pStaStateMachine->SwitchState(pStaStateMachine->pInitState);
    return;
}

void StaStateMachine::WpaStoppingState::GoOutState()
{
    WIFI_LOGI("WpaStoppingState GoOutState function.");
    return;
}

bool StaStateMachine::WpaStoppingState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    bool ret = NOT_EXECUTED;
    WIFI_LOGI("WpaStoppingState-msgCode=%{public}d not handled.\n", msg->GetMessageName());
    return ret;
}

/* --------------------------- state machine link State ------------------------------ */
StaStateMachine::LinkState::LinkState(StaStateMachine *staStateMachine)
    : State("LinkState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::LinkState::~LinkState()
{}

void StaStateMachine::LinkState::GoInState()
{
    WIFI_LOGI("LinkState GoInState function.");
    return;
}

void StaStateMachine::LinkState::GoOutState()
{
    WIFI_LOGI("LinkState GoOutState function.");
    return;
}

bool StaStateMachine::LinkState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }
    LOGD("LinkState ExecuteStateMsg function:msgName=[%{public}d].\n", msg->GetMessageName());
    auto iter = pStaStateMachine->staSmHandleFuncMap.find(msg->GetMessageName());
    if (iter != pStaStateMachine->staSmHandleFuncMap.end()) {
        (pStaStateMachine->*(iter->second))(msg);
        return EXECUTED;
    }
    return NOT_EXECUTED;
}

/* -- state machine Connect State Message processing function -- */
int StaStateMachine::InitStaSMHandleMap()
{
    staSmHandleFuncMap[CMD_SIGNAL_POLL] = &StaStateMachine::DealSignalPollResult;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_CONNECT_NETWORK] = &StaStateMachine::DealConnectToUserSelectedNetwork;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_CONNECT_SAVED_NETWORK] = &StaStateMachine::DealConnectToUserSelectedNetwork;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT] = &StaStateMachine::DealDisconnectEvent;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT] = &StaStateMachine::DealConnectionEvent;
    staSmHandleFuncMap[CMD_NETWORK_CONNECT_TIMEOUT] = &StaStateMachine::DealConnectTimeOutCmd;
    staSmHandleFuncMap[WPA_BLOCK_LIST_CLEAR_EVENT] = &StaStateMachine::DealWpaBlockListClearEvent;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_STARTWPS] = &StaStateMachine::DealStartWpsCmd;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_WPS_TIMEOUT_EVNET] = &StaStateMachine::DealWpsConnectTimeOutEvent;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_CANCELWPS] = &StaStateMachine::DealCancelWpsCmd;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_RECONNECT_NETWORK] = &StaStateMachine::DealReConnectCmd;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_REASSOCIATE_NETWORK] = &StaStateMachine::DealReassociateCmd;
    staSmHandleFuncMap[WIFI_SVR_COM_STA_START_ROAM] = &StaStateMachine::DealStartRoamCmd;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_WPA_PASSWD_WRONG_EVENT] = &StaStateMachine::DealWpaLinkFailEvent;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_WPA_FULL_CONNECT_EVENT] = &StaStateMachine::DealWpaLinkFailEvent;
    staSmHandleFuncMap[WIFI_SVR_CMD_STA_WPA_ASSOC_REJECT_EVENT] = &StaStateMachine::DealWpaLinkFailEvent;
    staSmHandleFuncMap[CMD_START_NETCHECK] = &StaStateMachine::DealNetworkCheck;
    staSmHandleFuncMap[CMD_START_GET_DHCP_IP_TIMEOUT] = &StaStateMachine::DealGetDhcpIpTimeout;
    staSmHandleFuncMap[CMD_START_RENEWAL_TIMEOUT] = &StaStateMachine::DealRenewalTimeout;
    staSmHandleFuncMap[WIFI_SCREEN_STATE_CHANGED_NOTIFY_EVENT] = &StaStateMachine::DealScreenStateChangedEvent;
    staSmHandleFuncMap[CMD_AP_ROAMING_TIMEOUT_CHECK] = &StaStateMachine::DealApRoamingStateTimeout;
    return WIFI_OPT_SUCCESS;
}

int setRssi(int rssi)
{
    if (rssi < INVALID_RSSI_VALUE) {
        rssi = INVALID_RSSI_VALUE;
    }

    if (rssi > MAX_RSSI_VALUE) {
        rssi = MAX_RSSI_VALUE;
    }
    return rssi;
}

void StaStateMachine::DealSignalPollResult(InternalMessage *msg)
{
    LOGD("enter SignalPoll.");
    if (msg == nullptr) {
        LOGE("msg is nullptr.");
        return;
    }
    WifiWpaSignalInfo signalInfo;
    WifiErrorNo ret = WifiStaHalInterface::GetInstance().GetConnectSignalInfo(linkedInfo.bssid, signalInfo);
    if (ret != WIFI_IDL_OPT_OK) {
        LOGE("GetConnectSignalInfo return fail: %{public}d.", ret);
        return;
    }
    LOGI("SignalPoll, bssid:%{public}s, freq:%{public}d, rssi:%{public}d, noise:%{public}d, "
        "chload:%{public}d, snr:%{public}d, ulDelay:%{public}d, txLinkSpeed:%{public}d, rxLinkSpeed:%{public}d, "
        "txBytes:%{public}d, rxBytes:%{public}d, txFailed:%{public}d, txPackets:%{public}d, rxPackets:%{public}d.",
        MacAnonymize(linkedInfo.bssid).c_str(), signalInfo.frequency, signalInfo.signal, signalInfo.noise,
        signalInfo.chload, signalInfo.snr, signalInfo.ulDelay, signalInfo.txrate, signalInfo.rxrate, signalInfo.txBytes,
        signalInfo.rxBytes, signalInfo.txFailed, signalInfo.txPackets, signalInfo.rxPackets);

    if (signalInfo.frequency > 0) {
        linkedInfo.frequency = signalInfo.frequency;
    }
    ConvertFreqToChannel();
    if (signalInfo.signal > INVALID_RSSI_VALUE && signalInfo.signal < MAX_RSSI_VALUE) {
        if (signalInfo.signal > 0) {
            linkedInfo.rssi = setRssi((signalInfo.signal - SIGNAL_INFO));
        } else {
            linkedInfo.rssi = setRssi(signalInfo.signal);
        }
        int currentSignalLevel = WifiSettings::GetInstance().GetSignalLevel(linkedInfo.rssi, linkedInfo.band, m_instId);
        LOGI("SignalPoll, networkId:%{public}d, ssid:%{public}s, rssi:%{public}d, band:%{public}d, "
            "connState:%{public}d, detailedState:%{public}d, currentSignal:%{public}d, lastSignal:%{public}d.\n",
            linkedInfo.networkId, SsidAnonymize(linkedInfo.ssid).c_str(), linkedInfo.rssi, linkedInfo.band,
            linkedInfo.connState, linkedInfo.detailedState, currentSignalLevel, lastSignalLevel);
        if (currentSignalLevel != lastSignalLevel) {
            WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
            InvokeOnStaRssiLevelChanged(linkedInfo.rssi);
#ifndef OHOS_ARCH_LITE
            if (NetSupplierInfo != nullptr) {
                TimeStats timeStats("Call UpdateNetSupplierInfo");
                NetSupplierInfo->isAvailable_ = true;
                NetSupplierInfo->isRoaming_ = isRoam;
                NetSupplierInfo->strength_ = linkedInfo.rssi;
                NetSupplierInfo->frequency_ = linkedInfo.frequency;
                WifiNetAgent::GetInstance().UpdateNetSupplierInfo(NetSupplierInfo);
            }
#endif
            lastSignalLevel = currentSignalLevel;
        }
    } else {
        linkedInfo.rssi = INVALID_RSSI_VALUE;
    }
    if (signalInfo.txrate > 0) {
        linkedInfo.txLinkSpeed = signalInfo.txrate / TRANSFORMATION_TO_MBPS;
        linkedInfo.linkSpeed = signalInfo.txrate / TRANSFORMATION_TO_MBPS;
    }

    if (signalInfo.rxrate > 0) {
        linkedInfo.rxLinkSpeed = signalInfo.rxrate / TRANSFORMATION_TO_MBPS;
    }

    linkedInfo.snr = signalInfo.snr;
    linkedInfo.chload = signalInfo.chload;
    if (linkedInfo.wifiStandard == WIFI_MODE_UNDEFINED) {
        WifiSettings::GetInstance().SetWifiLinkedStandardAndMaxSpeed(linkedInfo);
    }
    LOGD("SignalPoll GetWifiStandard:%{public}d, bssid:%{public}s rxmax:%{public}d txmax:%{public}d.",
         linkedInfo.wifiStandard, MacAnonymize(linkedInfo.bssid).c_str(), linkedInfo.maxSupportedRxLinkSpeed,
         linkedInfo.maxSupportedTxLinkSpeed);
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    DealSignalPacketChanged(signalInfo.txPackets, signalInfo.rxPackets);

    if (enableSignalPoll) {
        WIFI_LOGD("SignalPoll, StartTimer for SIGNAL_POLL.\n");
        StartTimer(static_cast<int>(CMD_SIGNAL_POLL), STA_SIGNAL_POLL_DELAY);
    }
}

void StaStateMachine::DealSignalPacketChanged(int txPackets, int rxPackets)
{
    int send = txPackets - linkedInfo.lastTxPackets;
    int received = rxPackets - linkedInfo.lastRxPackets;
    int direction = 0;
    if (send > STREAM_TXPACKET_THRESHOLD) {
        direction |= static_cast<int>(StreamDirection::STREAM_DIRECTION_UP);
    }
    if (received > STREAM_RXPACKET_THRESHOLD) {
        direction |= static_cast<int>(StreamDirection::STREAM_DIRECTION_DOWN);
    }
    if (direction != linkedInfo.lastPacketDirection) {
        WriteWifiSignalHiSysEvent(direction, txPackets, rxPackets);
        InvokeOnStaStreamChanged(static_cast<StreamDirection>(direction));
    }
    linkedInfo.lastPacketDirection = direction;
    linkedInfo.lastRxPackets = rxPackets;
    linkedInfo.lastTxPackets = txPackets;
}

void StaStateMachine::ConvertFreqToChannel()
{
    WifiDeviceConfig config;
    if (WifiSettings::GetInstance().GetDeviceConfig(linkedInfo.networkId, config) != 0) {
        LOGE("GetDeviceConfig failed!");
        return;
    }
    int lastBand = linkedInfo.band;
    config.frequency = linkedInfo.frequency;
    if (linkedInfo.frequency >= FREQ_2G_MIN && linkedInfo.frequency <= FREQ_2G_MAX) {
        config.band = linkedInfo.band = static_cast<int>(BandType::BAND_2GHZ);
        config.channel = (linkedInfo.frequency - FREQ_2G_MIN) / CENTER_FREQ_DIFF + CHANNEL_2G_MIN;
    } else if (linkedInfo.frequency == CHANNEL_14_FREQ) {
        config.channel = CHANNEL_14;
    } else if (linkedInfo.frequency >= FREQ_5G_MIN && linkedInfo.frequency <= FREQ_5G_MAX) {
        config.band = linkedInfo.band = static_cast<int>(BandType::BAND_5GHZ);
        config.channel = (linkedInfo.frequency - FREQ_5G_MIN) / CENTER_FREQ_DIFF + CHANNEL_5G_MIN;
    }
    if (lastBand != linkedInfo.band) {
        WriteWifiBandHiSysEvent(linkedInfo.band);
    }
    WifiSettings::GetInstance().AddDeviceConfig(config);
    return;
}

void StaStateMachine::OnConnectFailed(int networkId)
{
    WIFI_LOGE("Connect to network failed: %{public}d.\n", networkId);
    SaveLinkstate(ConnState::DISCONNECTED, DetailedState::FAILED);
    InvokeOnStaConnChanged(OperateResState::CONNECT_ENABLE_NETWORK_FAILED, linkedInfo);
    InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
    WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
}

void StaStateMachine::DealConnectToUserSelectedNetwork(InternalMessage *msg)
{
    LOGD("enter DealConnectToUserSelectedNetwork.\n");
    if (msg == nullptr) {
        LOGE("msg is null.\n");
        return;
    }

    int networkId = msg->GetParam1();
    int connTriggerMode = msg->GetParam2();
    auto bssid = msg->GetStringFromMessage();
    if (connTriggerMode != NETWORK_SELECTED_BY_RETRY) {
        linkedInfo.retryedConnCount = 0;
    }
    WriteWifiConnectionInfoHiSysEvent(networkId);
    if (networkId == linkedInfo.networkId) {
        if (linkedInfo.connState == ConnState::CONNECTED) {
            InvokeOnStaConnChanged(OperateResState::CONNECT_AP_CONNECTED, linkedInfo);
            WIFI_LOGI("This network is in use and does not need to be reconnected.\n");
            return;
        }
        if (linkedInfo.connState == ConnState::CONNECTING &&
            linkedInfo.detailedState == DetailedState::OBTAINING_IPADDR) {
            WIFI_LOGI("This network is connecting and does not need to be reconnected.\n");
            return;
        }
    }

    /* Save connection information. */
    SaveDiscReason(DisconnectedReason::DISC_REASON_DEFAULT);
    SaveLinkstate(ConnState::CONNECTING, DetailedState::CONNECTING);
    networkStatusHistoryInserted = false;
    /* Callback result to InterfaceService. */
    InvokeOnStaConnChanged(OperateResState::CONNECT_CONNECTING, linkedInfo);

    if (StartConnectToNetwork(networkId, bssid) != WIFI_OPT_SUCCESS) {
        OnConnectFailed(networkId);
        return;
    }

    /* Sets network status. */
    WifiSettings::GetInstance().EnableNetwork(networkId, connTriggerMode == NETWORK_SELECTED_BY_USER, m_instId);
    WifiSettings::GetInstance().SetDeviceState(networkId, (int)WifiDeviceConfigStatus::ENABLED, false);
}

void StaStateMachine::DealConnectTimeOutCmd(InternalMessage *msg)
{
    LOGW("enter DealConnectTimeOutCmd.\n");
    if (msg == nullptr) {
        WIFI_LOGE("msg is nul\n");
    }

    if (linkedInfo.connState == ConnState::CONNECTED) {
        WIFI_LOGE("Currently connected and do not process timeout.\n");
        return;
    }
    linkedInfo.retryedConnCount++;
    DealSetStaConnectFailedCount(1, false);
    std::string ssid = linkedInfo.ssid;
    WifiSettings::GetInstance().SetConnectTimeoutBssid(linkedInfo.bssid, m_instId);
    InitWifiLinkedInfo();
    SaveDiscReason(DisconnectedReason::DISC_REASON_DEFAULT);
    SaveLinkstate(ConnState::DISCONNECTED, DetailedState::CONNECTION_TIMEOUT);
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    linkedInfo.ssid = ssid;
    InvokeOnStaConnChanged(OperateResState::CONNECT_CONNECTING_TIMEOUT, linkedInfo);
    InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
    linkedInfo.ssid = "";
    WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
}

void StaStateMachine::DealConnectionEvent(InternalMessage *msg)
{
    if (msg == nullptr) {
        WIFI_LOGE("DealConnectionEvent, msg is nullptr.\n");
        return;
    }

    WIFI_LOGI("enter DealConnectionEvent");
    WifiSettings::GetInstance().SetDeviceAfterConnect(targetNetworkId);
    WifiSettings::GetInstance().SetDeviceState(targetNetworkId, (int)WifiDeviceConfigStatus::ENABLED, false);
    WifiSettings::GetInstance().SyncDeviceConfig();
#ifndef OHOS_ARCH_LITE
    SaveWifiConfigForUpdate(targetNetworkId);
#endif
    /* Stop clearing the Wpa_blocklist. */
    StopTimer(static_cast<int>(WPA_BLOCK_LIST_CLEAR_EVENT));
    ConnectToNetworkProcess(msg);
    StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
    StartTimer(static_cast<int>(CMD_SIGNAL_POLL), 0);

    if (wpsState != SetupMethod::INVALID) {
        wpsState = SetupMethod::INVALID;
    }
#ifndef OHOS_ARCH_LITE
    if (NetSupplierInfo != nullptr) {
        NetSupplierInfo->isAvailable_ = true;
        NetSupplierInfo->isRoaming_ = isRoam;
        WIFI_LOGI("On connect update net supplier info\n");
        WifiNetAgent::GetInstance().OnStaMachineUpdateNetSupplierInfo(NetSupplierInfo);
    }
#endif
    /* Callback result to InterfaceService. */
    InvokeOnStaConnChanged(OperateResState::CONNECT_OBTAINING_IP, linkedInfo);

    if (WifiSupplicantHalInterface::GetInstance().WpaSetPowerMode(true) != WIFI_IDL_OPT_OK) {
        LOGE("DealConnectionEvent WpaSetPowerMode() failed!");
    }
    /* The current state of StaStateMachine transfers to GetIpState. */
    SwitchState(pGetIpState);
    WifiSettings::GetInstance().SetUserLastSelectedNetworkId(INVALID_NETWORK_ID, m_instId);
}

void StaStateMachine::DealDisconnectEvent(InternalMessage *msg)
{
    LOGI("Enter DealDisconnectEvent.\n");
    if (msg == nullptr) {
        WIFI_LOGE("msg is null\n");
    }
    if (wpsState != SetupMethod::INVALID) {
        WIFI_LOGE("wpsState is INVALID\n");
        return;
    }
#ifndef OHOS_ARCH_LITE
    if (NetSupplierInfo != nullptr) {
        NetSupplierInfo->isAvailable_ = false;
        WIFI_LOGI("On disconnect update net supplier info\n");
        WifiNetAgent::GetInstance().OnStaMachineUpdateNetSupplierInfo(NetSupplierInfo);
    }
#endif
    StopTimer(static_cast<int>(CMD_SIGNAL_POLL));
    StopTimer(static_cast<int>(CMD_START_NETCHECK));
    WIFI_LOGI("StopTimer CMD_START_RENEWAL_TIMEOUT DealDisconnectEvent");
    StopTimer(static_cast<int>(CMD_START_RENEWAL_TIMEOUT));
    pNetcheck->StopNetCheckThread();
    std::string ifname = IF_NAME + std::to_string(m_instId);
    if (currentTpType == IPTYPE_IPV4) {
        StopDhcpClient(ifname.c_str(), false);
    } else {
        StopDhcpClient(ifname.c_str(), true);
    }
    getIpSucNum = 0;
    getIpFailNum = 0;
    isRoam = false;

    IpInfo ipInfo;
    WifiSettings::GetInstance().SaveIpInfo(ipInfo, m_instId);
    IpV6Info ipV6Info;
    WifiSettings::GetInstance().SaveIpV6Info(ipV6Info, m_instId);
#ifdef OHOS_ARCH_LITE
    IfConfig::GetInstance().FlushIpAddr(IF_NAME + std::to_string(m_instId), IPTYPE_IPV4);
#endif
    /* Initialize connection information. */
    std::string ssid = linkedInfo.ssid;
    InitWifiLinkedInfo();
    if (lastLinkedInfo.detailedState == DetailedState::CONNECTING) {
        linkedInfo.networkId = lastLinkedInfo.networkId;
        linkedInfo.ssid = lastLinkedInfo.ssid;
        linkedInfo.connState = ConnState::CONNECTING;
        linkedInfo.detailedState = DetailedState::CONNECTING;
        WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    } else {
        WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    }
    linkedInfo.ssid = ssid;
    /* Callback result to InterfaceService. */
    InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
    linkedInfo.ssid = "";
    WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
    SwitchState(pSeparatedState);
    return;
}

void StaStateMachine::DealWpaLinkFailEvent(InternalMessage *msg)
{
    LOGW("enter DealWpaLinkFailEvent.\n");
    if (msg == nullptr) {
        LOGE("msg is null.\n");
        return;
    }

    DealSetStaConnectFailedCount(1, false);
    if (msg->GetMessageName() != WIFI_SVR_CMD_STA_WPA_PASSWD_WRONG_EVENT &&
        DealReconnectSavedNetwork()) {
        return;
    }
    
    StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
    std::string ssid = linkedInfo.ssid;
    InitWifiLinkedInfo();
    linkedInfo.ssid = ssid;
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
    if (msg->GetMessageName() == WIFI_SVR_CMD_STA_WPA_PASSWD_WRONG_EVENT) {
        SaveDiscReason(DisconnectedReason::DISC_REASON_WRONG_PWD);
        SaveLinkstate(ConnState::DISCONNECTED, DetailedState::PASSWORD_ERROR);
        InvokeOnStaConnChanged(OperateResState::CONNECT_PASSWORD_WRONG, linkedInfo);
        InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
    } else if (msg->GetMessageName() == WIFI_SVR_CMD_STA_WPA_FULL_CONNECT_EVENT) {
        WifiStaHalInterface::GetInstance().DisableNetwork(WPA_DEFAULT_NETWORKID);
        SaveDiscReason(DisconnectedReason::DISC_REASON_CONNECTION_FULL);
        SaveLinkstate(ConnState::DISCONNECTED, DetailedState::CONNECTION_FULL);
        InvokeOnStaConnChanged(OperateResState::CONNECT_CONNECTION_FULL, linkedInfo);
        InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
        WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
    } else if (msg->GetMessageName() == WIFI_SVR_CMD_STA_WPA_ASSOC_REJECT_EVENT) {
        WifiStaHalInterface::GetInstance().DisableNetwork(WPA_DEFAULT_NETWORKID);
        SaveDiscReason(DisconnectedReason::DISC_REASON_DEFAULT);
        SaveLinkstate(ConnState::DISCONNECTED, DetailedState::CONNECTION_REJECT);
        InvokeOnStaConnChanged(OperateResState::CONNECT_CONNECTION_REJECT, linkedInfo);
        InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED, linkedInfo);
        WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
    }
    linkedInfo.ssid = "";
}

bool StaStateMachine::DealReconnectSavedNetwork()
{
    linkedInfo.retryedConnCount++;
    if (linkedInfo.retryedConnCount < MAX_RETRY_COUNT) {
        SendMessage(WIFI_SVR_CMD_STA_CONNECT_SAVED_NETWORK,
            targetNetworkId, NETWORK_SELECTED_BY_RETRY);
        WIFI_LOGW("DealConnectTimeOutCmd retry connect to saved network.\n");
        return true;
    }
    return false;
}

void StaStateMachine::DealSetStaConnectFailedCount(int count, bool set)
{
    WifiDeviceConfig config;
    int ret = WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, config);
    if (ret != 0) {
        WIFI_LOGW("DealConnectTimeOutCmd get device[%{public}d] config failed.\n", targetNetworkId);
        return;
    }
    if (set) {
        WifiSettings::GetInstance().SetDeviceConnFailedCount(config.bssid, DEVICE_CONFIG_INDEX_BSSID,
            count);
    } else {
        WifiSettings::GetInstance().IncreaseDeviceConnFailedCount(config.bssid, DEVICE_CONFIG_INDEX_BSSID,
            count);
    }
}

void StaStateMachine::DealReConnectCmd(InternalMessage *msg)
{
    LOGI("enter DealReConnectCmd.\n");
    if (msg == nullptr) {
        WIFI_LOGE("msg is null\n");
    }

    if (linkedInfo.connState == ConnState::CONNECTED) {
        WIFI_LOGE("Network is already connected, ignore the re-connect command!\n");
        return;
    }

    if (WifiStaHalInterface::GetInstance().Reconnect() == WIFI_IDL_OPT_OK) {
        DealSetStaConnectFailedCount(0, true);
        WIFI_LOGI("StaStateMachine ReConnect successfully!");
        /* Callback result to InterfaceService */
        InvokeOnStaConnChanged(OperateResState::CONNECT_CONNECTING, linkedInfo);
        StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
        StartTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT), STA_NETWORK_CONNECTTING_DELAY);
    } else {
        linkedInfo.retryedConnCount++;
        DealSetStaConnectFailedCount(1, false);
        WIFI_LOGE("ReConnect failed!");
    }
}

void StaStateMachine::DealReassociateCmd(InternalMessage *msg)
{
    LOGI("enter DealReassociateCmd.\n");
    if (msg == nullptr) {
        WIFI_LOGE("msg is null\n");
    }

    if (WifiStaHalInterface::GetInstance().Reassociate() == WIFI_IDL_OPT_OK) {
        /* Callback result to InterfaceService */
        InvokeOnStaConnChanged(OperateResState::CONNECT_ASSOCIATING, linkedInfo);
        WIFI_LOGD("StaStateMachine ReAssociate successfully!");
        StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
        StartTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT), STA_NETWORK_CONNECTTING_DELAY);
    } else {
        WIFI_LOGE("ReAssociate failed!");
    }
}

void StaStateMachine::DealStartWpsCmd(InternalMessage *msg)
{
    WIFI_LOGI("enter DealStartWpsCmd\n");
    if (msg == nullptr) {
        return;
    }

    if (WifiStaHalInterface::GetInstance().ClearDeviceConfig() != WIFI_IDL_OPT_OK) {
        LOGE("ClearDeviceConfig() failed!");
        return;
    }

    StartWpsMode(msg);
    if ((wpsState == SetupMethod::DISPLAY) || (wpsState == SetupMethod::KEYPAD)) {
        WIFI_LOGW("Clear WPA block list every ten second!");
        SendMessage(WPA_BLOCK_LIST_CLEAR_EVENT);
    }
}

void StaStateMachine::StartWpsMode(InternalMessage *msg)
{
    if (msg == nullptr) {
        return;
    }
    /*
     * Make judgement to wps configuration information: the function will exit if
     * the result is fail, then else continue to chose the Wps starting mode. The
     * current state of StaStateMachine transfers to WpsState after Wps code start
     * successfully.
     */
    WifiIdlWpsConfig wpsParam;
    WpsConfig wpsConfig;
    wpsConfig.setup = static_cast<SetupMethod>(msg->GetParam1());
    wpsConfig.pin = msg->GetStringFromMessage();
    wpsConfig.bssid = msg->GetStringFromMessage();
    if (wpsConfig.bssid.length() == 0 || wpsConfig.bssid == PBC_ANY_BSSID) {
        wpsParam.anyFlag = 1;
        wpsParam.bssid = PBC_ANY_BSSID;
    } else {
        wpsParam.anyFlag = 0;
        wpsParam.bssid = wpsConfig.bssid;
    }
    wpsParam.multiAp = MULTI_AP;
    WIFI_LOGI("wpsConfig  setup = %{public}d", wpsConfig.setup);
    WIFI_LOGI("wpsParam.AnyFlag = %{public}d, wpsParam.mulitAp = %{public}d, wpsParam.bssid = %{public}s",
        wpsParam.anyFlag,
        wpsParam.multiAp,
        MacAnonymize(wpsParam.bssid).c_str());

    if (wpsConfig.setup == SetupMethod::PBC) {
        if (WifiStaHalInterface::GetInstance().StartWpsPbcMode(wpsParam) == WIFI_IDL_OPT_OK) {
            wpsState = wpsConfig.setup;
            WIFI_LOGD("StartWpsPbcMode() succeed!");
            /* Callback result to InterfaceService. */
            InvokeOnWpsChanged(WpsStartState::START_PBC_SUCCEED, pinCode);
            SwitchState(pWpsState);
        } else {
            LOGE("StartWpsPbcMode() failed!");
            InvokeOnWpsChanged(WpsStartState::START_PBC_FAILED, pinCode);
        }
    } else if (wpsConfig.setup == SetupMethod::DISPLAY) {
        if (WifiStaHalInterface::GetInstance().StartWpsPinMode(wpsParam, pinCode) == WIFI_IDL_OPT_OK) {
            wpsState = wpsConfig.setup;
            /* Callback result to InterfaceService. */
            InvokeOnWpsChanged(WpsStartState::START_PIN_SUCCEED, pinCode);
            WIFI_LOGD("StartWpsPinMode() succeed!  pincode: %d", pinCode);
            SwitchState(pWpsState);
        } else {
            WIFI_LOGE("StartWpsPinMode() failed!");
            InvokeOnWpsChanged(WpsStartState::START_PIN_FAILED, pinCode);
        }
    } else if (wpsConfig.setup == SetupMethod::KEYPAD) {
        if (WifiStaHalInterface::GetInstance().StartWpsPinMode(wpsParam, pinCode) == WIFI_IDL_OPT_OK) {
            wpsState = wpsConfig.setup;
            /* Callback result to InterfaceService. */
            InvokeOnWpsChanged(WpsStartState::START_AP_PIN_SUCCEED, pinCode);
            SwitchState(pWpsState);
        } else {
            LOGE("StartWpsPinMode() failed.");
            InvokeOnWpsChanged(WpsStartState::START_AP_PIN_FAILED, pinCode);
        }
    } else {
        LOGE("Start Wps failed!");
        InvokeOnWpsChanged(WpsStartState::START_WPS_FAILED, pinCode);
    }
}

void StaStateMachine::DealWpaBlockListClearEvent(InternalMessage *msg)
{
    if (msg != nullptr) {
        WIFI_LOGE("enter DealWpaBlockListClearEvent\n");
    }
    if (WifiStaHalInterface::GetInstance().WpaBlocklistClear() != WIFI_IDL_OPT_OK) {
        WIFI_LOGE("Clearing the Wpa_blocklist failed\n");
    }
    StartTimer(static_cast<int>(WPA_BLOCK_LIST_CLEAR_EVENT), BLOCK_LIST_CLEAR_TIMER);
    WIFI_LOGI("Clearing the Wpa_blocklist.\n");
}

void StaStateMachine::DealWpsConnectTimeOutEvent(InternalMessage *msg)
{
    WIFI_LOGW("enter DealWpsConnectTimeOutEvent\n");
    if (msg == nullptr) {
        WIFI_LOGE("msg is nullptr!\n");
        return;
    }
    int failreason = msg->GetParam1();
    if (failreason > 0) {
        DisConnectProcess();
        OnWifiWpa3SelfCure(failreason, targetNetworkId);
    }
    DealCancelWpsCmd(msg);

    /* Callback InterfaceService that WPS time out. */
    InvokeOnWpsChanged(WpsStartState::WPS_TIME_OUT, pinCode);
    SwitchState(pSeparatedState);
}

void StaStateMachine::DealCancelWpsCmd(InternalMessage *msg)
{
    if (msg == nullptr) {
        WIFI_LOGE("msg is null\n");
    }

    StopTimer(static_cast<int>(WPA_BLOCK_LIST_CLEAR_EVENT));
    isWpsConnect = IsWpsConnected::WPS_INVALID;
    if (WifiStaHalInterface::GetInstance().StopWps() == WIFI_IDL_OPT_OK) {
        WIFI_LOGI("CancelWps succeed!");
        /* Callback result to InterfaceService that stop Wps connection successfully. */
        if (wpsState == SetupMethod::PBC) {
            InvokeOnWpsChanged(WpsStartState::STOP_PBC_SUCCEED, pinCode);
        } else if (wpsState == SetupMethod::DISPLAY) {
            InvokeOnWpsChanged(WpsStartState::STOP_PIN_SUCCEED, pinCode);
        } else if (wpsState == SetupMethod::KEYPAD) {
            InvokeOnWpsChanged(WpsStartState::STOP_AP_PIN_SUCCEED, pinCode);
        }
        if (wpsState != SetupMethod::INVALID) {
            wpsState = SetupMethod::INVALID;

            if (WifiStaHalInterface::GetInstance().EnableNetwork(WPA_DEFAULT_NETWORKID) == WIFI_IDL_OPT_OK) {
                WIFI_LOGI("EnableNetwork success!");
            } else {
                WIFI_LOGE("EnableNetwork failed");
            }
        }
    } else {
        WIFI_LOGE("CancelWps failed!");
        if (wpsState == SetupMethod::PBC) {
            InvokeOnWpsChanged(WpsStartState::STOP_PBC_FAILED, pinCode);
        } else if (wpsState == SetupMethod::DISPLAY) {
            InvokeOnWpsChanged(WpsStartState::STOP_PIN_FAILED, pinCode);
        } else if (wpsState == SetupMethod::KEYPAD) {
            InvokeOnWpsChanged(WpsStartState::STOP_AP_PIN_FAILED, pinCode);
        }
    }
    SwitchState(pSeparatedState);
}

void StaStateMachine::DealStartRoamCmd(InternalMessage *msg)
{
    if (msg == nullptr) {
        return;
    }

    WIFI_LOGI("enter DealStartRoamCmd\n");
    std::string bssid = msg->GetStringFromMessage();
    /* GetDeviceConfig from Configuration center. */
    WifiDeviceConfig network;
    WifiSettings::GetInstance().GetDeviceConfig(linkedInfo.networkId, network);

    /* Setting the network. */
    WifiIdlDeviceConfig idlConfig;
    idlConfig.networkId = linkedInfo.networkId;
    idlConfig.ssid = linkedInfo.ssid;
    idlConfig.bssid = bssid;
    idlConfig.psk = network.preSharedKey;
    idlConfig.keyMgmt = network.keyMgmt;
    idlConfig.priority = network.priority;
    idlConfig.scanSsid = network.hiddenSSID ? 1 : 0;
    idlConfig.eap = network.wifiEapConfig.eap;
    idlConfig.identity = network.wifiEapConfig.identity;
    idlConfig.password = network.wifiEapConfig.password;
    idlConfig.clientCert = network.wifiEapConfig.clientCert;
    idlConfig.privateKey = network.wifiEapConfig.privateKey;
    idlConfig.phase2Method = static_cast<int>(network.wifiEapConfig.phase2Method);

    if (WifiStaHalInterface::GetInstance().SetDeviceConfig(linkedInfo.networkId, idlConfig) != WIFI_IDL_OPT_OK) {
        WIFI_LOGE("DealStartRoamCmd SetDeviceConfig() failed!");
        return;
    }
    WIFI_LOGD("DealStartRoamCmd  SetDeviceConfig() succeed!");

    /* Save to Configuration center. */
    network.bssid = bssid;
    WifiSettings::GetInstance().AddDeviceConfig(network);
    WifiSettings::GetInstance().SyncDeviceConfig();

    /* Save linkedinfo */
    linkedInfo.bssid = bssid;
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);

    if (WifiStaHalInterface::GetInstance().Reassociate() != WIFI_IDL_OPT_OK) {
        WIFI_LOGE("START_ROAM-ReAssociate() failed!");
    }
    WIFI_LOGI("START_ROAM-ReAssociate() succeeded!");
    /* Start roaming */
    SwitchState(pApRoamingState);
}

ErrCode StaStateMachine::StartConnectToNetwork(int networkId, const std::string & bssid)
{
    targetNetworkId = networkId;
    SetRandomMac(targetNetworkId);
    WifiDeviceConfig deviceConfig;
    if (WifiSettings::GetInstance().GetDeviceConfig(networkId, deviceConfig) != 0) {
        LOGE("StartConnectToNetwork get GetDeviceConfig failed!");
        return WIFI_OPT_FAILED;
    }
    WifiStaHalInterface::GetInstance().ClearDeviceConfig();
    int wpaNetworkId = INVALID_NETWORK_ID;
    if (WifiStaHalInterface::GetInstance().GetNextNetworkId(wpaNetworkId) != WIFI_IDL_OPT_OK) {
        LOGE("StartConnectToNetwork GetNextNetworkId failed!");
        return WIFI_OPT_FAILED;
    }
    ConvertDeviceCfg(deviceConfig);
    if (bssid.empty()) {
        WifiStaHalInterface::GetInstance().SetBssid(WPA_DEFAULT_NETWORKID, deviceConfig.userSelectBssid);
    } else {
        WifiStaHalInterface::GetInstance().SetBssid(WPA_DEFAULT_NETWORKID, bssid);
    }
    if (WifiStaHalInterface::GetInstance().EnableNetwork(WPA_DEFAULT_NETWORKID) != WIFI_IDL_OPT_OK) {
        LOGE("EnableNetwork() failed!");
        return WIFI_OPT_FAILED;
    }

    if (WifiStaHalInterface::GetInstance().Connect(WPA_DEFAULT_NETWORKID) != WIFI_IDL_OPT_OK) {
        LOGE("Connect failed!");
        InvokeOnStaConnChanged(OperateResState::CONNECT_SELECT_NETWORK_FAILED, linkedInfo);
        return WIFI_OPT_FAILED;
    }
    StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
    StartTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT), STA_NETWORK_CONNECTTING_DELAY);
    WriteWifiOperateStateHiSysEvent(static_cast<int>(WifiOperateType::STA_CONNECT),
        static_cast<int>(WifiOperateState::STA_CONNECTING));
    return WIFI_OPT_SUCCESS;
}

void StaStateMachine::MacAddressGenerate(WifiStoreRandomMac &randomMacInfo)
{
    LOGD("enter MacAddressGenerate\n");
    constexpr int arraySize = 4;
    constexpr int macBitSize = 12;
    constexpr int firstBit = 1;
    constexpr int lastBit = 11;
    constexpr int two = 2;
    constexpr int hexBase = 16;
    constexpr int octBase = 8;
    int ret = 0;
    char strMacTmp[arraySize] = {0};
    std::mt19937_64 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count()
        + std::hash<std::string>{}(randomMacInfo.peerBssid) + std::hash<std::string>{}(randomMacInfo.preSharedKey));
    for (int i = 0; i < macBitSize; i++) {
        if (i != firstBit) {
            std::uniform_int_distribution<> distribution(0, hexBase - 1);
            ret = sprintf_s(strMacTmp, arraySize, "%x", distribution(gen));
        } else {
            std::uniform_int_distribution<> distribution(0, octBase - 1);
            ret = sprintf_s(strMacTmp, arraySize, "%x", two * distribution(gen));
        }
        if (ret == -1) {
            LOGE("StaStateMachine::MacAddressGenerate failed, sprintf_s return -1!\n");
        }
        randomMacInfo.randomMac += strMacTmp;
        if ((i % two) != 0 && (i != lastBit)) {
            randomMacInfo.randomMac.append(":");
        }
    }
}

int StaStateMachine::GetWpa3FailCount(int failreason, std::string ssid) const
{
    if (failreason < 0 || failreason >= WPA3_FAIL_REASON_MAX) {
        WIFI_LOGE("GetWpa3FailCount, Err failreason");
        return 0;
    }
    auto iter = wpa3ConnectFailCountMapArray[failreason].find(ssid);
    if (iter == wpa3ConnectFailCountMapArray[failreason].end()) {
        WIFI_LOGI("GetWpa3FailCount, no failreason count");
        return 0;
    }
    WIFI_LOGI("GetWpa3FailCount, failreason=%{public}d, count=%{public}d",
        failreason, iter->second);
    return iter->second;
}

void StaStateMachine::AddWpa3FailCount(int failreason, std::string ssid)
{
    if (failreason < 0 || failreason >= WPA3_FAIL_REASON_MAX) {
        WIFI_LOGE("AddWpa3FailCount, Err failreason");
        return;
    }
    auto iter = wpa3ConnectFailCountMapArray[failreason].find(ssid);
    if (iter == wpa3ConnectFailCountMapArray[failreason].end()) {
        WIFI_LOGI("AddWpa3FailCount, new failreason count");
        wpa3ConnectFailCountMapArray[failreason].insert(std::make_pair(ssid, 1));
    } else {
        WIFI_LOGI("AddWpa3FailCount, existed failreason count");
        iter->second = iter->second + 1;
    }
}

void StaStateMachine::AddWpa3BlackMap(std::string ssid)
{
    if (wpa3BlackMap.size() == WPA3_BLACKMAP_MAX_NUM) {
        auto iter = wpa3BlackMap.begin();
        auto oldestIter = wpa3BlackMap.begin();
        for (; iter != wpa3BlackMap.end(); iter++) {
            if (iter->second < oldestIter->second) {
                oldestIter = iter;
            }
        }
        WIFI_LOGI("AddWpa3BlackMap, map full, delete oldest");
        wpa3BlackMap.erase(oldestIter);
    }
    WIFI_LOGI("AddWpa3BlackMap success");
    wpa3BlackMap.insert(std::make_pair(ssid, time(0)));
}

bool StaStateMachine::IsInWpa3BlackMap(std::string ssid) const
{
    auto iter = wpa3BlackMap.find(ssid);
    if (iter != wpa3BlackMap.end()) {
        WIFI_LOGI("check is InWpa3BlackMap");
        return true;
    }
    return false;
}

void StaStateMachine::OnWifiWpa3SelfCure(int failreason, int networkId)
{
    WifiDeviceConfig config;
    int failCountReason = 0;

    WIFI_LOGI("OnWifiWpa3SelfCure Enter.");
    auto iter = wpa3FailreasonMap.find(failreason);
    if (iter == wpa3FailreasonMap.end()) {
        WIFI_LOGE("OnWifiWpa3SelfCure, Invalid fail reason");
        return;
    }
    failCountReason = iter->second;
    if (WifiSettings::GetInstance().GetDeviceConfig(networkId, config) == -1) {
        WIFI_LOGE("OnWifiWpa3SelfCure, get deviceconfig failed");
        return;
    }
    if (!IsWpa3Transition(config.ssid)) {
        WIFI_LOGE("OnWifiWpa3SelfCure, is not wpa3 transition");
        return;
    }
    if (linkedInfo.rssi <= WPA3_BLACKMAP_RSSI_THRESHOLD) {
        WIFI_LOGE("OnWifiWpa3SelfCure, rssi less then -70");
        return;
    }
    if (config.lastConnectTime > 0) {
        WIFI_LOGE("OnWifiWpa3SelfCure, has ever connected");
        return;
    }
    AddWpa3FailCount(failCountReason, config.ssid);
    if (GetWpa3FailCount(failCountReason, config.ssid) < WPA3_CONNECT_FAIL_COUNT_THRESHOLD) {
        WIFI_LOGI("OnWifiWpa3SelfCure, fail count not enough.");
        return;
    }
    AddWpa3BlackMap(config.ssid);
    StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
    SendMessage(WIFI_SVR_CMD_STA_CONNECT_NETWORK, networkId, NETWORK_SELECTED_BY_USER);
}

bool StaStateMachine::IsWpa3Transition(std::string ssid) const
{
    std::vector<WifiScanInfo> scanInfoList;
    WifiSettings::GetInstance().GetScanInfoList(scanInfoList);
    for (auto scanInfo : scanInfoList) {
        if ((ssid == scanInfo.ssid) &&
            (scanInfo.capabilities.find("PSK+SAE") != std::string::npos)) {
            LOGI("IsWpa3Transition, check is transition");
            return true;
        }
    }
    return false;
}

bool StaStateMachine::ComparedKeymgmt(const std::string scanInfoKeymgmt, const std::string deviceKeymgmt)
{
    if (deviceKeymgmt == "WPA-PSK") {
        return scanInfoKeymgmt.find("PSK") != std::string::npos;
    } else if (deviceKeymgmt == "WPA-EAP") {
        return scanInfoKeymgmt.find("EAP") != std::string::npos;
    } else if (deviceKeymgmt == "SAE") {
        return scanInfoKeymgmt.find("SAE") != std::string::npos;
    } else if (deviceKeymgmt == "NONE") {
        return (scanInfoKeymgmt.find("PSK") == std::string::npos) &&
               (scanInfoKeymgmt.find("EAP") == std::string::npos) && (scanInfoKeymgmt.find("SAE") == std::string::npos);
    } else {
        return false;
    }
}

bool StaStateMachine::SetRandomMac(int networkId)
{
    LOGD("enter SetRandomMac.");
#ifdef SUPPORT_LOCAL_RANDOM_MAC
    WifiDeviceConfig deviceConfig;
    if (WifiSettings::GetInstance().GetDeviceConfig(networkId, deviceConfig) != 0) {
        LOGE("SetRandomMac : GetDeviceConfig failed!");
        return false;
    }
    std::string lastMac;
    std::string currentMac;
    if (deviceConfig.wifiPrivacySetting == WifiPrivacyConfig::DEVICEMAC) {
        WifiSettings::GetInstance().GetRealMacAddress(currentMac, m_instId);
    } else {
        WifiStoreRandomMac randomMacInfo;
        std::vector<WifiScanInfo> scanInfoList;
        WifiSettings::GetInstance().GetScanInfoList(scanInfoList);
        for (auto scanInfo : scanInfoList) {
            if ((deviceConfig.ssid == scanInfo.ssid) &&
                (ComparedKeymgmt(scanInfo.capabilities, deviceConfig.keyMgmt))) {
                randomMacInfo.ssid = scanInfo.ssid;
                randomMacInfo.keyMgmt = deviceConfig.keyMgmt;
                randomMacInfo.preSharedKey = deviceConfig.preSharedKey;
                randomMacInfo.peerBssid = scanInfo.bssid;
                break;
            }
        }
        if (randomMacInfo.ssid.empty()) {
            LOGE("scanInfo has no target wifi!");
            return false;
        }

        WifiSettings::GetInstance().GetRandomMac(randomMacInfo);
        if (randomMacInfo.randomMac.empty()) {
            /* Sets the MAC address of WifiSettings. */
            std::string macAddress;
            WifiSettings::GetInstance().GenerateRandomMacAddress(macAddress);
            randomMacInfo.randomMac = macAddress;
            LOGD("%{public}s: generate a random mac, randomMac:%{private}s, ssid:%{private}s, peerbssid:%{private}s",
                __func__, randomMacInfo.randomMac.c_str(), randomMacInfo.ssid.c_str(),
                randomMacInfo.peerBssid.c_str());
            WifiSettings::GetInstance().AddRandomMac(randomMacInfo);
        } else {
            LOGD("%{public}s: randomMac:%{private}s, ssid:%{private}s, peerbssid:%{private}s",
                __func__, randomMacInfo.randomMac.c_str(), randomMacInfo.ssid.c_str(),
                randomMacInfo.peerBssid.c_str());
        }
        currentMac = randomMacInfo.randomMac;
    }

    if ((WifiStaHalInterface::GetInstance().GetStaDeviceMacAddress(lastMac)) != WIFI_IDL_OPT_OK) {
        LOGE("GetStaDeviceMacAddress failed!");
        return false;
    }

    LOGD("%{public}s, currentMac:%{private}s, lastMac:%{private}s", __func__, currentMac.c_str(), lastMac.c_str());
    if (MacAddress::IsValidMac(currentMac.c_str())) {
        if (lastMac != currentMac) {
            if (WifiStaHalInterface::GetInstance().SetConnectMacAddr(currentMac) != WIFI_IDL_OPT_OK) {
                LOGE("set Mac [%{public}s] failed.", MacAnonymize(currentMac).c_str());
                return false;
            }
        }
        WifiSettings::GetInstance().SetMacAddress(currentMac, m_instId);
        deviceConfig.macAddress = currentMac;
        WifiSettings::GetInstance().AddDeviceConfig(deviceConfig);
        WifiSettings::GetInstance().SyncDeviceConfig();
    } else {
        LOGE("Check MacAddress error.");
        return false;
    }
#endif
    return true;
}

void StaStateMachine::StartRoamToNetwork(std::string bssid)
{
    InternalMessage *msg = CreateMessage();
    if (msg == nullptr) {
        return;
    }

    msg->SetMessageName(WIFI_SVR_COM_STA_START_ROAM);
    msg->AddStringMessageBody(bssid);
    SendMessage(msg);
}

bool StaStateMachine::IsRoaming(void)
{
    return isRoam;
}

void StaStateMachine::OnNetworkConnectionEvent(int networkId, std::string bssid)
{
    InternalMessage *msg = CreateMessage();
    if (msg == nullptr) {
        LOGE("msg is nullptr.\n");
        return;
    }

    msg->SetMessageName(WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT);
    msg->SetParam1(networkId);
    msg->AddStringMessageBody(bssid);
    SendMessage(msg);
}

void StaStateMachine::OnNetworkDisconnectEvent(int reason)
{
    WriteWifiAbnormalDisconnectHiSysEvent(reason);
}

void StaStateMachine::OnNetworkAssocEvent(int assocState)
{
    if (assocState == WPA_CB_ASSOCIATING) {
        InvokeOnStaConnChanged(OperateResState::CONNECT_ASSOCIATING, linkedInfo);
    } else {
        InvokeOnStaConnChanged(OperateResState::CONNECT_ASSOCIATED, linkedInfo);
    }
}

void StaStateMachine::OnNetworkHiviewEvent(int state)
{
    if (state == WPA_CB_ASSOCIATING) {
        WriteWifiOperateStateHiSysEvent(static_cast<int>(WifiOperateType::STA_ASSOC),
            static_cast<int>(WifiOperateState::STA_ASSOCIATING));
    } else if (state == WPA_CB_ASSOCIATED) {
        WriteWifiOperateStateHiSysEvent(static_cast<int>(WifiOperateType::STA_ASSOC),
            static_cast<int>(WifiOperateState::STA_ASSOCIATED));
    }
}

void StaStateMachine::OnBssidChangedEvent(std::string reason, std::string bssid)
{
    InternalMessage *msg = CreateMessage();
    if (msg == nullptr) {
        LOGE("msg is nullptr.\n");
        return;
    }

    msg->SetMessageName(WIFI_SVR_CMD_STA_BSSID_CHANGED_EVENT);
    msg->AddStringMessageBody(reason);
    msg->AddStringMessageBody(bssid);
    SendMessage(msg);
}

void StaStateMachine::OnDhcpResultNotifyEvent(DhcpReturnCode result, int ipType)
{
    InternalMessage *msg = CreateMessage();
    if (msg == nullptr) {
        LOGE("msg is nullptr.\n");
        return;
    }

    msg->SetMessageName(WIFI_SVR_CMD_STA_DHCP_RESULT_NOTIFY_EVENT);
    msg->SetParam1(result);
    msg->SetParam2(ipType);
    SendMessage(msg);
}

/* --------------------------- state machine Separating State ------------------------------ */
StaStateMachine::SeparatingState::SeparatingState() : State("SeparatingState")
{}

StaStateMachine::SeparatingState::~SeparatingState()
{}

void StaStateMachine::SeparatingState::GoInState()
{
    WIFI_LOGI("SeparatingState GoInState function.");
    return;
}

void StaStateMachine::SeparatingState::GoOutState()
{
    WIFI_LOGI("SeparatingState GoOutState function.");
}

bool StaStateMachine::SeparatingState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    bool ret = NOT_EXECUTED;
    WIFI_LOGI("SeparatingState-msgCode=%{public}d not handled.\n", msg->GetMessageName());
    return ret;
}

/* --------------------------- state machine Disconnected State ------------------------------ */
StaStateMachine::SeparatedState::SeparatedState(StaStateMachine *staStateMachine)
    : State("SeparatedState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::SeparatedState::~SeparatedState()
{}

void StaStateMachine::SeparatedState::GoInState()
{
    WIFI_LOGI("SeparatedState GoInState function.");
    return;
}

void StaStateMachine::SeparatedState::GoOutState()
{
    WIFI_LOGI("SeparatedState GoOutState function.");
    return;
}

bool StaStateMachine::SeparatedState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    WIFI_LOGI("SeparatedState-msgCode=%{public}d received.\n", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT:
            break;

        case WIFI_SVR_CMD_STA_ENABLE_WIFI: {
            ret = EXECUTED;
            WIFI_LOGE("Wifi has already started! start Wifi failed!");
            /* Callback result to InterfaceService. */
            pStaStateMachine->InvokeOnStaOpenRes(OperateResState::OPEN_WIFI_OVERRIDE_OPEN_FAILED);
            break;
        }

        default:
            break;
    }

    return ret;
}

/* --------------------------- state machine ApConnected State ------------------------------ */
StaStateMachine::ApLinkedState::ApLinkedState(StaStateMachine *staStateMachine)
    : State("ApLinkedState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::ApLinkedState::~ApLinkedState()
{}

void StaStateMachine::ApLinkedState::GoInState()
{
    WIFI_LOGI("ApLinkedState GoInState function.");
    WriteWifiOperateStateHiSysEvent(static_cast<int>(WifiOperateType::STA_CONNECT),
        static_cast<int>(WifiOperateState::STA_CONNECTED));
    return;
}

void StaStateMachine::ApLinkedState::GoOutState()
{
    WIFI_LOGI("ApLinkedState GoOutState function.");
    return;
}

bool StaStateMachine::ApLinkedState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    WIFI_LOGD("ApLinkedState-msgCode=%{public}d received.\n", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        /* The current state of StaStateMachine transfers to SeparatingState when
         * receive the Separating message.
         */
        case WIFI_SVR_CMD_STA_DISCONNECT: {
            ret = EXECUTED;
            pStaStateMachine->DisConnectProcess();
            break;
        }
        case WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT: {
            ret = EXECUTED;
            pStaStateMachine->StopTimer(static_cast<int>(WPA_BLOCK_LIST_CLEAR_EVENT));
            WIFI_LOGI("Stop clearing wpa block list");
            /* Save linkedinfo */
            pStaStateMachine->linkedInfo.networkId = pStaStateMachine->targetNetworkId;
            pStaStateMachine->linkedInfo.bssid = msg->GetStringFromMessage();
            WifiSettings::GetInstance().SaveLinkedInfo(pStaStateMachine->linkedInfo, pStaStateMachine->GetInstanceId());

            break;
        }
        case WIFI_SVR_CMD_STA_BSSID_CHANGED_EVENT: {
            ret = EXECUTED;
            std::string reason = msg->GetStringFromMessage();
            std::string bssid = msg->GetStringFromMessage();
            WIFI_LOGI("ApLinkedState reveived bssid changed event, reason:%{public}s,bssid:%{public}s.\n",
                reason.c_str(), MacAnonymize(bssid).c_str());
            if (strcmp(reason.c_str(), "ASSOC_COMPLETE") != 0) {
                WIFI_LOGE("Bssid change not for ASSOC_COMPLETE, do nothing.");
                return false;
            }
            /* BSSID change is not received during roaming, only set BSSID */
            if (WifiStaHalInterface::GetInstance().SetBssid(WPA_DEFAULT_NETWORKID, bssid) != WIFI_IDL_OPT_OK) {
                WIFI_LOGE("SetBssid return fail.");
                return false;
            }
            break;
        }
        default:
            break;
    }
    return ret;
}

void StaStateMachine::DisConnectProcess()
{
    WIFI_LOGI("Enter DisConnectProcess!");
    InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTING, linkedInfo);
    if (WifiStaHalInterface::GetInstance().Disconnect() == WIFI_IDL_OPT_OK) {
        WIFI_LOGI("Disconnect() succeed!");
        mPortalUrl = "";
#ifndef OHOS_ARCH_LITE
        if (NetSupplierInfo != nullptr) {
            NetSupplierInfo->isAvailable_ = false;
            WIFI_LOGI("Disconnect process update netsupplierinfo");
            WifiNetAgent::GetInstance().OnStaMachineUpdateNetSupplierInfo(NetSupplierInfo);
        }
#endif
#ifdef SUPPORT_LOCAL_RANDOM_MAC
        std::string macAddress;
        WifiSettings::GetInstance().GenerateRandomMacAddress(macAddress);
        if (MacAddress::IsValidMac(macAddress.c_str())) {
            if (WifiStaHalInterface::GetInstance().SetConnectMacAddr(macAddress) != WIFI_IDL_OPT_OK) {
                LOGE("%{public}s: failed to set sta MAC address:%{private}s", __func__, macAddress.c_str());
            }
            WifiSettings::GetInstance().SetMacAddress(macAddress, m_instId);
        }
#endif
        WIFI_LOGI("Disconnect update wifi status");
        /* Save connection information to WifiSettings. */
        SaveLinkstate(ConnState::DISCONNECTED, DetailedState::DISCONNECTED);
        WifiStaHalInterface::GetInstance().DisableNetwork(WPA_DEFAULT_NETWORKID);

        getIpSucNum = 0;
        /* The current state of StaStateMachine transfers to SeparatedState. */
        SwitchState(pSeparatedState);
    } else {
        SaveLinkstate(ConnState::DISCONNECTING, DetailedState::FAILED);
        InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECT_FAILED, linkedInfo);
        WIFI_LOGE("Disconnect() failed!");
    }
}

/* --------------------------- state machine Wps State ------------------------------ */
StaStateMachine::StaWpsState::StaWpsState(StaStateMachine *staStateMachine)
    : State("StaWpsState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::StaWpsState::~StaWpsState()
{}

void StaStateMachine::StaWpsState::GoInState()
{
    WIFI_LOGI("WpsState GoInState function.");
    return;
}

void StaStateMachine::StaWpsState::GoOutState()
{
    WIFI_LOGI("WpsState GoOutState function.");
}

bool StaStateMachine::StaWpsState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_WPS_START_EVENT: {
            /* Wps starts successfully and Wait until the connection is complete. */
            break;
        }
        case WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT: {
            ret = EXECUTED;
            /* Stop clearing the Wpa_blocklist. */
            pStaStateMachine->StopTimer(static_cast<int>(WPA_BLOCK_LIST_CLEAR_EVENT));

            WIFI_LOGI("WPS mode connect to a network!");
            pStaStateMachine->ConnectToNetworkProcess(msg);
            /* Callback result to InterfaceService. */
            pStaStateMachine->SaveLinkstate(ConnState::CONNECTING, DetailedState::OBTAINING_IPADDR);
            pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_OBTAINING_IP,
                pStaStateMachine->linkedInfo);
            pStaStateMachine->SwitchState(pStaStateMachine->pGetIpState);
            break;
        }
        case WIFI_SVR_CMD_STA_STARTWPS: {
            ret = EXECUTED;
            auto setup = static_cast<SetupMethod>(msg->GetParam1());
            /* Callback InterfaceService that wps has started successfully. */
            WIFI_LOGE("WPS has already started, start wps failed!");
            if (setup == SetupMethod::PBC) {
                pStaStateMachine->InvokeOnWpsChanged(WpsStartState::PBC_STARTED_ALREADY,
                    pStaStateMachine->pinCode);
            } else if ((setup == SetupMethod::DISPLAY) || (setup == SetupMethod::KEYPAD)) {
                pStaStateMachine->InvokeOnWpsChanged(WpsStartState::PIN_STARTED_ALREADY,
                    pStaStateMachine->pinCode);
            }
            break;
        }
        case WIFI_SVR_CMD_STA_WPS_OVERLAP_EVENT: {
            ret = EXECUTED;
            WIFI_LOGI("Wps PBC Overlap!");
            /* Callback InterfaceService that PBC is conflicting. */
            pStaStateMachine->InvokeOnWpsChanged(WpsStartState::START_PBC_FAILED_OVERLAP,
                pStaStateMachine->pinCode);
            pStaStateMachine->SwitchState(pStaStateMachine->pSeparatedState);
            break;
        }
        case WIFI_SVR_CMD_STA_CANCELWPS: {
            ret = EXECUTED;
            pStaStateMachine->DealCancelWpsCmd(msg);
            break;
        }
        default:
            break;
    }
    return ret;
}

int StaStateMachine::RegisterCallBack()
{
    clientCallBack.OnIpSuccessChanged = DhcpResultNotify::OnSuccess;
    clientCallBack.OnIpFailChanged = DhcpResultNotify::OnFailed;
    std::string ifname = IF_NAME + std::to_string(m_instId);
    DhcpErrorCode dhcpRet = RegisterDhcpClientCallBack(ifname.c_str(), &clientCallBack);
    if (dhcpRet != DHCP_SUCCESS) {
        WIFI_LOGE("RegisterDhcpClientCallBack failed. dhcpRet=%{public}d", dhcpRet);
        return DHCP_FAILED;
    }
    LOGI("RegisterDhcpClientCallBack ok");
    return DHCP_SUCCESS;
}
/* --------------------------- state machine GetIp State ------------------------------ */
StaStateMachine::GetIpState::GetIpState(StaStateMachine *staStateMachine)
    : State("GetIpState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::GetIpState::~GetIpState()
{}

void StaStateMachine::GetIpState::GoInState()
{
    WIFI_LOGI("GetIpState GoInState function.");
#ifdef WIFI_DHCP_DISABLED
    SaveDiscReason(DisconnectedReason::DISC_REASON_DEFAULT);
    SaveLinkstate(ConnState::CONNECTED, DetailedState::WORKING);
    InvokeOnStaConnChanged(OperateResState::CONNECT_NETWORK_ENABLED, linkedInfo);
    SwitchState(pLinkedState);
    return;
#endif
    pStaStateMachine->getIpSucNum = 0;
    WifiDeviceConfig config;
    AssignIpMethod assignMethod = AssignIpMethod::DHCP;
    int ret = WifiSettings::GetInstance().GetDeviceConfig(pStaStateMachine->linkedInfo.networkId, config);
    if (ret == 0) {
        assignMethod = config.wifiIpConfig.assignMethod;
    }

    pStaStateMachine->pDhcpResultNotify->SetStaStateMachine(pStaStateMachine);
    if (assignMethod == AssignIpMethod::STATIC) {
        pStaStateMachine->currentTpType = config.wifiIpConfig.staticIpAddress.ipAddress.address.family;
        if (!pStaStateMachine->ConfigStaticIpAddress(config.wifiIpConfig.staticIpAddress)) {
            pStaStateMachine->InvokeOnStaConnChanged(
                OperateResState::CONNECT_NETWORK_DISABLED, pStaStateMachine->linkedInfo);
            pStaStateMachine->DisConnectProcess();
            LOGE("ConfigstaticIpAddress failed!\n");
        }
        return;
    }
    do {
        int result = pStaStateMachine->RegisterCallBack();
        if (result != DHCP_SUCCESS) {
            WIFI_LOGE("RegisterCallBack failed!");
            break;
        }
        int dhcpRet;
        std::string ifname = IF_NAME + std::to_string(pStaStateMachine->GetInstanceId());
        pStaStateMachine->currentTpType = static_cast<int>(WifiSettings::GetInstance().GetDhcpIpType());
        if (pStaStateMachine->currentTpType == IPTYPE_IPV4) {
            dhcpRet = StartDhcpClient(ifname.c_str(), false);
        } else {
            dhcpRet = StartDhcpClient(ifname.c_str(), true);
        }
        LOGI("StartDhcpClient type:%{public}d dhcpRet:%{public}d isRoam:%{public}d", pStaStateMachine->currentTpType,
            dhcpRet, pStaStateMachine->isRoam);
        if (dhcpRet == 0) {
            LOGI("StartTimer CMD_START_GET_DHCP_IP_TIMEOUT 30s");
            pStaStateMachine->StartTimer(static_cast<int>(CMD_START_GET_DHCP_IP_TIMEOUT),
                STA_SIGNAL_START_GET_DHCP_IP_DELAY);
            return;
        }
    } while (0);
    WIFI_LOGE("Dhcp connection failed, isRoam:%{public}d", pStaStateMachine->isRoam);
    pStaStateMachine->SaveLinkstate(ConnState::DISCONNECTED, DetailedState::OBTAINING_IPADDR_FAIL);
    pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_OBTAINING_IP_FAILED,
        pStaStateMachine->linkedInfo);
    if (!pStaStateMachine->isRoam) {
        pStaStateMachine->DisConnectProcess();
    }
    return;
}

void StaStateMachine::GetIpState::GoOutState()
{
    WIFI_LOGI("GetIpState GoOutState function.");
}

bool StaStateMachine::GetIpState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    bool ret = NOT_EXECUTED;
    WIFI_LOGI("GetIpState-msgCode=%{public}d received.\n", msg->GetMessageName());
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_DHCP_RESULT_NOTIFY_EVENT: {
            ret = EXECUTED;
            int result = msg->GetParam1();
            int ipType = msg->GetParam2();
            WIFI_LOGI("GetIpState, get ip result:%{public}d, ipType = %{public}d\n", result, ipType);
            switch (result) {
                case DhcpReturnCode::DHCP_RESULT: {
                    pStaStateMachine->pDhcpResultNotify->DealDhcpResult(ipType);
                    break;
                }
                case DhcpReturnCode::DHCP_JUMP: {
                    pStaStateMachine->SwitchState(pStaStateMachine->pLinkedState);
                    break;
                }
                case DhcpReturnCode::DHCP_FAIL: {
                    pStaStateMachine->pDhcpResultNotify->DealDhcpResultFailed();
                    break;
                }
                default:
                    break;
            }
        }
        default:
            break;
    }

    return ret;
}

void StaStateMachine::ReplaceEmptyDns(DhcpResult *result)
{
    if (result == nullptr) {
        WIFI_LOGE("Enter ReplaceEmptyDns::result is nullptr");
        return;
    }
    std::string strDns1 = result->strOptDns1;
    std::string strDns2 = result->strOptDns2;
    if (strDns1.empty()) {
        WIFI_LOGI("Enter ReplaceEmptyDns::dns1 is null");
        if (strDns2 == FIRST_DNS) {
            if (strcpy_s(result->strOptDns1, INET_ADDRSTRLEN, SECOND_DNS) != EOK) {
                WIFI_LOGE("ReplaceEmptyDns strDns1 strcpy_s SECOND_DNS failed!");
            }
        } else {
            if (strcpy_s(result->strOptDns1, INET_ADDRSTRLEN, FIRST_DNS) != EOK) {
                WIFI_LOGE("ReplaceEmptyDns strDns1 strcpy_s FIRST_DNS failed!");
            }
        }
    }
    if (strDns2.empty()) {
        WIFI_LOGI("Enter ReplaceEmptyDns::dns2 is null");
        if (strDns1 == FIRST_DNS) {
            if (strcpy_s(result->strOptDns2, INET_ADDRSTRLEN, SECOND_DNS) != EOK) {
                WIFI_LOGE("ReplaceEmptyDns strDns2 strcpy_s SECOND_DNS failed!");
            }
        } else {
            if (strcpy_s(result->strOptDns2, INET_ADDRSTRLEN, FIRST_DNS) != EOK) {
                WIFI_LOGE("ReplaceEmptyDns strDns2 strcpy_s SECOND_DNS failed!");
            }
        }
    }
}

/* --- state machine GetIp State functions ----- */
bool StaStateMachine::ConfigStaticIpAddress(StaticIpAddress &staticIpAddress)
{
    WIFI_LOGI("Enter StaStateMachine::SetDhcpResultFromStatic.");
    std::string ifname = IF_NAME + std::to_string(m_instId);
    DhcpResult result;
    switch (currentTpType) {
        case IPTYPE_IPV4: {
            result.iptype = IPTYPE_IPV4;
            if (strcpy_s(result.strOptClientId, INET_ADDRSTRLEN,
                staticIpAddress.ipAddress.address.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptClientId strcpy_s failed!");
            }
            if (strcpy_s(result.strOptRouter1, INET_ADDRSTRLEN,
                staticIpAddress.gateway.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptRouter1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptSubnet, INET_ADDRSTRLEN, staticIpAddress.GetIpv4Mask().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptSubnet strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns1, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer1.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns2, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer2.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns2 strcpy_s failed!");
            }
            ReplaceEmptyDns(&result);
            pDhcpResultNotify->OnSuccess(1, ifname.c_str(), &result);
            break;
        }
        case IPTYPE_IPV6: {
            result.iptype = IPTYPE_IPV6;
            if (strcpy_s(result.strOptClientId, INET_ADDRSTRLEN,
                staticIpAddress.ipAddress.address.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptClientId strcpy_s failed!");
            }
            if (strcpy_s(result.strOptRouter1, INET_ADDRSTRLEN,
                staticIpAddress.gateway.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptRouter1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptSubnet, INET_ADDRSTRLEN, staticIpAddress.GetIpv6Mask().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptSubnet strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns1, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer1.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns2, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer2.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns2 strcpy_s failed!");
            }
            pDhcpResultNotify->OnSuccess(1, ifname.c_str(), &result);
            break;
        }
        case IPTYPE_MIX: {
            result.iptype = IPTYPE_IPV4;
            if (strcpy_s(result.strOptClientId, INET_ADDRSTRLEN,
                staticIpAddress.ipAddress.address.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptClientId strcpy_s failed!");
            }
            if (strcpy_s(result.strOptRouter1, INET_ADDRSTRLEN,
                staticIpAddress.gateway.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptRouter1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptSubnet, INET_ADDRSTRLEN,
                staticIpAddress.GetIpv4Mask().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptSubnet strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns1, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer1.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns2, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer2.GetIpv4Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns2 strcpy_s failed!");
            }
            pDhcpResultNotify->OnSuccess(1, ifname.c_str(), &result);
            if (strcpy_s(result.strOptClientId, INET_ADDRSTRLEN,
                staticIpAddress.ipAddress.address.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptClientId strcpy_s failed!");
            }
            if (strcpy_s(result.strOptRouter1, INET_ADDRSTRLEN,
                staticIpAddress.gateway.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptRouter1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptSubnet, INET_ADDRSTRLEN, staticIpAddress.GetIpv6Mask().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptSubnet strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns1, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer1.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns1 strcpy_s failed!");
            }
            if (strcpy_s(result.strOptDns2, INET_ADDRSTRLEN,
                staticIpAddress.dnsServer2.GetIpv6Address().c_str()) != EOK) {
                WIFI_LOGE("ConfigStaticIpAddress strOptDns2 strcpy_s failed!");
            }
            pDhcpResultNotify->OnSuccess(1, ifname.c_str(), &result);
            break;
        }

        default:
            WIFI_LOGE("Invalid currentTpType: %{public}d", currentTpType);
            return false;
    }
    return true;
}

void StaStateMachine::HandlePortalNetworkPorcess()
{
#ifndef OHOS_ARCH_LITE
    WIFI_LOGI("portal uri is %{public}s\n", mPortalUrl.c_str());
    AAFwk::Want want;
    want.SetAction(PORTAL_ACTION);
    want.SetUri(mPortalUrl);
    want.AddEntity(PORTAL_ENTITY);
    portalFlag = true;
    OHOS::ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
    if (err != ERR_OK) {
        WIFI_LOGI("StartAbility is failed %{public}d", err);
        WriteBrowserFailedForPortalHiSysEvent(err, mPortalUrl);
    }
#endif
}

void StaStateMachine::NetStateObserverCallback(SystemNetWorkState netState)
{
    WIFI_LOGI("NetStateObserverCallback is %{public}d\n", netState);
    m_netState = netState;
    if (netState == NETWORK_CELL_NOWORK) {
        WifiLinkedInfo linkedInfo;
        GetLinkedInfo(linkedInfo);
        if (linkedInfo.detailedState == DetailedState::CAPTIVE_PORTAL_CHECK && portalFlag == false) {
            SendMessage(WIFI_SVR_CMD_STA_NET_DETECTION_NOTIFY_EVENT, NETWORK_CHECK_PORTAL, 0, mPortalUrl);
        }
    }
}


void StaStateMachine::HandleNetCheckResult(StaNetState netState, const std::string portalUrl)
{
    WIFI_LOGI("Enter HandleNetCheckResult, netState:%{public}d screen:%{public}d.", netState, enableSignalPoll);
    if (linkedInfo.connState != ConnState::CONNECTED) {
        WIFI_LOGE("connState is NOT in connected state, connState:%{public}d\n", linkedInfo.connState);
        WriteIsInternetHiSysEvent(NO_NETWORK);
        return;
    }
    mPortalUrl = portalUrl;
    if (netState == StaNetState::NETWORK_STATE_WORKING) {
        WIFI_LOGI("HandleNetCheckResult network state is working\n");
        /* Save connection information to WifiSettings. */
        WriteIsInternetHiSysEvent(NETWORK);
        SaveLinkstate(ConnState::CONNECTED, DetailedState::WORKING);
        InvokeOnStaConnChanged(OperateResState::CONNECT_NETWORK_ENABLED, linkedInfo);
        if (portalFlag == true) {
            WIFI_LOGD("portal network normal working need redetect for expired!");
            StartTimer(static_cast<int>(CMD_START_NETCHECK), PORTAL_CHECK_TIME * PORTAL_MILLSECOND);
        }
        InsertOrUpdateNetworkStatusHistory(NetworkStatus::HAS_INTERNET);
        netNoWorkNum = 0;
    } else if (netState == StaNetState::NETWORK_CHECK_PORTAL) {
        WifiLinkedInfo linkedInfo;
        GetLinkedInfo(linkedInfo);
        if ((linkedInfo.detailedState != DetailedState::CAPTIVE_PORTAL_CHECK || portalFlag == false) &&
           m_netState == NETWORK_CELL_NOWORK) {
            WriteIsInternetHiSysEvent(NO_NETWORK);
            HandlePortalNetworkPorcess();
        }
        WriteIsInternetHiSysEvent(NETWORK);
        StartTimer(static_cast<int>(CMD_START_NETCHECK), PORTAL_CHECK_TIME * PORTAL_MILLSECOND);
        SaveLinkstate(ConnState::CONNECTED, DetailedState::CAPTIVE_PORTAL_CHECK);
        InvokeOnStaConnChanged(OperateResState::CONNECT_CHECK_PORTAL, linkedInfo);
        InsertOrUpdateNetworkStatusHistory(NetworkStatus::PORTAL);
        netNoWorkNum = 0;
    } else {
        WIFI_LOGI("HandleNetCheckResult network state is notworking.\n");
        WriteIsInternetHiSysEvent(NO_NETWORK);
        SaveLinkstate(ConnState::CONNECTED, DetailedState::NOTWORKING);
        InvokeOnStaConnChanged(OperateResState::CONNECT_NETWORK_DISABLED, linkedInfo);
        int delay = 1 << netNoWorkNum;
        delay = delay > PORTAL_CHECK_TIME ? PORTAL_CHECK_TIME : delay;
        netNoWorkNum++;
        StartTimer(static_cast<int>(CMD_START_NETCHECK), delay * PORTAL_MILLSECOND);
        InsertOrUpdateNetworkStatusHistory(NetworkStatus::NO_INTERNET);
    }
}
void StaStateMachine::NetDetectionProcess(StaNetState netState, const std::string portalUrl)
{
    SendMessage(WIFI_SVR_CMD_STA_NET_DETECTION_NOTIFY_EVENT, netState, 0, portalUrl);
}

void StaStateMachine::HandleArpCheckResult(StaArpState arpState)
{
}

void StaStateMachine::HandleDnsCheckResult(StaDnsState dnsState)
{
}

/* --------------------------- state machine Connected State ------------------------------ */
StaStateMachine::LinkedState::LinkedState(StaStateMachine *staStateMachine)
    : State("LinkedState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::LinkedState::~LinkedState()
{}

void StaStateMachine::LinkedState::GoInState()
{
    WIFI_LOGI("LinkedState GoInState function.");

    return;
}

void StaStateMachine::LinkedState::GoOutState()
{
    WIFI_LOGI("LinkedState GoOutState function.");
}

bool StaStateMachine::LinkedState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        WIFI_LOGI("msg is nullptr.");
        return false;
    }

    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_BSSID_CHANGED_EVENT: {
            ret = EXECUTED;
            std::string reason = msg->GetStringFromMessage();
            std::string bssid = msg->GetStringFromMessage();
            WIFI_LOGI("reveived bssid changed event, reason:%{public}s,bssid:%{public}s.\n",
                reason.c_str(), MacAnonymize(bssid).c_str());
            if (strcmp(reason.c_str(), "ASSOC_COMPLETE") != 0) {
                WIFI_LOGE("Bssid change not for ASSOC_COMPLETE, do nothing.");
                return false;
            }
            if (WifiStaHalInterface::GetInstance().SetBssid(WPA_DEFAULT_NETWORKID, bssid) != WIFI_IDL_OPT_OK) {
                WIFI_LOGE("SetBssid return fail.");
                return false;
            }
            pStaStateMachine->isRoam = true;
            /* The current state of StaStateMachine transfers to pApRoamingState. */
            pStaStateMachine->SwitchState(pStaStateMachine->pApRoamingState);
            break;
        }
        case WIFI_SVR_CMD_STA_DHCP_RESULT_NOTIFY_EVENT: {
            ret = EXECUTED;
            int result = msg->GetParam1();
            int ipType = msg->GetParam2();
            WIFI_LOGI("LinkedState, result:%{public}d, ipType = %{public}d\n", result, ipType);
            if (result == DhcpReturnCode::DHCP_RENEW_FAIL) {
                pStaStateMachine->StopTimer(static_cast<int>(CMD_START_GET_DHCP_IP_TIMEOUT));
            } else if (result == DhcpReturnCode::DHCP_RESULT) {
                pStaStateMachine->pDhcpResultNotify->DealDhcpResult(ipType);
            }
            break;
        }
        case WIFI_SVR_CMD_STA_NET_DETECTION_NOTIFY_EVENT: {
            ret = EXECUTED;
            StaNetState netstate = (StaNetState)msg->GetParam1();
            std::string url;
            if (!msg->GetMessageObj(url)) {
                WIFI_LOGW("Failed to obtain portal url.");
            }
            WIFI_LOGI("netdetection, netstate:%{public}d url:%{public}s.\n", netstate, url.c_str());
            pStaStateMachine->HandleNetCheckResult(netstate, url);
            break;
        }
        default:
            WIFI_LOGD("NOT handle this event!");
            break;
    }

    return ret;
}

void StaStateMachine::DealApRoamingStateTimeout(InternalMessage *msg)
{
    if (msg == nullptr) {
        LOGE("DealApRoamingStateTimeout InternalMessage msg is null.");
        return;
    }
    LOGI("DealApRoamingStateTimeout StopTimer aproaming timer");
    StopTimer(static_cast<int>(CMD_AP_ROAMING_TIMEOUT_CHECK));
    DisConnectProcess();
}

/* --------------------------- state machine Roaming State ------------------------------ */
StaStateMachine::ApRoamingState::ApRoamingState(StaStateMachine *staStateMachine)
    : State("ApRoamingState"), pStaStateMachine(staStateMachine)
{}

StaStateMachine::ApRoamingState::~ApRoamingState()
{}

void StaStateMachine::ApRoamingState::GoInState()
{
    WIFI_LOGI("ApRoamingState GoInState function. start aproaming timer!");
    pStaStateMachine->StartTimer(static_cast<int>(CMD_AP_ROAMING_TIMEOUT_CHECK), STA_AP_ROAMING_TIMEOUT);
}

void StaStateMachine::ApRoamingState::GoOutState()
{
    WIFI_LOGI("ApRoamingState GoOutState function. stop aproaming timer!");
    pStaStateMachine->StopTimer(static_cast<int>(CMD_AP_ROAMING_TIMEOUT_CHECK));
}

bool StaStateMachine::ApRoamingState::ExecuteStateMsg(InternalMessage *msg)
{
    if (msg == nullptr) {
        return false;
    }

    WIFI_LOGI("ApRoamingState, reveived msgCode=%{public}d msg.", msg->GetMessageName());
    bool ret = NOT_EXECUTED;
    switch (msg->GetMessageName()) {
        case WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT: {
            WIFI_LOGI("ApRoamingState, receive WIFI_SVR_CMD_STA_NETWORK_CONNECTION_EVENT event.");
            ret = EXECUTED;
            pStaStateMachine->isRoam = true;
            pStaStateMachine->StopTimer(static_cast<int>(CMD_AP_ROAMING_TIMEOUT_CHECK));
            pStaStateMachine->StopTimer(static_cast<int>(CMD_NETWORK_CONNECT_TIMEOUT));
            pStaStateMachine->ConnectToNetworkProcess(msg);
            /* Notify result to InterfaceService. */
            pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_ASSOCIATED,
                pStaStateMachine->linkedInfo);
            if (!pStaStateMachine->CanArpReachable()) {
                WIFI_LOGI("Arp is not reachable");
                pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_OBTAINING_IP,
                    pStaStateMachine->linkedInfo);
                /* The current state of StaStateMachine transfers to GetIpState. */
                pStaStateMachine->SwitchState(pStaStateMachine->pGetIpState);
            } else {
                WIFI_LOGI("Arp is reachable");
                pStaStateMachine->SaveLinkstate(ConnState::CONNECTED, DetailedState::CONNECTED);
                pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_AP_CONNECTED,
                    pStaStateMachine->linkedInfo);
                pStaStateMachine->SwitchState(pStaStateMachine->pLinkedState);
            }
            break;
        }
        case WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT:
            WIFI_LOGI("ApRoamingState, receive WIFI_SVR_CMD_STA_NETWORK_DISCONNECTION_EVENT event.");
            pStaStateMachine->StopTimer(static_cast<int>(CMD_AP_ROAMING_TIMEOUT_CHECK));
            pStaStateMachine->DisConnectProcess();
            break;
        default:
            WIFI_LOGI("ApRoamingState-msgCode=%d not handled.", msg->GetMessageName());
            break;
    }

    return ret;
}

bool StaStateMachine::CanArpReachable()
{
    ArpChecker arpChecker;
    std::string macAddress;
    WifiSettings::GetInstance().GetMacAddress(macAddress, m_instId);
    IpInfo ipInfo;
    WifiSettings::GetInstance().GetIpInfo(ipInfo, m_instId);
    std::string ipAddress = IpTools::ConvertIpv4Address(ipInfo.ipAddress);
    std::string ifName = "wlan" + std::to_string(m_instId);
    if (ipInfo.gateway == 0) {
        WIFI_LOGI("gateway is empty");
        return false;
    }
    std::string gateway = IpTools::ConvertIpv4Address(ipInfo.gateway);
    arpChecker.Start(ifName, macAddress, ipAddress, gateway);
    for (int i = 0; i < DEFAULT_NUM_ARP_PINGS; i++) {
        if (arpChecker.DoArpCheck(MAX_ARP_CHECK_TIME, true)) {
            return true;
        }
    }
    return false;
}

void StaStateMachine::ConnectToNetworkProcess(InternalMessage *msg)
{
    if (msg == nullptr) {
        return;
    }

    std::string bssid = msg->GetStringFromMessage();
    WIFI_LOGI("ConnectToNetworkProcess, Receive msg: wpa NetworkId=%{public}d, bssid=%{public}s",
        msg->GetParam1(), MacAnonymize(bssid).c_str());
    if ((wpsState == SetupMethod::DISPLAY) || (wpsState == SetupMethod::PBC) || (wpsState == SetupMethod::KEYPAD)) {
        targetNetworkId = WPA_DEFAULT_NETWORKID;
    }
    WifiDeviceConfig deviceConfig;
    int result = WifiSettings::GetInstance().GetDeviceConfig(targetNetworkId, deviceConfig);
    WIFI_LOGD("Device config networkId = %{public}d", deviceConfig.networkId);

    if (result == 0 && deviceConfig.bssid == bssid) {
        LOGI("Device Configuration already exists.");
    } else {
        deviceConfig.bssid = bssid;
        if ((wpsState == SetupMethod::DISPLAY) || (wpsState == SetupMethod::PBC) || (wpsState == SetupMethod::KEYPAD)) {
            /* Save connection information. */
            WifiIdlGetDeviceConfig config;
            config.networkId = WPA_DEFAULT_NETWORKID;
            config.param = "ssid";
            if (WifiStaHalInterface::GetInstance().GetDeviceConfig(config) != WIFI_IDL_OPT_OK) {
                LOGE("GetDeviceConfig failed!");
            }

            deviceConfig.networkId = WPA_DEFAULT_NETWORKID;
            deviceConfig.bssid = bssid;
            deviceConfig.ssid = config.value;
            /* Remove the double quotation marks at the head and tail. */
            deviceConfig.ssid.erase(0, 1);
            deviceConfig.ssid.erase(deviceConfig.ssid.length() - 1, 1);
            WifiSettings::GetInstance().AddWpsDeviceConfig(deviceConfig);
            isWpsConnect = IsWpsConnected::WPS_CONNECTED;
        } else {
            WifiSettings::GetInstance().AddDeviceConfig(deviceConfig);
        }
        WifiSettings::GetInstance().SyncDeviceConfig();
        WIFI_LOGD("Device ssid = %s", SsidAnonymize(deviceConfig.ssid).c_str());
    }

    std::string macAddr;
    std::string realMacAddr;
    WifiSettings::GetInstance().GetMacAddress(macAddr, m_instId);
    WifiSettings::GetInstance().GetRealMacAddress(realMacAddr, m_instId);
    linkedInfo.networkId = targetNetworkId;
    linkedInfo.bssid = bssid;
    linkedInfo.ssid = deviceConfig.ssid;
    linkedInfo.macType = (macAddr == realMacAddr ?
        static_cast<int>(WifiPrivacyConfig::DEVICEMAC) : static_cast<int>(WifiPrivacyConfig::RANDOMMAC));
    linkedInfo.macAddress = macAddr;
    linkedInfo.ifHiddenSSID = deviceConfig.hiddenSSID;
    lastLinkedInfo.bssid = bssid;
    lastLinkedInfo.macType = static_cast<int>(deviceConfig.wifiPrivacySetting);
    lastLinkedInfo.macAddress = deviceConfig.macAddress;
    lastLinkedInfo.ifHiddenSSID = deviceConfig.hiddenSSID;
    SetWifiLinkedInfo(targetNetworkId);
    SaveLinkstate(ConnState::CONNECTING, DetailedState::OBTAINING_IPADDR);
}

void StaStateMachine::SetWifiLinkedInfo(int networkId)
{
    WIFI_LOGI("SetWifiLinkedInfo, linkedInfo.networkId=%{public}d, lastLinkedInfo.networkId=%{public}d",
        linkedInfo.networkId, lastLinkedInfo.networkId);
    if (linkedInfo.networkId == INVALID_NETWORK_ID) {
        if (lastLinkedInfo.networkId != INVALID_NETWORK_ID) {
            /* Update connection information according to the last connecting information. */
            linkedInfo.retryedConnCount = 0;
            linkedInfo.networkId = lastLinkedInfo.networkId;
            linkedInfo.ssid = lastLinkedInfo.ssid;
            linkedInfo.bssid = lastLinkedInfo.bssid;
            linkedInfo.macAddress = lastLinkedInfo.macAddress;
            linkedInfo.rssi = lastLinkedInfo.rssi;
            linkedInfo.band = lastLinkedInfo.band;
            linkedInfo.frequency = lastLinkedInfo.frequency;
            linkedInfo.linkSpeed = lastLinkedInfo.linkSpeed;
            linkedInfo.ipAddress = lastLinkedInfo.ipAddress;
            linkedInfo.connState = lastLinkedInfo.connState;
            linkedInfo.ifHiddenSSID = lastLinkedInfo.ifHiddenSSID;
            linkedInfo.rxLinkSpeed = lastLinkedInfo.rxLinkSpeed;
            linkedInfo.txLinkSpeed = lastLinkedInfo.txLinkSpeed;
            linkedInfo.chload = lastLinkedInfo.chload;
            linkedInfo.snr = lastLinkedInfo.snr;
            linkedInfo.isDataRestricted = lastLinkedInfo.isDataRestricted;
            linkedInfo.platformType = lastLinkedInfo.platformType;
            linkedInfo.portalUrl = lastLinkedInfo.portalUrl;
            linkedInfo.detailedState = lastLinkedInfo.detailedState;
            linkedInfo.isAncoConnected = lastLinkedInfo.isAncoConnected;
        } else if (networkId != INVALID_NETWORK_ID) {
            linkedInfo.retryedConnCount = 0;
            linkedInfo.networkId = networkId;
            WifiDeviceConfig config;
            int ret = WifiSettings::GetInstance().GetDeviceConfig(networkId, config);
            if (ret == 0) {
                /* Update connection information according to configuration. */
                linkedInfo.networkId = config.networkId;
                linkedInfo.ssid = config.ssid;
                linkedInfo.bssid = config.bssid;
                linkedInfo.band = config.band;
                linkedInfo.connState = ConnState::CONNECTING;
                linkedInfo.ifHiddenSSID = config.hiddenSSID;
                linkedInfo.detailedState = DetailedState::OBTAINING_IPADDR;

                lastLinkedInfo.networkId = config.networkId;
                lastLinkedInfo.ssid = config.ssid;
                lastLinkedInfo.bssid = config.bssid;
                lastLinkedInfo.band = config.band;
                lastLinkedInfo.connState = ConnState::CONNECTING;
                lastLinkedInfo.ifHiddenSSID = config.hiddenSSID;
                lastLinkedInfo.detailedState = DetailedState::OBTAINING_IPADDR;
            }
        }
        WriteWifiBandHiSysEvent(linkedInfo.band);
    }
}

void StaStateMachine::DealNetworkCheck(InternalMessage *msg)
{
    LOGD("enter DealNetworkCheck.\n");
    if (msg == nullptr) {
        LOGE("InternalMessage msg is null.");
        return;
    }

    if (pNetcheck == nullptr) {
        LOGE("pNetcheck is null.");
        return;
    }
    if (linkedInfo.connState != ConnState::CONNECTED) {
        WIFI_LOGE("DealNetworkCheck NOT in connected state, connState:%{public}d\n", linkedInfo.connState);
        return;
    }
    pNetcheck->SignalNetCheckThread();
}

void StaStateMachine::DealGetDhcpIpTimeout(InternalMessage *msg)
{
    if (msg == nullptr) {
        LOGE("DealGetDhcpIpTimeout InternalMessage msg is null.");
        return;
    }
    LOGI("StopTimer CMD_START_GET_DHCP_IP_TIMEOUT DealGetDhcpIpTimeout");
    StopTimer(static_cast<int>(CMD_START_GET_DHCP_IP_TIMEOUT));
    DisConnectProcess();
}

void StaStateMachine::DealScreenStateChangedEvent(InternalMessage *msg)
{
    if (msg == nullptr) {
        WIFI_LOGE("DealScreenStateChangedEvent InternalMessage msg is null.");
        return;
    }

    int screenState = msg->GetParam1();
    WIFI_LOGI("DealScreenStateChangedEvent, Receive msg: screenState=%{public}d", screenState);
    if (screenState == MODE_STATE_OPEN) {
        enableSignalPoll = true;
        StartTimer(static_cast<int>(CMD_SIGNAL_POLL), 0);
        pNetcheck->NetWorkCheckSetScreenState(MODE_STATE_OPEN);
    }

    if (screenState == MODE_STATE_CLOSE) {
        enableSignalPoll = false;
        StopTimer(static_cast<int>(CMD_SIGNAL_POLL));
        pNetcheck->NetWorkCheckSetScreenState(MODE_STATE_CLOSE);
    }

    return;
}

void StaStateMachine::DhcpResultNotify::SaveDhcpResult(DhcpResult *dest, DhcpResult *source)
{
    if (dest == nullptr || source == nullptr) {
        LOGE("SaveDhcpResult dest or source is nullptr.");
        return;
    }

    dest->iptype = source->iptype;
    dest->isOptSuc = source->isOptSuc;
    dest->uOptLeasetime = source->uOptLeasetime;
    dest->uAddTime = source->uAddTime;
    dest->uGetTime = source->uGetTime;
    if (strcpy_s(dest->strOptClientId, DHCP_MAX_FILE_BYTES, source->strOptClientId) != EOK) {
        LOGE("SaveDhcpResult strOptClientId strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptServerId, DHCP_MAX_FILE_BYTES, source->strOptServerId) != EOK) {
        LOGE("SaveDhcpResult strOptServerId strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptSubnet, DHCP_MAX_FILE_BYTES, source->strOptSubnet) != EOK) {
        LOGE("SaveDhcpResult strOptSubnet strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptDns1, DHCP_MAX_FILE_BYTES, source->strOptDns1) != EOK) {
        LOGE("SaveDhcpResult strOptDns1 strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptDns2, DHCP_MAX_FILE_BYTES, source->strOptDns2) != EOK) {
        LOGE("SaveDhcpResult strOptDns2 strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptRouter1, DHCP_MAX_FILE_BYTES, source->strOptRouter1) != EOK) {
        LOGE("SaveDhcpResult strOptRouter1 strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptRouter2, DHCP_MAX_FILE_BYTES, source->strOptRouter2) != EOK) {
        LOGE("SaveDhcpResult strOptRouter2 strcpy_s failed!");
        return;
    }
    if (strcpy_s(dest->strOptVendor, DHCP_MAX_FILE_BYTES, source->strOptVendor) != EOK) {
        LOGE("SaveDhcpResult strOptVendor strcpy_s failed!");
        return;
    }
    LOGI("SaveDhcpResult ok, ipType:%{public}d", dest->iptype);
}

/* ------------------ state machine dhcp callback function ----------------- */
StaStateMachine* StaStateMachine::DhcpResultNotify::pStaStateMachine = nullptr;
DhcpResult StaStateMachine::DhcpResultNotify::DhcpIpv4Result;
DhcpResult StaStateMachine::DhcpResultNotify::DhcpIpv6Result;
StaStateMachine::DhcpResultNotify::DhcpResultNotify()
{
}

StaStateMachine::DhcpResultNotify::~DhcpResultNotify()
{
}

void StaStateMachine::DhcpResultNotify::SetStaStateMachine(StaStateMachine *staStateMachine)
{
    StaStateMachine::DhcpResultNotify::pStaStateMachine = staStateMachine;
}

void StaStateMachine::DhcpResultNotify::OnSuccess(int status, const char *ifname, DhcpResult *result)
{
    if (ifname == nullptr || result == nullptr || pStaStateMachine == nullptr) {
        LOGE("StaStateMachine DhcpResultNotify OnSuccess ifname or result is nullptr.");
        return;
    }
    LOGI("Enter Sta DhcpResultNotify OnSuccess. ifname=[%{public}s] status=[%{public}d]", ifname, status);
    LOGI("iptype=%{public}d, isOptSuc=%{public}d, clientip =%{private}s, serverip=%{private}s, subnet=%{private}s",
        result->iptype, result->isOptSuc, result->strOptClientId,  result->strOptServerId, result->strOptSubnet);
    LOGI("gateway1=%{private}s, gateway2=%{private}s, strDns1=%{private}s, strDns2=%{private}s, strVendor=%{public}s, \
        uOptLeasetime=%{public}d, uAddTime=%{public}d, uGetTime=%{public}d, currentTpType=%{public}d",
        result->strOptRouter1, result->strOptRouter2, result->strOptDns1, result->strOptDns2, result->strOptVendor,
        result->uOptLeasetime, result->uAddTime, result->uGetTime, pStaStateMachine->currentTpType);

    WriteWifiConnectFailedEventHiSysEvent(static_cast<int>(WifiOperateState::STA_DHCP_SUCCESS));
    WriteWifiOperateStateHiSysEvent(static_cast<int>(WifiOperateType::STA_DHCP),
        static_cast<int>(WifiOperateState::STA_DHCP_SUCCESS));
    if (result->iptype == 0) { /* 0-ipv4,1-ipv6 */
        LOGI("StopTimer CMD_START_GET_DHCP_IP_TIMEOUT OnSuccess");
        pStaStateMachine->StopTimer(static_cast<int>(CMD_START_GET_DHCP_IP_TIMEOUT));
        StaStateMachine::DhcpResultNotify::SaveDhcpResult(&(StaStateMachine::DhcpResultNotify::DhcpIpv4Result), result);
    } else {
        StaStateMachine::DhcpResultNotify::SaveDhcpResult(&(StaStateMachine::DhcpResultNotify::DhcpIpv6Result), result);
    }
    pStaStateMachine->OnDhcpResultNotifyEvent(DhcpReturnCode::DHCP_RESULT, result->iptype);
}

void StaStateMachine::DhcpResultNotify::DealDhcpResult(int ipType)
{
    DhcpResult *result = nullptr;
    IpInfo ipInfo;
    IpV6Info ipv6Info;
    WifiSettings::GetInstance().GetIpInfo(ipInfo, pStaStateMachine->GetInstanceId());
    WifiSettings::GetInstance().GetIpv6Info(ipv6Info, pStaStateMachine->GetInstanceId());
    if (ipType == 0) { /* 0-ipv4,1-ipv6 */
        result = &(StaStateMachine::DhcpResultNotify::DhcpIpv4Result);
        TryToSaveIpV4Result(ipInfo, ipv6Info, result);
    } else {
        result = &(StaStateMachine::DhcpResultNotify::DhcpIpv6Result);
        TryToSaveIpV6Result(ipInfo, ipv6Info, result);
    }
    TryToCloseDhcpClient(result->iptype);

    WifiDeviceConfig config;
    AssignIpMethod assignMethod = AssignIpMethod::DHCP;
    int ret = WifiSettings::GetInstance().GetDeviceConfig(pStaStateMachine->linkedInfo.networkId, config);
    if (ret == 0) {
        assignMethod = config.wifiIpConfig.assignMethod;
    }
    LOGI("DhcpResultNotify OnSuccess, uLeaseTime=%{public}d %{public}d %{public}d", result->uOptLeasetime, assignMethod,
        pStaStateMachine->currentTpType);
    if ((assignMethod == AssignIpMethod::DHCP) && (result->uOptLeasetime > 0) &&
        (pStaStateMachine->currentTpType != IPTYPE_IPV6)) {
        if (result->uOptLeasetime < STA_RENEWAL_MIN_TIME) {
            result->uOptLeasetime = STA_RENEWAL_MIN_TIME;
        }
        int64_t interval = result->uOptLeasetime / 2 * TIME_USEC_1000; // s->ms
        LOGI("StartTimer CMD_START_RENEWAL_TIMEOUT uOptLeasetime=%{public}d", result->uOptLeasetime);
        pStaStateMachine->StartTimer(static_cast<int>(CMD_START_RENEWAL_TIMEOUT), interval);
    }

    if (WifiSupplicantHalInterface::GetInstance().WpaSetPowerMode(false) != WIFI_IDL_OPT_OK) {
        LOGE("DhcpResultNotify OnSuccess WpaSetPowerMode() failed!");
    }
    return;
}

void StaStateMachine::DhcpResultNotify::TryToSaveIpV4Result(IpInfo &ipInfo, IpV6Info &ipv6Info, DhcpResult *result)
{
    if (result == nullptr) {
        LOGE("TryToSaveIpV4Result resultis nullptr.");
        return;
    }

    if (!((IpTools::ConvertIpv4Address(result->strOptClientId) == ipInfo.ipAddress) &&
        (IpTools::ConvertIpv4Address(result->strOptRouter1) == ipInfo.gateway))) {
        if (result->iptype == 0) {  /* 0-ipv4,1-ipv6 */
            ipInfo.ipAddress = IpTools::ConvertIpv4Address(result->strOptClientId);
            ipInfo.gateway = IpTools::ConvertIpv4Address(result->strOptRouter1);
            ipInfo.netmask = IpTools::ConvertIpv4Address(result->strOptSubnet);
            ipInfo.primaryDns = IpTools::ConvertIpv4Address(result->strOptDns1);
            ipInfo.secondDns = IpTools::ConvertIpv4Address(result->strOptDns2);
            ipInfo.serverIp = IpTools::ConvertIpv4Address(result->strOptServerId);
            ipInfo.leaseDuration = result->uOptLeasetime;
            WifiSettings::GetInstance().SaveIpInfo(ipInfo);

            pStaStateMachine->linkedInfo.ipAddress = IpTools::ConvertIpv4Address(result->strOptClientId);
            /* If not phone hotspot, set .isDataRestricted = 0. */
            std::string strVendor = result->strOptVendor;
            std::string ipAddress = result->strOptClientId;
            pStaStateMachine->linkedInfo.isDataRestricted = 
                (strVendor.find("ANDROID_METERED") == std::string::npos && 
                strVendor.find("OPEN_HARMONY") == std::string::npos) ? 0 : 1;
            if (!pStaStateMachine->linkedInfo.isDataRestricted) {
                pStaStateMachine->linkedInfo.isDataRestricted =
                    (strVendor.find("hostname:") != std::string::npos &&
                    ipAddress.find("172.20.10.") != std::string::npos);
            }
            pStaStateMachine->linkedInfo.platformType = strVendor;
            WIFI_LOGI("WifiLinkedInfo.isDataRestricted = %{public}d, WifiLinkedInfo.platformType = %{public}s",
                pStaStateMachine->linkedInfo.isDataRestricted, pStaStateMachine->linkedInfo.platformType.c_str());
            WifiSettings::GetInstance().SaveLinkedInfo(pStaStateMachine->linkedInfo);
#ifndef OHOS_ARCH_LITE
            LOGI("TryToSaveIpV4Result Update NetLink info, strYourCli=%{private}s, strSubnet=%{private}s, \
                strRouter1=%{private}s, strDns1=%{private}s, strDns2=%{private}s",
                IpAnonymize(result->strOptClientId).c_str(), IpAnonymize(result->strOptSubnet).c_str(),
                IpAnonymize(result->strOptRouter1).c_str(), IpAnonymize(result->strOptDns1).c_str(),
                IpAnonymize(result->strOptDns2).c_str());
            WIFI_LOGI("On dhcp success update net linke info");
            WifiDeviceConfig config;
            WifiSettings::GetInstance().GetDeviceConfig(pStaStateMachine->linkedInfo.networkId, config);
            WifiNetAgent::GetInstance().OnStaMachineUpdateNetLinkInfo(ipInfo, ipv6Info, config.wifiProxyconfig,
                pStaStateMachine->GetInstanceId());
#endif
        }
#ifdef OHOS_ARCH_LITE
        IfConfig::GetInstance().SetIfDnsAndRoute(result, result->iptype, pStaStateMachine->GetInstanceId());
#endif
    }
}

void StaStateMachine::DhcpResultNotify::TryToSaveIpV6Result(IpInfo &ipInfo, IpV6Info &ipv6Info, DhcpResult *result)
{
    if (result == nullptr) {
        LOGE("TryToSaveIpV6Result resultis nullptr.");
        return;
    }
    if (result->iptype == 1 &&  /* 0-ipv4,1-ipv6 */
        (ipv6Info.globalIpV6Address != result->strOptClientId || ipv6Info.gateway != result->strOptRouter1)) {
        ipv6Info.linkIpV6Address = "";
        ipv6Info.globalIpV6Address = result->strOptClientId;
        ipv6Info.randGlobalIpV6Address = "";
        ipv6Info.gateway = result->strOptRouter1;
        ipv6Info.netmask = result->strOptSubnet;
        ipv6Info.primaryDns = result->strOptRouter1;
        ipv6Info.secondDns = result->strOptRouter2;
        WifiSettings::GetInstance().SaveIpV6Info(ipv6Info, pStaStateMachine->GetInstanceId());
        WIFI_LOGI("SaveIpV6 addr=%{private}s, gateway=%{private}s, mask=%{private}s, dns=%{private}s, dns2=%{private}s",
            ipv6Info.globalIpV6Address.c_str(), ipv6Info.gateway.c_str(), ipv6Info.netmask.c_str(),
            ipv6Info.primaryDns.c_str(), ipv6Info.secondDns.c_str());
#ifndef OHOS_ARCH_LITE
        WifiDeviceConfig config;
        WifiSettings::GetInstance().GetDeviceConfig(pStaStateMachine->linkedInfo.networkId, config);
        WifiNetAgent::GetInstance().OnStaMachineUpdateNetLinkInfo(ipInfo, ipv6Info, config.wifiProxyconfig,
            pStaStateMachine->GetInstanceId());
#endif
    }
}

void StaStateMachine::DhcpResultNotify::TryToCloseDhcpClient(int iptype)
{
    std::string ifname = IF_NAME + std::to_string(pStaStateMachine->GetInstanceId());
    if (iptype == 1) {
        StopDhcpClient(ifname.c_str(), true);
        LOGI("TryToCloseDhcpClient iptype ipv6 return, StopDhcpClient ipv6");
        return;
    }

    WIFI_LOGI("TryToCloseDhcpClient, getIpSucNum=%{public}d, isRoam=%{public}d",
        pStaStateMachine->getIpSucNum, pStaStateMachine->isRoam);
    pStaStateMachine->OnDhcpResultNotifyEvent(DhcpReturnCode::DHCP_JUMP);
    if (pStaStateMachine->getIpSucNum == 0 || pStaStateMachine->isRoam) {
        pStaStateMachine->SaveDiscReason(DisconnectedReason::DISC_REASON_DEFAULT);
        pStaStateMachine->SaveLinkstate(ConnState::CONNECTED, DetailedState::CONNECTED);
        pStaStateMachine->InvokeOnStaConnChanged(
            OperateResState::CONNECT_AP_CONNECTED, pStaStateMachine->linkedInfo);
        WriteWifiConnectionHiSysEvent(WifiConnectionType::CONNECT, "");
        /* Delay to wait for the network adapter information to take effect. */
        constexpr int NETCHECK_DELAY_TIME = 2000; // 2000 ms
        pStaStateMachine->portalFlag = false;
        pStaStateMachine->netNoWorkNum = 0;
        pStaStateMachine->StartTimer(static_cast<int>(CMD_START_NETCHECK), NETCHECK_DELAY_TIME);
        pStaStateMachine->DealSetStaConnectFailedCount(0, true);
    }
    pStaStateMachine->getIpSucNum++;

    StopDhcpClient(ifname.c_str(), false);
    LOGI("TryToCloseDhcpClient, stop dhcp ipv4 client, getIpSucNum=%{public}d", pStaStateMachine->getIpSucNum);
}

void StaStateMachine::DhcpResultNotify::OnFailed(int status, const char *ifname, const char *reason)
{
    // for dhcp: 4-DHCP_OPT_RENEW_FAILED  5-DHCP_OPT_RENEW_TIMEOUT
    if ((status == DHCP_RENEW_FAILED) || (status == DHCP_RENEW_TIMEOUT)) {
        LOGI("DhcpResultNotify::OnFailed, ifname[%{public}s], status[%{public}d], reason[%{public}s]", ifname, status,
            reason);
        pStaStateMachine->OnDhcpResultNotifyEvent(DhcpReturnCode::DHCP_RENEW_FAIL);
        return;
    }
    LOGI("Enter DhcpResultNotify::OnFailed. ifname=%{public}s, status=%{public}d, reason=%{public}s, state=%{public}d",
        ifname, status, reason, static_cast<int>(pStaStateMachine->linkedInfo.detailedState));
    WriteWifiConnectFailedEventHiSysEvent(static_cast<int>(WifiOperateState::STA_DHCP_FAIL));
    pStaStateMachine->OnDhcpResultNotifyEvent(DhcpReturnCode::DHCP_FAIL);
}

void StaStateMachine::DhcpResultNotify::DealDhcpResultFailed()
{
    pStaStateMachine->StopTimer(static_cast<int>(CMD_START_GET_DHCP_IP_TIMEOUT));

    LOGI("DhcpResultNotify OnFailed type: %{public}d, sucNum: %{public}d, failNum: %{public}d, isRoam: %{public}d",
        pStaStateMachine->currentTpType, pStaStateMachine->getIpSucNum,
        pStaStateMachine->getIpFailNum, pStaStateMachine->isRoam);

    if (pStaStateMachine->getIpFailNum == 0) {
        pStaStateMachine->InvokeOnStaConnChanged(OperateResState::CONNECT_OBTAINING_IP_FAILED,
            pStaStateMachine->linkedInfo);
        pStaStateMachine->DisConnectProcess();
        pStaStateMachine->SaveLinkstate(ConnState::DISCONNECTED, DetailedState::OBTAINING_IPADDR_FAIL);
        pStaStateMachine->InvokeOnStaConnChanged(OperateResState::DISCONNECT_DISCONNECTED,
            pStaStateMachine->linkedInfo);
        WriteWifiConnectionHiSysEvent(WifiConnectionType::DISCONNECT, "");
    }
    pStaStateMachine->getIpFailNum++;
}


/* ------------------ state machine Comment function ----------------- */
void StaStateMachine::SaveDiscReason(DisconnectedReason discReason)
{
    WifiSettings::GetInstance().SaveDisconnectedReason(discReason, m_instId);
}

void StaStateMachine::SaveLinkstate(ConnState state, DetailedState detailState)
{
    linkedInfo.connState = state;
    linkedInfo.detailedState = detailState;
    lastLinkedInfo.connState = state;
    lastLinkedInfo.detailedState = detailState;
    linkedInfo.isAncoConnected = WifiConfigCenter::GetInstance().GetWifiConnectedMode(m_instId);
    lastLinkedInfo.isAncoConnected = linkedInfo.isAncoConnected;
    WifiSettings::GetInstance().SaveLinkedInfo(linkedInfo, m_instId);
}

int StaStateMachine::GetLinkedInfo(WifiLinkedInfo& linkedInfo)
{
    return WifiSettings::GetInstance().GetLinkedInfo(linkedInfo, m_instId);
}

void StaStateMachine::SetOperationalMode(int mode)
{
    SendMessage(WIFI_SVR_CMD_STA_OPERATIONAL_MODE, mode, 0);
}

#ifndef OHOS_ARCH_LITE
void StaStateMachine::OnNetManagerRestart(void)
{
    LOGI("OnNetManagerRestart()");
    int state = WifiSettings::GetInstance().GetWifiState(m_instId);
    if (state != static_cast<int>(WifiState::ENABLED)) {
        return;
    }
    m_NetWorkState->StartNetStateObserver();
    WifiNetAgent::GetInstance().OnStaMachineNetManagerRestart(NetSupplierInfo, m_instId);
}

void StaStateMachine::ReUpdateNetSupplierInfo(sptr<NetManagerStandard::NetSupplierInfo> supplierInfo)
{
    LOGI("ReUpdateNetSupplierInfo()");
    WifiLinkedInfo linkedInfo;
    WifiSettings::GetInstance().GetLinkedInfo(linkedInfo, m_instId);
    if ((linkedInfo.detailedState == DetailedState::NOTWORKING) && (linkedInfo.connState == ConnState::CONNECTED)) {
        if (supplierInfo != nullptr) {
            TimeStats timeStats("Call UpdateNetSupplierInfo");
            WifiNetAgent::GetInstance().UpdateNetSupplierInfo(supplierInfo);
        }
    }
}

void StaStateMachine::ReUpdateNetLinkInfo(const WifiDeviceConfig &config)
{
    WifiLinkedInfo linkedInfo;
    WifiSettings::GetInstance().GetLinkedInfo(linkedInfo, m_instId);
    LOGI("ReUpdateNetLinkInfo, detailedState:%{public}d, connState:%{public}d",
        linkedInfo.detailedState, linkedInfo.connState);
    if ((linkedInfo.detailedState == DetailedState::NOTWORKING) && (linkedInfo.connState == ConnState::CONNECTED)
        && (linkedInfo.ssid == config.ssid) && (linkedInfo.bssid == config.bssid)) {
        IpInfo wifiIpInfo;
        WifiSettings::GetInstance().GetIpInfo(wifiIpInfo, m_instId);
        IpV6Info wifiIpV6Info;
        WifiSettings::GetInstance().GetIpv6Info(wifiIpV6Info, m_instId);
        WifiDeviceConfig config;
        WifiSettings::GetInstance().GetDeviceConfig(linkedInfo.networkId, config);
        WifiNetAgent::GetInstance().UpdateNetLinkInfo(wifiIpInfo, wifiIpV6Info, config.wifiProxyconfig, m_instId);
    }
}

void StaStateMachine::SaveWifiConfigForUpdate(int networkId)
{
    WIFI_LOGI("Enter SaveWifiConfigForUpdate.");
    WifiDeviceConfig config;
    if (WifiSettings::GetInstance().GetDeviceConfig(networkId, config) == -1) {
        WIFI_LOGE("SaveWifiConfigForUpdate, get current config failed.");
        return;
    }
}
#endif // OHOS_ARCH_LITE

void StaStateMachine::DealRenewalTimeout(InternalMessage *msg)
{
    if (msg == nullptr) {
        LOGE("DealRenewalTimeout InternalMessage msg is null.");
        return;
    }
    LOGI("StopTimer CMD_START_RENEWAL_TIMEOUT DealRenewalTimeout");
    StopTimer(static_cast<int>(CMD_START_RENEWAL_TIMEOUT));
    StartDhcpRenewal(); // start renewal
}

void StaStateMachine::StartDhcpRenewal()
{
    WIFI_LOGI("enter StartDhcpRenewal!");
    WifiLinkedInfo linkedInfo;
    GetLinkedInfo(linkedInfo);
    if (linkedInfo.connState != ConnState::CONNECTED) {
        WIFI_LOGE("StartDhcpRenewal network is not connected, connState:%{public}d", linkedInfo.connState);
        return;
    }

    std::string ifname = IF_NAME + std::to_string(m_instId);
    int dhcpRet = RenewDhcpClient(ifname.c_str());
    if (dhcpRet != 0) {
        WIFI_LOGE("StartDhcpRenewal dhcp renew failed, dhcpRet:%{public}d", dhcpRet);
    } else {
        WIFI_LOGI("StartDhcpRenewal dhcp renew success.");
    }
}

WifiDeviceConfig StaStateMachine::getCurrentWifiDeviceConfig()
{
    WifiDeviceConfig wifiDeviceConfig;
    WifiSettings::GetInstance().GetDeviceConfig(linkedInfo.networkId, wifiDeviceConfig);
    return wifiDeviceConfig;
}

void StaStateMachine::InsertOrUpdateNetworkStatusHistory(const NetworkStatus &networkStatus)
{
    WifiDeviceConfig &&wifiDeviceConfig = getCurrentWifiDeviceConfig();
    if (networkStatusHistoryInserted) {
        NetworkStatusHistoryManager::Update(wifiDeviceConfig.networkStatusHistory, networkStatus);
        WIFI_LOGI("After updated, current network status history is %{public}s.",
                  NetworkStatusHistoryManager::ToString(wifiDeviceConfig.networkStatusHistory).c_str());
    } else {
        NetworkStatusHistoryManager::Insert(wifiDeviceConfig.networkStatusHistory, networkStatus);
        networkStatusHistoryInserted = true;
        WIFI_LOGI("After inserted, current network status history is %{public}s.",
                  NetworkStatusHistoryManager::ToString(wifiDeviceConfig.networkStatusHistory).c_str());
    }
    if (networkStatus == NetworkStatus::PORTAL) {
        wifiDeviceConfig.isPortal = true;
        wifiDeviceConfig.noInternetAccess = true;
    }
    if (networkStatus == NetworkStatus::HAS_INTERNET) {
        wifiDeviceConfig.lastHasInternetTime = time(0);
        wifiDeviceConfig.noInternetAccess = false;
    }
    if (networkStatus == NetworkStatus::NO_INTERNET) {
        wifiDeviceConfig.noInternetAccess = true;
    }
    WifiSettings::GetInstance().AddDeviceConfig(wifiDeviceConfig);
    WifiSettings::GetInstance().SyncDeviceConfig();
}

int StaStateMachine::GetInstanceId()
{
    return m_instId;
}
} // namespace Wifi
} // namespace OHOS
