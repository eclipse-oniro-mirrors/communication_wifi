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

#include "wifi_protect.h"
#ifndef OHOS_ARCH_LITE
#include "wifi_log.h"

#undef LOG_TAG
#define LOG_TAG "OHWIFI_MANAGER_PROTECT"
#endif

namespace OHOS {
namespace Wifi {
WifiProtect::WifiProtect(
    const WifiProtectType &protectType, const WifiProtectMode &protectMode, const std::string &name)
    : mName(name),
      mType(protectType),
      mMode(protectMode),
      mAcqTimestamp(0)
{}

WifiProtect::WifiProtect(const std::string &name)
    : mName(name),
      mType(WifiProtectType::WIFI_PROTECT_COMMON),
      mMode(WifiProtectMode::WIFI_PROTECT_NO_HELD),
      mAcqTimestamp(0)
{}

WifiProtect::WifiProtect()
    : mName(""),
      mType(WifiProtectType::WIFI_PROTECT_COMMON),
      mMode(WifiProtectMode::WIFI_PROTECT_NO_HELD),
      mAcqTimestamp(0)
{}

WifiProtect::~WifiProtect()
{}

void WifiProtect::SetProtectType(const WifiProtectType &protectType)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mType = protectType;
}

WifiProtectType WifiProtect::GetProtectType() const
{
    return mType;
}

void WifiProtect::SetProtectMode(const WifiProtectMode &protectMode)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mMode = protectMode;
}

WifiProtectMode WifiProtect::GetProtectMode() const
{
    return mMode;
}

void WifiProtect::SetName(const std::string &name)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mName = name;
}

std::string WifiProtect::GetName() const
{
    return mName;
}

long WifiProtect::GetAcqTimestamp() const
{
    return mAcqTimestamp;
}
#ifndef OHOS_ARCH_LITE
void WifiProtect::SetAppState(int state)
{
    std::unique_lock<std::mutex> lock(mMutex);
    mAppState = state;
}

int WifiProtect::GetAppState() const
{
    return mAppState;
}
#endif
}  // namespace Wifi
}  // namespace OHOS
