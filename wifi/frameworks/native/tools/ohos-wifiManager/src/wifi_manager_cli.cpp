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
#include "wifi_manager_cli.h"
#include "wifi_exception_cli.h"
#include "wifi_hotspot_cli.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstring>

#include "cJSON.h"
#include "wifi_logger.h"
#include "wifi_common_util.h"
#include "wifi_device.h"
#include "wifi_scan.h"
#include "wifi_errcode.h"
#include "define.h"
#include "securec.h"

namespace OHOS {
namespace WifiCli {

DEFINE_WIFILOG_LABEL("WifiCli");
constexpr int MIN_ARG_COUNT = 2;
using WifiDevicePtr = std::shared_ptr<OHOS::Wifi::WifiDevice>;
using WifiScanPtr = std::shared_ptr<OHOS::Wifi::WifiScan>;

struct ErrorInfo {
    std::string errCode;
    std::string message;
    std::string suggestion;
};

static ErrorInfo GetErrInfoExt(OHOS::Wifi::ErrCode code)
{
    switch (static_cast<WifiCliErrCode>(code)) {
        case WifiCliErrCode::NETWORK_NOT_FOUND:
            return {"WIFI_ERROR", "Failed to find the network",
                "Check if the network is available; run 'ohos-wifiManager scan-list' to verify"};
        case WifiCliErrCode::BAND_NOT_SUPPORTED:
            return {"WIFI_ERROR", "Band not supported on this device",
                "Supported bands are shown in 'ohos-wifiManager hotspot-config-get' (validBands)"};
        case WifiCliErrCode::SSID_INVALID:
            return {"ERR_PARAM_INVALID", "Invalid SSID length",
                "SSID length must be 1~32 characters"};
        case WifiCliErrCode::PSK_INVALID:
            return {"ERR_PARAM_INVALID", "Invalid password length",
                "Password length must be 8~63 characters"};
        case WifiCliErrCode::BAND_INVALID:
            return {"ERR_PARAM_INVALID", "Invalid --band value",
                "Run 'ohos-wifiManager hotspot-config-get' to get valid bands"};
        case WifiCliErrCode::MISSING_PARAM:
            return {"ERR_PARAM_INVALID", "Missing required parameter",
                "Run 'ohos-wifiManager --help' to check required parameters"};
        default:
            return {"WIFI_ERROR", "Failed to execute", "Check WiFi service status and try again"};
    }
}

static ErrorInfo GetErrorInfo(OHOS::Wifi::ErrCode code)
{
    switch (code) {
        case OHOS::Wifi::WIFI_OPT_INVALID_PARAM:
        case OHOS::Wifi::WIFI_OPT_INVALID_PARAM_NEW:
        case OHOS::Wifi::WIFI_OPT_INVALID_CONFIG:
            return {"ERR_PARAM_INVALID", "Invalid command parameters",
                "Run 'ohos-wifiManager --help' to check usage"};
        case OHOS::Wifi::WIFI_OPT_FORBID_AIRPLANE:
            return {"WIFI_ERROR", "WiFi operation is forbidden in airplane mode",
                "Turn off airplane mode first"};
        case OHOS::Wifi::WIFI_OPT_FORBID_POWSAVING:
            return {"WIFI_ERROR", "WiFi operation is forbidden in power saving mode",
                "Turn off power saving mode"};
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            return {"WIFI_ERROR", "Permission denied", "Check WiFi permissions"};
        case OHOS::Wifi::WIFI_OPT_ENTERPRISE_DENIED:
            return {"WIFI_ERROR", "WiFi operation is forbidden by enterprise policy",
                "Contact your administrator to allow WiFi usage"};
        case OHOS::Wifi::WIFI_OPT_NOT_SUPPORTED:
            return {"WIFI_ERROR", "Operation is not supported", "Check device capability"};
        case OHOS::Wifi::WIFI_OPT_NON_SYSTEMAPP:
            return {"WIFI_ERROR", "Operation requires system permission",
                "Run this command from a system-authorized shell"};
        case OHOS::Wifi::WIFI_OPT_OPEN_FAIL_WHEN_CLOSING:
            return {"WIFI_ERROR", "WiFi hotspot is closing, retry later",
                "Wait for the hotspot to finish closing and retry"};
        case OHOS::Wifi::WIFI_OPT_CLOSE_FAIL_WHEN_OPENING:
            return {"WIFI_ERROR", "WiFi hotspot is starting, retry later",
                "Wait for the hotspot to finish starting and retry"};
        case OHOS::Wifi::WIFI_OPT_STA_NOT_OPENED:
            return {"WIFI_ERROR", "WiFi STA service is not opened",
                "Run 'ohos-wifiManager sta-enable' first and retry"};
        case OHOS::Wifi::WIFI_OPT_SCAN_NOT_OPENED:
            return {"WIFI_ERROR", "WiFi scan service is not opened",
                "Run 'ohos-wifiManager sta-enable' and 'scan-start' first"};
        default:
            return GetErrInfoExt(code);
    }
}

struct Command {
    const char* name;
    const char* description;
    std::function<OHOS::Wifi::ErrCode(int, char**)> handler;
};

static std::unordered_map<std::string, Command> g_commands;

std::shared_ptr<OHOS::Wifi::WifiDevice> GetWifiDevice()
{
    static WifiDevicePtr g_wifiDevice =
        OHOS::Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
    return g_wifiDevice;
}

std::shared_ptr<OHOS::Wifi::WifiScan> GetWifiScan()
{
    static WifiScanPtr g_wifiScan =
        OHOS::Wifi::WifiScan::GetInstance(WIFI_SCAN_ABILITY_ID);
    return g_wifiScan;
}

void OutputSuccessJson(cJSON* data)
{
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "result");
    cJSON_AddStringToObject(root, "status", "success");
    cJSON_AddItemToObject(root, "data", data);

