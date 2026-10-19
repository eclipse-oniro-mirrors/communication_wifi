/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "wifitoolkit_fuzzer.h"
#include "wifi_fuzz_common_func.h"

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include "securec.h"
#include "define.h"
#include "wifi_log.h"
#include "wifi_app_parser.h"
#include "wifi_settings.h"
#include "network_parser.h"
#include "wifi_internal_msg.h"
#include "wifi_errcode.h"
#include "xml_parser.h"
#include "softap_parser.h"
#include "wifi_randommac_helper.h"
#include "wifi_asset_manager.h"
#include "wifi_encryption_util.h"
#include "wifi_notification_util.h"
#include "network_selection_manager.h"
#include "wifi_channel_helper.h"
#include "network_selection.h"
#include "network_selector_factory.h"
#include "wifi_filter_impl.h"
#include "wifi_scorer_impl.h"
#include "wifi_comparator_impl.h"
#include "network_selector_impl.h"
#include "wifi_watchdog_utils.h"
#include <mutex>
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace Wifi {
constexpr int U32_AT_SIZE_ZERO = 4;
constexpr int WIFI_MAX_SSID_LEN = 16;
constexpr int TWO = 2;
constexpr int SIX = 6;
constexpr int SEVEN = 7;
constexpr int FORTYTHREE = 43;
constexpr int HUNDRED = 100;
constexpr int TEST_GAME_RTT = 50;
constexpr int TEST_GAME_RTT_GAP = 10;
constexpr int TEST_ASYNC_DELAY_TIME = 100;
static bool g_isInsted = false;
FuzzedDataProvider *FDP = nullptr;
static const int32_t NUM_BYTES = 1;


class MockXmlParser : public XmlParser {
public:
    bool ParseInternal(xmlNodePtr node)
    {
        return false;
    }
};
static cJSON *monitorRoot = nullptr;
static cJSON *networkControlRoot = nullptr;
static cJSON *packageFilterRoot = nullptr;
static xmlNodePtr root_node;
static std::unique_ptr<NetworkXmlParser> m_networkXmlParser = nullptr;
static std::unique_ptr<AppParser> m_appXmlParser = nullptr;
static std::unique_ptr<MockXmlParser> m_xmlParser = nullptr;
static std::unique_ptr<SoftapXmlParser> m_softapXmlParser = nullptr;
static std::unique_ptr<WifiRandomMacHelper> m_WifiRandomMacHelper = nullptr;
static std::unique_ptr<WifiAssetManager> m_WifiAssetManager = nullptr;
static std::unique_ptr<WifiNotificationUtil> m_WifiNotificationUtil = nullptr;
static std::unique_ptr<NetworkSelectionManager> m_NetworkSelectionManager = nullptr;
static std::unique_ptr<WifiWatchDogUtils> m_WifiWatchDogUtils = nullptr;
void MyExit()
{
    cJSON_Delete(monitorRoot);
    cJSON_Delete(networkControlRoot);
    cJSON_Delete(packageFilterRoot);
    m_networkXmlParser.reset();
    m_appXmlParser.reset();
    m_xmlParser.reset();
    m_softapXmlParser.reset();
    m_WifiAssetManager.reset();
    m_WifiNotificationUtil.reset();
    m_NetworkSelectionManager.reset();
    sleep(3);
    printf("exiting\n");
}

void InitAppParserTest()
{
    root_node = xmlNewNode(NULL, BAD_CAST("MonitorAPP"));
    xmlNodePtr gameAppNode = xmlNewTextChild(root_node, NULL, BAD_CAST("GameInfo"), NULL);
    xmlNewProp(gameAppNode, BAD_CAST("gameName"), BAD_CAST "gameApp");
    xmlNodePtr whileListAppNode = xmlNewTextChild(root_node, NULL, BAD_CAST("AppWhiteList"), NULL);
    xmlNewProp(whileListAppNode, BAD_CAST("packageName"), BAD_CAST "whiteListApp");
    xmlNodePtr blackListAppNode = xmlNewTextChild(root_node, NULL, BAD_CAST("AppBlackList"), NULL);
    xmlNewProp(blackListAppNode, BAD_CAST("packageName"), BAD_CAST "blackListApp");
    xmlNodePtr chariotAppNode = xmlNewTextChild(root_node, NULL, BAD_CAST("ChariotApp"), NULL);
    xmlNewProp(chariotAppNode, BAD_CAST("packageName"), BAD_CAST "chariotApp");
}

void BuildMonitorAppJsonTest()
{
    /* wifi_monitor_apps.json structure */
    monitorRoot = cJSON_CreateObject();
    cJSON *gameInfoArray = cJSON_CreateArray();
    cJSON *gameItem = cJSON_CreateObject();
    cJSON_AddStringToObject(gameItem, "gameName", "gameApp");
    cJSON_AddNumberToObject(gameItem, "mGameRtt", TEST_GAME_RTT);
    cJSON_AddNumberToObject(gameItem, "mGameRttGap", TEST_GAME_RTT_GAP);
    cJSON_AddItemToArray(gameInfoArray, gameItem);
    cJSON_AddItemToObject(monitorRoot, "GameInfo", gameInfoArray);
    const char *whiteList[] = {"whiteListApp"};
    cJSON_AddItemToObject(monitorRoot, "AppWhiteList", cJSON_CreateStringArray(whiteList, 1));
    const char *blockListApp[] = {"blockListApp"};
    cJSON_AddItemToObject(monitorRoot, "AppBlack" "List", cJSON_CreateStringArray(blockListApp, 1));
    const char *multilinkList[] = {"multilinkApp"};
    cJSON_AddItemToObject(monitorRoot, "MultiLinkBlackList", cJSON_CreateStringArray(multilinkList, 1));
    const char *chariotList[] = {"chariotApp"};
    cJSON_AddItemToObject(monitorRoot, "ChariotApp", cJSON_CreateStringArray(chariotList, 1));
    const char *hiGameExcludeList[] = {"hiGameExcludeApp"};
    cJSON_AddItemToObject(monitorRoot, "HiGameExcludeList", cJSON_CreateStringArray(hiGameExcludeList, 1));
}
 
void BuildNetworkControlAppJsonTest()
{
    /* wifi_network_control_apps.json structure */
    networkControlRoot = cJSON_CreateObject();
    const char *keyForegroundList[] = {"keyForegroundApp"};
    cJSON_AddItemToObject(networkControlRoot, "KeyAppForegroundList",
        cJSON_CreateStringArray(keyForegroundList, 1));
    const char *keyBackgroundLimitList[] = {"keyBackgroundLimitApp"};
    cJSON_AddItemToObject(networkControlRoot, "KeyBackgroundLimitListApp",
        cJSON_CreateStringArray(keyBackgroundLimitList, 1));
    const char *gameBackgroundLimitList[] = {"gameBackgroundLimitApp"};
    cJSON_AddItemToObject(networkControlRoot, "GameBackgroundLimitListApp",
        cJSON_CreateStringArray(gameBackgroundLimitList, 1));
    const char *rssGameList[] = {"rssGameApp"};
    cJSON_AddItemToObject(networkControlRoot, "RssGameList", cJSON_CreateStringArray(rssGameList, 1));
    const char *liveStreamList[] = {"liveStreamApp"};
    cJSON_AddItemToObject(networkControlRoot, "AppLiveStream", cJSON_CreateStringArray(liveStreamList, 1));
    cJSON_AddNumberToObject(networkControlRoot, "AsyncDelayTime", TEST_ASYNC_DELAY_TIME);
}
 
void BuildPackageFilterJsonTest()
{
    /* wifi_package_filter.json structure */
    packageFilterRoot = cJSON_CreateObject();
    cJSON *scanControlSection = cJSON_CreateObject();
    const char *scanControlPkgs[] = {"scanControlApp"};
    cJSON_AddItemToObject(scanControlSection, "scanControlTag", cJSON_CreateStringArray(scanControlPkgs, 1));
    cJSON_AddItemToObject(packageFilterRoot, "ScanControlPackages", scanControlSection);
    const char *candidateFilterPkgs[] = {"candidateFilterApp"};
    cJSON_AddItemToObject(packageFilterRoot, "CandidateFilterPackages",
        cJSON_CreateStringArray(candidateFilterPkgs, 1));
    cJSON *coreSection = cJSON_CreateObject();
    cJSON_AddStringToObject(coreSection, "coreTag", "coreApp");
    cJSON_AddItemToObject(packageFilterRoot, "CorePackages", coreSection);
    cJSON *aclAuthArray = cJSON_CreateArray();
    cJSON *aclAuthItem = cJSON_CreateObject();
    cJSON_AddStringToObject(aclAuthItem, "name", "aclAuthApp");
    cJSON_AddStringToObject(aclAuthItem, "appid", "100");
    cJSON_AddItemToArray(aclAuthArray, aclAuthItem);
    cJSON_AddItemToObject(packageFilterRoot, "AclAuthPackages", aclAuthArray);
    cJSON *scanLimitArray = cJSON_CreateArray();
    cJSON *scanLimitItem = cJSON_CreateObject();
    cJSON_AddStringToObject(scanLimitItem, "name", "scanLimitApp");
    cJSON_AddStringToObject(scanLimitItem, "appid", "101");
    cJSON_AddItemToArray(scanLimitArray, scanLimitItem);
    cJSON_AddItemToObject(packageFilterRoot, "ScanLimitPackages", scanLimitArray);
    const char *landscapeLimitList[] = {"landscapeLimitApp"};
    cJSON_AddItemToObject(packageFilterRoot, "LandscapeSwitchLimitList",
        cJSON_CreateStringArray(landscapeLimitList, 1));
}
 
void InitAppParserJsonTest()
{
    if (m_appXmlParser == nullptr) {
        return;
    }
    BuildMonitorAppJsonTest();
    BuildNetworkControlAppJsonTest();
    BuildPackageFilterJsonTest();
}

void InitParam()
{
    if (!g_isInsted) {
        m_networkXmlParser = std::make_unique<NetworkXmlParser>();
        m_appXmlParser = std::make_unique<AppParser>();
        m_xmlParser = std::make_unique<MockXmlParser>();
        m_softapXmlParser = std::make_unique<SoftapXmlParser>();
        m_WifiAssetManager = std::make_unique<WifiAssetManager>();
        m_WifiNotificationUtil = std::make_unique<WifiNotificationUtil>();
        m_NetworkSelectionManager = std::make_unique<NetworkSelectionManager>();
        InitAppParserTest();
        InitAppParserJsonTest();
        if (m_networkXmlParser == nullptr) {
            return;
        }
        atexit(MyExit);
        g_isInsted = true;
    }
    return;
}

void NetworkXmlParserTest()
{
    WifiDeviceConfig config;
    config.ssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    config.bssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    config.preSharedKey = FDP->ConsumeBytesAsString(NUM_BYTES);
    config.keyMgmt = FDP->ConsumeBytesAsString(NUM_BYTES);

    m_networkXmlParser->GetIpConfig(root_node);
    m_networkXmlParser->GetConfigNameAsInt(root_node);
    m_networkXmlParser->GotoNetworkList(root_node);
    m_networkXmlParser->GetNodeNameAsInt(root_node);
    m_networkXmlParser->ParseIpConfig(root_node);
    m_networkXmlParser->GetProxyMethod(root_node);
    m_networkXmlParser->ParseProxyConfig(root_node);
    m_networkXmlParser->HasWepKeys(config);
    m_networkXmlParser->GetKeyMgmt(root_node, config);
    m_networkXmlParser->GetRandMacSetting(nullptr);
    m_networkXmlParser->ParseWifiConfig(root_node);
    m_networkXmlParser->ParseNetworkStatus(root_node, config);
    m_networkXmlParser->ParseWepKeys(root_node, config);
    m_networkXmlParser->ParseStatus(root_node, config);
    m_networkXmlParser->ParseNetwork(root_node);
    m_networkXmlParser->IsWifiConfigValid(config);

    m_networkXmlParser->ParseNetworkList(root_node);
    m_networkXmlParser->GetParseType(root_node);
    m_networkXmlParser->ParseInternal(root_node);
    m_networkXmlParser->EnableNetworks();
}

void AppXmlParserTest(const uint8_t* data, size_t size)
{
    char buf[WIFI_MAX_SSID_LEN] = {0};
    size_t n = size;
    if (n > static_cast<size_t>(WIFI_MAX_SSID_LEN - 1)) {
        n = WIFI_MAX_SSID_LEN - 1;
    }
    if (n > 0 && data != nullptr) {
        if (memcpy_s(buf, sizeof(buf), data, n) != EOK) {
            return;
        }
    }
    int gameRtt = 0;
    if (data != nullptr && size >= sizeof(gameRtt)) {
        if (memcpy_s(&gameRtt, sizeof(gameRtt), data, sizeof(gameRtt)) != EOK) {
            gameRtt = 0;
        }
    }
    if (!m_appXmlParser) {
        return;
    }
    m_appXmlParser->Init();
    std::string conditionName(reinterpret_cast<const char*>(data), size);
    m_appXmlParser->IsOverGameRtt(conditionName, gameRtt);
    m_appXmlParser->IsOverGameLowRttThresh(conditionName, gameRtt);
    m_appXmlParser->GetAsyncLimitSpeedDelayTime();
    m_appXmlParser->GetPackageName(conditionName);
    std::vector<PackageInfo> packageInfo;
    m_appXmlParser->GetPackageInfoByName(conditionName, packageInfo);
    std::map<std::string, std::vector<PackageInfo>> packageInfoMap;
    m_appXmlParser->GetPackageInfoMap(packageInfoMap);
    m_appXmlParser->ParseMonitorAppConfig(monitorRoot);
    m_appXmlParser->ParseNetworkControlAppConfig(networkControlRoot);
    m_appXmlParser->ParsePackageFilterConfig(packageFilterRoot);
    m_xmlParser->LoadConfiguration(buf);
    m_xmlParser->LoadConfigurationMemory(buf);
    ConvertStringToBool(buf);
}

void AppXmlParserTest2(const uint8_t* data, size_t size)
{
    char buf[WIFI_MAX_SSID_LEN] = {0};
    size_t n = size;
    if (n > static_cast<size_t>(WIFI_MAX_SSID_LEN - 1)) {
        n = WIFI_MAX_SSID_LEN - 1;
    }
    if (n > 0 && data != nullptr) {
        if (memcpy_s(buf, sizeof(buf), data, n) != EOK) {
            return;
        }
    }
    int gameRtt = 0;
    if (data != nullptr && size >= sizeof(gameRtt)) {
        if (memcpy_s(&gameRtt, sizeof(gameRtt), data, sizeof(gameRtt)) != EOK) {
            gameRtt = 0;
        }
    }
    if (!m_appXmlParser) {
        return;
    }
    m_appXmlParser->Init();
    std::string conditionName(reinterpret_cast<const char*>(data), size);
    m_appXmlParser->IsLowLatencyApp(conditionName);
    m_appXmlParser->IsWhiteListApp(conditionName);
    m_appXmlParser->IsBlackListApp(conditionName);
    m_appXmlParser->IsMultiLinkApp(conditionName);
    m_appXmlParser->IsChariotApp(conditionName);
    m_appXmlParser->IsKeyForegroundApp(conditionName);
    m_appXmlParser->IsKeyBackgroundLimitApp(conditionName);
    m_appXmlParser->IsLiveStreamApp(conditionName);
    m_appXmlParser->IsGameBackgroundLimitApp(conditionName);
    m_appXmlParser->IsRssGameApp(conditionName);
    m_appXmlParser->IsHiGameExcludeApp(conditionName);
}

void AppParserTest()
{
    m_xmlParser->Parse();
    m_xmlParser->GetNameValue(root_node);
    m_xmlParser->GetNodeValue(root_node);
    m_xmlParser->GetStringValue(root_node);
    m_xmlParser->GetStringArrValue(root_node);
    m_xmlParser->GetByteArrValue(root_node);
    m_xmlParser->GetStringMapValue(root_node);
    m_xmlParser->IsDocValid(root_node);
}

void SoftapParserTest()
{
    m_softapXmlParser->ParseInternal(root_node);
    m_softapXmlParser->GotoSoftApNode(root_node);
    m_softapXmlParser->ParseSoftap(root_node);
    m_softapXmlParser->GetConfigNameAsInt(root_node);
    m_softapXmlParser->GetBandInfo(root_node);
    m_softapXmlParser->TransBandinfo(root_node);
    m_softapXmlParser->GetSoftapConfigs();
}

void WifiRandomMacHelperTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider FDP(data, size);
    int index = 0;
    unsigned long long random = static_cast<unsigned long long>(data[index++]);
    std::string content = FDP.ConsumeBytesAsString(NUM_BYTES);
    std::string randomMacAddr = FDP.ConsumeBytesAsString(NUM_BYTES);
    std::vector<uint8_t> outPlant;
    std::vector<uint8_t> byte;
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> addr;
    bool value = FDP.ConsumeBool();
    #ifdef SUPPORT_LOCAL_RANDOM_MAC
    m_WifiRandomMacHelper->CalculateRandomMacForWifiDeviceConfig(content, randomMacAddr);
    #endif
    m_WifiRandomMacHelper->GetRandom();
    m_WifiRandomMacHelper->GenerateRandomMacAddress(randomMacAddr);
    m_WifiRandomMacHelper->LongLongToBytes(value, outPlant);
    m_WifiRandomMacHelper->BytesToLonglong(byte);
    m_WifiRandomMacHelper->BytesArrayToString(bytes);
    m_WifiRandomMacHelper->LongAddrFromByteAddr(addr);
    m_WifiRandomMacHelper->GenerateRandomMacAddressByLong(random, randomMacAddr);
}

