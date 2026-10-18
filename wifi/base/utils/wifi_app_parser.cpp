/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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
#include "wifi_app_parser.h"
#include <algorithm>
#include <fstream>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include "wifi_logger.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_LABEL("WifiAppJsonParser");

constexpr auto WIFI_MONITOR_APP_FILE_PATH = "/system/etc/wifi/wifi_monitor_apps.json";
constexpr auto WIFI_NETWORK_CONTROL_APP_FILE_PATH = "/system/etc/wifi/wifi_network_control_apps.json";
constexpr auto WIFI_PACKAGE_FILTER_FILE_PATH = "/system/etc/wifi/wifi_package_filter.json";

/* common keys */
constexpr auto JSON_KEY_NAME = "name";
constexpr auto JSON_KEY_APPID = "appid";

/* wifi_monitor_apps.json keys */
constexpr auto JSON_KEY_GAME_INFO = "GameInfo";
constexpr auto JSON_KEY_GAME_NAME = "gameName";
constexpr auto JSON_KEY_GAME_RTT = "mGameRtt";
constexpr auto JSON_KEY_GAME_RTT_GAP = "mGameRttGap";
constexpr auto JSON_KEY_APP_WHITE_LIST = "AppWhiteList";
constexpr auto JSON_KEY_APP_BLACK_LIST = "AppBlackList";
constexpr auto JSON_KEY_MULTILINK_BLACK_LIST = "MultiLinkBlackList";
constexpr auto JSON_KEY_CHARIOT_APP = "ChariotApp";
constexpr auto JSON_KEY_HIGAME_EXCLUDE_LIST = "HiGameExcludeList";

/* wifi_network_control_apps.json keys */
constexpr auto JSON_KEY_APP_KEY_FOREGROUND_LIST = "KeyAppForegroundList";
constexpr auto JSON_KEY_APP_KEY_BACKGROUND_LIMIT_LIST = "KeyBackgroundLimitListApp";
constexpr auto JSON_KEY_APP_GAME_BACKGROUND_LIMIT_LIST = "GameBackgroundLimitListApp";
constexpr auto JSON_KEY_RSS_GAME_LIST = "RssGameList";
constexpr auto JSON_KEY_APP_LIVE_STREAM_LIST = "AppLiveStream";
constexpr auto JSON_KEY_ASYNC_DELAY_TIME = "AsyncDelayTime";

/* wifi_package_filter.json keys */
constexpr auto JSON_KEY_SCAN_CONTROL_PACKAGES = "ScanControlPackages";
constexpr auto JSON_KEY_CANDIDATE_FILTER_PACKAGES = "CandidateFilterPackages";
constexpr auto JSON_KEY_CORE_PACKAGES = "CorePackages";
constexpr auto JSON_KEY_ACL_AUTH_PACKAGES = "AclAuthPackages";
constexpr auto JSON_KEY_SCAN_LIMIT_PACKAGES = "ScanLimitPackages";
constexpr auto JSON_KEY_LANDSCAPE_SWITCH_LIMIT_LIST = "LandscapeSwitchLimitList";

constexpr int MAX_JSON_FILE_SIZE = 1024 * 1024;

