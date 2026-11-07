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
#include <mutex>
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "magic_enum.h"
#include "define.h"
#include "wifi_ap_msg.h"
#include "wifi_errcode.h"
#include "wifi_hotspot.h"
#include "wifi_logger.h"

namespace OHOS {
namespace WifiCli {

constexpr int SET_ARGC_NUMS = 6;
constexpr size_t SSID_MAX_LENGTH = 32;
constexpr size_t PASSWORD_MIN_LEN = 8;
constexpr int PASSWORD_MAX_LEN = 63;
constexpr char OPTION_PREFIX[] = "--";


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

static const std::shared_ptr<OHOS::Wifi::WifiHotspot>& GetHotspotInstance()
{
    static std::mutex mtx;
    static std::shared_ptr<OHOS::Wifi::WifiHotspot> instance;
    std::lock_guard<std::mutex> lock(mtx);
    if (instance == nullptr) {
        instance = OHOS::Wifi::WifiHotspot::GetInstance(WIFI_HOTSPOT_ABILITY_ID);
        if (instance == nullptr) {
            OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiHotspot instance",
                "Please check if WiFi hotspot service is available"});
        }
    }
    return instance;
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
        const size_t OPTION_PREFIX_LEN = sizeof(OPTION_PREFIX) - 1;
        if (i + 1 >= argc || argv[i + 1] == nullptr || argv[i + 1][0] == '\0' ||
            strncmp(argv[i + 1], OPTION_PREFIX, OPTION_PREFIX_LEN) == 0) {
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

static ErrMsg GetErrorInfo(OHOS::Wifi::ErrCode ret)
{
    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_FORBID_AIRPLANE:
            return {"WIFI_ERROR", "Hotspot cannot be toggled in airplane mode",
                "Turn off airplane mode first"};
        case OHOS::Wifi::WIFI_OPT_FORBID_POWSAVING:
            return {"WIFI_ERROR", "Hotspot operation is forbidden in power saving mode",
                "Turn off power saving mode"};
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            return {"WIFI_ERROR", "Permission denied", "Check WiFi hotspot permissions"};
        case OHOS::Wifi::WIFI_OPT_ENTERPRISE_DENIED:
            return {"WIFI_ERROR", "Hotspot is forbidden by enterprise policy",
                "Contact your administrator to allow hotspot usage"};
        case OHOS::Wifi::WIFI_OPT_NOT_SUPPORTED:
            return {"WIFI_ERROR", "Hotspot operation is not supported", "Check device capability"};
        case OHOS::Wifi::WIFI_OPT_NON_SYSTEMAPP:
            return {"WIFI_ERROR", "Hotspot operation requires system permission",
                "Run this command from a system-authorized shell"};
        case OHOS::Wifi::WIFI_OPT_OPEN_FAIL_WHEN_CLOSING:
            return {"WIFI_ERROR", "Hotspot is closing, retry later",
                "Wait for the hotspot to finish closing and retry"};
        case OHOS::Wifi::WIFI_OPT_CLOSE_FAIL_WHEN_OPENING:
            return {"WIFI_ERROR", "Hotspot is starting, retry later",
                "Wait for the hotspot to finish starting and retry"};
        case OHOS::Wifi::WIFI_OPT_INVALID_CONFIG:
        case OHOS::Wifi::WIFI_OPT_INVALID_PARAM:
        case OHOS::Wifi::WIFI_OPT_INVALID_PARAM_NEW:
            return {"WIFI_ERROR", "Invalid hotspot configuration", "Check ssid/password/band values"};
        default:
            return {"WIFI_ERROR", "Failed to execute", "Check WiFi service status and try again"};
    }
}

CliErrCode CmdEnableHotspot(int argc, char** argv)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return CliErrCode::FAILED;
    }
    OHOS::Wifi::ErrCode ret = hotspotPtr->EnableHotspot();
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_OPEN_SUCC_WHEN_OPENED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
            return CliErrCode::FAILED;
        }
        cJSON_AddStringToObject(data, "message", "Hotspot enabled");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }
    OutputErrorJson(GetErrorInfo(ret));
    return CliErrCode::FAILED;
}

CliErrCode CmdDisableHotspot(int argc, char** argv)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return CliErrCode::FAILED;
    }
    OHOS::Wifi::ErrCode ret = hotspotPtr->DisableHotspot();
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
            return CliErrCode::FAILED;
        }
        cJSON_AddStringToObject(data, "message", "Hotspot disabled");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }
    OutputErrorJson(GetErrorInfo(ret));
    return CliErrCode::FAILED;
}

static void AddValidBandsToJson(cJSON* data, const std::shared_ptr<OHOS::Wifi::WifiHotspot>& hotspotPtr)
{
    std::vector<OHOS::Wifi::BandType> bands;
    if (hotspotPtr->GetValidBands(bands) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "GetValidBands failed", "Retry the command"});
        return;
    }

    cJSON* bandArr = cJSON_CreateArray();
    if (bandArr == nullptr) {
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
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
    cJSON_AddStringToObject(data, "ipAddress", cfg.GetIpAddress().c_str());
    cJSON_AddBoolToObject(data, "hasPassword", !cfg.GetPreSharedKey().empty());
}

