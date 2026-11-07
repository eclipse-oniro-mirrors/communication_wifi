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
#include <vector>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include "cJSON.h"
#include "wifi_logger.h"
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

struct Command {
    const char* name;
    const char* description;
    std::function<CliErrCode(int, char**)> handler;
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

void OutputErrorJson(const ErrMsg& errMessage)
{
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "result");
    cJSON_AddStringToObject(root, "status", "failed");
    cJSON_AddStringToObject(root, "errCode", errMessage.code.c_str());
    cJSON_AddStringToObject(root, "errMsg", errMessage.errMsg.c_str());
    if (!errMessage.suggestion.empty()) {
        cJSON_AddStringToObject(root, "suggestion", errMessage.suggestion.c_str());
    }

    char* jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr != nullptr) {
        std::cout << jsonStr << std::endl;
        cJSON_free(jsonStr);
    }
    cJSON_Delete(root);
}

CliErrCode CmdStaEnable(int argc, char** argv)
{
    WIFI_LOGI("sta-enable command started");

    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        WIFI_LOGE("Failed to get WifiDevice instance");
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiDevice instance",
            "Please check if WiFi service is available"});
        return CliErrCode::FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiDevice->EnableWifi();
    WIFI_LOGI("EnableWifi returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "message", "WiFi STA mode enabled successfully");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }

    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_FORBID_AIRPLANE:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "WiFi cannot be enabled in airplane mode",
                "Check WiFi permissions and airplane mode"});
            break;
        case OHOS::Wifi::WIFI_OPT_OPEN_SUCC_WHEN_OPENED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "WiFi is already enabled",
                "Check WiFi permissions and airplane mode"});
            break;
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Permission denied",
                "Check WiFi permissions and airplane mode"});
            break;
        default:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to enable WiFi STA mode",
                "Check WiFi permissions and airplane mode"});
            break;
    }
    return CliErrCode::FAILED;
}

CliErrCode CmdStaDisable(int argc, char** argv)
{
    WIFI_LOGI("sta-disable command started");
    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        WIFI_LOGE("Failed to get WifiDevice instance");
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiDevice instance",
                "Please check if WiFi service is available"});
        return CliErrCode::FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiDevice->DisableWifi();
    WIFI_LOGI("DisableWifi returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "message", "WiFi STA mode disabled successfully");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }

    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "WiFi is already disabled", "Check WiFi permissions"});
            break;
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Permission denied", "Check WiFi permissions"});
            break;
        default:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to disable WiFi STA mode", "Check WiFi permissions"});
            break;
    }
    return CliErrCode::FAILED;
}

CliErrCode CmdScanStart(int argc, char** argv)
{
    WIFI_LOGI("scan-start command started");
    auto wifiScan = GetWifiScan();
    if (wifiScan == nullptr) {
        WIFI_LOGE("Failed to get WifiScan instance");
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiScan instance",
            "Please check if WiFi scan service is available"});
        return CliErrCode::FAILED;
    }

    OHOS::Wifi::ErrCode ret = wifiScan->Scan(false);
    WIFI_LOGI("Scan(false) returned %{public}d", ret);

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "message", "WiFi scan started successfully");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }

    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_SCAN_NOT_OPENED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Scan service is not opened",
                "Check WiFi permissions and ensure STA is enabled"});
            break;
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Permission denied",
                "Check WiFi permissions and ensure STA is enabled"});
            break;
        default:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to start WiFi scan",
                "Check WiFi permissions and ensure STA is enabled"});
            break;
    }
    return CliErrCode::FAILED;
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