namespace {
cJSON *LoadJsonFile(const char *jsonFilePath)
{
    if (jsonFilePath == nullptr) {
        return nullptr;
    }
    std::ifstream inFile(jsonFilePath, std::ios::in | std::ios::binary);
    if (!inFile.is_open()) {
        WIFI_LOGE("%{public}s open %{public}s failed", __FUNCTION__, jsonFilePath);
        return nullptr;
    }
    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    if (content.empty() || content.size() > static_cast<size_t>(MAX_JSON_FILE_SIZE)) {
        WIFI_LOGE("%{public}s %{public}s content invalid, size:%{public}zu", __FUNCTION__,
            jsonFilePath, content.size());
        return nullptr;
    }
    cJSON *root = cJSON_Parse(content.c_str());
    if (root == nullptr) {
        WIFI_LOGE("%{public}s parse %{public}s failed", __FUNCTION__, jsonFilePath);
    }
    return root;
}

bool GetStringFromItem(const cJSON *item, std::string &out)
{
    if (item == nullptr || !cJSON_IsString(item) || item->valuestring == nullptr) {
        return false;
    }
    out = item->valuestring;
    return true;
}

/* Parse a string array section (e.g. "AppWhiteList": ["pkg1", "pkg2"]) directly
 * into the typed app info list. */
template<typename T>
void ParseStringArray(const cJSON *root, const char *key, std::vector<T> &out)
{
    static_assert(std::is_base_of_v<CommonAppInfo, T>, "T must derive from CommonAppInfo");
    cJSON *array = cJSON_GetObjectItem(root, key);
    if (array == nullptr || !cJSON_IsArray(array)) {
        return;
    }
    int size = cJSON_GetArraySize(array);
    for (int i = 0; i < size; ++i) {
        cJSON *item = cJSON_GetArrayItem(array, i);
        std::string packageName;
        if (GetStringFromItem(item, packageName) && !packageName.empty()) {
            T appInfo {};
            appInfo.packageName = packageName;
            out.push_back(appInfo);
        } else {
            WIFI_LOGE("%{public}s parse %{public}s item error, index:%{public}d", __FUNCTION__, key, i);
        }
    }
}

/* Parse the game info array (e.g. [{"gameName": "pkg", "mGameRtt": 50, "mGameRttGap": 10}]). */
void ParseGameInfoArray(const cJSON *root, AppParserResult &out)
{
    cJSON *gameArray = cJSON_GetObjectItem(root, JSON_KEY_GAME_INFO);
    if (gameArray == nullptr || !cJSON_IsArray(gameArray)) {
        return;
    }
    int size = cJSON_GetArraySize(gameArray);
    for (int i = 0; i < size; ++i) {
        cJSON *entry = cJSON_GetArrayItem(gameArray, i);
        if (entry == nullptr || !cJSON_IsObject(entry)) {
            continue;
        }
        std::string gameName;
        if (!GetStringFromItem(cJSON_GetObjectItem(entry, JSON_KEY_GAME_NAME), gameName)
            || gameName.empty()) {
            WIFI_LOGE("%{public}s parse game info error, index:%{public}d", __FUNCTION__, i);
            continue;
        }
        LowLatencyAppInfo appInfo {};
        appInfo.packageName = gameName;
        out.m_lowLatencyAppVec.push_back(appInfo);
        cJSON *rttItem = cJSON_GetObjectItem(entry, JSON_KEY_GAME_RTT);
        if (rttItem != nullptr && cJSON_IsNumber(rttItem)) {
            out.m_gameInfo[gameName].rtt = rttItem->valueint;
        }
        cJSON *rttGapItem = cJSON_GetObjectItem(entry, JSON_KEY_GAME_RTT_GAP);
        if (rttGapItem != nullptr && cJSON_IsNumber(rttGapItem)) {
            out.m_gameInfo[gameName].rttGap = rttGapItem->valueint;
        }
    }
}

/* Parse the AsyncDelayTime field (number or numeric string).
 * Returns true and fills delayTime if the key exists, otherwise keeps it untouched. */
bool ParseAsyncDelayTime(const cJSON *root, std::string &delayTime)
{
    cJSON *delayItem = cJSON_GetObjectItem(root, JSON_KEY_ASYNC_DELAY_TIME);
    if (delayItem == nullptr) {
        return false;
    }
    if (cJSON_IsNumber(delayItem)) {
        delayTime = std::to_string(delayItem->valueint);
    } else {
        GetStringFromItem(delayItem, delayTime);
    }
    if (delayTime.empty()) {
        WIFI_LOGE("%{public}s delay time is null, will set 0.", __FUNCTION__);
        delayTime = "0";
    }
    return true;
}

/* wifi_package_filter.json section parsers: each function reads its own
 * section from the config root and fills the package info map. */

/* Parse a package name string array (e.g. ["pkg1", "pkg2"]) into a list. */
void ParseNameList(const cJSON *array, std::vector<PackageInfo> &out)
{
    if (array == nullptr || !cJSON_IsArray(array)) {
        return;
    }
    int size = cJSON_GetArraySize(array);
    for (int i = 0; i < size; ++i) {
        std::string name;
        if (GetStringFromItem(cJSON_GetArrayItem(array, i), name) && !name.empty()) {
            PackageInfo info {};
            info.name = name;
            out.push_back(info);
        }
    }
}

void ParseScanControlPackages(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_SCAN_CONTROL_PACKAGES);
    if (section == nullptr || !cJSON_IsObject(section)) {
        WIFI_LOGE("%{public}s ScanControlPackages section invalid", __FUNCTION__);
        return;
    }
    cJSON *tagNode = section->child;
    while (tagNode != nullptr) {
        if (tagNode->string == nullptr || !cJSON_IsArray(tagNode)) {
            tagNode = tagNode->next;
            continue;
        }
        std::vector<PackageInfo> list;
        ParseNameList(tagNode, list);
        out.insert_or_assign(tagNode->string, list);
        tagNode = tagNode->next;
    }
}

