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
#include "wifi_common_util.h"
#include "cJSON.h"

namespace OHOS {
namespace WifiCli {
enum class CliErrCode {
    SUCCESS = 0,
    FAILED = 1,
};

struct ErrMsg {
    ErrMsg(std::string c, std::string m)
        : ErrMsg(std::move(c), std::move(m), {}) {}

    ErrMsg(std::string c, std::string m, std::string s)
        : code(std::move(c)),
          errMsg(std::move(m)),
          suggestion(std::move(s)) {}

    std::string code;
    std::string errMsg;
    std::string suggestion;
};

void OutputSuccessJson(cJSON* data);
void OutputErrorJson(const ErrMsg& errMessage);
int RunCommand(int argc, char** argv);

} // namespace WifiCli
} // namespace OHOS

#endif // OHOS_WIFI_MANAGER_CLI_H
