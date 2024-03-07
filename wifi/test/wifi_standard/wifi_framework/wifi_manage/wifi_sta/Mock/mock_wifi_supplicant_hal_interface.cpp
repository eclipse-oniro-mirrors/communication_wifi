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
#include "mock_wifi_sta_interface.h"

namespace OHOS {
namespace Wifi {

MockWifiStaInterface &MockWifiStaInterface::GetInstance(void)
{
    static MockWifiStaInterface inst;
    return inst;
}

namespace WifiSupplicantHalInterface {

WifiErrorNo StartSupplicant()
{
    return MockWifiStaInterface::GetInstance().pSupplicant.startSipplicant ? WIFI_IDL_OPT_OK : WIFI_IDL_OPT_FAILED;
}

WifiErrorNo WpaSetCountryCode(const std::string &countryCode)
{
    return MockWifiStaInterface::GetInstance().pSupplicant.wpaSetCountryCode ? WIFI_IDL_OPT_OK : WIFI_IDL_OPT_FAILED;
}

WifiErrorNo WpaSetSuspendMode(bool mode)
{
    return MockWifiStaInterface::GetInstance().pSupplicant.wpaSetSuspendMode ? WIFI_IDL_OPT_OK : WIFI_IDL_OPT_FAILED;
}

WifiErrorNo WpaSetPowerMode(bool mode)
{
    return MockWifiStaInterface::GetInstance().pSupplicant.wpaSetPowerMode ? WIFI_IDL_OPT_OK : WIFI_IDL_OPT_FAILED;
}
};
}  // namespace Wifi
}  // namespace OHOS/ namespace OHOS