void ParseCandidateFilterPackages(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_CANDIDATE_FILTER_PACKAGES);
    if (section == nullptr || !cJSON_IsArray(section)) {
        WIFI_LOGE("%{public}s CandidateFilterPackages section invalid", __FUNCTION__);
        return;
    }
    std::vector<PackageInfo> list;
    ParseNameList(section, list);
    out.insert_or_assign(JSON_KEY_CANDIDATE_FILTER_PACKAGES, list);
}

void ParseCorePackages(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_CORE_PACKAGES);
    if (section == nullptr || !cJSON_IsObject(section)) {
        WIFI_LOGE("%{public}s CorePackages section invalid", __FUNCTION__);
        return;
    }
    cJSON *tagNode = section->child;
    while (tagNode != nullptr) {
        std::string name;
        if (tagNode->string != nullptr && GetStringFromItem(tagNode, name) && !name.empty()) {
            PackageInfo info {};
            info.name = name;
            out.insert_or_assign(tagNode->string, std::vector<PackageInfo> {info});
        }
        tagNode = tagNode->next;
    }
}

void ParseAclAuthPackages(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_ACL_AUTH_PACKAGES);
    if (section == nullptr || !cJSON_IsArray(section)) {
        WIFI_LOGE("%{public}s AclAuthPackages section invalid", __FUNCTION__);
        return;
    }
    std::vector<PackageInfo> list;
    int size = cJSON_GetArraySize(section);
    for (int i = 0; i < size; ++i) {
        cJSON *item = cJSON_GetArrayItem(section, i);
        if (item == nullptr || !cJSON_IsObject(item)) {
            continue;
        }
        PackageInfo info {};
        if (!GetStringFromItem(cJSON_GetObjectItem(item, JSON_KEY_NAME), info.name) || info.name.empty()) {
            WIFI_LOGE("%{public}s parse AclAuthPackages item error, index:%{public}d", __FUNCTION__, i);
            continue;
        }
        /* appid is optional: keep the default empty value when absent or invalid */
        (void)GetStringFromItem(cJSON_GetObjectItem(item, JSON_KEY_APPID), info.appid);
        list.push_back(info);
    }
    out.insert_or_assign(JSON_KEY_ACL_AUTH_PACKAGES, list);
}

