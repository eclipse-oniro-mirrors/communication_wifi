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

#include "wifi_p2p_callback_stub.h"
#include "define.h"
#include "wifi_manager_service_ipc_interface_code.h"
#include "wifi_errcode.h"
#include "wifi_hisysevent.h"
#include "wifi_logger.h"
#include "wifi_p2p_msg.h"

DEFINE_WIFILOG_P2P_LABEL("WifiP2pCallbackStub");
namespace OHOS {
namespace Wifi {
WifiP2pCallbackStub::WifiP2pCallbackStub() : userCallback_(nullptr), mRemoteDied(false)
{
    InitHandleMap();
}

WifiP2pCallbackStub::~WifiP2pCallbackStub()
{}

void WifiP2pCallbackStub::InitHandleMap()
{
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_P2P_STATE_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pStateChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_PERSISTENT_GROUPS_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pPersistentGroupsChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_THIS_DEVICE_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pThisDeviceChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_PEER_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pPeersChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_SERVICE_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pServicesChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_CONNECT_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pConnectionChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_DISCOVERY_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pDiscoveryChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_P2P_ACTION_RESULT)] =
        &WifiP2pCallbackStub::RemoteOnP2pActionResult;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_CFG_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnConfigChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_P2P_GC_JOIN_GROUP)] =
        &WifiP2pCallbackStub::RemoteOnP2pGcJoinGroup;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_P2P_GC_LEAVE_GROUP)] =
        &WifiP2pCallbackStub::RemoteOnP2pGcLeaveGroup;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_PRIVATE_PEER_CHANGE)] =
        &WifiP2pCallbackStub::RemoteOnP2pPrivatePeersChanged;
    handleFuncMap[static_cast<uint32_t>(P2PInterfaceCode::WIFI_CBK_CMD_CHR_ERRCODE_REPORT)] =
        &WifiP2pCallbackStub::RemoteOnP2pChrErrCodeReport;
    return;
}

int WifiP2pCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    WIFI_LOGD("WifiP2pCallbackStub::OnRemoteRequest code:%{public}u!", code);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WIFI_LOGE("P2p callback stub token verification error: %{public}d", code);
        return WIFI_OPT_FAILED;
    }

    int exception = data.ReadInt32();
    if (exception) {
        WIFI_LOGD("WifiP2pCallbackStub::OnRemoteRequest exception! %{public}d!", exception);
        return WIFI_OPT_FAILED;
    }

    HandleFuncMap::iterator iter = handleFuncMap.find(code);
    if (iter == handleFuncMap.end()) {
        WIFI_LOGI("not find function to deal, code %{public}u", code);
    } else {
        (this->*(iter->second))(code, data, reply);
    }
    return 0;
}

void WifiP2pCallbackStub::RegisterCallBack(const sptr<IWifiP2pCallback> &userCallback)
{
    if (userCallback_ != nullptr) {
        WIFI_LOGD("Callback has registered!");
        return;
    }
    userCallback_ = userCallback;
}

bool WifiP2pCallbackStub::IsRemoteDied() const
{
    return mRemoteDied;
}

void WifiP2pCallbackStub::SetRemoteDied(bool val)
{
    WIFI_LOGI("WifiP2pCallbackStub::SetRemoteDied: %{public}d", val);
    mRemoteDied = val;
}

void WifiP2pCallbackStub::OnP2pStateChanged(int state)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pStateChanged: %{public}d", state);
    if (userCallback_) {
        userCallback_->OnP2pStateChanged(state);
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_STATE_CHANGE, state);
}

void WifiP2pCallbackStub::OnP2pPersistentGroupsChanged(void)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pPersistentGroupsChanged");
    if (userCallback_) {
        userCallback_->OnP2pPersistentGroupsChanged();
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_PERSISTENT_GROUP_CHANGE, HISYS_EVENT_DEFAULT_VALUE);
}

void WifiP2pCallbackStub::OnP2pThisDeviceChanged(const WifiP2pDevice &device)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pThisDeviceChanged");
    if (userCallback_) {
        userCallback_->OnP2pThisDeviceChanged(device);
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_DEVICE_STATE_CHANGE, HISYS_EVENT_DEFAULT_VALUE);
}

void WifiP2pCallbackStub::OnP2pPeersChanged(const std::vector<WifiP2pDevice> &device)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pPeersChanged");
    if (userCallback_) {
        userCallback_->OnP2pPeersChanged(device);
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_PEER_DEVICE_CHANGE, HISYS_EVENT_DEFAULT_VALUE);
}

void WifiP2pCallbackStub::OnP2pPrivatePeersChanged(const std::string &priWfdInfo)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pPrivatePeersChanged");
    if (userCallback_) {
        userCallback_->OnP2pPrivatePeersChanged(priWfdInfo);
    }
}

