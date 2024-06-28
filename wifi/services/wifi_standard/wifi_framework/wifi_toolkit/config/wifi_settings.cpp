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

#include "wifi_settings.h"
#include "define.h"
#include "wifi_cert_utils.h"
#include "wifi_global_func.h"
#include "wifi_log.h"
#include "wifi_config_country_freqs.h"
#ifndef OHOS_ARCH_LITE
#include <sys/sendfile.h>
#include "wifi_country_code_define.h"
#include "network_parser.h"
#include "softap_parser.h"
#include "wifi_backup_config.h"
#endif
#ifdef INIT_LIB_ENABLE
#include "parameter.h"
#endif

namespace OHOS {
namespace Wifi {
#ifdef DTFUZZ_TEST
static WifiSettings* gWifiSettings = nullptr;
#endif

WifiSettings &WifiSettings::GetInstance()
{
#ifndef DTFUZZ_TEST
    static WifiSettings gWifiSettings;
    return gWifiSettings;
#else
    if (gWifiSettings == nullptr) {
        gWifiSettings = new (std::nothrow) WifiSettings();
    }
    return *gWifiSettings;
#endif
}

WifiSettings::WifiSettings()
    : mNetworkId(0),
      mApMaxConnNum(MAX_AP_CONN),
      mMaxNumConfigs(MAX_CONFIGS_NUM)
{
}

WifiSettings::~WifiSettings()
{
    SyncDeviceConfig();
    SyncHotspotConfig();
    SyncBlockList();
    SyncWifiP2pGroupInfoConfig();
    SyncP2pVendorConfig();
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    SyncWifiConfig();
}

int WifiSettings::Init()
{
    /* read ini config */
    mSavedDeviceConfig.SetConfigFilePath(DEVICE_CONFIG_FILE_PATH);
    mSavedHotspotConfig.SetConfigFilePath(HOTSPOT_CONFIG_FILE_PATH);
    mSavedBlockInfo.SetConfigFilePath(BLOCK_LIST_FILE_PATH);
    mSavedWifiConfig.SetConfigFilePath(WIFI_CONFIG_FILE_PATH);
    mSavedWifiP2pGroupInfo.SetConfigFilePath(WIFI_P2P_GROUP_INFO_FILE_PATH);
    mSavedWifiP2pVendorConfig.SetConfigFilePath(WIFI_P2P_VENDOR_CONFIG_FILE_PATH);
    mTrustListPolicies.SetConfigFilePath(WIFI_TRUST_LIST_POLICY_FILE_PATH);
    mMovingFreezePolicy.SetConfigFilePath(WIFI_MOVING_FREEZE_POLICY_FILE_PATH);
    mSavedWifiStoreRandomMac.SetConfigFilePath(WIFI_STA_RANDOM_MAC_FILE_PATH);
    mSavedPortal.SetConfigFilePath(PORTAL_CONFIG_FILE_PATH);
    mPackageFilterConfig.SetConfigFilePath(PACKAGE_FILTER_CONFIG_FILE_PATH);
#ifndef OHOS_ARCH_LITE
    MergeWifiConfig();
    MergeSoftapConfig();
#endif
#ifdef FEATURE_ENCRYPTION_SUPPORT
    SetUpHks();
#endif
    InitWifiConfig();
    ReloadDeviceConfig();
    InitHotspotConfig();
    InitP2pVendorConfig();
    ReloadWifiP2pGroupInfoConfig();
    ReloadTrustListPolicies();
    ReloadMovingFreezePolicy();
    ReloadStaRandomMac();
    ReloadPortalconf();
    InitPackageFilterConfig();
    IncreaseNumRebootsSinceLastUse();
    return 0;
}

int WifiSettings::AddDeviceConfig(const WifiDeviceConfig &config)
{
    if (config.ssid.empty()) {
        LOGE("AddDeviceConfig ssid is empty");
        return -1;
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    auto iter = mWifiDeviceConfig.find(config.networkId);
    if (iter != mWifiDeviceConfig.end()) {
        iter->second = config;
    } else {
        mWifiDeviceConfig.emplace(std::make_pair(config.networkId, config));
    }
    return config.networkId;
}

int WifiSettings::RemoveDevice(int networkId)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter != mWifiDeviceConfig.end()) {
        if (!iter->second.wifiEapConfig.clientCert.empty()) {
            if (WifiCertUtils::UninstallCert(iter->second.wifiEapConfig.clientCert) != 0) {
                LOGE("uninstall cert %{public}s fail", iter->second.wifiEapConfig.clientCert.c_str());
            } else {
                LOGD("uninstall cert %{public}s success", iter->second.wifiEapConfig.clientCert.c_str());
            }
        }
        mWifiDeviceConfig.erase(iter);
    }
    return 0;
}

void WifiSettings::ClearDeviceConfig(void)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if (iter->second.wifiEapConfig.clientCert.empty()) {
            continue;
        }
        if (WifiCertUtils::UninstallCert(iter->second.wifiEapConfig.clientCert) != 0) {
            LOGE("uninstall cert %{public}s fail", iter->second.wifiEapConfig.clientCert.c_str());
        } else {
            LOGD("uninstall cert %{public}s success", iter->second.wifiEapConfig.clientCert.c_str());
        }
    }
    mWifiDeviceConfig.clear();
    return;
}

int WifiSettings::GetDeviceConfig(std::vector<WifiDeviceConfig> &results)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        results.push_back(iter->second);
    }
    return 0;
}

int WifiSettings::GetDeviceConfig(const int &networkId, WifiDeviceConfig &config)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if (iter->second.networkId == networkId) {
            config = iter->second;
#ifdef FEATURE_ENCRYPTION_SUPPORT
            DecryptionDeviceConfig(config);
#endif
            return 0;
        }
    }
    return -1;
}

int WifiSettings::GetDeviceConfig(const std::string &index, const int &indexType, WifiDeviceConfig &config)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (indexType == DEVICE_CONFIG_INDEX_SSID) {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.ssid == index) {
                config = iter->second;
#ifdef FEATURE_ENCRYPTION_SUPPORT
                DecryptionDeviceConfig(config);
#endif
                return 0;
            }
        }
    } else {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.bssid == index) {
                config = iter->second;
#ifdef FEATURE_ENCRYPTION_SUPPORT
                DecryptionDeviceConfig(config);
#endif
                return 0;
            }
        }
    }
    return -1;
}

int WifiSettings::GetDeviceConfig(const std::string &ssid, const std::string &keymgmt, WifiDeviceConfig &config)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if ((iter->second.ssid == ssid) && (iter->second.keyMgmt == keymgmt)) {
            config = iter->second;
#ifdef FEATURE_ENCRYPTION_SUPPORT
            DecryptionDeviceConfig(config);
#endif
            return 0;
        }
    }
    return -1;
}

int WifiSettings::GetDeviceConfig(const std::string &ancoCallProcessName, const std::string &ssid,
    const std::string &keymgmt, WifiDeviceConfig &config)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }
    if (ancoCallProcessName.empty()) {
        LOGD("anco do not deal with");
        return -1;
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if ((iter->second.ssid == ssid) && (iter->second.keyMgmt == keymgmt) &&
            iter->second.ancoCallProcessName == ancoCallProcessName) {
            config = iter->second;
#ifdef FEATURE_ENCRYPTION_SUPPORT
            DecryptionDeviceConfig(config);
#endif
            return 0;
        }
    }
    return -1;
}

int WifiSettings::SetDeviceState(int networkId, int state, bool bSetOther)
{
    if (state < 0 || state >= (int)WifiDeviceConfigStatus::UNKNOWN) {
        return -1;
    }
    std::unique_lock<std::mutex> lock(mStaMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter == mWifiDeviceConfig.end()) {
        return -1;
    }
    iter->second.status = state;
    if (bSetOther && state == (int)WifiDeviceConfigStatus::ENABLED) {
        for (iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); ++iter) {
            if (iter->first != networkId && iter->second.status == state) {
                iter->second.status = 1;
            }
        }
    }
    return 0;
}