void WifiRandomMacHelperTest01(const uint8_t* data)
{
    int index = 0;
    unsigned long long addr1 = static_cast<unsigned long long>(data[index++]);
    std::string randomMacAddr = FDP->ConsumeBytesAsString(NUM_BYTES);
    m_WifiRandomMacHelper->StringAddrFromLongAddr(addr1, randomMacAddr);
}


void AssetManagerTest()
{
    WifiDeviceConfig config;
    int32_t randomInt = FDP->ConsumeIntegral<int32_t>();
    OperateResState state = static_cast<OperateResState>(randomInt % FORTYTHREE);
    WifiLinkedInfo info;
    int32_t tmpInt = FDP->ConsumeIntegral<int32_t>();
    if (tmpInt <= 0 || tmpInt > HUNDRED) {
        return;
    }
    std::vector<WifiDeviceConfig>wifiDeviceConfigs;
    std::vector<WifiDeviceConfig> newWifiDeviceConfigs;
    bool flagSync = FDP->ConsumeIntegral<bool>();
    bool firstSync = FDP->ConsumeIntegral<bool>();
    m_WifiAssetManager->WifiAssetQuery(tmpInt);
    m_WifiAssetManager->WifiAssetUpdate(config, tmpInt);
    m_WifiAssetManager->WifiAssetAddPack(wifiDeviceConfigs, tmpInt, flagSync, firstSync);
    m_WifiAssetManager->WifiAssetUpdatePack(wifiDeviceConfigs, tmpInt);
    m_WifiAssetManager->WifiAssetRemovePack(wifiDeviceConfigs, tmpInt, flagSync);
    m_WifiAssetManager->WifiAssetRemoveAll(tmpInt, flagSync);
    m_WifiAssetManager->IsWifiConfigUpdated(newWifiDeviceConfigs, config);
    m_WifiAssetManager->WifiAssetRemoveAll(tmpInt, flagSync);
    m_WifiAssetManager->IsWifiConfigUpdated(newWifiDeviceConfigs, config);
    m_WifiAssetManager->DealStaConnChanged(state, info, tmpInt);
}

