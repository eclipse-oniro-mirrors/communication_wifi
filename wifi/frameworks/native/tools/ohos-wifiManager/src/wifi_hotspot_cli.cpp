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
#include "wifi_hotspot_cli.h"
#include "wifi_manager_cli.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>
#include <string>

#include "magic_enum.h"
#include "define.h"
#include "wifi_ap_msg.h"
#include "wifi_errcode.h"
#include "wifi_hotspot.h"
#include "wifi_common_util.h"
#include "securec.h"

namespace OHOS {
namespace WifiCli {

constexpr int SET_ARGC_NUMS = 6;

static const std::shared_ptr<OHOS::Wifi::WifiHotspot> g_hotspotPtr =
    OHOS::Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);

static const std::pair<std::string, OHOS::Wifi::BandType> BAND_NAMES[] = {
    {"2.4G", OHOS::Wifi::BandType::BAND_2GHZ },
    {"5G",   OHOS::Wifi::BandType::BAND_5GHZ },
    {"6G",   OHOS::Wifi::BandType::BAND_6GHZ },
    {"60G",  OHOS::Wifi::BandType::BAND_60GHZ},
    {"ANY",  OHOS::Wifi::BandType::BAND_ANY },
};

static bool BandFromName(const std::string& name, OHOS::Wifi::BandType& band)
{
    for (const auto& [bandName, bType] : BAND_NAMES) {
        if (bandName == name) {
            band = bType;
            return true;
        }
    }
    return false;
}

static const char* BandTypeToStr(OHOS::Wifi::BandType band)
{
    for (const auto& [bandName, bType] : BAND_NAMES) {
        if (bType == band) {
            return bandName.c_str();
        }
    }
    return "UNKNOWN";
}

struct HotspotUserConfig {
    std::string ssid;
    std::string preSharedKey;
    std::string band;
};

static bool ParseConfig(int argc, char** argv, HotspotUserConfig& config)
{
    if (argc > SET_ARGC_NUMS) {
        return false;
    }
    int i = 0;
    while (i < argc) {
        const char* opt = argv[i];
        if (opt == nullptr) {
            return false;
        }
        std::string* target = nullptr;
        if (strcmp(opt, "--ssid") == 0) {
            target = &config.ssid;
        } else if (strcmp(opt, "--preSharedKey") == 0) {
            target = &config.preSharedKey;
        } else if (strcmp(opt, "--band") == 0) {
            target = &config.band;
        } else {
            return false;
        }
        if (i + 1 >= argc || argv[i + 1] == nullptr || argv[i + 1][0] == '\0' ||
            strcmp(argv[i + 1], "--ssid") == 0 || strcmp(argv[i + 1], "--preSharedKey") == 0 ||
            strcmp(argv[i + 1], "--band") == 0) {
            return false;
        }
        *target = argv[++i];
        if (target == &config.preSharedKey) {
            size_t scrubLen = strlen(argv[i]) + 1;
            memset_s(argv[i], scrubLen, 0, scrubLen);
        }
        ++i;
    }
    return true;
}

static void AddValidBandsToJson(cJSON* data)
{
    if (g_hotspotPtr == nullptr) {
        return;
    }
    std::vector<OHOS::Wifi::BandType> bands;
    if (g_hotspotPtr->GetValidBands(bands) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return;
    }

    cJSON* bandArr = cJSON_CreateArray();
    if (bandArr == nullptr) {
        return;
    }

    for (const auto& b : bands) {
        cJSON_AddItemToArray(bandArr, cJSON_CreateString(BandTypeToStr(b)));
    }
    cJSON_AddItemToObject(data, "validBands", bandArr);
}

static void FillHotspotConfigToJson(cJSON* data, const OHOS::Wifi::HotspotConfig& cfg, OHOS::Wifi::ApState state,
    bool active)
{
    cJSON_AddBoolToObject(data, "active", active);
    cJSON_AddStringToObject(data, "state", OHOS::Wifi::magic_enum::Enum2Name(state).c_str());
    cJSON_AddStringToObject(data, "ssid", cfg.GetSsid().c_str());
    cJSON_AddStringToObject(data, "band", BandTypeToStr(cfg.GetBand()));
    cJSON_AddNumberToObject(data, "channel", cfg.GetChannel());
    cJSON_AddNumberToObject(data, "maxConn", cfg.GetMaxConn());
    cJSON_AddStringToObject(data, "ipAddress",
        cfg.GetIpAddress().empty() ? "" : cfg.GetIpAddress().c_str());
    cJSON_AddBoolToObject(data, "hasPassword", !cfg.GetPreSharedKey().empty());
}

OHOS::Wifi::ErrCode CmdEnableHotspot(int argc, char** argv)
{
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    OHOS::Wifi::ErrCode ret = g_hotspotPtr->EnableHotspot();
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_OPEN_SUCC_WHEN_OPENED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "message", "Hotspot enabled");
        OutputSuccessJson(data);
        return OHOS::Wifi::WIFI_OPT_SUCCESS;
    }
    return ret;
}