int WifiSettings::SetDeviceAfterConnect(int networkId)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter == mWifiDeviceConfig.end()) {
        return -1;
    }
    LOGD("Set Device After Connect");
    iter->second.lastConnectTime = time(0);
    iter->second.numRebootsSinceLastUse = 0;
    iter->second.numAssociation++;
    return 0;
}

int WifiSettings::SetDeviceRandomizedMacSuccessEver(int networkId)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    auto iter = mWifiDeviceConfig.find(networkId);
    if (iter == mWifiDeviceConfig.end()) {
        return -1;
    }
    iter->second.randomizedMacSuccessEver = true;
    return 0;
}

int WifiSettings::GetCandidateConfig(const int uid, const int &networkId, WifiDeviceConfig &config)
{
    std::vector<WifiDeviceConfig> configs;
    if (GetAllCandidateConfig(uid, configs) != 0) {
        return -1;
    }

    for (const auto &it : configs) {
        if (it.networkId == networkId) {
            config = it;
            return it.networkId;
        }
    }
    return -1;
}

int WifiSettings::GetAllCandidateConfig(const int uid, std::vector<WifiDeviceConfig> &configs)
{
    if (!deviceConfigLoadFlag.test_and_set()) {
        LOGD("Reload wifi config");
        ReloadDeviceConfig();
    }

    std::unique_lock<std::mutex> lock(mStaMutex);
    bool found = false;
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        if (iter->second.uid == uid) {
            configs.push_back(iter->second);
            found = true;
        }
    }
    return found ? 0 : -1;
}

int WifiSettings::IncreaseDeviceConnFailedCount(const std::string &index, const int &indexType, int count)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (indexType == DEVICE_CONFIG_INDEX_SSID) {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.ssid == index) {
                iter->second.connFailedCount += count;
                return 0;
            }
        }
    } else {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.bssid == index) {
                iter->second.connFailedCount += count;
                return 0;
            }
        }
    }
    return -1;
}

int WifiSettings::SetDeviceConnFailedCount(const std::string &index, const int &indexType, int count)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (indexType == DEVICE_CONFIG_INDEX_SSID) {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.ssid == index) {
                iter->second.connFailedCount = count;
                return 0;
            }
        }
    } else {
        for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
            if (iter->second.bssid == index) {
                iter->second.connFailedCount = count;
                return 0;
            }
        }
    }
    return -1;
}

int WifiSettings::SyncDeviceConfig()
{
#ifndef CONFIG_NO_CONFIG_WRITE
    std::unique_lock<std::mutex> lock(mStaMutex);
    std::vector<WifiDeviceConfig> tmp;
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); ++iter) {
        if (!iter->second.isEphemeral) {
            tmp.push_back(iter->second);
        }
    }
    RemoveExcessDeviceConfigs(tmp);
    mSavedDeviceConfig.SetValue(tmp);
    return mSavedDeviceConfig.SaveConfig();
#else
    return 0;
#endif
}

int WifiSettings::ReloadDeviceConfig()
{
#ifndef CONFIG_NO_CONFIG_WRITE
    int ret = mSavedDeviceConfig.LoadConfig();
    if (ret < 0) {
        deviceConfigLoadFlag.clear();
        LOGD("Loading device config failed: %{public}d", ret);
        return -1;
    }
    deviceConfigLoadFlag.test_and_set();
    std::vector<WifiDeviceConfig> tmp;
    mSavedDeviceConfig.GetValue(tmp);
    std::unique_lock<std::mutex> lock(mStaMutex);
    mNetworkId = 0;
    mWifiDeviceConfig.clear();
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        WifiDeviceConfig &item = tmp[i];
        item.networkId = mNetworkId++;
        mWifiDeviceConfig.emplace(item.networkId, item);
    }
    if (!mEncryptionOnBootFalg.test_and_set()) {
        mWifiEncryptionThread = std::make_unique<WifiEventHandler>("WifiEncryptionThread");
        mWifiEncryptionThread->PostAsyncTask([this]() {
            LOGI("ReloadDeviceConfig EncryptionWifiDeviceConfigOnBoot start.");
            EncryptionWifiDeviceConfigOnBoot();
        });
    }
    return 0;
#else
    std::unique_lock<std::mutex> lock(mStaMutex);
    mWifiDeviceConfig.clear();
    return 0;
#endif
}

int WifiSettings::GetNextNetworkId()
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    return mNetworkId++;
}

int WifiSettings::AddWpsDeviceConfig(const WifiDeviceConfig &config)
{
    int ret = mSavedDeviceConfig.LoadConfig();
    if (ret < 0) {
        LOGE("Add Wps config loading config failed: %{public}d", ret);
        return -1;
    }
    std::vector<WifiDeviceConfig> tmp;
    mSavedDeviceConfig.GetValue(tmp);
    std::unique_lock<std::mutex> lock(mStaMutex);
    mWifiDeviceConfig.clear();
    mNetworkId = 0;
    mWifiDeviceConfig.emplace(mNetworkId++, config);
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        WifiDeviceConfig &item = tmp[i];
        item.networkId = mNetworkId++;
        mWifiDeviceConfig.emplace(item.networkId, item);
    }
    return 0;
}

#ifndef OHOS_ARCH_LITE
int WifiSettings::OnRestore(UniqueFd &fd, const std::string &restoreInfo)
{
    LOGI("OnRestore enter.");
    const std::string versionForXml = "9";
    std::string key;
    std::string iv;
    std::string version;
    ParseBackupJson(restoreInfo, key, iv, version);

    std::vector<WifiDeviceConfig> deviceConfigs;
    int ret = 0;
    if (version == versionForXml) {
        ret = GetConfigbyBackupXml(deviceConfigs, fd);
    } else {
        ret = GetConfigbyBackupFile(deviceConfigs, fd, key, iv);
    }
    std::fill(key.begin(), key.end(), 0);
    if (ret < 0) {
        LOGE("OnRestore fail to get config from backup.");
        return ret;
    }

    LOGI("OnRestore end. Restore count: %{public}d", static_cast<int>(deviceConfigs.size()));
    ConfigsDeduplicateAndSave(deviceConfigs);
    return 0;
}

int WifiSettings::OnBackup(UniqueFd &fd, const std::string &backupInfo)
{
    LOGI("OnBackup enter.");
    std::string key;
    std::string iv;
    std::string version;
    ParseBackupJson(backupInfo, key, iv, version);
    if (key.size() == 0 || iv.size() == 0) {
        LOGE("OnBackup key or iv is empty.");
        return -1;
    }
    mSavedDeviceConfig.LoadConfig();
    std::vector<WifiDeviceConfig> localConfigs;
    mSavedDeviceConfig.GetValue(localConfigs);

    std::vector<WifiBackupConfig> backupConfigs;
    for (auto &config : localConfigs) {
        if (config.wifiEapConfig.eap.length() != 0 || config.isPasspoint == true) {
            continue;
        }
#ifdef FEATURE_ENCRYPTION_SUPPORT
        DecryptionDeviceConfig(config);
#endif
        WifiBackupConfig backupConfig;
        ConvertDeviceCfgToBackupCfg(config, backupConfig);
        backupConfigs.push_back(backupConfig);
    }
    std::vector<WifiDeviceConfig>().swap(localConfigs);

    WifiConfigFileImpl<WifiBackupConfig> wifiBackupConfig;
    wifiBackupConfig.SetConfigFilePath(BACKUP_CONFIG_FILE_PATH);
    wifiBackupConfig.SetEncryptionInfo(key, iv);
    wifiBackupConfig.SetValue(backupConfigs);
    wifiBackupConfig.SaveConfig();
    wifiBackupConfig.UnsetEncryptionInfo();
    std::fill(key.begin(), key.end(), 0);

    fd = UniqueFd(open(BACKUP_CONFIG_FILE_PATH, O_RDONLY));
    if (fd.Get() < 0) {
        LOGE("OnBackup open fail.");
        return -1;
    }
    LOGI("OnBackup end. Backup count: %{public}d, fd: %{public}d.", static_cast<int>(backupConfigs.size()), fd.Get());
    return 0;
}