void WifiencryptionutilTest()
{
    WifiEncryptionInfo wifiEncryptionInfo;
    EncryptedData encryptedData;
    std::string decryptedData = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string key = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string inputString = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string keyName = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string data = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::vector<uint8_t> outPlant;
    SetUpHks();
    WifiDecryption(wifiEncryptionInfo, encryptedData, decryptedData);
    ImportKey(wifiEncryptionInfo, key);
    DeleteKey(wifiEncryptionInfo);
    WifiLoopEncrypt(wifiEncryptionInfo, inputString, encryptedData);
    WifiLoopDecrypt(wifiEncryptionInfo, encryptedData, decryptedData);
    WifiGenerateMacRandomizationSecret(keyName, data, outPlant);
}

void WifinetworkselectionmanagerTest()
{
    WifiDeviceConfig deviceConfig;
    WifiScanInfo wifiScanInfo;
    InterScanInfo interScanInfo;
    NetworkSelectionResult networkSelectionResult;
    std::string autoSelectBssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::vector<NetworkSelection::NetworkCandidate> networkCandidates;
    std::vector<InterScanInfo> scanInfos;
    if (m_NetworkSelectionManager == nullptr) {
        return;
    }
    m_NetworkSelectionManager->SelectNetworkWithSsid(deviceConfig, autoSelectBssid);
    m_NetworkSelectionManager->GetAllDeviceConfigs(networkCandidates, scanInfos);
    m_NetworkSelectionManager->ConvertScanInfo(wifiScanInfo, interScanInfo);
    m_NetworkSelectionManager->GetFilteredReasonForChr(networkCandidates);
}