    char* jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr != nullptr) {
        std::cout << jsonStr << std::endl;
        cJSON_free(jsonStr);
    }
    cJSON_Delete(root);
}

void OutputErrorJson(OHOS::Wifi::ErrCode code)
{
    ErrorInfo info = GetErrorInfo(code);
    cJSON* root = cJSON_CreateObject();
    if (root == nullptr) {
        return;
    }
    cJSON_AddStringToObject(root, "type", "result");
    cJSON_AddStringToObject(root, "status", "failed");
    cJSON_AddStringToObject(root, "errCode", info.errCode.c_str());
    cJSON_AddStringToObject(root, "errMsg", info.message.c_str());
    cJSON_AddStringToObject(root, "suggestion", info.suggestion.c_str());

    char* jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr != nullptr) {
        std::cout << jsonStr << std::endl;
        cJSON_free(jsonStr);
    }
    cJSON_Delete(root);
}

OHOS::Wifi::ErrCode CmdStaEnable(int argc, char** argv)
{
    WIFI_LOGI("sta-enable command started");

    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        WIFI_LOGE("Failed to get WifiDevice instance");
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiDevice->EnableWifi();
    WIFI_LOGI("EnableWifi returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_OPEN_SUCC_WHEN_OPENED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "message", "WiFi STA mode enabled successfully");
        OutputSuccessJson(data);
        return OHOS::Wifi::WIFI_OPT_SUCCESS;
    }
    return ret;
}

OHOS::Wifi::ErrCode CmdStaDisable(int argc, char** argv)
{
    WIFI_LOGI("sta-disable command started");
    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        WIFI_LOGE("Failed to get WifiDevice instance");
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiDevice->DisableWifi();
    WIFI_LOGI("DisableWifi returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS || ret == OHOS::Wifi::WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "message", "WiFi STA mode disabled successfully");
        OutputSuccessJson(data);
        return OHOS::Wifi::WIFI_OPT_SUCCESS;
    }
    return ret;
}