void WifiSettings::MergeWifiCloneConfig(std::string &cloneData)
{
    LOGI("MergeWifiCloneConfig enter");
    std::unique_ptr<NetworkXmlParser> xmlParser = std::make_unique<NetworkXmlParser>();
    bool ret = xmlParser->LoadConfigurationMemory(cloneData.c_str());
    if (!ret) {
        LOGE("MergeWifiCloneConfig load fail");
        return;
    }
    ret = xmlParser->Parse();
    if (!ret) {
        LOGE("MergeWifiCloneConfig Parse fail");
        return;
    }
    std::vector<WifiDeviceConfig> cloneConfigs = xmlParser->GetNetworks();
    if (cloneConfigs.empty()) {
        return;
    }
    ConfigsDeduplicateAndSave(cloneConfigs);
}

void WifiSettings::RemoveBackupFile()
{
    remove(BACKUP_CONFIG_FILE_PATH);
}
#endif

bool WifiSettings::AddRandomMac(WifiStoreRandomMac &randomMacInfo)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (randomMacInfo.randomMac.empty()) {
        LOGE("%{public}s failed randomMac is empty.", __func__);
        return false;
    }
    bool isAdded = false;
    std::string fuzzyBssid = "";
    if (IsPskEncryption(randomMacInfo.keyMgmt)) {
        fuzzyBssid = FuzzyBssid(randomMacInfo.peerBssid);
    }
    
    for (auto &ele : mWifiStoreRandomMac) {
        if (IsPskEncryption(ele.keyMgmt)) {
            if (ele.randomMac != randomMacInfo.randomMac) {
                continue;
            }
            if (ele.fuzzyBssids.find(fuzzyBssid) != ele.fuzzyBssids.end()) {
                LOGI("AddRandomMac is contains fuzzyBssid:%{public}s", MacAnonymize(fuzzyBssid).c_str());
                return true;
            }
            if (ele.fuzzyBssids.size() <= FUZZY_BSSID_MAX_MATCH_CNT) {
                ele.fuzzyBssids.insert(fuzzyBssid);
                LOGI("AddRandomMac insert fuzzyBssid:%{public}s", MacAnonymize(fuzzyBssid).c_str());
                isAdded = true;
                break;
            } else {
                LOGI("AddRandomMac ele.fuzzyBssids.size is max count");
                return false;
            }
        }
        if (ele.ssid == randomMacInfo.ssid && ele.keyMgmt == randomMacInfo.keyMgmt) {
            return true;
        }
    }

    LOGI("AddRandomMac isAdded:%{public}d", isAdded);
    if (!isAdded) {
        if (IsPskEncryption(randomMacInfo.keyMgmt)) {
            randomMacInfo.fuzzyBssids.insert(fuzzyBssid);
        }
        mWifiStoreRandomMac.push_back(randomMacInfo);
    }

    mSavedWifiStoreRandomMac.SetValue(mWifiStoreRandomMac);
    mSavedWifiStoreRandomMac.SaveConfig();
    return isAdded;
}

bool WifiSettings::GetRandomMac(WifiStoreRandomMac &randomMacInfo)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    std::string fuzzyBssid = "";
    if (IsPskEncryption(randomMacInfo.keyMgmt)) {
        fuzzyBssid = FuzzyBssid(randomMacInfo.peerBssid);
    }

    for (auto &item : mWifiStoreRandomMac) {
        if (item.randomMac.empty()) {
            continue;
        }
        if (IsPskEncryption(item.keyMgmt)) {
            if (item.fuzzyBssids.find(fuzzyBssid) != item.fuzzyBssids.end()) {
                LOGI("GetStaRandomMac fuzzyBssids contains fuzzyBssid:%{public}s",
                    MacAnonymize(fuzzyBssid).c_str());
                randomMacInfo.randomMac = item.randomMac;
                break;
            }
        } else {
            if (item.ssid == randomMacInfo.ssid && item.keyMgmt == randomMacInfo.keyMgmt) {
                randomMacInfo.randomMac = item.randomMac;
                break;
            }
        }
    }
    return randomMacInfo.randomMac.empty();
}

void WifiSettings::GetPortalUri(WifiPortalConf &urlInfo)
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    urlInfo.portalHttpUrl = mPortalUri.portalHttpUrl;
    urlInfo.portalHttpsUrl = mPortalUri.portalHttpsUrl;
    urlInfo.portalBakHttpUrl = mPortalUri.portalBakHttpUrl;
    urlInfo.portalBakHttpsUrl = mPortalUri.portalBakHttpsUrl;
}

const std::vector<TrustListPolicy> WifiSettings::ReloadTrustListPolicies()
{
    std::unique_lock<std::mutex> lock(mScanMutex);
    mTrustListPolicies.LoadConfig();
    if (mTrustListPolicies.GetValue().size() <= 0) {
        std::vector<TrustListPolicy> policies;
        TrustListPolicy policy;
        policies.push_back(policy);
        mTrustListPolicies.SetValue(policies);
        mTrustListPolicies.SaveConfig();
        mTrustListPolicies.LoadConfig();
    }

    return mTrustListPolicies.GetValue();
}

const MovingFreezePolicy WifiSettings::ReloadMovingFreezePolicy()
{
    std::unique_lock<std::mutex> lock(mScanMutex);
    mMovingFreezePolicy.LoadConfig();

    if (mMovingFreezePolicy.GetValue().size() <= 0) {
        std::vector<MovingFreezePolicy> policies;
        MovingFreezePolicy policy;
        policies.push_back(policy);
        mMovingFreezePolicy.SetValue(policies);
        mMovingFreezePolicy.SaveConfig();
        mMovingFreezePolicy.LoadConfig();
    }

    if (mMovingFreezePolicy.GetValue().size() <= 0) {
        return MovingFreezePolicy();
    }
    return mMovingFreezePolicy.GetValue()[0];
}

int WifiSettings::GetPackageFilterMap(std::map<std::string, std::vector<std::string>> &filterMap)
{
    std::unique_lock<std::mutex> lock(mScanMutex);
    filterMap = mFilterMap;
    return 0;
}

int WifiSettings::SyncHotspotConfig()
{
    std::unique_lock<std::mutex> lock(mApMutex);
    std::vector<HotspotConfig> tmp;

    for (int i = 0; i < AP_INSTANCE_MAX_NUM; i++) {
        auto iter = mHotspotConfig.find(i);
        if (iter != mHotspotConfig.end()) {
            tmp.push_back(iter->second);
        }
    }
    mSavedHotspotConfig.SetValue(tmp);
    mSavedHotspotConfig.SaveConfig();

    return 0;
}

int WifiSettings::SetHotspotConfig(const HotspotConfig &config, int id)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    mHotspotConfig[id] = config;
    return 0;
}

int WifiSettings::GetHotspotConfig(HotspotConfig &config, int id)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    auto iter = mHotspotConfig.find(id);
    if (iter != mHotspotConfig.end()) {
        config = iter->second;
    }
    return 0;
}