void WifinetworkselectionmanagerSelectTest()
{
    if (m_NetworkSelectionManager == nullptr) {
        return;
    }
    NetworkSelectionResult networkSelectionResult;
    std::vector<InterScanInfo> scanInfos;
    InterScanInfo scanInfo;
    scanInfo.ssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    scanInfo.bssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    scanInfo.frequency = FDP->ConsumeIntegral<int>();
    scanInfo.rssi = FDP->ConsumeIntegral<int>();
    scanInfos.push_back(scanInfo);
    int selectType = FDP->ConsumeIntegral<int>() % SEVEN;
    NetworkSelectType type = static_cast<NetworkSelectType>(selectType);
    std::string failReason;
    m_NetworkSelectionManager->SelectNetwork(networkSelectionResult, type, scanInfos, failReason);
    std::vector<NetworkSelectionResult> allSortedResults;
    m_NetworkSelectionManager->SelectNetwork(networkSelectionResult, type, scanInfos, failReason, allSortedResults);
    std::vector<NetworkSelection::NetworkCandidate> networkCandidates;
    NetworkSelectorFactory selectorFactory;
    auto selectorOptional = selectorFactory.GetNetworkSelector(type);
    if (selectorOptional.has_value()) {
        NetworkSelectionManager::TryNominate(networkCandidates, selectorOptional.value());
    }
}

