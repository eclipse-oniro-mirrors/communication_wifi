/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "mock_wifi_global_func.h"
#include <cstring>
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("MockWifiGlobalFunc");

constexpr const char* WIFI_COUNTRY_CODE_CONFIG = "const.wifi.country_code.conf";
constexpr const char* WIFI_COUNTRY_CODE_RUN_MODE = "const.wifi.country_code.runmode";
constexpr const char* FACTORY_WIFI_COUNTRY_CODE = "const.wifi.country_code.factory";
constexpr const char* OPERATOR_NUMERIC_KEY = "ril.operator.numeric";  // plmn cached
constexpr const char* WIFI_COUNTRY_CODE_DYNAMIC_UPDATE_KEY = "persist.wifi.country_code.dynamic_update";
constexpr const char* DEFAULT_REGION_KEY = "const.global.region";
constexpr const char* MDM_AP_WHITE_LIST_PARAM = "persist.edm.is_hotspot_client_policy_enabled";

namespace {
struct ParamStub {
    const char *key;
    const char *value;
};

constexpr ParamStub PARAM_STUBS[] = {
    {WIFI_COUNTRY_CODE_CONFIG, "31"},
    {MDM_AP_WHITE_LIST_PARAM, "true"},
    {WIFI_COUNTRY_CODE_RUN_MODE, "factory"},
    {FACTORY_WIFI_COUNTRY_CODE, "US"},
    {OPERATOR_NUMERIC_KEY, "4600000"},
    {WIFI_COUNTRY_CODE_DYNAMIC_UPDATE_KEY, "CN"},
    {DEFAULT_REGION_KEY, "JP"},
};
} // namespace

int GetParamValue(const char *key, [[maybe_unused]] const char *def, char *value, uint32_t len)
{
    for (const auto &stub : PARAM_STUBS) {
        if (strcasecmp(key, stub.key) != 0) {
            continue;
        }
        WIFI_LOGI("GetParamValue %{public}s", stub.key);
        size_t valueLen = strlen(stub.value);
        if (value == nullptr || len < valueLen) {
            return 0;
        }
        memcpy(value, stub.value, valueLen);
        return 1;
    }
    return 0;
}
}
}