OHOS::Wifi::ErrCode CmdScanStart(int argc, char** argv)
{
    WIFI_LOGI("scan-start command started");
    auto wifiScan = GetWifiScan();
    if (wifiScan == nullptr) {
        WIFI_LOGE("Failed to get WifiScan instance");
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiScan->Scan(false);
    WIFI_LOGI("Scan(false) returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "message", "WiFi scan started successfully");
        OutputSuccessJson(data);
    }
    return ret;
}

static cJSON* BuildScanResultJson(const std::vector<OHOS::Wifi::WifiScanInfo>& scanInfoList)
{
    cJSON* data = cJSON_CreateObject();
    cJSON* networks = cJSON_CreateArray();

    for (const auto& info : scanInfoList) {
        cJSON* network = cJSON_CreateObject();
        cJSON_AddStringToObject(network, "ssid",
            info.ssid.empty() ? "<unknown>" : info.ssid.c_str());
        cJSON_AddStringToObject(network, "bssid",
            info.bssid.empty() ? "<unknown>" : OHOS::Wifi::MacAnonymize(info.bssid).c_str());
        cJSON_AddNumberToObject(network, "securityType", static_cast<int>(info.securityType));
        cJSON_AddNumberToObject(network, "rssi", info.rssi);
        cJSON_AddNumberToObject(network, "frequency", info.frequency);

        cJSON_AddItemToArray(networks, network);
    }

    cJSON_AddItemToObject(data, "networks", networks);
    cJSON_AddNumberToObject(data, "count", static_cast<int>(scanInfoList.size()));
    return data;
}

OHOS::Wifi::ErrCode CmdScanList(int argc, char** argv)
{
    WIFI_LOGI("scan-list command started");
    auto wifiScan = GetWifiScan();
    if (wifiScan == nullptr) {
        WIFI_LOGE("Failed to get WifiScan instance");
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    std::vector<OHOS::Wifi::WifiScanInfo> scanInfoList;
    OHOS::Wifi::ErrCode ret = wifiScan->GetScanInfoList(scanInfoList, false);
    WIFI_LOGI("GetScanInfoList returned %{public}d, found %{public}zu networks",
              ret, scanInfoList.size());

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputSuccessJson(BuildScanResultJson(scanInfoList));
    }
    return ret;
}

void SetKeyMgmtBySecurityType(OHOS::Wifi::WifiSecurity securityType, std::string& keyMgmt)
{
    switch (securityType) {
        case OHOS::Wifi::WifiSecurity::PSK:
        case OHOS::Wifi::WifiSecurity::PSK_SAE:
            keyMgmt = "WPA-PSK";
            break;
        case OHOS::Wifi::WifiSecurity::EAP:
            keyMgmt = "WPA-EAP";
            break;
        case OHOS::Wifi::WifiSecurity::SAE:
            keyMgmt = "SAE";
            break;
        case OHOS::Wifi::WifiSecurity::WEP:
            keyMgmt = "WEP";
            break;
        case OHOS::Wifi::WifiSecurity::EAP_SUITE_B:
            keyMgmt = "WPA-EAP-SUITE-B-192";
            break;
        case OHOS::Wifi::WifiSecurity::WAPI_CERT:
            keyMgmt = "WAPI-CERT";
            break;
        case OHOS::Wifi::WifiSecurity::WAPI_PSK:
            keyMgmt = "WAPI-PSK";
            break;
        case OHOS::Wifi::WifiSecurity::OPEN:
        case OHOS::Wifi::WifiSecurity::OWE:
        default:
            keyMgmt = "NONE";
            break;
    }
}

static OHOS::Wifi::ErrCode ParseConnectArgs(int argc, char** argv, OHOS::Wifi::WifiDeviceConfig& tmpConfig)
{
    for (int i = 0; i < argc - 1; ++i) {
        std::string arg = argv[i];
        if (arg == "--ssid") {
            tmpConfig.ssid = argv[i + 1];
        }
        if (arg == "--preSharedKey") {
            tmpConfig.preSharedKey = argv[i + 1];
            size_t scrubLen = strlen(argv[i + 1]) + 1;
            memset_s(argv[i + 1], scrubLen, 0, scrubLen);
        }
    }
    if (tmpConfig.ssid.empty()) {
        return static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::MISSING_PARAM);
    }
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

void ClearString(std::string& str)
{
    if (str.empty()) {
        return;
    }
    memset_s(str.data(), str.size(), 0, str.size());
    str.clear();
}

static OHOS::Wifi::ErrCode ExecuteWifiConnect(std::shared_ptr<OHOS::Wifi::WifiDevice>& wifiDevice,
    OHOS::Wifi::WifiDeviceConfig& tmpConfig)
{
    OHOS::Wifi::ErrCode ret = wifiDevice->ConnectToDevice(tmpConfig);
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        if (data == nullptr) {
            return OHOS::Wifi::WIFI_OPT_FAILED;
        }
        cJSON_AddStringToObject(data, "ssid", tmpConfig.ssid.c_str());
        cJSON_AddStringToObject(data, "message", "WiFi connection initiated successfully");
        OutputSuccessJson(data);
    }
    return ret;
}

