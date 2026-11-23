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
#include "securec.h"
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
int SetStub(char *value, uint32_t len, const char *text)
{
    return memcpy_s(value, len, text, strlen(text)) == EOK ? 1 : 0;
}
} // namespace

int GetParamValue(const char *key, const char *, char *value, uint32_t len)
{
    if (strcasecmp(key, WIFI_COUNTRY_CODE_CONFIG) == 0) {
        WIFI_LOGI("GetParamValue WIFI_COUNTRY_CODE_CONFIG");
        return SetStub(value, len, "31");
    } else if (strcasecmp(key, MDM_AP_WHITE_LIST_PARAM) == 0) {
        WIFI_LOGI("GetParamValue MDM_AP_WHITE_LIST_PARAM");
        return SetStub(value, len, "true");
    } else if (strcasecmp(key, WIFI_COUNTRY_CODE_RUN_MODE) == 0) {
        WIFI_LOGI("GetParamValue WIFI_COUNTRY_CODE_RUN_MODE");
        return SetStub(value, len, "factory");
    } else if (strcasecmp(key, FACTORY_WIFI_COUNTRY_CODE) == 0) {
        WIFI_LOGI("GetParamValue FACTORY_WIFI_COUNTRY_CODE");
        return SetStub(value, len, "US");
    } else if (strcasecmp(key, OPERATOR_NUMERIC_KEY) == 0) {
        WIFI_LOGI("GetParamValue OPERATOR_NUMERIC_KEY");
        return SetStub(value, len, "4600000");
    } else if (strcasecmp(key, WIFI_COUNTRY_CODE_DYNAMIC_UPDATE_KEY) == 0) {
        WIFI_LOGI("GetParamValue WIFI_COUNTRY_CODE_DYNAMIC_UPDATE_KEY");
        return SetStub(value, len, "CN");
    } else if (strcasecmp(key, DEFAULT_REGION_KEY) == 0) {
        WIFI_LOGI("GetParamValue DEFAULT_REGION_KEY");
        return SetStub(value, len, "JP");
    }
    return 0;
}
}
}