void WifiSettings::ClearHotspotConfig()
{
    std::unique_lock<std::mutex> lock(mApMutex);
    mHotspotConfig.clear();
    HotspotConfig config;
#ifdef INIT_LIB_ENABLE
    std::string ssid = GetMarketName();
#endif
    config.SetSecurityType(KeyMgmt::WPA2_PSK);
    config.SetBand(BandType::BAND_2GHZ);
    config.SetChannel(AP_CHANNEL_DEFAULT);
    config.SetMaxConn(GetApMaxConnNum());
    config.SetBandWidth(AP_BANDWIDTH_DEFAULT);
#ifdef INIT_LIB_ENABLE
    config.SetSsid(ssid);
#else
    config.SetSsid("OHOS_" + GetRandomStr(RANDOM_STR_LEN));
#endif
    config.SetPreSharedKey(GetRandomStr(RANDOM_PASSWD_LEN));
    auto ret = mHotspotConfig.emplace(0, config);
    if (!ret.second) {
        mHotspotConfig[0] = config;
    }
}

int WifiSettings::GetBlockList(std::vector<StationInfo> &results, int id)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    for (auto iter = mBlockListInfo.begin(); iter != mBlockListInfo.end(); iter++) {
        results.push_back(iter->second);
    }
    return 0;
}

int WifiSettings::ManageBlockList(const StationInfo &info, int mode, int id)
{
    std::unique_lock<std::mutex> lock(mApMutex);
    auto iter = mBlockListInfo.find(info.bssid);
    if (MODE_ADD == mode || MODE_UPDATE == mode) {
        if (iter != mBlockListInfo.end()) {
            iter->second = info;
        } else {
            mBlockListInfo.emplace(std::make_pair(info.bssid, info));
        }
    } else if (MODE_DEL == mode) {
        if (iter != mBlockListInfo.end()) {
            mBlockListInfo.erase(iter);
        }
    } else {
        return -1;
    }
    return 0;
}

int WifiSettings::SyncWifiP2pGroupInfoConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mSavedWifiP2pGroupInfo.SetValue(mGroupInfoList);
    return mSavedWifiP2pGroupInfo.SaveConfig();
}

int WifiSettings::SetWifiP2pGroupInfo(const std::vector<WifiP2pGroupInfo> &groups)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mGroupInfoList = groups;
    return 0;
}

int WifiSettings::RemoveWifiP2pGroupInfo()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mGroupInfoList.clear();
    return 0;
}

int WifiSettings::RemoveWifiP2pSupplicantGroupInfo()
{
    if (!std::filesystem::exists(P2P_SUPPLICANT_CONFIG_FILE)) {
        LOGE("p2p_supplicant file do not exists!, file:%{public}s", P2P_SUPPLICANT_CONFIG_FILE);
        return -1;
    }
    std::error_code ec;
    int retval = std::filesystem::remove(P2P_SUPPLICANT_CONFIG_FILE, ec);
    if (!ec) { // successful
        LOGI("p2p_supplicant file removed successful, retval:%{public}d value:%{public}d message:%{public}s",
            retval, ec.value(), ec.message().c_str());
        return 0;
    } // unsuccessful
    LOGE("p2p_supplicant file removed unsuccessful, value:%{public}d value:%{public}d message:%{public}s",
        retval, ec.value(), ec.message().c_str());
    return -1;
}

int WifiSettings::GetWifiP2pGroupInfo(std::vector<WifiP2pGroupInfo> &groups)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    groups = mGroupInfoList;
    return 0;
}

int WifiSettings::SyncP2pVendorConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    std::vector<P2pVendorConfig> tmp;
    tmp.push_back(mP2pVendorConfig);
    mSavedWifiP2pVendorConfig.SetValue(tmp);
    return mSavedWifiP2pVendorConfig.SaveConfig();
}

int WifiSettings::SetP2pDeviceName(const std::string &deviceName)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mP2pVendorConfig.SetDeviceName(deviceName);
    std::vector<P2pVendorConfig> tmp;
    tmp.push_back(mP2pVendorConfig);
    mSavedWifiP2pVendorConfig.SetValue(tmp);
    return mSavedWifiP2pVendorConfig.SaveConfig();
}

int WifiSettings::SetP2pVendorConfig(const P2pVendorConfig &config)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    mP2pVendorConfig = config;
    return 0;
}

int WifiSettings::GetP2pVendorConfig(P2pVendorConfig &config)
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    config = mP2pVendorConfig;
    return 0;
}

bool WifiSettings::GetScanAlwaysState(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scanAlwaysSwitch;
    }
    return mWifiConfig[0].scanAlwaysSwitch;
}

int WifiSettings::GetSignalLevel(const int &rssi, const int &band, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    int level = 0;
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        do {
            if (band == static_cast<int>(BandType::BAND_5GHZ)) {
                if (rssi < iter->second.firstRssiLevel5G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.secondRssiLevel5G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.thirdRssiLevel5G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.fourthRssiLevel5G) {
                    break;
                }
                ++level;
            } else {
                if (rssi < iter->second.firstRssiLevel2G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.secondRssiLevel2G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.thirdRssiLevel2G) {
                    break;
                }
                ++level;
                if (rssi < iter->second.fourthRssiLevel2G) {
                    break;
                }
                ++level;
            }
        } while (0);
    }
    return level;
}

int WifiSettings::GetOperatorWifiType(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.staAirplaneMode;
    }
    return mWifiConfig[0].staAirplaneMode;
}

int WifiSettings::SetOperatorWifiType(int type, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].staAirplaneMode = type;
    SyncWifiConfig();
    return 0;
}

int WifiSettings::GetLastAirplaneMode(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.lastAirplaneMode;
    }
    return mWifiConfig[0].lastAirplaneMode;
}

int WifiSettings::SetLastAirplaneMode(int mode, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].lastAirplaneMode = mode;
    SyncWifiConfig();
    return 0;
}

bool WifiSettings::GetCanOpenStaWhenAirplaneMode(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.canOpenStaWhenAirplane;
    }
    return mWifiConfig[0].canOpenStaWhenAirplane;
}

int WifiSettings::SetWifiFlagOnAirplaneMode(bool ifOpen, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].openWifiWhenAirplane = ifOpen;
    SyncWifiConfig();
    return 0;
}

bool WifiSettings::GetWifiFlagOnAirplaneMode(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.openWifiWhenAirplane;
    }
    return mWifiConfig[0].openWifiWhenAirplane;
}

int WifiSettings::SetWifiDisabledByAirplane(bool disabledByAirplane, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].wifiDisabledByAirplane = disabledByAirplane;
    SyncWifiConfig();
    return 0;
}

bool WifiSettings::GetWifiDisabledByAirplane(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.wifiDisabledByAirplane;
    }
    return mWifiConfig[0].wifiDisabledByAirplane;
}

int WifiSettings::GetStaLastRunState(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.staLastState;
    }
    return mWifiConfig[0].staLastState;
}

int WifiSettings::SetStaLastRunState(int bRun, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].staLastState = bRun;
    SyncWifiConfig();
    return 0;
}

int WifiSettings::GetDhcpIpType(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.dhcpIpType;
    }
    return mWifiConfig[0].dhcpIpType;
}

bool WifiSettings::GetWhetherToAllowNetworkSwitchover(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.whetherToAllowNetworkSwitchover;
    }
    return mWifiConfig[0].whetherToAllowNetworkSwitchover;
}

int WifiSettings::GetScoretacticsScoreSlope(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsScoreSlope;
    }
    return mWifiConfig[0].scoretacticsScoreSlope;
}

int WifiSettings::GetScoretacticsInitScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsInitScore;
    }
    return mWifiConfig[0].scoretacticsInitScore;
}

int WifiSettings::GetScoretacticsSameBssidScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsSameBssidScore;
    }
    return mWifiConfig[0].scoretacticsSameBssidScore;
}

int WifiSettings::GetScoretacticsSameNetworkScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsSameNetworkScore;
    }
    return mWifiConfig[0].scoretacticsSameNetworkScore;
}