void WifinotificationutilTest()
{
    int32_t randomInt = FDP->ConsumeIntegral<int32_t>();
    WifiNotificationId notificationId = static_cast<WifiNotificationId>(randomInt % TWO);
    WifiDialogType type = static_cast<WifiDialogType>(randomInt % SIX);
    std::string bundleName = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string abilityName = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string navEntryKey = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string comInfo = FDP->ConsumeBytesAsString(NUM_BYTES);
    std::string settings = FDP->ConsumeBytesAsString(NUM_BYTES);
    m_WifiNotificationUtil->CancelWifiNotification(notificationId);
    m_WifiNotificationUtil->DisplaySettingWlanPage(bundleName, abilityName, navEntryKey);
    m_WifiNotificationUtil->ShowDialog(type, comInfo);
    m_WifiNotificationUtil->ShowSettingsDialog(type, settings);
}

void WifiWatchDogUtilsTest()
{
    uint64_t taskId = FDP->ConsumeIntegral<uint64_t>();
    std::string taskInfoStr = FDP->ConsumeBytesAsString(NUM_BYTES);
    uint32_t delayedTaskCount = FDP->ConsumeIntegral<uint32_t>();
    bool usingHiviewDfx = FDP->ConsumeIntegral<bool>();
    bool notResetProcess = true;
    std::string threadName = FDP->ConsumeBytesAsString(NUM_BYTES);

    const size_t kMaxTaskInfoLength = 128;
    if (taskInfoStr.length() > kMaxTaskInfoLength) {
        taskInfoStr = taskInfoStr.substr(0, kMaxTaskInfoLength);
    }

    if (!taskInfoStr.empty() && taskInfoStr.back() != '\0') {
        taskInfoStr += '\0';
    }

    if (!taskInfoStr.empty() && taskInfoStr.find('\0') != std::string::npos) {
        size_t nullPos = taskInfoStr.find('\0');
        taskInfoStr = taskInfoStr.substr(0, nullPos);
    }

    if (!taskInfoStr.empty()) {
        m_WifiWatchDogUtils->FfrtCallback(taskId, taskInfoStr.c_str(), delayedTaskCount);
    } else {
        return;
    }

    m_WifiWatchDogUtils->ResetProcess(usingHiviewDfx, threadName, notResetProcess);
    m_WifiWatchDogUtils->ReportResetEvent(threadName);
}