void WifiP2pCallbackStub::OnP2pServicesChanged(const std::vector<WifiP2pServiceInfo> &srvInfo)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pServicesChanged");
    if (userCallback_) {
        userCallback_->OnP2pServicesChanged(srvInfo);
    }
}

void WifiP2pCallbackStub::OnP2pConnectionChanged(const WifiP2pLinkedInfo &info)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pConnectionChanged: %{public}d", static_cast<int>(info.GetConnectState()));
    if (userCallback_) {
        userCallback_->OnP2pConnectionChanged(info);
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_CONN_STATE_CHANGE, static_cast<int>(info.GetConnectState()));
}

void WifiP2pCallbackStub::OnP2pDiscoveryChanged(bool isChange)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pDiscoveryChanged, isChange:%{public}d", isChange);
    if (userCallback_) {
        userCallback_->OnP2pDiscoveryChanged(isChange);
    }
    WriteWifiEventReceivedHiSysEvent(HISYS_P2P_DISCOVERY_CHANGE, isChange);
}

void WifiP2pCallbackStub::OnP2pActionResult(P2pActionCallback action, ErrCode code)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnP2pActionResult");
    if (userCallback_) {
        userCallback_->OnP2pActionResult(action, code);
    }
}

void WifiP2pCallbackStub::OnConfigChanged(CfgType type, char* data, int dataLen)
{
    WIFI_LOGI("WifiP2pCallbackStub::OnConfigChanged");
    if (userCallback_) {
        userCallback_->OnConfigChanged(type, data, dataLen);
    }
}

void WifiP2pCallbackStub::OnP2pGcJoinGroup(const OHOS::Wifi::GcInfo &info)
{
    WIFI_LOGD("WifiP2pCallbackStub::OnP2pGcJoinGroup");
    if (userCallback_) {
        userCallback_->OnP2pGcJoinGroup(info);
    }
}

void WifiP2pCallbackStub::OnP2pGcLeaveGroup(const OHOS::Wifi::GcInfo &info)
{
    WIFI_LOGD("WifiP2pCallbackStub::OnP2pGcLeaveGroup");
    if (userCallback_) {
        userCallback_->OnP2pGcLeaveGroup(info);
    }
}

void WifiP2pCallbackStub::OnP2pChrErrCodeReport(const int errCode)
{
    WIFI_LOGD("WifiP2pCallbackStub::OnP2pChrErrCodeReport, %{public}d", errCode);
    if (userCallback_) {
        userCallback_->OnP2pChrErrCodeReport(errCode);
    }
}

void WifiP2pCallbackStub::RemoteOnP2pStateChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    int state = data.ReadInt32();
    OnP2pStateChanged(state);
}

void WifiP2pCallbackStub::RemoteOnP2pPersistentGroupsChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    OnP2pPersistentGroupsChanged();
}

void WifiP2pCallbackStub::ReadWifiP2pDeviceData(MessageParcel &data, WifiP2pDevice &device)
{
    device.SetDeviceName(data.ReadString());
    device.SetDeviceAddress(data.ReadString());
    device.SetDeviceAddressType(data.ReadInt32());
    device.SetPrimaryDeviceType(data.ReadString());
    device.SetSecondaryDeviceType(data.ReadString());
    device.SetP2pDeviceStatus(static_cast<P2pDeviceStatus>(data.ReadInt32()));
    WifiP2pWfdInfo wfdInfo;
    wfdInfo.SetWfdEnabled(data.ReadBool());
    wfdInfo.SetDeviceInfo(data.ReadInt32());
    wfdInfo.SetCtrlPort(data.ReadInt32());
    wfdInfo.SetMaxThroughput(data.ReadInt32());
    device.SetWfdInfo(wfdInfo);
    device.SetWpsConfigMethod(data.ReadInt32());
    device.SetDeviceCapabilitys(data.ReadInt32());
    device.SetGroupCapabilitys(data.ReadInt32());
    device.SetChrErrCode(static_cast<P2pChrEvent>(data.ReadInt32()));
}

void WifiP2pCallbackStub::RemoteOnP2pThisDeviceChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    WifiP2pDevice config;
    ReadWifiP2pDeviceData(data, config);
    OnP2pThisDeviceChanged(config);
}

void WifiP2pCallbackStub::RemoteOnP2pPeersChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    constexpr int MAX_DEVICE_SIZE = 512;
    std::vector<WifiP2pDevice> device;
    int size = data.ReadInt32();
    if (size > MAX_DEVICE_SIZE) {
        WIFI_LOGE("Peers change list size error: %{public}d", size);
        return;
    }
    for (int i = 0; i < size; ++i) {
        WifiP2pDevice config;
        ReadWifiP2pDeviceData(data, config);
        device.emplace_back(config);
    }
    OnP2pPeersChanged(device);
}

