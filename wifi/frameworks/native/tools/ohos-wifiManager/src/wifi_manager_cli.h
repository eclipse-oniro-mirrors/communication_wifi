/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_WIFI_MANAGER_CLI_H
#define OHOS_WIFI_MANAGER_CLI_H
#include <string>

#include "cJSON.h"
#include "wifi_errcode.h"

namespace OHOS {
namespace WifiCli {

enum class WifiCliErrCode {
    NETWORK_NOT_FOUND = 240,
    BAND_NOT_SUPPORTED,
    SSID_INVALID,
    PSK_INVALID,
    BAND_INVALID,
    MISSING_PARAM,
};

void OutputSuccessJson(cJSON* data);
void OutputErrorJson(OHOS::Wifi::ErrCode code);
int RunCommand(int argc, char** argv);
void ClearString(std::string& str);

} // namespace WifiCli
} // namespace OHOS

#endif // OHOS_WIFI_MANAGER_CLI_H
