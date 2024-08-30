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

#include "sta_interface.h"
#include "sta_service.h"
#include "wifi_logger.h"

DEFINE_WIFILOG_LABEL("StaInterface");

namespace OHOS {
namespace Wifi {
StaInterface::StaInterface(int instId) : pStaService(nullptr), m_instId(instId)
{}

StaInterface::~StaInterface()
{
    WIFI_LOGI("~StaInterface");
}

ErrCode StaInterface::EnableStaService()
{
    WIFI_LOGI("Enter EnableStaService.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DisableStaService()
{
    LOGI("Enter DisableStaService.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectToNetwork(int networkId)
{
    LOGI("Enter Connect.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectToDevice(const WifiDeviceConfig &config)
{
    LOGI("Enter Connect.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::StartRoamToNetwork(const int networkId, const std::string bssid)
{
    LOGD("Enter StartRoamToNetwork");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::StartConnectToUserSelectNetwork(int networkId, std::string bssid)
{
    LOGD("Enter StartConnectToUserSelectNetwork");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ReConnect()
{
    LOGI("Enter ReConnect.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ReAssociate()
{
    LOGI("Enter ReAssociate.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::Disconnect()
{
    LOGI("Enter Disconnect.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::AddCandidateConfig(const int uid, const WifiDeviceConfig &config, int& netWorkId)
{
    LOGI("Enter AddCandidateConfig.\n");
        return WIFI_OPT_SUCCESS;
}
ErrCode StaInterface::ConnectToCandidateConfig(const int uid, const int networkId)
{
    LOGI("Enter ConnectToCandidateConfig.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RemoveCandidateConfig(const int uid, const int networkId)
{
    LOGI("Enter RemoveCandidateConfig.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RemoveAllCandidateConfig(const int uid)
{
    LOGI("Enter RemoveAllCandidateConfig.\n");
    return WIFI_OPT_SUCCESS;
}

int StaInterface::AddDeviceConfig(const WifiDeviceConfig &config)
{
    LOGI("Enter AddDeviceConfig.\n");
    return pStaService->AddDeviceConfig(config);
}

int StaInterface::UpdateDeviceConfig(const WifiDeviceConfig &config)
{
    LOGI("Enter UpdateDeviceConfig.\n");
    return pStaService->UpdateDeviceConfig(config);
}

ErrCode StaInterface::RemoveDevice(int networkId)
{
    LOGI("Enter RemoveDeviceConfig.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RemoveAllDevice()
{
    LOGI("Enter RemoveAllDevice.\n");
    return WIFI_OPT_SUCCESS;
}
ErrCode StaInterface::EnableDeviceConfig(int networkId, bool attemptEnable)
{
    LOGI("Enter EnableDeviceConfig.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DisableDeviceConfig(int networkId)
{
    LOGI("Enter DisableDeviceConfig.\n");
    return WIFI_OPT_SUCCESS;
}
ErrCode StaInterface::StartWps(const WpsConfig &config)
{
    LOGI("Enter StartWps.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::CancelWps()
{
    LOGI("Enter CancelWps.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::ConnectivityManager(const std::vector<InterScanInfo> &scanInfos)
{
    LOGI("Enter Connection management.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RegisterStaServiceCallback(const StaServiceCallback &callbacks)
{
    LOGD("Enter RegisterStaServiceCallback.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::UnRegisterStaServiceCallback(const StaServiceCallback &callbacks)
{
    LOGD("Enter UnRegisterStaServiceCallback.\n");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::SetSuspendMode(bool mode)
{
    LOGI("Enter SetSuspendMode, mode=[%{public}d]!", mode);
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::SetPowerMode(bool mode)
{
    LOGI("Enter SetPowerMode, mode=[%{public}d]!", mode);
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::SetTxPower(int power)
{
    LOGI("Enter SetTxPower, power=[%{public}d]!", power);
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::OnSystemAbilityChanged(int systemAbilityid, bool add)
{
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::OnScreenStateChanged(int screenState)
{
    WIFI_LOGI("Enter OnScreenStateChanged, screenState=%{public}d.", screenState);
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DisableAutoJoin(const std::string &conditionName)
{
    LOGI("Enter DisableAutoJoin");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::EnableAutoJoin(const std::string &conditionName)
{
    LOGI("Enter EnableAutoJoin");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RegisterAutoJoinCondition(const std::string &conditionName,
                                                const std::function<bool()> &autoJoinCondition)
{
    LOGI("Enter RegisterAutoJoinCondition");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DeregisterAutoJoinCondition(const std::string &conditionName)
{
    LOGI("Enter DeregisterAutoJoinCondition");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RegisterFilterBuilder(const FilterTag &filterTag,
                                            const std::string &filterName,
                                            const FilterBuilder &filterBuilder)
{
    LOGI("Enter RegisterFilterBuilder");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DeregisterFilterBuilder(const FilterTag &filterTag, const std::string &filterName)
{
    LOGI("Enter DeregisterFilterBuilder");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::RegisterScoreBuilder(const ScoreTag &scoreTag, const std::string &scoreName,
                                           const ScoreBuilder &scoreBuilder)
{
    LOGI("Enter RegisterScoreBuilder");
    return WIFI_OPT_SUCCESS;
}
 
ErrCode StaInterface::DeregisterScoreBuilder(const ScoreTag &scoreTag, const std::string &scoreName)
{
    LOGI("Enter DeregisterScoreBuilder");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::StartPortalCertification()
{
    WIFI_LOGI("Enter StartPortalCertification");
    return WIFI_OPT_SUCCESS;
}

#ifndef OHOS_ARCH_LITE
ErrCode StaInterface::HandleForegroundAppChangedAction(const AppExecFwk::AppStateData &appStateData)
{
    WIFI_LOGD("Enter HandleForegroundAppChangedAction");
    return WIFI_OPT_SUCCESS;
}
#endif

ErrCode StaInterface::EnableHiLinkHandshake(const WifiDeviceConfig &config, const std::string &bssid)
{
    WIFI_LOGI("Enter EnableHiLinkHandshake");
    return WIFI_OPT_SUCCESS;
}

ErrCode StaInterface::DeliverStaIfaceData(const std::string &currentMac)
{
    WIFI_LOGI("Enter DeliverStaIfaceData");
    return WIFI_OPT_SUCCESS;
}

bool StaInterface::InitStaServiceLocked()
{
    return true;
}
}  // namespace Wifi
}  // namespace OHOS