void WifiP2pCallbackStub::RemoteOnP2pPrivatePeersChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
 
    std::string priWfdInfo = data.ReadString();
    OnP2pPrivatePeersChanged(priWfdInfo);
}

void WifiP2pCallbackStub::RemoteOnP2pServicesChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    const char *readStr = nullptr;
    constexpr int MAX_SIZE = 512;
    constexpr int maxInfoSize = 100;
    std::vector<WifiP2pServiceInfo> srvInfo;
    int size = data.ReadInt32();
    if (size > MAX_SIZE) {
        WIFI_LOGE("Service change size error: %{public}d", size);
        return;
    }
    for (int i = 0; i < size; ++i) {
        WifiP2pServiceInfo info;
        readStr = data.ReadCString();
        info.SetServiceName((readStr != nullptr) ? readStr : "");
        readStr = data.ReadCString();
        info.SetDeviceAddress((readStr != nullptr) ? readStr : "");
        info.SetServicerProtocolType(static_cast<P2pServicerProtocolType>(data.ReadInt32()));
        int length = data.ReadInt32();
        if (length > maxInfoSize) {
            WIFI_LOGE("Data was incompletes. Service info length error: %{public}d", length);
            break;
        }
        std::vector<std::string> queryList;
        for (int j = 0; j < length; j++) {
            readStr = data.ReadCString();
            std::string queryStr = (readStr != nullptr) ? readStr : "";
            queryList.push_back(queryStr);
        }
        info.SetQueryList(queryList);
        srvInfo.emplace_back(info);
    }
    OnP2pServicesChanged(srvInfo);
}

void WifiP2pCallbackStub::RemoteOnP2pConnectionChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    const char *readStr = nullptr;
    WifiP2pLinkedInfo info;
    info.SetConnectState(static_cast<P2pConnectedState>(data.ReadInt32()));
    info.SetIsGroupOwner(data.ReadBool());
    readStr = data.ReadCString();
    info.SetIsGroupOwnerAddress((readStr != nullptr) ? readStr : "");
    OnP2pConnectionChanged(info);
}

void WifiP2pCallbackStub::RemoteOnP2pDiscoveryChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    bool isChange = data.ReadBool();
    OnP2pDiscoveryChanged(isChange);
}

void WifiP2pCallbackStub::RemoteOnP2pActionResult(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    P2pActionCallback action = static_cast<P2pActionCallback>(data.ReadInt32());
    ErrCode errCode = static_cast<ErrCode>(data.ReadInt32());
    OnP2pActionResult(action, errCode);
}

void WifiP2pCallbackStub::RemoteOnConfigChanged(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());

    CfgType cfgType = static_cast<CfgType>(data.ReadInt32());
    int cfgLen = data.ReadInt32();
    if (cfgLen <= 0) {
        WIFI_LOGE("Config change size error: %{public}d", cfgLen);
        return;
    }

    const char *dataBuffer = (const char *)data.ReadBuffer(cfgLen);
    if (dataBuffer == nullptr) {
        WIFI_LOGE("read buffer error!");
        return;
    }

    if (cfgLen > MAX_LEN) {
        WIFI_LOGE("cfgLen size error!");
        return;
    }
    char* cfgData = new (std::nothrow) char[cfgLen];
    if (cfgData == nullptr) {
        WIFI_LOGE("new buffer error!");
        return;
    }
    if (memcpy_s(cfgData, cfgLen, dataBuffer, cfgLen) != EOK) {
        delete[] cfgData;
        cfgData = nullptr;
        WIFI_LOGE("memcpy_s failed!");
        return;
    }
    OnConfigChanged(cfgType, cfgData, cfgLen);
    delete[] cfgData;
    cfgData = nullptr;
}

void WifiP2pCallbackStub::RemoteOnP2pGcJoinGroup(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    GcInfo info;
    info.mac = data.ReadString();
    info.ip = data.ReadString();
    info.host = data.ReadString();
    OnP2pGcJoinGroup(info);
}

void WifiP2pCallbackStub::RemoteOnP2pGcLeaveGroup(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    GcInfo info;
    info.mac = data.ReadString();
    info.ip = data.ReadString();
    info.host = data.ReadString();
    OnP2pGcLeaveGroup(info);
}

void WifiP2pCallbackStub::RemoteOnP2pChrErrCodeReport(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    WIFI_LOGD("run %{public}s code %{public}u, datasize %{public}zu", __func__, code, data.GetRawDataSize());
    int errCode = data.ReadInt32();
    OnP2pChrErrCodeReport(errCode);
}
}  // namespace Wifi
}  // namespace OHOS