CliErrCode CmdGetHotspotConfig(int argc, char** argv)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return CliErrCode::FAILED;
    }
    OHOS::Wifi::HotspotConfig cfg;
    OHOS::Wifi::ErrCode ret = hotspotPtr->GetHotspotConfig(cfg);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to get hotspot config", "Ensure WiFi service is available"});
        return CliErrCode::FAILED;
    }
    do {
        bool active = false;
        if (hotspotPtr->IsHotspotActive(active) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "IsHotspotActive failed", "Ensure WiFi service is available"});
            break;
        }
        int state = 0;
        if (hotspotPtr->GetHotspotState(state) != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "GetHotspotState failed", "Ensure WiFi service is available"});
            break;
        }
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
            break;
        }
        AddValidBandsToJson(data, hotspotPtr);
        FillHotspotConfigToJson(data, cfg, static_cast<OHOS::Wifi::ApState>(state), active);
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    } while (0);
    return CliErrCode::FAILED;
}

static bool ValidateConfig(const HotspotUserConfig& config, OHOS::Wifi::BandType& band)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return false;
    }
    if (config.ssid.empty() && config.preSharedKey.empty() && config.band.empty()) {
        OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "No Parameter to Set",
            "Usage: ohos-wifiManager hotspot-config-set [--ssid <ssid>] [--preSharedKey <pwd>] [--band <2.4G|5G>]"});
        return false;
    }
    if (!config.ssid.empty() && config.ssid.size() > SSID_MAX_LENGTH) {
        OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Invalid SSID (length 1~32)",
            "Use a non-empty SSID of at most 32 characters"});
        return false;
    }
    if (!config.preSharedKey.empty() && (config.preSharedKey.size() < PASSWORD_MIN_LEN ||
        config.preSharedKey.size() > PASSWORD_MAX_LEN)) {
        OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Invalid password length (8~63 chars)",
            "Provide a password of 8~63 characters"});
        return false;
    }
    if (!config.band.empty()) {
        if (!BandFromName(config.band, band)) {
            OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Invalid --band: " + config.band, "Use 2.4G or 5G"});
            return false;
        }
        std::vector<OHOS::Wifi::BandType> validBands;
        if (hotspotPtr->GetValidBands(validBands) == OHOS::Wifi::WIFI_OPT_SUCCESS &&
            std::find(validBands.begin(), validBands.end(), band) == validBands.end()) {
            OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Band not supported on this device",
                "Supported bands are shown in 'ohos-wifiManager hotspot-config-get'"});
            return false;
        }
    }
    return true;
}

CliErrCode CmdSetHotspotConfig(int argc, char** argv)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return CliErrCode::FAILED;
    }
    HotspotUserConfig config;
    if (!ParseConfig(argc, argv, config)) {
        OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Failed to Parse Config Info",
            "Usage: ohos-wifiManager hotspot-config-set [--ssid <ssid>] [--preSharedKey <pwd>] [--band <2.4G|5G>]"});
        return CliErrCode::FAILED;
    }
    OHOS::Wifi::BandType band{};
    if (!ValidateConfig(config, band)) {
        return CliErrCode::FAILED;
    }
    OHOS::Wifi::HotspotConfig cfg;
    OHOS::Wifi::ErrCode ret = hotspotPtr->GetHotspotConfig(cfg);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to read current hotspot config",
            "Ensure WiFi service is available"});
        return CliErrCode::FAILED;
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
    ret = hotspotPtr->SetHotspotConfig(cfg);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(GetErrorInfo(ret));
        return CliErrCode::FAILED;
    }
    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
        return CliErrCode::FAILED;
    }
    cJSON_AddStringToObject(data, "ssid", cfg.GetSsid().c_str());
    cJSON_AddStringToObject(data, "band", BandTypeToStr(cfg.GetBand()));
    cJSON_AddStringToObject(data, "message", "Hotspot configuration saved");
    OutputSuccessJson(data);
    return CliErrCode::SUCCESS;
}

CliErrCode CmdGetHotspotBlocklist(int argc, char** argv)
{
    const auto& hotspotPtr = GetHotspotInstance();
    if (!hotspotPtr) {
        return CliErrCode::FAILED;
    }

    std::vector<OHOS::Wifi::StationInfo> infos;
    OHOS::Wifi::ErrCode ret = hotspotPtr->GetBlockLists(infos);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to get blocklist", "Ensure WiFi hotspot service is available"});
        return CliErrCode::FAILED;
    }

    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
        return CliErrCode::FAILED;
    }
    cJSON* arr = cJSON_CreateArray();
    if (arr == nullptr) {
        cJSON_Delete(data);
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to allocate JSON output", "Retry the command"});
        return CliErrCode::FAILED;
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
    return CliErrCode::SUCCESS;
}

} // namespace WifiCli
} // namespace OHOS