void ParseScanLimitPackages(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_SCAN_LIMIT_PACKAGES);
    if (section == nullptr || !cJSON_IsArray(section)) {
        WIFI_LOGE("%{public}s ScanLimitPackages section invalid", __FUNCTION__);
        return;
    }
    std::vector<PackageInfo> list;
    int size = cJSON_GetArraySize(section);
    for (int i = 0; i < size; ++i) {
        cJSON *item = cJSON_GetArrayItem(section, i);
        if (item == nullptr || !cJSON_IsObject(item)) {
            continue;
        }
        PackageInfo info {};
        if (!GetStringFromItem(cJSON_GetObjectItem(item, JSON_KEY_NAME), info.name) || info.name.empty()) {
            WIFI_LOGE("%{public}s parse ScanLimitPackages item error, index:%{public}d", __FUNCTION__, i);
            continue;
        }
        /* appid is optional: keep the default empty value when absent or invalid */
        (void)GetStringFromItem(cJSON_GetObjectItem(item, JSON_KEY_APPID), info.appid);
        list.push_back(info);
    }
    out.insert_or_assign(JSON_KEY_SCAN_LIMIT_PACKAGES, list);
}

void ParseLandscapeSwitchLimitList(const cJSON *root, std::map<std::string, std::vector<PackageInfo>> &out)
{
    cJSON *section = cJSON_GetObjectItem(root, JSON_KEY_LANDSCAPE_SWITCH_LIMIT_LIST);
    if (section == nullptr || !cJSON_IsArray(section)) {
        WIFI_LOGE("%{public}s LandscapeSwitchLimitList section invalid", __FUNCTION__);
        return;
    }
    std::vector<PackageInfo> list;
    ParseNameList(section, list);
    out.insert_or_assign(JSON_KEY_LANDSCAPE_SWITCH_LIMIT_LIST, list);
}

/* Log one app list with its key: the size first, then each package name. */
template<typename T>
void LogAppList(const char *key, const std::vector<T> &list)
{
    WIFI_LOGI("%{public}s %{public}s size:%{public}zu", __FUNCTION__, key, list.size());
    for (size_t i = 0; i < list.size(); ++i) {
        WIFI_LOGI("%{public}s %{public}s [%{public}zu]: %{public}s",
            __FUNCTION__, key, i, list[i].packageName.c_str());
    }
}
} // namespace

AppParser::AppParser()
{
    WIFI_LOGI("%{public}s enter", __FUNCTION__);
}

AppParser::~AppParser()
{
    WIFI_LOGI("%{public}s enter", __FUNCTION__);
}

AppParser &AppParser::GetInstance()
{
    static AppParser instance;
    return instance;
}

bool AppParser::Init()
{
    std::lock_guard<std::mutex> lock(configUpdateMutex_);
    if (initFlag_) {
        return true;
    }
    bool parserRet = InitAppParser(WIFI_MONITOR_APP_FILE_PATH, &AppParser::ParseMonitorAppConfig);
    parserRet &= InitAppParser(WIFI_NETWORK_CONTROL_APP_FILE_PATH, &AppParser::ParseNetworkControlAppConfig);
    parserRet &= InitAppParser(WIFI_PACKAGE_FILTER_FILE_PATH, &AppParser::ParsePackageFilterConfig);
    if (!parserRet) {
        WIFI_LOGE("%{public}s InitAppParser fail", __FUNCTION__);
        return false;
    }
    initFlag_ = true;
    WIFI_LOGD("%{public}s InitAppParser successful", __FUNCTION__);
    DumpAppParserResult("init");
    return true;
}

bool AppParser::InitAppParser(const char *appJsonFilePath, bool (AppParser::*parseFunc)(cJSON *root))
{
    if (appJsonFilePath == nullptr) {
        WIFI_LOGE("%{public}s appJsonFilePath is null", __FUNCTION__);
        return false;
    }
    cJSON *root = LoadJsonFile(appJsonFilePath);
    if (root == nullptr) {
        WIFI_LOGE("%{public}s load %{public}s failed", __FUNCTION__, appJsonFilePath);
        return false;
    }
    bool ret = (this->*parseFunc)(root);
    cJSON_Delete(root);
    return ret;
}

