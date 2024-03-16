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

#ifndef OHOS_WIFI_LOGGER_H
#define OHOS_WIFI_LOGGER_H

#include <stdint.h>
#ifdef OHOS_ARCH_LITE
#include "hilog/log.h"
#else
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"
#endif

namespace OHOS {
namespace Wifi {

const unsigned int LOG_ID_WIFI = 0xD001560;
const unsigned int LOG_ID_WIFI_HOTSPOT = LOG_ID_WIFI | 0x01;
const unsigned int LOG_ID_WIFI_SCAN = LOG_ID_WIFI | 0x02;
const unsigned int LOG_ID_WIFI_P2P = LOG_ID_WIFI | 0x03;
const unsigned int LOG_ID_WIFI_AWARE = LOG_ID_WIFI | 0x04;
const unsigned int LOG_ID_WIFI_DHCP = LOG_ID_WIFI | 0x05;

#ifdef OHOS_ARCH_LITE

#define DEFINE_WIFILOG_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI, name};
#define DEFINE_WIFILOG_HOTSPOT_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI_HOTSPOT, name};
#define DEFINE_WIFILOG_SCAN_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI_SCAN, name};
#define DEFINE_WIFILOG_P2P_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI_P2P, name};
#define DEFINE_WIFILOG_AWARE_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI_AWARE, name};
#define DEFINE_WIFILOG_DHCP_LABEL(name) \
    static constexpr OHOS::HiviewDFX::HiLogLabel WIFI_LOG_LABEL = {LOG_CORE, OHOS::Wifi::LOG_ID_WIFI_DHCP, name};

#define WIFI_LOGF(...) (void)OHOS::HiviewDFX::HiLog::Fatal(WIFI_LOG_LABEL, ##__VA_ARGS__)
#define WIFI_LOGE(...) (void)OHOS::HiviewDFX::HiLog::Error(WIFI_LOG_LABEL, ##__VA_ARGS__)
#define WIFI_LOGW(...) (void)OHOS::HiviewDFX::HiLog::Warn(WIFI_LOG_LABEL, ##__VA_ARGS__)
#define WIFI_LOGI(...) (void)OHOS::HiviewDFX::HiLog::Info(WIFI_LOG_LABEL, ##__VA_ARGS__)
#define WIFI_LOGD(...) (void)OHOS::HiviewDFX::HiLog::Debug(WIFI_LOG_LABEL, ##__VA_ARGS__)

#else

struct TestLogLable {
    uint32_t dominId;
    const char* tag;
};

#define DEFINE_WIFILOG_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI, name};
#define DEFINE_WIFILOG_HOTSPOT_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI_HOTSPOT, name};
#define DEFINE_WIFILOG_SCAN_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI_SCAN, name};
#define DEFINE_WIFILOG_P2P_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI_P2P, name};
#define DEFINE_WIFILOG_AWARE_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI_AWARE, name};
#define DEFINE_WIFILOG_DHCP_LABEL(name) \
    static constexpr OHOS::Wifi::TestLogLable WIFI_LOG_LABEL = {OHOS::Wifi::LOG_ID_WIFI_DHCP, name};

#define WIFI_LOGF(...) HILOG_IMPL(LOG_CORE, LOG_FATAL, WIFI_LOG_LABEL.dominId, WIFI_LOG_LABEL.tag, __VA_ARGS__)
#define WIFI_LOGE(...) HILOG_IMPL(LOG_CORE, LOG_ERROR, WIFI_LOG_LABEL.dominId, WIFI_LOG_LABEL.tag, __VA_ARGS__)
#define WIFI_LOGW(...) HILOG_IMPL(LOG_CORE, LOG_WARN, WIFI_LOG_LABEL.dominId, WIFI_LOG_LABEL.tag, __VA_ARGS__)
#define WIFI_LOGI(...) HILOG_IMPL(LOG_CORE, LOG_INFO, WIFI_LOG_LABEL.dominId, WIFI_LOG_LABEL.tag, __VA_ARGS__)
#define WIFI_LOGD(...) HILOG_IMPL(LOG_CORE, LOG_DEBUG, WIFI_LOG_LABEL.dominId, WIFI_LOG_LABEL.tag, __VA_ARGS__)

#endif

#ifndef CHECK_NULL_AND_RETURN
#define CHECK_NULL_AND_RETURN(ptr, retValue) \
if (!(ptr)) { \
    WIFI_LOGI("Pointer %{public}s in %{public}s is NULL!", #ptr, __func__); \
    return retValue; \
}
#endif
}  // namespace Wifi
}  // namespace OHOS
#endif