OHOS::Wifi::ErrCode CmdDisableHotspot(int argc, char** argv)
{
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    OHOS::Wifi::ErrCode ret = g_hotspotPtr->DisableHotspot();
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "message", "Hotspot disabled");
        OutputSuccessJson(data);
        return OHOS::Wifi::WIFI_OPT_SUCCESS;
    }
    return ret;
}

OHOS::Wifi::ErrCode CmdGetHotspotConfig(int argc, char** argv)
{
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    OHOS::Wifi::HotspotConfig cfg;
    OHOS::Wifi::ErrCode ret = g_hotspotPtr->GetHotspotConfig(cfg);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return ret;
    }
    bool active = false;
    if (g_hotspotPtr->IsHotspotActive(active) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    int state = static_cast<int>(OHOS::Wifi::ApState::AP_STATE_NONE);
    if (g_hotspotPtr->GetHotspotState(state) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    AddValidBandsToJson(data);
    FillHotspotConfigToJson(data, cfg, static_cast<OHOS::Wifi::ApState>(state), active);
    OutputSuccessJson(data);
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

static OHOS::Wifi::ErrCode ValidateConfig(const HotspotUserConfig& config, OHOS::Wifi::BandType& band)
{
    if (config.ssid.empty() && config.preSharedKey.empty() && config.band.empty()) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::MISSING_PARAM);
    }
    if (!config.ssid.empty() && config.ssid.size() > WIFI_SSID_MAX_LEN) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::SSID_INVALID);
    }
    if (!config.preSharedKey.empty() && (config.preSharedKey.size() < OHOS::Wifi::MIN_PSK_LEN ||
        config.preSharedKey.size() > OHOS::Wifi::MAX_PSK_LEN)) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::PSK_INVALID);
    }
    if (config.band.empty()) {
        return OHOS::Wifi::WIFI_OPT_SUCCESS;
    }
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    if (!BandFromName(config.band, band)) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::BAND_INVALID);
    }
    std::vector<OHOS::Wifi::BandType> validBands;
    OHOS::Wifi::ErrCode ret = g_hotspotPtr->GetValidBands(validBands);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return ret;
    }
    if (std::find(validBands.begin(), validBands.end(), band) == validBands.end()) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::BAND_NOT_SUPPORTED);
    }
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

OHOS::Wifi::ErrCode CmdSetHotspotConfig(int argc, char** argv)
{
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    HotspotUserConfig config;
    OHOS::Wifi::ErrCode ret = OHOS::Wifi::WIFI_OPT_FAILED;
    do {
        if (!ParseConfig(argc, argv, config)) {
            ret = OHOS::Wifi::WIFI_OPT_INVALID_PARAM;
            break;
        }
        OHOS::Wifi::BandType band{};
        ret = ValidateConfig(config, band);
        if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            break;
        }
        OHOS::Wifi::HotspotConfig cfg;
        ret = g_hotspotPtr->GetHotspotConfig(cfg);
        if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            break;
        }
        if (!config.ssid.empty()) {
            cfg.SetSsid(config.ssid);
        }
        if (!config.preSharedKey.empty()) {
            cfg.SetPreSharedKey(config.preSharedKey);
        }
        if (!config.band.empty()) {
            cfg.SetBand(band);
        }
        ret = g_hotspotPtr->SetHotspotConfig(cfg);
        if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            break;
        }

        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            ret = OHOS::Wifi::WIFI_OPT_FAILED;
            break;
        }
        cJSON_AddStringToObject(data, "ssid", cfg.GetSsid().c_str());
        cJSON_AddStringToObject(data, "band", BandTypeToStr(cfg.GetBand()));
        cJSON_AddStringToObject(data, "message", "Hotspot configuration saved");
        OutputSuccessJson(data);
        ret = OHOS::Wifi::WIFI_OPT_SUCCESS;
    } while (0);
    ClearString(config.preSharedKey);
    return ret;
}

OHOS::Wifi::ErrCode CmdGetHotspotBlocklist(int argc, char** argv)
{
    if (g_hotspotPtr == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    std::vector<OHOS::Wifi::StationInfo> infos;
    OHOS::Wifi::ErrCode ret = g_hotspotPtr->GetBlockLists(infos);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return ret;
    }

    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    cJSON* arr = cJSON_CreateArray();
    if (arr == nullptr) {
        cJSON_Delete(data);
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    for (const auto& info : infos) {
        cJSON* item = cJSON_CreateObject();
        if (item == nullptr) {
            continue;
        }
        cJSON_AddStringToObject(item, "bssid",
            info.bssid.empty() ? "<unknown>" : OHOS::Wifi::MacAnonymize(info.bssid).c_str());
        cJSON_AddStringToObject(item, "ipAddr",
            info.ipAddr.empty() ? "" : OHOS::Wifi::IpAnonymize(info.ipAddr).c_str());
        cJSON_AddStringToObject(item, "deviceName", info.deviceName.empty() ? "" : info.deviceName.c_str());
        cJSON_AddItemToArray(arr, item);
    }
    cJSON_AddItemToObject(data, "stations", arr);
    cJSON_AddNumberToObject(data, "count", static_cast<int>(infos.size()));
    OutputSuccessJson(data);
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

} // namespace WifiCli
} // namespace OHOS