bool AppParser::UpdateAllConfigs(const std::function<bool()> &updateFunc)
{
    if (updateFunc == nullptr) {
        WIFI_LOGE("%{public}s updateFunc is null", __FUNCTION__);
        return false;
    }
    std::lock_guard<std::mutex> lock(configUpdateMutex_);
    bool success = updateFunc();
    if (success) {
        initFlag_ = true;
    }
    return success;
}

bool AppParser::ParseMonitorAppConfig(cJSON *root)
{
    if (root == nullptr) {
        WIFI_LOGE("%{public}s invalid monitor app config", __FUNCTION__);
        return false;
    }
    AppParserResult tmp {};
    ParseGameInfoArray(root, tmp);
    ParseStringArray(root, JSON_KEY_APP_WHITE_LIST, tmp.m_whiteAppVec);
    ParseStringArray(root, JSON_KEY_APP_BLACK_LIST, tmp.m_blackAppVec);
    ParseStringArray(root, JSON_KEY_MULTILINK_BLACK_LIST, tmp.m_multilinkAppVec);
    ParseStringArray(root, JSON_KEY_CHARIOT_APP, tmp.m_chariotAppVec);
    ParseStringArray(root, JSON_KEY_HIGAME_EXCLUDE_LIST, tmp.m_hiGameExcludeAppVec);
    WIFI_LOGI("%{public}s out, lowLatency:%{public}zu whiteList:%{public}zu blackList:%{public}zu multilink:%{public}zu"
        " chariot:%{public}zu hiGameExclude:%{public}zu gameInfo:%{public}zu",
        __FUNCTION__, tmp.m_lowLatencyAppVec.size(), tmp.m_whiteAppVec.size(),
        tmp.m_blackAppVec.size(), tmp.m_multilinkAppVec.size(), tmp.m_chariotAppVec.size(),
        tmp.m_hiGameExcludeAppVec.size(), tmp.m_gameInfo.size());
    {
        std::unique_lock<std::shared_mutex> lock(appParserMutex_);
        result_.m_lowLatencyAppVec = std::move(tmp.m_lowLatencyAppVec);
        result_.m_whiteAppVec = std::move(tmp.m_whiteAppVec);
        result_.m_blackAppVec = std::move(tmp.m_blackAppVec);
        result_.m_multilinkAppVec = std::move(tmp.m_multilinkAppVec);
        result_.m_chariotAppVec = std::move(tmp.m_chariotAppVec);
        result_.m_hiGameExcludeAppVec = std::move(tmp.m_hiGameExcludeAppVec);
        result_.m_gameInfo = std::move(tmp.m_gameInfo);
    }
    return true;
}

bool AppParser::ParseNetworkControlAppConfig(cJSON *root)
{
    if (root == nullptr) {
        WIFI_LOGE("%{public}s invalid network control app config", __FUNCTION__);
        return false;
    }
    AppParserResult tmp {};
    ParseStringArray(root, JSON_KEY_APP_KEY_FOREGROUND_LIST, tmp.m_keyForegroundListAppVec);
    ParseStringArray(root, JSON_KEY_APP_KEY_BACKGROUND_LIMIT_LIST, tmp.m_keyBackgroundLimitListAppVec);
    ParseStringArray(root, JSON_KEY_APP_GAME_BACKGROUND_LIMIT_LIST, tmp.m_gameBackgroundLimitListAppVec);
    ParseStringArray(root, JSON_KEY_RSS_GAME_LIST, tmp.m_rssGameListAppVec);
    ParseStringArray(root, JSON_KEY_APP_LIVE_STREAM_LIST, tmp.m_liveStreamAppVec);
    std::string delayTime;
    bool hasDelayTime = ParseAsyncDelayTime(root, delayTime);
    WIFI_LOGI("%{public}s out, keyForeground:%{public}zu keyBackgroundLimit:%{public}zu gameBackgroundLimit:%{public}zu"
        " rssGame:%{public}zu liveStream:%{public}zu delayTime:%{public}s",
        __FUNCTION__, tmp.m_keyForegroundListAppVec.size(), tmp.m_keyBackgroundLimitListAppVec.size(),
        tmp.m_gameBackgroundLimitListAppVec.size(), tmp.m_rssGameListAppVec.size(),
        tmp.m_liveStreamAppVec.size(), hasDelayTime ? delayTime.c_str() : "(keep)");
    {
        std::unique_lock<std::shared_mutex> lock(appParserMutex_);
        result_.m_keyForegroundListAppVec = std::move(tmp.m_keyForegroundListAppVec);
        result_.m_keyBackgroundLimitListAppVec = std::move(tmp.m_keyBackgroundLimitListAppVec);
        result_.m_gameBackgroundLimitListAppVec = std::move(tmp.m_gameBackgroundLimitListAppVec);
        result_.m_rssGameListAppVec = std::move(tmp.m_rssGameListAppVec);
        result_.m_liveStreamAppVec = std::move(tmp.m_liveStreamAppVec);
        if (hasDelayTime) {
            result_.m_delayTime = delayTime;
        }
    }
    return true;
}