CliErrCode CmdScanList(int argc, char** argv)
{
    WIFI_LOGI("scan-list command started");
    auto wifiScan = GetWifiScan();
    if (wifiScan == nullptr) {
        WIFI_LOGE("Failed to get WifiScan instance");
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiScan instance",
            "Please check if WiFi scan service is available"});
        return CliErrCode::FAILED;
    }

    std::vector<OHOS::Wifi::WifiScanInfo> scanInfoList;
    OHOS::Wifi::ErrCode ret = wifiScan->GetScanInfoList(scanInfoList, false);
    WIFI_LOGI("GetScanInfoList returned %{public}d, found %{public}zu networks",
              ret, scanInfoList.size());

    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputSuccessJson(BuildScanResultJson(scanInfoList));
        return CliErrCode::SUCCESS;
    }

    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_SCAN_NOT_OPENED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Scan service is not opened",
                "Ensure WiFi scan has been performed first"});
            break;
        case OHOS::Wifi::WIFI_OPT_PERMISSION_DENIED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Permission denied",
                "Ensure WiFi scan has been performed first"});
            break;
        default:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to get scan results",
                "Ensure WiFi scan has been performed first"});
            break;
    }
    return CliErrCode::FAILED;
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

void ParseConnectArgs(int argc, char** argv, OHOS::Wifi::WifiDeviceConfig& tmpConfig)
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
}

CliErrCode ExecuteWifiConnect(std::shared_ptr<OHOS::Wifi::WifiDevice>& wifiDevice,
    OHOS::Wifi::WifiDeviceConfig& tmpConfig)
{
    OHOS::Wifi::ErrCode ret = wifiDevice->ConnectToDevice(tmpConfig);
    if (ret == OHOS::Wifi::WIFI_OPT_SUCCESS) {
        cJSON* data = cJSON_CreateObject();
        cJSON_AddStringToObject(data, "ssid", tmpConfig.ssid.c_str());
        cJSON_AddStringToObject(data, "message", "WiFi connection initiated successfully");
        OutputSuccessJson(data);
        return CliErrCode::SUCCESS;
    }

    switch (ret) {
        case OHOS::Wifi::WIFI_OPT_STA_NOT_OPENED:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "WiFi STA is not enabled",
                "Enable WiFi first with 'ohos-wifiManager sta-enable'"});
            break;
        default:
            OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to connect to network",
                "Check SSID/password and ensure network is in range"});
            break;
    }
    return CliErrCode::FAILED;
}

CliErrCode CmdStaConnect(int argc, char** argv)
{
    WIFI_LOGI("sta-connect command started");
    OHOS::Wifi::WifiDeviceConfig tmpConfig;
    ParseConnectArgs(argc, argv, tmpConfig);
    if (tmpConfig.ssid.empty()) {
        OutputErrorJson(ErrMsg{"ERR_PARAM_INVALID", "Missing required parameter: --ssid",
            "Usage: ohos-wifiManager sta-connect --ssid <ssid> [--preSharedKey <preSharedKey>]"});
        return CliErrCode::FAILED;
    }
    auto wifiDevice = GetWifiDevice();
    auto wifiScan = GetWifiScan();
    if (wifiDevice == nullptr || wifiScan == nullptr) {
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get WifiScan instance",
            "Please check if WiFi scan service is available"});
        return CliErrCode::FAILED;
    }

    std::vector<OHOS::Wifi::WifiScanInfo> scanInfoList;
    OHOS::Wifi::ErrCode ret = wifiScan->GetScanInfoList(scanInfoList, false);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Cannot get the scan list",
            "Please check if WiFi scan service is available"});
        return CliErrCode::FAILED;
    }
    bool isFound = false;
    for (const auto& scanInfo : scanInfoList) {
        if (scanInfo.ssid == tmpConfig.ssid) {
            SetKeyMgmtBySecurityType(scanInfo.securityType, tmpConfig.keyMgmt);
            isFound = true;
        }
    }
    if (!isFound) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to find the network",
            "Check if the network is available"});
        return CliErrCode::FAILED;
    }

    return ExecuteWifiConnect(wifiDevice, tmpConfig);
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

