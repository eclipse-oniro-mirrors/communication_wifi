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

#ifndef OHOS_WIFI_AUTH_CENTER_H
#define OHOS_WIFI_AUTH_CENTER_H

#include <map>
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
class WifiAuthCenter {
public:
    static WifiAuthCenter &GetInstance();

    /**
     * @Description Init WifiAuthCenter object
     *
     * @return int - init result, when 0 means success, other means some fails happened
     */
    int Init();

    /**
     * @Description Verify whether the app is system app
     *
     * @return bool - true : system app
     */
    static bool IsSystemAccess();

    /**
     * @Description Verify whether the app is native process
     *
     * @return bool - true : native process
     */
    static bool IsNativeProcess();

    /**
     * @Description Verify whether the app has the same process permission
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifySameProcessPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to operator wifi
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifySetWifiInfoPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the edm permission to visit wifi
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyManageEdmPolicyPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to visit wifi
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiInfoPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to get scan info.
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetScanInfosPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to get mac address
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiLocalMacPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to open / close hotspot
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyWifiConnectionPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to override hotspot config, hotspot blocklist config
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifySetWifiConfigPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to get hotspot config, hotspot blocklist config
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiConfigPermission(const int &pid, const int &uid);

    /**
     * @Description : Verify location information about nearby P2P devices Permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiDirectDevicePermission(const int &pid, const int &uid);

    /**
     * @Description : Verify manage wifi hotspot Permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyManageWifiHotspotPermission(const int &pid, const int &uid);

    /**
     * @Description : Get wifi peers mac Permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiPeersMacPermission(const int &pid, const int &uid);

    /**
     * @Description : Get wifi peers mac Permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @param tokenId - Token ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiPeersMacPermissionEx(const int &pid, const int &uid, const int &tokenId);

    /**
     * @Description : Get wifi internal wifi info Permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyGetWifiInfoInternalPermission(const int &pid, const int &uid);

    /**
     * @Description : Verify manage wifi hotspot extend permission.
     *
     * @param pid - Process ID.[in]
     * @param uid - User ID.[in]
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyManageWifiHotspotExtPermission(const int &pid, const int &uid);

    /**
     * @Description Verify whether the app has the permission to open / close hotspot
     *
     * @param pid - the app's process id
     * @param uid - the app id
     * @return int - PERMISSION_DENIED or PERMISSION_GRANTED
     */
    int VerifyEnterpriseWifiConnectionPermission(const int &pid, const int &uid);
private:
    /* system auth service client */
};
} // namespace Wifi
} // namespace OHOS

#endif