int WifiSettings::GetScoretacticsFrequency5GHzScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsFrequency5GHzScore;
    }
    return mWifiConfig[0].scoretacticsFrequency5GHzScore;
}

int WifiSettings::GetScoretacticsLastSelectionScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsLastSelectionScore;
    }
    return mWifiConfig[0].scoretacticsLastSelectionScore;
}

int WifiSettings::GetScoretacticsSecurityScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsSecurityScore;
    }
    return mWifiConfig[0].scoretacticsSecurityScore;
}

int WifiSettings::GetScoretacticsNormalScore(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scoretacticsNormalScore;
    }
    return mWifiConfig[0].scoretacticsNormalScore;
}

int WifiSettings::GetSavedDeviceAppraisalPriority(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.savedDeviceAppraisalPriority;
    }
    return mWifiConfig[0].savedDeviceAppraisalPriority;
}

bool WifiSettings::IsModulePreLoad(const std::string &name)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    if (name == WIFI_SERVICE_STA) {
        return mWifiConfig[0].preLoadSta;
    } else if (name == WIFI_SERVICE_SCAN) {
        return mWifiConfig[0].preLoadScan;
    } else if (name == WIFI_SERVICE_AP) {
        return mWifiConfig[0].preLoadAp;
    } else if (name == WIFI_SERVICE_P2P) {
        return mWifiConfig[0].preLoadP2p;
    } else if (name == WIFI_SERVICE_AWARE) {
        return mWifiConfig[0].preLoadAware;
    } else if (name == WIFI_SERVICE_ENHANCE) {
        return mWifiConfig[0].preLoadEnhance;
    } else {
        return false;
    }
}

bool WifiSettings::GetSupportHwPnoFlag(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.supportHwPnoFlag;
    }
    return mWifiConfig[0].supportHwPnoFlag;
}

int WifiSettings::GetMinRssi2Dot4Ghz(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.minRssi2Dot4Ghz;
    }
    return mWifiConfig[0].minRssi2Dot4Ghz;
}

int WifiSettings::GetMinRssi5Ghz(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.minRssi5Ghz;
    }
    return mWifiConfig[0].minRssi5Ghz;
}

int WifiSettings::SetRealMacAddress(const std::string &macAddress, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].realMacAddress = macAddress;
    SyncWifiConfig();
    return 0;
}

int WifiSettings::GetRealMacAddress(std::string &macAddress, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        macAddress = iter->second.realMacAddress;
        return 0;
    }
    macAddress = mWifiConfig[0].realMacAddress;
    return 0;
}

void WifiSettings::SetDefaultFrequenciesByCountryBand(const BandType band, std::vector<int> &frequencies, int instId)
{
    for (auto& item : g_countryDefaultFreqs) {
        if (item.band == band) {
            frequencies = item.freqs;
        }
    }
}

void WifiSettings::SetScanOnlySwitchState(const int &state, int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    mWifiConfig[instId].scanOnlySwitch = state;
    SyncWifiConfig();
}

int WifiSettings::GetScanOnlySwitchState(int instId)
{
    std::unique_lock<std::mutex> lock(mWifiConfigMutex);
    if (IsFactoryMode()) {
        return 0;
    }
    auto iter = mWifiConfig.find(instId);
    if (iter != mWifiConfig.end()) {
        return iter->second.scanOnlySwitch;
    }
    return mWifiConfig[0].scanOnlySwitch;
}