CliErrCode CmdStaGetLinkedInfo(int argc, char** argv)
{
    auto wifiDevice = GetWifiDevice();
    if (wifiDevice == nullptr) {
        OutputErrorJson(ErrMsg{"INTERNAL_ERROR", "Failed to get wifiDevice instance",
            "Please check if WiFi service is available"});
        return CliErrCode::FAILED;
    }

    OHOS::Wifi::WifiLinkedInfo info;
    OHOS::Wifi::ErrCode ret = wifiDevice->GetLinkedInfo(info);
    if (ret != OHOS::Wifi::WIFI_OPT_SUCCESS) {
        OutputErrorJson(ErrMsg{"WIFI_ERROR", "Failed to get WiFi linked info",
            "Ensure WiFi is enabled and connected"});
        return CliErrCode::FAILED;
    }
    cJSON* data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "ssid", info.ssid.empty() ? "" : info.ssid.c_str());
    cJSON_AddStringToObject(data, "bssid", info.bssid.empty() ? "" : OHOS::Wifi::MacAnonymize(info.bssid).c_str());
    cJSON_AddNumberToObject(data, "rssi", info.rssi);
    cJSON_AddNumberToObject(data, "frequency", info.frequency);
    cJSON_AddNumberToObject(data, "linkSpeed", info.linkSpeed);
    std::string detailedState = GetDetailedStateStr(info.detailedState);
    cJSON_AddStringToObject(data, "detailedState", detailedState.c_str());
    OutputSuccessJson(data);
    WIFI_LOGI("Get WiFi linked info successfully");
    return CliErrCode::SUCCESS;
}

CliErrCode CmdHelp(int argc, char** argv)
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
    return CliErrCode::SUCCESS;
}

void InitCommands()
{
    g_commands["sta-enable"] = {"sta-enable", "Enable WiFi STA mode", CmdStaEnable};
    g_commands["sta-disable"] = {"sta-disable", "Disable WiFi STA mode", CmdStaDisable};
    g_commands["scan-start"] = {"scan-start", "Start WiFi scan", CmdScanStart};
    g_commands["scan-list"] = {"scan-list", "List scan results", CmdScanList};
    g_commands["sta-connect"] = {"sta-connect", "Start WiFi connect", CmdStaConnect};
    g_commands["sta-getLinkedInfo"] = {"sta-getLinkedInfo", "Return linked info", CmdStaGetLinkedInfo};
#ifdef WIFI_EXCEPTION_RECORD_ENABLE
    g_commands["wifi-exception"] = {"wifi-exception", "List/Clear WiFi exception records", CmdWifiException};
#endif
    g_commands["hotspot-enable"] = {"hotspot-enable", "Enable WiFi hotspot", CmdEnableHotspot};
    g_commands["hotspot-disable"] = {"hotspot-disable", "Disable WiFi hotspot", CmdDisableHotspot};
    g_commands["hotspot-config-set"] = {"hotspot-config-set", "Set hotspot SSID/password/band",
                                        CmdSetHotspotConfig};
    g_commands["hotspot-config-get"] = {"hotspot-config-get", "Show hotspot config and running state",
                                        CmdGetHotspotConfig};
    g_commands["hotspot-blocklist-get"] = {"hotspot-blocklist-get", "List blocklisted hotspot devices",
                                       CmdGetHotspotBlocklist};
    g_commands["--help"] = {"--help", "Show help information", CmdHelp};
}

int RunCommand(int argc, char** argv)
{
    WIFI_LOGI("enter ohos-wifiManager");
    InitCommands();
    if (argc < MIN_ARG_COUNT) {
        CmdHelp(argc, argv);
        return static_cast<int>(CliErrCode::FAILED);
    }
    std::string cmdName = argv[1];
    auto it = g_commands.find(cmdName);
    if (it == g_commands.end()) {
        WIFI_LOGE("Unknown command: %{public}s", cmdName.c_str());
        CmdHelp(argc, argv);
        return static_cast<int>(CliErrCode::FAILED);
    }

    int cmdArgc = argc - MIN_ARG_COUNT;
    char** cmdArgv = argv + MIN_ARG_COUNT;

    WIFI_LOGI("Executing command: %{public}s", cmdName.c_str());
    CliErrCode ret = it->second.handler(cmdArgc, cmdArgv);
    WIFI_LOGI("Command %{public}s finished with code %{public}d", cmdName.c_str(), static_cast<int>(ret));

    return static_cast<int>(ret);
}

} // namespace WifiCli
} // namespace OHOS