bool AppParser::ParsePackageFilterConfig(cJSON *root)
{
    if (root == nullptr) {
        WIFI_LOGE("%{public}s invalid package filter config", __FUNCTION__);
        return false;
    }
    std::map<std::string, std::vector<PackageInfo>> tmpMap;
    ParseScanControlPackages(root, tmpMap);
    ParseCandidateFilterPackages(root, tmpMap);
    ParseCorePackages(root, tmpMap);
    ParseAclAuthPackages(root, tmpMap);
    ParseScanLimitPackages(root, tmpMap);
    ParseLandscapeSwitchLimitList(root, tmpMap);
    WIFI_LOGI("%{public}s out, packageInfoMap size:%{public}zu", __FUNCTION__, tmpMap.size());
    {
        std::unique_lock<std::shared_mutex> lock(appParserMutex_);
        mPackageInfoMap_ = std::move(tmpMap);
    }
    return true;
}

bool AppParser::IsLowLatencyApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_lowLatencyAppVec.begin(), result_.m_lowLatencyAppVec.end(),
        [bundleName](const LowLatencyAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsWhiteListApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_whiteAppVec.begin(), result_.m_whiteAppVec.end(),
        [bundleName](const WhiteListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsBlackListApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_blackAppVec.begin(), result_.m_blackAppVec.end(),
        [bundleName](const BlackListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsMultiLinkApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_multilinkAppVec.begin(), result_.m_multilinkAppVec.end(),
        [bundleName](const MultiLinkAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsChariotApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_chariotAppVec.begin(), result_.m_chariotAppVec.end(),
        [bundleName](const ChariotAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsKeyForegroundApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_keyForegroundListAppVec.begin(), result_.m_keyForegroundListAppVec.end(),
        [bundleName](const KeyForegroundListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsKeyBackgroundLimitApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_keyBackgroundLimitListAppVec.begin(), result_.m_keyBackgroundLimitListAppVec.end(),
        [bundleName](const KeyBackgroundLimitListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsLiveStreamApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_liveStreamAppVec.begin(), result_.m_liveStreamAppVec.end(),
        [bundleName](const LiveStreamAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsGameBackgroundLimitApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_gameBackgroundLimitListAppVec.begin(), result_.m_gameBackgroundLimitListAppVec.end(),
        [bundleName](const GameBackgroundLimitListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsRssGameApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_rssGameListAppVec.begin(), result_.m_rssGameListAppVec.end(),
        [bundleName](const RssGameListAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsHiGameExcludeApp(const std::string &bundleName) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return std::any_of(result_.m_hiGameExcludeAppVec.begin(), result_.m_hiGameExcludeAppVec.end(),
        [bundleName](const HiGameExcludeAppInfo &app) { return app.packageName == bundleName; });
}

bool AppParser::IsOverGameRtt(const std::string &bundleName, const int gameRtt) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    if (result_.m_gameInfo.find(bundleName) == result_.m_gameInfo.end()) {
        return false;
    }
    return result_.m_gameInfo.at(bundleName).rtt <= gameRtt;
}

bool AppParser::IsOverGameLowRttThresh(const std::string &bundleName, const int gameRtt) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    if (result_.m_gameInfo.find(bundleName) == result_.m_gameInfo.end()) {
        return false;
    }
    return result_.m_gameInfo.at(bundleName).rtt - result_.m_gameInfo.at(bundleName).rttGap <= gameRtt;
}

std::string AppParser::GetAsyncLimitSpeedDelayTime() const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    return result_.m_delayTime;
}

std::string AppParser::GetPackageName(const std::string &tag) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    auto iter = mPackageInfoMap_.find(tag);
    if (iter != mPackageInfoMap_.end() && !iter->second.empty()) {
        return iter->second[0].name;
    }
    return "";
}

int AppParser::GetPackageInfoByName(const std::string &name, std::vector<PackageInfo> &packageInfo) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    auto iter = mPackageInfoMap_.find(name);
    if (iter != mPackageInfoMap_.end()) {
        packageInfo = iter->second;
        return 0;
    }
    return -1;
}

int AppParser::GetPackageInfoMap(std::map<std::string, std::vector<PackageInfo>> &packageInfoMap) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    packageInfoMap = mPackageInfoMap_;
    return 0;
}

void AppParser::DumpAppParserResult(const std::string &trigger) const
{
    std::shared_lock<std::shared_mutex> lock(appParserMutex_);
    if (trigger != "cloudPush") {
        return;
    }
    WIFI_LOGI("%{public}s enter", __FUNCTION__);
    /* wifi_monitor_apps.json sections */
    LogAppList("lowLatencyApp", result_.m_lowLatencyAppVec);
    LogAppList("whiteApp", result_.m_whiteAppVec);
    LogAppList("blackApp", result_.m_blackAppVec);
    LogAppList("multilinkApp", result_.m_multilinkAppVec);
    LogAppList("chariotApp", result_.m_chariotAppVec);
    LogAppList("hiGameExcludeApp", result_.m_hiGameExcludeAppVec);
    for (const auto &game : result_.m_gameInfo) {
        WIFI_LOGI("%{public}s gameInfo %{public}s rtt:%{public}d rttGap:%{public}d",
            __FUNCTION__, game.first.c_str(), game.second.rtt, game.second.rttGap);
    }
    /* wifi_network_control_apps.json sections */
    LogAppList("keyForegroundApp", result_.m_keyForegroundListAppVec);
    LogAppList("keyBackgroundLimitApp", result_.m_keyBackgroundLimitListAppVec);
    LogAppList("gameBackgroundLimitApp", result_.m_gameBackgroundLimitListAppVec);
    LogAppList("rssGameApp", result_.m_rssGameListAppVec);
    LogAppList("liveStreamApp", result_.m_liveStreamAppVec);
    WIFI_LOGI("%{public}s delayTime:%{public}s", __FUNCTION__, result_.m_delayTime.c_str());
    /* wifi_package_filter.json sections */
    for (const auto &tag : mPackageInfoMap_) {
        WIFI_LOGI("%{public}s packageFilter %{public}s size:%{public}zu",
            __FUNCTION__, tag.first.c_str(), tag.second.size());
        for (size_t i = 0; i < tag.second.size(); ++i) {
            WIFI_LOGI("%{public}s packageFilter %{public}s [%{public}zu] name:%{public}s appid:%{public}s",
                __FUNCTION__, tag.first.c_str(), i, tag.second[i].name.c_str(), tag.second[i].appid.c_str());
        }
    }
    WIFI_LOGI("%{public}s exit", __FUNCTION__);
}
} // namespace Wifi
} // namespace OHOS