bool WifiSettings::EncryptionDeviceConfig(WifiDeviceConfig &config) const
{
#ifdef FEATURE_ENCRYPTION_SUPPORT
    if (config.version == 1) {
        return true;
    }
    WifiEncryptionInfo mWifiEncryptionInfo;
    mWifiEncryptionInfo.SetFile(GetTClassName<WifiDeviceConfig>());

    config.encryptedData = "";
    config.IV = "";
    if (!config.preSharedKey.empty()) {
        EncryptedData encry;
        if (WifiEncryption(mWifiEncryptionInfo, config.preSharedKey, encry) == HKS_SUCCESS) {
            config.encryptedData = encry.encryptedPassword;
            config.IV = encry.IV;
        } else {
            LOGE("EncryptionDeviceConfig WifiEncryption preSharedKey failed");
            WriteWifiEncryptionFailHiSysEvent(ENCRYPTION_EVENT,
                SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
            return false;
        }
    }

    if (config.wepTxKeyIndex < 0 || config.wepTxKeyIndex >= WEPKEYS_SIZE) {
        config.wepTxKeyIndex = 0;
    }
    config.encryWepKeys[config.wepTxKeyIndex] = "";
    config.IVWep = "";
    if (!config.wepKeys[config.wepTxKeyIndex].empty()) {
        EncryptedData encryWep;
        if (WifiEncryption(mWifiEncryptionInfo, config.wepKeys[config.wepTxKeyIndex], encryWep) == HKS_SUCCESS) {
            config.encryWepKeys[config.wepTxKeyIndex] = encryWep.encryptedPassword;
            config.IVWep = encryWep.IV;
        } else {
            LOGE("EncryptionDeviceConfig WifiEncryption wepKeys failed");
            WriteWifiEncryptionFailHiSysEvent(ENCRYPTION_EVENT,
                SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
            return false;
        }
    }

    config.wifiEapConfig.encryptedData = "";
    config.wifiEapConfig.IV = "";
    if (!config.wifiEapConfig.eap.empty()) {
        EncryptedData encryEap;
        if (WifiEncryption(mWifiEncryptionInfo, config.wifiEapConfig.password, encryEap) == HKS_SUCCESS) {
            config.wifiEapConfig.encryptedData = encryEap.encryptedPassword;
            config.wifiEapConfig.IV = encryEap.IV;
        } else {
            LOGE("EncryptionDeviceConfig WifiEncryption eap failed");
            WriteWifiEncryptionFailHiSysEvent(ENCRYPTION_EVENT,
                SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
            return false;
        }
    }
    if (!EncryptionWapiConfig(mWifiEncryptionInfo, config)) {
        return false;
    }
    config.version = 1;
#endif
    return true;
}

int WifiSettings::IncreaseNumRebootsSinceLastUse()
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    for (auto iter = mWifiDeviceConfig.begin(); iter != mWifiDeviceConfig.end(); iter++) {
        iter->second.numRebootsSinceLastUse++;
    }
    return 0;
}

void WifiSettings::EncryptionWifiDeviceConfigOnBoot()
{
#ifdef FEATURE_ENCRYPTION_SUPPORT
    std::unique_lock<std::mutex> lock(mConfigOnBootMutex);
    mSavedDeviceConfig.LoadConfig();
    std::vector<WifiDeviceConfig> tmp;
    mSavedDeviceConfig.GetValue(tmp);
    int count = 0;

    for (std::size_t i = 0; i < tmp.size(); ++i) {
        WifiDeviceConfig &item = tmp[i];
        if (item.version == -1 && EncryptionDeviceConfig(item)) {
            count ++;
        }
    }
    if (count > 0) {
        mSavedDeviceConfig.SetValue(tmp);
        mSavedDeviceConfig.SaveConfig();
        ReloadDeviceConfig();
    }
    LOGI("EncryptionWifiDeviceConfigOnBoot end count:%{public}d", count);
#endif
}

int WifiSettings::ReloadStaRandomMac()
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (mSavedWifiStoreRandomMac.LoadConfig()) {
        return -1;
    }
    mWifiStoreRandomMac.clear();
    mSavedWifiStoreRandomMac.GetValue(mWifiStoreRandomMac);
    bool shouldReset = false;
    for (const auto &item: mWifiStoreRandomMac) {
        if (item.version == -1) {
            shouldReset = true;
            break;
        }
    }
    LOGI("%{public}s shouldReset:%{public}s", __func__, shouldReset ? "true" : "false");
    if (shouldReset) {
        for (auto &item: mWifiStoreRandomMac) {
            item.version = 0;
        }
        mSavedWifiStoreRandomMac.SetValue(mWifiStoreRandomMac);
        mSavedWifiStoreRandomMac.SaveConfig();
    }
    return 0;
}

int WifiSettings::ReloadPortalconf()
{
    std::unique_lock<std::mutex> lock(mStaMutex);
    if (mSavedPortal.LoadConfig() >= 0) {
        std::vector<WifiPortalConf> tmp;
        mSavedPortal.GetValue(tmp);
        if (tmp.size() > 0) {
            mPortalUri = tmp[0];
        } else {
            mPortalUri.portalHttpUrl = "test";
        }
    } else {
        mPortalUri.portalHttpUrl = "test";
    }
    return 0;
}

void WifiSettings::InitPackageFilterConfig()
{
    if (mPackageFilterConfig.LoadConfig() >= 0) {
        std::vector<PackageFilterConf> tmp;
        mPackageFilterConfig.GetValue(tmp);
        std::unique_lock<std::mutex> lock(mScanMutex);
        for (int i = 0; i < tmp.size(); i++) {
            mFilterMap.insert(std::make_pair(tmp[i].filterName, tmp[i].packageList));
        }
    }
    return;
}

void WifiSettings::InitDefaultHotspotConfig()
{
    HotspotConfig cfg;
#ifdef INIT_LIB_ENABLE
    std::string ssid = GetMarketName();
#endif
    cfg.SetSecurityType(KeyMgmt::WPA2_PSK);
    cfg.SetBand(BandType::BAND_2GHZ);
    cfg.SetChannel(AP_CHANNEL_DEFAULT);
    cfg.SetMaxConn(GetApMaxConnNum());
    cfg.SetBandWidth(AP_BANDWIDTH_DEFAULT);
#ifdef INIT_LIB_ENABLE
    cfg.SetSsid(ssid);
#else
    cfg.SetSsid("OHOS_" + GetRandomStr(RANDOM_STR_LEN));
#endif
    cfg.SetPreSharedKey(GetRandomStr(RANDOM_PASSWD_LEN));
    auto ret = mHotspotConfig.emplace(0, cfg);
    if (!ret.second) {
        mHotspotConfig[0] = cfg;
    }
}

void WifiSettings::InitHotspotConfig()
{
    /* init hotspot config */
    if (mSavedHotspotConfig.LoadConfig() >= 0) {
        std::vector<HotspotConfig> tmp;
        mSavedHotspotConfig.GetValue(tmp);
        if (tmp.size() > 0) {
            for (size_t i = 0; i < tmp.size(); i++) {
                mHotspotConfig[i] = tmp[i];
            }
        } else {
            LOGI("load hotspot config success, but tmp.size() = 0, use default config");
            InitDefaultHotspotConfig();
        }
    } else {
        LOGI("load hotspot config fail, use default config");
        InitDefaultHotspotConfig();
    }
    /* init block list info */
    if (mSavedBlockInfo.LoadConfig() >= 0) {
        std::vector<StationInfo> tmp;
        mSavedBlockInfo.GetValue(tmp);
        for (std::size_t i = 0; i < tmp.size(); ++i) {
            StationInfo &item = tmp[i];
            mBlockListInfo.emplace(item.bssid, item);
        }
    }
    return;
}

int WifiSettings::SyncBlockList()
{
    std::unique_lock<std::mutex> lock(mApMutex);
    std::vector<StationInfo> tmp;
    for (auto iter = mBlockListInfo.begin(); iter != mBlockListInfo.end(); ++iter) {
        tmp.push_back(iter->second);
    }
    mSavedBlockInfo.SetValue(tmp);
    return mSavedBlockInfo.SaveConfig();
}

int WifiSettings::ReloadWifiP2pGroupInfoConfig()
{
    std::unique_lock<std::mutex> lock(mP2pMutex);
    if (mSavedWifiP2pGroupInfo.LoadConfig()) {
        return -1;
    }
    mSavedWifiP2pGroupInfo.GetValue(mGroupInfoList);
    return 0;
}

void WifiSettings::InitDefaultP2pVendorConfig()
{
    mP2pVendorConfig.SetRandomMacSupport(false);
    mP2pVendorConfig.SetIsAutoListen(false);
    mP2pVendorConfig.SetDeviceName("");
    mP2pVendorConfig.SetPrimaryDeviceType("");
    mP2pVendorConfig.SetSecondaryDeviceType("");
}

void WifiSettings::InitP2pVendorConfig()
{
    if (mSavedWifiP2pVendorConfig.LoadConfig() >= 0) {
        std::vector<P2pVendorConfig> tmp;
        mSavedWifiP2pVendorConfig.GetValue(tmp);
        if (tmp.size() > 0) {
            mP2pVendorConfig = tmp[0];
        } else {
            InitDefaultP2pVendorConfig();
        }
    } else {
        InitDefaultP2pVendorConfig();
    }
    return;
}

int WifiSettings::GetApMaxConnNum()
{
    return mApMaxConnNum;
}

void WifiSettings::InitDefaultWifiConfig()
{
    WifiConfig wifiConfig;
    mWifiConfig[0] = wifiConfig;
}

void WifiSettings::InitWifiConfig()
{
    if (mSavedWifiConfig.LoadConfig() < 0) {
        return;
    }
    std::vector<WifiConfig> tmp;
    mSavedWifiConfig.GetValue(tmp);
    if (tmp.size() > 0) {
        for (size_t i = 0; i < tmp.size(); ++i) {
            mWifiConfig[i] = tmp[i];
        }
    } else {
        InitDefaultWifiConfig();
    }
    return;
}

int WifiSettings::SyncWifiConfig()
{
    std::unique_lock<std::mutex> lock(mSyncWifiConfigMutex);
    std::vector<WifiConfig> tmp;
    for (auto &item : mWifiConfig) {
        tmp.push_back(item.second);
    }
    mSavedWifiConfig.SetValue(tmp);
    return mSavedWifiConfig.SaveConfig();
}

int WifiSettings::RemoveExcessDeviceConfigs(std::vector<WifiDeviceConfig> &configs) const
{
    int maxNumConfigs = mMaxNumConfigs;
    if (maxNumConfigs < 0) {
        return 1;
    }
    int numExcessNetworks = static_cast<int>(configs.size()) - maxNumConfigs;
    if (numExcessNetworks <= 0) {
        return 1;
    }
    sort(configs.begin(), configs.end(), [](WifiDeviceConfig a, WifiDeviceConfig b) {
        if (a.status != b.status) {
            return (a.status == 0) < (b.status == 0);
        } else if (a.lastConnectTime != b.lastConnectTime) {
            return a.lastConnectTime < b.lastConnectTime;
        } else if (a.numRebootsSinceLastUse != b.numRebootsSinceLastUse) {
            return a.numRebootsSinceLastUse > b.numRebootsSinceLastUse;
        } else if (a.numAssociation != b.numAssociation) {
            return a.numAssociation < b.numAssociation;
        } else {
            return a.networkId < b.networkId;
        }
    });
    std::stringstream removeConfig;
    int maxIndex = numExcessNetworks > MAX_CONFIGS_NUM ? MAX_CONFIGS_NUM : numExcessNetworks;
    for (int i = 0; i < maxIndex; i++) {
        removeConfig << SsidAnonymize(configs[i].ssid) << ",";
    }
    LOGI("saved config size greater than %{public}d, remove ssid(print up to 1000)=%{public}s",
        maxNumConfigs, removeConfig.str().c_str());
    configs.erase(configs.begin(), configs.begin() + numExcessNetworks);
    return 0;
}

std::string WifiSettings::FuzzyBssid(const std::string bssid)
{
    if (bssid.empty() || bssid.length() != MAC_STRING_SIZE) {
        return "";
    }
    return "xx" + bssid.substr(COMPARE_MAC_OFFSET, COMPARE_MAC_LENGTH) + "xx";
}

#ifndef OHOS_ARCH_LITE
void WifiSettings::MergeWifiConfig()
{
    if (std::filesystem::exists(WIFI_CONFIG_FILE_PATH) || std::filesystem::exists(DEVICE_CONFIG_FILE_PATH)
        || std::filesystem::exists(WIFI_STA_RANDOM_MAC_FILE_PATH)) {
        LOGI("file exists don't need to merge");
        return;
    }
    if (!std::filesystem::exists(DUAL_WIFI_CONFIG_FILE_PATH)) {
        LOGI("dual frame file do not exists, don't need to merge");
        return;
    }
    std::unique_ptr<NetworkXmlParser> xmlParser = std::make_unique<NetworkXmlParser>();
    bool ret = xmlParser->LoadConfiguration(DUAL_WIFI_CONFIG_FILE_PATH);
    if (!ret) {
        LOGE("MergeWifiConfig load fail");
        return;
    }
    ret = xmlParser->Parse();
    if (!ret) {
        LOGE("MergeWifiConfig Parse fail");
        return;
    }
    std::vector<WifiDeviceConfig> wifideviceConfig = xmlParser->GetNetworks();
    if (wifideviceConfig.size() == 0) {
        LOGE("MergeWifiConfig wifideviceConfig empty");
        return;
    }
    mSavedDeviceConfig.SetValue(wifideviceConfig);
    mSavedDeviceConfig.SaveConfig();
    std::unique_lock<std::mutex> lock(mStaMutex);
    std::vector<WifiStoreRandomMac> wifiStoreRandomMac = xmlParser->GetRandomMacmap();
    mSavedWifiStoreRandomMac.SetValue(wifiStoreRandomMac);
    mSavedWifiStoreRandomMac.SaveConfig();
}

void WifiSettings::MergeSoftapConfig()
{
    if (std::filesystem::exists(WIFI_CONFIG_FILE_PATH) || std::filesystem::exists(HOTSPOT_CONFIG_FILE_PATH)) {
        LOGI("MergeSoftapConfig file exists don't need to merge");
        return;
    }
    if (!std::filesystem::exists(DUAL_SOFTAP_CONFIG_FILE_PATH)) {
        LOGI("MergeSoftapConfig dual frame file do not exists, don't need to merge");
        return;
    }
    std::unique_ptr<SoftapXmlParser> xmlParser = std::make_unique<SoftapXmlParser>();
    bool ret = xmlParser->LoadConfiguration(DUAL_SOFTAP_CONFIG_FILE_PATH);
    if (!ret) {
        LOGE("MergeSoftapConfig fail");
        return;
    }
    ret = xmlParser->Parse();
    if (!ret) {
        LOGE("MergeSoftapConfig Parse fail");
        return;
    }
    std::vector<HotspotConfig> hotspotConfig = xmlParser->GetSoftapConfigs();
    if (hotspotConfig.size() == 0) {
        LOGE("MergeSoftapConfig hotspotConfig empty");
        return;
    }
    mSavedHotspotConfig.SetValue(hotspotConfig);
    mSavedHotspotConfig.SaveConfig();
}

void WifiSettings::ConfigsDeduplicateAndSave(std::vector<WifiDeviceConfig> &newConfigs)
{
    if (newConfigs.size() == 0) {
        LOGE("NewConfigs is empty!");
        return;
    }
    mSavedDeviceConfig.LoadConfig();
    std::vector<WifiDeviceConfig> localConfigs;
    mSavedDeviceConfig.GetValue(localConfigs);

    std::set<std::string> tmp;
    for (const auto &localConfig : localConfigs) {
        std::string configKey = localConfig.ssid + localConfig.keyMgmt;
        tmp.insert(configKey);
    }
    for (auto &config : newConfigs) {
        std::string configKey = config.ssid + config.keyMgmt;
        auto iter = tmp.find(configKey);
        if (iter == tmp.end()) {
            tmp.insert(configKey);
#ifdef FEATURE_ENCRYPTION_SUPPORT
            EncryptionDeviceConfig(config);
#endif
            localConfigs.push_back(config);
        }
    }
    std::vector<WifiDeviceConfig>().swap(newConfigs);
    mSavedDeviceConfig.SetValue(localConfigs);
    mSavedDeviceConfig.SaveConfig();
    ReloadDeviceConfig();
}

void WifiSettings::ParseBackupJson(const std::string &backupInfo, std::string &key, std::string &iv,
    std::string &version)
{
    const std::string type = "detail";
    const std::string encryptionSymkey = "encryption_symkey";
    const std::string gcmParamsIv = "gcmParams_iv";
    const std::string apiVersion = "api_version";
    std::string keyStr;
    std::string ivStr;

    ParseJson(backupInfo, type, encryptionSymkey, keyStr);
    ParseJson(backupInfo, type, gcmParamsIv, ivStr);
    ParseJson(backupInfo, type, apiVersion, version);
    LOGI("ParseBackupJson version: %{public}s.", version.c_str());
    ConvertDecStrToHexStr(keyStr, key);
    std::fill(keyStr.begin(), keyStr.end(), 0);
    LOGI("ParseBackupJson key.size: %{public}d.", static_cast<int>(key.size()));
    ConvertDecStrToHexStr(ivStr, iv);
    LOGI("ParseBackupJson iv.size: %{public}d.", static_cast<int>(iv.size()));
}

int WifiSettings::GetConfigbyBackupXml(std::vector<WifiDeviceConfig> &deviceConfigs, UniqueFd &fd)
{
    const std::string wifiBackupXmlBegin = "<WifiBackupData>";
    const std::string wifiBackupXmlEnd = "</WifiBackupData>";
    struct stat statBuf;
    if (fd.Get() < 0 || fstat(fd.Get(), &statBuf) < 0) {
        LOGE("GetConfigbyBackupXml fstat fd fail.");
        return -1;
    }
    char *buffer = (char *)malloc(statBuf.st_size);
    if (buffer == nullptr) {
        LOGE("GetConfigbyBackupXml malloc fail.");
        return -1;
    }
    ssize_t bufferLen = read(fd.Get(), buffer, statBuf.st_size);
    if (bufferLen < 0) {
        LOGE("GetConfigbyBackupXml read fail.");
        free(buffer);
        return -1;
    }
    std::string backupData = std::string(buffer, buffer + bufferLen);
    if (memset_s(buffer, statBuf.st_size, 0, statBuf.st_size) != EOK) {
        LOGE("GetConfigbyBackupXml memset_s fail.");
        free(buffer);
        return -1;
    }
    free(buffer);
    buffer = nullptr;

    std::string wifiBackupXml;
    SplitStringBySubstring(backupData, wifiBackupXml, wifiBackupXmlBegin, wifiBackupXmlEnd);
    std::fill(backupData.begin(), backupData.end(), 0);
    std::unique_ptr<NetworkXmlParser> xmlParser = std::make_unique<NetworkXmlParser>();
    bool ret = xmlParser->LoadConfigurationMemory(wifiBackupXml.c_str());
    if (!ret) {
        LOGE("GetConfigbyBackupXml load fail");
        return -1;
    }
    ret = xmlParser->Parse();
    if (!ret) {
        LOGE("GetConfigbyBackupXml Parse fail");
        return -1;
    }
    deviceConfigs = xmlParser->GetNetworks();
    std::fill(wifiBackupXml.begin(), wifiBackupXml.end(), 0);
    return 0;
}

int WifiSettings::GetConfigbyBackupFile(std::vector<WifiDeviceConfig> &deviceConfigs, UniqueFd &fd,
    const std::string &key, const std::string &iv)
{
    if (key.size() == 0 || iv.size() == 0) {
        LOGE("GetConfigbyBackupFile key or iv is empty.");
        return -1;
    }
    struct stat statBuf;
    if (fd.Get() < 0 || fstat(fd.Get(), &statBuf) < 0) {
        LOGE("GetConfigbyBackupFile fstat fd fail.");
        return -1;
    }
    int destFd = open(BACKUP_CONFIG_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (destFd < 0) {
        LOGE("GetConfigbyBackupFile open file fail.");
        return -1;
    }
    if (sendfile(destFd, fd.Get(), nullptr, statBuf.st_size) < 0) {
        LOGE("GetConfigbyBackupFile fd sendfile(size: %{public}d) to destFd fail.", static_cast<int>(statBuf.st_size));
        close(destFd);
        return -1;
    }
    close(destFd);

    WifiConfigFileImpl<WifiBackupConfig> wifiBackupConfig;
    wifiBackupConfig.SetConfigFilePath(BACKUP_CONFIG_FILE_PATH);
    wifiBackupConfig.SetEncryptionInfo(key, iv);
    wifiBackupConfig.LoadConfig();
    std::vector<WifiBackupConfig> backupConfigs;
    wifiBackupConfig.GetValue(backupConfigs);
    wifiBackupConfig.UnsetEncryptionInfo();

    for (const auto &backupCfg : backupConfigs) {
        WifiDeviceConfig config;
        ConvertBackupCfgToDeviceCfg(backupCfg, config);
        deviceConfigs.push_back(config);
    }
    return 0;
}
#endif

#ifdef FEATURE_ENCRYPTION_SUPPORT
bool WifiSettings::IsWifiDeviceConfigDeciphered(const WifiDeviceConfig &config) const
{
    
    int keyIndex = (config.wepTxKeyIndex < 0 || config.wepTxKeyIndex >= WEPKEYS_SIZE) ? 0 : config.wepTxKeyIndex;
    if (!config.preSharedKey.empty() || !config.wepKeys[keyIndex].empty() || !config.wifiEapConfig.password.empty()) {
        return true;
    }

    return false;
}

void WifiSettings::DecryptionWapiConfig(const WifiEncryptionInfo &wifiEncryptionInfo, WifiDeviceConfig &config) const
{
    if (config.keyMgmt != KEY_MGMT_WAPI_CERT) {
        return;
    }

    EncryptedData *encryWapiAs = new EncryptedData(config.wifiWapiConfig.encryptedAsCertData,
        config.wifiWapiConfig.asCertDataIV);
    std::string decryWapiAs = "";
    if (WifiDecryption(wifiEncryptionInfo, *encryWapiAs, decryWapiAs) == HKS_SUCCESS) {
        config.wifiWapiConfig.wapiAsCertData = decryWapiAs;
    } else {
        WriteWifiEncryptionFailHiSysEvent(DECRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        config.wifiWapiConfig.wapiAsCertData = "";
    }
    delete encryWapiAs;

    EncryptedData *encryWapiUser = new EncryptedData(config.wifiWapiConfig.encryptedUserCertData,
        config.wifiWapiConfig.userCertDataIV);
    std::string decryWapiUser = "";
    if (WifiDecryption(wifiEncryptionInfo, *encryWapiUser, decryWapiUser) == HKS_SUCCESS) {
        config.wifiWapiConfig.wapiUserCertData = decryWapiUser;
    } else {
        WriteWifiEncryptionFailHiSysEvent(DECRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        config.wifiWapiConfig.wapiUserCertData = "";
    }
    delete encryWapiUser;
}

int WifiSettings::DecryptionDeviceConfig(WifiDeviceConfig &config)
{
    if (IsWifiDeviceConfigDeciphered(config)) {
        LOGI("DecryptionDeviceConfig IsWifiDeviceConfigDeciphered true");
        return 0;
    }
    LOGD("DecryptionDeviceConfig start");
    WifiEncryptionInfo mWifiEncryptionInfo;
    mWifiEncryptionInfo.SetFile(GetTClassName<WifiDeviceConfig>());
    EncryptedData *encry = new EncryptedData(config.encryptedData, config.IV);
    std::string decry = "";
    if (WifiDecryption(mWifiEncryptionInfo, *encry, decry) == HKS_SUCCESS) {
        config.preSharedKey = decry;
    } else {
        WriteWifiEncryptionFailHiSysEvent(DECRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        config.preSharedKey = "";
    }
    delete encry;

    if (config.wepTxKeyIndex < 0 || config.wepTxKeyIndex >= WEPKEYS_SIZE) {
        config.wepTxKeyIndex = 0;
    }
    EncryptedData *encryWep = new EncryptedData(config.encryWepKeys[config.wepTxKeyIndex], config.IVWep);
    std::string decryWep = "";
    if (WifiDecryption(mWifiEncryptionInfo, *encryWep, decryWep) == HKS_SUCCESS) {
        config.wepKeys[config.wepTxKeyIndex] = decryWep;
    } else {
        WriteWifiEncryptionFailHiSysEvent(DECRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        config.wepKeys[config.wepTxKeyIndex] = "";
    }
    delete encryWep;

    EncryptedData *encryEap = new EncryptedData(config.wifiEapConfig.encryptedData, config.wifiEapConfig.IV);
    std::string decryEap = "";
    if (WifiDecryption(mWifiEncryptionInfo, *encryEap, decryEap) == HKS_SUCCESS) {
        config.wifiEapConfig.password = decryEap;
    } else {
        WriteWifiEncryptionFailHiSysEvent(DECRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        config.wifiEapConfig.password = "";
    }
    delete encryEap;
    DecryptionWapiConfig(mWifiEncryptionInfo, config);
    LOGD("DecryptionDeviceConfig end");
    return 0;
}

bool WifiSettings::EncryptionWapiConfig(const WifiEncryptionInfo &wifiEncryptionInfo, WifiDeviceConfig &config) const
{
    if (config.keyMgmt != KEY_MGMT_WAPI_CERT) {
        return true;
    }

    if (config.wifiWapiConfig.wapiAsCertData.empty() || config.wifiWapiConfig.wapiUserCertData.empty()) {
        LOGE("EncryptionDeviceConfig wapiCertData empty");
        return false;
    }

    config.wifiWapiConfig.encryptedAsCertData = "";
    config.wifiWapiConfig.asCertDataIV = "";

    EncryptedData encryWapiAs;
    if (WifiEncryption(wifiEncryptionInfo, config.wifiWapiConfig.wapiAsCertData, encryWapiAs) == HKS_SUCCESS) {
        config.wifiWapiConfig.encryptedAsCertData = encryWapiAs.encryptedPassword;
        config.wifiWapiConfig.asCertDataIV = encryWapiAs.IV;
    } else {
        LOGE("EncryptionDeviceConfig WifiEncryption wapiAsCertData failed");
        WriteWifiEncryptionFailHiSysEvent(ENCRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        return false;
    }

    config.wifiWapiConfig.encryptedUserCertData = "";
    config.wifiWapiConfig.userCertDataIV = "";

    EncryptedData encryWapiUser;
    if (WifiEncryption(wifiEncryptionInfo, config.wifiWapiConfig.wapiUserCertData, encryWapiUser) == HKS_SUCCESS) {
        config.wifiWapiConfig.encryptedUserCertData = encryWapiUser.encryptedPassword;
        config.wifiWapiConfig.userCertDataIV = encryWapiUser.IV;
    } else {
        LOGE("EncryptionDeviceConfig WifiEncryption wapiUserCertData failed");
        WriteWifiEncryptionFailHiSysEvent(ENCRYPTION_EVENT,
            SsidAnonymize(config.ssid), config.keyMgmt, STA_MOUDLE_EVENT);
        return false;
    }
    return true;
}
#endif
}  // namespace Wifi
}  // namespace OHOS
