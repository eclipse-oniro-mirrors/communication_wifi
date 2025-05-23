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
#include "wifi_permission_utils.h"
#ifndef OHOS_ARCH_LITE
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#endif
#include "wifi_auth_center.h"

namespace OHOS {
namespace Wifi {
#ifdef OHOS_ARCH_LITE
int WifiPermissionUtils::VerifySetWifiInfoPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiInfoPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifySameProcessPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyWifiConnectionPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetScanInfosPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyManageEdmPolicyPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiLocalMacPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifySetWifiConfigPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiConfigPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiDirectDevicePermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyManageWifiHotspotPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiPeersMacPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiInfoInternalPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyGetWifiPeersMacPermissionEx(const int &pid, const int &uid, const int &tokenId)
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyManageWifiHotspotExtPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::VerifyEnterpriseWifiConnectionPermission()
{
    return PERMISSION_GRANTED;
}

int WifiPermissionUtils::GetApiVersion()
{
    return API_VERSION_INVALID;
}

#else
using namespace OHOS::Security::AccessToken;

int WifiPermissionUtils::VerifySetWifiInfoPermission()
{
    return WifiAuthCenter::GetInstance().VerifySetWifiInfoPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiInfoPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiInfoPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifySameProcessPermission()
{
    return WifiAuthCenter::GetInstance().VerifySameProcessPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyWifiConnectionPermission()
{
    return WifiAuthCenter::GetInstance().VerifyWifiConnectionPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetScanInfosPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetScanInfosPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiLocalMacPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiLocalMacPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifySetWifiConfigPermission()
{
    return WifiAuthCenter::GetInstance().VerifySetWifiConfigPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyManageEdmPolicyPermission()
{
    return WifiAuthCenter::GetInstance().VerifyManageEdmPolicyPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiConfigPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiConfigPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiDirectDevicePermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiDirectDevicePermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyManageWifiHotspotPermission()
{
    return WifiAuthCenter::GetInstance().VerifyManageWifiHotspotPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiPeersMacPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiPeersMacPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyGetWifiPeersMacPermissionEx(const int &pid, const int &uid, const int &tokenId)
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiPeersMacPermissionEx(pid, uid, tokenId);
}

int WifiPermissionUtils::VerifyGetWifiInfoInternalPermission()
{
    return WifiAuthCenter::GetInstance().VerifyGetWifiInfoInternalPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyManageWifiHotspotExtPermission()
{
    return WifiAuthCenter::GetInstance().VerifyManageWifiHotspotExtPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::VerifyEnterpriseWifiConnectionPermission()
{
    return WifiAuthCenter::GetInstance().VerifyEnterpriseWifiConnectionPermission(
        IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid());
}

int WifiPermissionUtils::GetApiVersion()
{
    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    ATokenTypeEnum callingType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (callingType != ATokenTypeEnum::TOKEN_HAP) {
        return API_VERSION_INVALID;
    }
    HapTokenInfo hapTokenInfo;
    if (AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo) != AccessTokenKitRet::RET_SUCCESS) {
        return API_VERSION_INVALID;
    }
    return API_VERSION_INVALID;
}
#endif
}  // namespace Wifi
}  // namespace OHOS