void BuildScanInfoFromFuzz(InterScanInfo &scanInfo)
{
    scanInfo.ssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    scanInfo.bssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    scanInfo.capabilities = FDP->ConsumeBytesAsString(NUM_BYTES);
    scanInfo.frequency = FDP->ConsumeIntegral<int>();
    scanInfo.band = FDP->ConsumeIntegral<int>();
    scanInfo.rssi = FDP->ConsumeIntegral<int>();
    scanInfo.securityType = static_cast<WifiSecurity>(FDP->ConsumeIntegral<int>());
    scanInfo.riskType = static_cast<WifiRiskType>(FDP->ConsumeIntegral<int>());
    scanInfo.supportedWifiCategory = static_cast<WifiCategory>(FDP->ConsumeIntegral<int>());
}

void BuildDeviceConfigFromFuzz(NetworkSelection::NetworkCandidate &candidate)
{
    candidate.wifiDeviceConfig.networkId = FDP->ConsumeIntegral<int>();
    candidate.wifiDeviceConfig.uid = FDP->ConsumeIntegral<int>();
    candidate.wifiDeviceConfig.isShared = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.isEphemeral = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.isPasspoint = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.isAllowAutoConnect = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.isSecureWifi = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.noInternetAccess = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.isPortal = FDP->ConsumeBool();
    candidate.wifiDeviceConfig.userSelectBssid = FDP->ConsumeBytesAsString(NUM_BYTES);
    candidate.wifiDeviceConfig.keyMgmt = FDP->ConsumeBytesAsString(NUM_BYTES);
    candidate.wifiDeviceConfig.lastHasInternetTime = FDP->ConsumeIntegral<int64_t>();
    candidate.wifiDeviceConfig.lastDisconnectTime = FDP->ConsumeIntegral<int64_t>();
    candidate.wifiDeviceConfig.networkSelectionStatus.status =
        static_cast<WifiDeviceConfigStatus>(FDP->ConsumeIntegral<int>());
}

void WifiFilterImplTest1(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::HiddenWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SignalStrengthWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SavedWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::EphemeralWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::PassPointWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::DisableWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::MatchedUserSelectBssidWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::HasInternetWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::RecoveryWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::PoorPortalWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
}

void WifiFilterImplTest2(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::PortalWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::MaybePortalWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::NoInternetWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::WeakAlgorithmWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::NotCurrentNetworkFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SameBssidNetworkFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SignalLevelFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::ValidNetworkIdFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::NotNetworkBlackListFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::NotP2pFreqAt5gFilter filter;
        (void)filter.DoFilter(candidate);
    }
}