OHOS::Wifi::ErrCode CmdStaConnect(int argc, char** argv)
{
    WIFI_LOGI("sta-connect command started");
    OHOS::Wifi::WifiDeviceConfig tmpConfig;
    OHOS::Wifi::ErrCode ret = OHOS::Wifi::WIFI_OPT_FAILED;

    do {
        ret = ParseConnectArgs(argc, argv, tmpConfig);
        if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            break;
        }
        auto wifiDevice = GetWifiDevice();
        auto wifiScan = GetWifiScan();
        if (wifiDevice == nullptr || wifiScan == nullptr) {
            WIFI_LOGE("Failed to get WifiDevice or WifiScan instance");
            ret = OHOS::Wifi::WIFI_OPT_FAILED;
            break;
        }
        std::vector<OHOS::Wifi::WifiScanInfo> scanInfoList;
        ret = wifiScan->GetScanInfoList(scanInfoList, false);
        if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
            break;
        }
        bool isFound = false;
        for (const auto& scanInfo : scanInfoList) {
            if (scanInfo.ssid == tmpConfig.ssid) {
                SetKeyMgmtBySecurityType(scanInfo.securityType, tmpConfig.keyMgmt);
                isFound = true;
            }
        }
        if (!isFound) {
            ret = static_cast<OHOS::Wifi::ErrCode>(WifiCliErrCode::NETWORK_NOT_FOUND);
            break;
        }
        ret = ExecuteWifiConnect(wifiDevice, tmpConfig);
    } while (0);
    ClearString(tmpConfig.preSharedKey);
    return ret;
}

std::string GetDetailedStateErr(OHOS::Wifi::DetailedState state)
{
    switch (state) {
        case OHOS::Wifi::DetailedState::VERIFYING_POOR_LINK:
            return "VerifyingPoorLink";
        case OHOS::Wifi::DetailedState::PASSWORD_ERROR:
            return "PasswordError";
        case OHOS::Wifi::DetailedState::CONNECTION_REJECT:
            return "ConnectionReject";
        case OHOS::Wifi::DetailedState::CONNECTION_FULL:
            return "ConnectionFull";
        case OHOS::Wifi::DetailedState::CONNECTION_TIMEOUT:
            return "ConnectionTimeout";
        case OHOS::Wifi::DetailedState::OBTAINING_IPADDR_FAIL:
            return "ObtainingIpaddrFail";
        case OHOS::Wifi::DetailedState::INVALID:
        default:
            return "Invalid";
    }
}

std::string GetDetailedStateStr(OHOS::Wifi::DetailedState state)
{
    switch (state) {
        case OHOS::Wifi::DetailedState::AUTHENTICATING:
            return "Authenticating";
        case OHOS::Wifi::DetailedState::BLOCKED:
            return "Blocked";
        case OHOS::Wifi::DetailedState::CAPTIVE_PORTAL_CHECK:
            return "CaptivePortalCheck";
        case OHOS::Wifi::DetailedState::CONNECTED:
            return "Connected";
        case OHOS::Wifi::DetailedState::CONNECTING:
            return "Connecting";
        case OHOS::Wifi::DetailedState::DISCONNECTED:
            return "Disconnected";
        case OHOS::Wifi::DetailedState::DISCONNECTING:
            return "Disconnecting";
        case OHOS::Wifi::DetailedState::FAILED:
            return "Failed";
        case OHOS::Wifi::DetailedState::IDLE:
            return "Idle";
        case OHOS::Wifi::DetailedState::OBTAINING_IPADDR:
            return "ObtainingIpaddr";
        case OHOS::Wifi::DetailedState::WORKING:
            return "Working";
        case OHOS::Wifi::DetailedState::NOTWORKING:
            return "NotWorking";
        case OHOS::Wifi::DetailedState::SCANNING:
            return "Scanning";
        case OHOS::Wifi::DetailedState::SUSPENDED:
            return "Suspended";
        default:
            return GetDetailedStateErr(state);
    }
}

OHOS::Wifi::ErrCode CmdStaGetLinkedInfo(int argc, char** argv)
{
    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }

    OHOS::Wifi::WifiLinkedInfo info;
    OHOS::Wifi::ErrCode ret = wifiDevice->GetLinkedInfo(info);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        return ret;
    }
    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        return OHOS::Wifi::WIFI_OPT_FAILED;
    }
    cJSON_AddStringToObject(data, "ssid", info.ssid.empty() ? "" : info.ssid.c_str());
    cJSON_AddStringToObject(data, "bssid", info.bssid.empty() ? "" : OHOS::Wifi::MacAnonymize(info.bssid).c_str());
    cJSON_AddNumberToObject(data, "rssi", info.rssi);
    cJSON_AddNumberToObject(data, "frequency", info.frequency);
    cJSON_AddNumberToObject(data, "linkSpeed", info.linkSpeed);
    std::string detailedState = GetDetailedStateStr(info.detailedState);
    cJSON_AddStringToObject(data, "detailedState", detailedState.c_str());
    OutputSuccessJson(data);
    WIFI_LOGI("Get WiFi linked info successfully");
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

OHOS::Wifi::ErrCode CmdHelp(int argc, char** argv)
{
    WIFI_LOGI("help command called");
    cJSON* data = cJSON_CreateObject();
    cJSON* cmdArr = cJSON_CreateArray();

    for (const auto& pair : g_commands) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "cmd", pair.first.c_str());
        cJSON_AddStringToObject(item, "desc", pair.second.description);
        cJSON_AddItemToArray(cmdArr, item);
    }
    cJSON_AddItemToObject(data, "commands", cmdArr);
    OutputSuccessJson(data);
    return OHOS::Wifi::WIFI_OPT_SUCCESS;
}

void InitCommands()
{
    g_commands["sta-enable"] = {"sta-enable",
        "ohos-wifiManager sta-enable - Enable WiFi STA mode (idempotent)", CmdStaEnable};
    g_commands["sta-disable"] = {"sta-disable",
        "ohos-wifiManager sta-disable - Disable WiFi STA mode (idempotent)", CmdStaDisable};
    g_commands["scan-start"] = {"scan-start",
        "ohos-wifiManager scan-start - Start a WiFi scan; run sta-enable first", CmdScanStart};
    g_commands["scan-list"] = {"scan-list",
        "ohos-wifiManager scan-list - List scan results; run scan-start first", CmdScanList};
    g_commands["sta-connect"] = {"sta-connect",
        "ohos-wifiManager sta-connect --ssid <ssid> [--preSharedKey <pwd>] - Connect to a network; "
        "--ssid required, network must appear in scan-list; password only for secured networks; "
        "run sta-enable and scan-start first", CmdStaConnect};
    g_commands["sta-getLinkedInfo"] = {"sta-getLinkedInfo",
        "ohos-wifiManager sta-getLinkedInfo - Show current connection "
        "(ssid/bssid/rssi/frequency/linkSpeed/detailedState); run sta-enable first", CmdStaGetLinkedInfo};
#ifdef WIFI_EXCEPTION_RECORD_ENABLE
    g_commands["wifi-exception"] = {"wifi-exception",
        "ohos-wifiManager wifi-exception [--clear] [--ssid <ssid>] [--category <name>] "
        "[--reason <code>] [--per-ap <n>] - List WiFi exception records, or clear all with --clear; "
        "--reason/--per-ap must be integers", CmdWifiException};
#endif
    g_commands["hotspot-enable"] = {"hotspot-enable",
        "ohos-wifiManager hotspot-enable - Enable WiFi hotspot (idempotent)", CmdEnableHotspot};
    g_commands["hotspot-disable"] = {"hotspot-disable",
        "ohos-wifiManager hotspot-disable - Disable WiFi hotspot (idempotent)", CmdDisableHotspot};
    g_commands["hotspot-config-set"] = {"hotspot-config-set",
        "ohos-wifiManager hotspot-config-set [--ssid <ssid>] [--preSharedKey <pwd>] [--band <2.4G|5G>] "
        "- Update hotspot config; at least one option required; ssid <= 32 chars, password 8~63 chars, "
        "band must be device-supported (see validBands); applies immediately if hotspot is on, "
        "otherwise on next enable", CmdSetHotspotConfig};
    g_commands["hotspot-config-get"] = {"hotspot-config-get",
        "ohos-wifiManager hotspot-config-get - Show hotspot config, running state and supported bands "
        "(validBands); password shown only as hasPassword", CmdGetHotspotConfig};
    g_commands["hotspot-blocklist-get"] = {"hotspot-blocklist-get",
        "ohos-wifiManager hotspot-blocklist-get - List blocklisted hotspot devices "
        "(bssid/ipAddr anonymized); works even when hotspot is off", CmdGetHotspotBlocklist};
    g_commands["--help"] = {"--help",
        "ohos-wifiManager --help - Show this command list with usage", CmdHelp};
}

int RunCommand(int argc, char** argv)
{
    WIFI_LOGI("enter ohos-wifiManager");
    InitCommands();
    if (argc < MIN_ARG_COUNT) {
        CmdHelp(argc, argv);
        return 1;
    }
    std::string cmdName = argv[1];
    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        WIFI_LOGE("Unknown command: %{public}s", cmdName.c_str());
        CmdHelp(argc, argv);
        return 1;
    }

    int cmdArgc = argc - MIN_ARG_COUNT;
    char** cmdArgv = argv + MIN_ARG_COUNT;

    WIFI_LOGI("Executing command: %{public}s", cmdName.c_str());
    OHOS::Wifi::ErrCode ret = it->second.handler(cmdArgc, cmdArgv);
    WIFI_LOGI("Command %{public}s finished with code %{public}d", cmdName.c_str(), static_cast<int>(ret));

    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ret);
        return 1;
    }
    return 0;
}

} // namespace WifiCli
} // namespace OHOS