void WifiFilterImplTest3(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::ValidConfigNetworkFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        int32_t rssiInterval = FDP->ConsumeIntegral<int32_t>();
        NetworkSelection::WifiSwitchThresholdFilter filter(rssiInterval);
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SuggestionNetworkWifiFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::WifiSwitchThresholdQoeFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::WifiSwitch5GNot2GFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::SecureNetworkFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::HigherCategoryFilter filter;
        (void)filter.DoFilter(candidate);
    }
    {
        NetworkSelection::Perf5gBlackListFilter filter;
        (void)filter.DoFilter(candidate);
    }
}

void WifiFilterImplTest()
{
    InterScanInfo scanInfo;
    BuildScanInfoFromFuzz(scanInfo);
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    BuildDeviceConfigFromFuzz(candidate);
    WifiFilterImplTest1(candidate);
    WifiFilterImplTest2(candidate);
    WifiFilterImplTest3(candidate);
}

void WifiScorerImplTest1(NetworkSelection::NetworkCandidate &candidate)
{
    NetworkSelection::ScoreResult scoreResult;
    {
        NetworkSelection::RssiScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::LastHaveInternetTimeScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::NetworkStatusHistoryScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::ThroughputScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::SecurityBonusScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::RssiLevelBonusScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
}

void WifiScorerImplTest2(NetworkSelection::NetworkCandidate &candidate)
{
    NetworkSelection::ScoreResult scoreResult;
    {
        NetworkSelection::SignalLevelScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::Network5gBonusScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        std::string name = FDP->ConsumeBytesAsString(NUM_BYTES);
        NetworkSelection::SavedNetworkScorer scorer(name);
        scorer.DoScore(candidate, scoreResult);
    }
    {
        NetworkSelection::NoInternetNetworkStatusHistoryScorer scorer;
        scorer.DoScore(candidate, scoreResult);
    }
    {
        std::string name = FDP->ConsumeBytesAsString(NUM_BYTES);
        NetworkSelection::ApQualityScorer scorer(name);
        scorer.DoScore(candidate, scoreResult);
    }
}

void WifiScorerImplTest()
{
    InterScanInfo scanInfo;
    BuildScanInfoFromFuzz(scanInfo);
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    BuildDeviceConfigFromFuzz(candidate);
    WifiScorerImplTest1(candidate);
    WifiScorerImplTest2(candidate);
}

void BuildComparatorCandidates(std::vector<NetworkSelection::NetworkCandidate *> &ptrs)
{
    constexpr int NUM_CANDIDATES = 3;
    static std::vector<InterScanInfo> scanInfos(NUM_CANDIDATES);
    static std::vector<NetworkSelection::NetworkCandidate> candidates;
    candidates.clear();
    for (int i = 0; i < NUM_CANDIDATES; ++i) {
        scanInfos[i].ssid = FDP->ConsumeBytesAsString(NUM_BYTES);
        scanInfos[i].bssid = FDP->ConsumeBytesAsString(NUM_BYTES);
        scanInfos[i].capabilities = FDP->ConsumeBytesAsString(NUM_BYTES);
        scanInfos[i].frequency = FDP->ConsumeIntegral<int>();
        scanInfos[i].band = FDP->ConsumeIntegral<int>();
        scanInfos[i].rssi = FDP->ConsumeIntegral<int>();
        scanInfos[i].securityType = static_cast<WifiSecurity>(FDP->ConsumeIntegral<int>());
        candidates.emplace_back(scanInfos[i]);
        candidates.back().wifiDeviceConfig.networkId = FDP->ConsumeIntegral<int>();
        candidates.back().wifiDeviceConfig.uid = FDP->ConsumeIntegral<int>();
        candidates.back().wifiDeviceConfig.isShared = FDP->ConsumeBool();
        candidates.back().wifiDeviceConfig.isEphemeral = FDP->ConsumeBool();
        candidates.back().wifiDeviceConfig.lastHasInternetTime = FDP->ConsumeIntegral<int64_t>();
        candidates.back().wifiDeviceConfig.keyMgmt = FDP->ConsumeBytesAsString(NUM_BYTES);
    }
    for (auto &c : candidates) {
        ptrs.push_back(&c);
    }
}

void WifiComparatorImplTest1(std::vector<NetworkSelection::NetworkCandidate *> &ptrs)
{
    {
        NetworkSelection::WifiScorerComparator comparator("fuzzComparator");
        comparator.AddScorer(std::make_shared<NetworkSelection::RssiScorer>());
        comparator.AddScorer(std::make_shared<NetworkSelection::SignalLevelScorer>());
        comparator.AddScorer(std::make_shared<NetworkSelection::Network5gBonusScorer>());
        std::vector<NetworkSelection::NetworkCandidate *> bestCandidates;
        comparator.GetBestCandidates(ptrs, bestCandidates);
        std::vector<NetworkSelection::NetworkCandidate *> sortedCandidates;
        comparator.GetAllSortedCandidates(ptrs, sortedCandidates);
    }
    {
        NetworkSelection::WifiScorerComparator comparator("fuzzComparator2");
        comparator.AddScorer(std::make_shared<NetworkSelection::ThroughputScorer>());
        comparator.AddScorer(std::make_shared<NetworkSelection::RssiScorer>());
        std::vector<NetworkSelection::NetworkCandidate *> bestCandidates;
        comparator.GetBestCandidates(ptrs, bestCandidates);
        std::vector<NetworkSelection::NetworkCandidate *> sortedCandidates;
        comparator.GetAllSortedCandidates(ptrs, sortedCandidates);
    }
}

void WifiComparatorImplTest2(std::vector<NetworkSelection::NetworkCandidate *> &ptrs)
{
    {
        NetworkSelection::WifiScorerComparator comparator("fuzzComparator3");
        comparator.AddScorer(nullptr);
        comparator.AddScorer(std::make_shared<NetworkSelection::RssiScorer>());
        std::vector<NetworkSelection::NetworkCandidate *> bestCandidates;
        comparator.GetBestCandidates(ptrs, bestCandidates);
    }
    {
        NetworkSelection::WifiScorerComparator comparator("fuzzComparator4");
        std::vector<NetworkSelection::NetworkCandidate *> emptyCandidates;
        std::vector<NetworkSelection::NetworkCandidate *> bestCandidates;
        comparator.GetBestCandidates(emptyCandidates, bestCandidates);
        comparator.GetAllSortedCandidates(emptyCandidates, bestCandidates);
    }
}

void WifiComparatorImplTest()
{
    std::vector<NetworkSelection::NetworkCandidate *> ptrs;
    BuildComparatorCandidates(ptrs);
    WifiComparatorImplTest1(ptrs);
    WifiComparatorImplTest2(ptrs);
}

void NetworkSelectorImplTest1(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::BlackListNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::AllNoInternetHistoryNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::HasInternetNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::RecoveryNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::PortalNetworkSelector selector;
        selector.InitFilter();
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
}

void NetworkSelectorImplTest2(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::NoInternetNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::PreferredApSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::SuggestionNetworkTracker selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::HigherCategoryNetworkSelector selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::SameSsidMultiBssidIntegrator selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
}

void NetworkSelectorImplTest3(NetworkSelection::NetworkCandidate &candidate)
{
    {
        NetworkSelection::AutoConnectIntegrator selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::Wifi2WifiIntegrator selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::Wifi2WifiNoNetIntegrator selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::Wifi2WifiQoeSlowIntegrator selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
    {
        NetworkSelection::SavedNetworkTracker selector;
        (void)selector.TryNominate(candidate);
        std::vector<NetworkSelection::NetworkCandidate *> result;
        selector.GetBestCandidates(result);
    }
}

void NetworkSelectorImplTest()
{
    InterScanInfo scanInfo;
    BuildScanInfoFromFuzz(scanInfo);
    NetworkSelection::NetworkCandidate candidate(scanInfo);
    BuildDeviceConfigFromFuzz(candidate);
    NetworkSelectorImplTest1(candidate);
    NetworkSelectorImplTest2(candidate);
    NetworkSelectorImplTest3(candidate);
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= OHOS::Wifi::U32_AT_SIZE_ZERO)) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::Wifi::FDP = &fdp;
    OHOS::Wifi::InitParam();
    OHOS::Wifi::AppXmlParserTest(data, size);
    OHOS::Wifi::WifiRandomMacHelperTest(data, size);
    OHOS::Wifi::WifiRandomMacHelperTest01(data);
    OHOS::Wifi::AppXmlParserTest2(data, size);
    OHOS::Wifi::NetworkXmlParserTest();
    OHOS::Wifi::AppParserTest();
    OHOS::Wifi::SoftapParserTest();
    OHOS::Wifi::WifiencryptionutilTest();
    OHOS::Wifi::WifinetworkselectionmanagerTest();
    OHOS::Wifi::WifinetworkselectionmanagerSelectTest();
    OHOS::Wifi::AssetManagerTest();
    OHOS::Wifi::WifinotificationutilTest();
    OHOS::Wifi::WifiWatchDogUtilsTest();
    OHOS::Wifi::WifiFilterImplTest();
    OHOS::Wifi::WifiScorerImplTest();
    OHOS::Wifi::WifiComparatorImplTest();
    OHOS::Wifi::NetworkSelectorImplTest();
    return 0;
}
}
}