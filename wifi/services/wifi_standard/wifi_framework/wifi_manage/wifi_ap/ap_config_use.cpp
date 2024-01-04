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

#include "ap_config_use.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include "log_helper.h"
#include "wifi_ap_hal_interface.h"
#include "wifi_common_util.h"
#include "wifi_country_code_manager.h"
#include "wifi_global_func.h"
#include "wifi_logger.h"
#include "wifi_msg.h"
#include "wifi_p2p_msg.h"
#include "wifi_settings.h"

namespace OHOS {
namespace Wifi {
DEFINE_WIFILOG_HOTSPOT_LABEL("WifiApConfigUse");

ApConfigUse::ApConfigUse(int id) : m_id(id)
{
    m_softapChannelPolicyPtr = std::make_unique<SoftapChannelPolicyParser>();
}

void ApConfigUse::UpdateApChannelConfig(HotspotConfig &apConfig)
{
    JudgeConflictBand(apConfig);
    int bestChannel = AP_CHANNEL_INVALID;
    switch (apConfig.GetBand()) {
        case BandType::BAND_2GHZ:
            bestChannel = GetBestChannelFor2G();
            break;
        case BandType::BAND_5GHZ:
            bestChannel = GetBestChannelFor5G();
            break;
        default:
            // BAND_6GHZ, BAND_60GHZ do nothing
            break;
    }

    if (bestChannel == AP_CHANNEL_INVALID) {
        // if there is no suitable channel, use the default band=2.4GHZ, channel=6
        apConfig.SetBand(BandType::BAND_2GHZ);
        apConfig.SetChannel(AP_CHANNEL_DEFAULT);
    } else {
        apConfig.SetChannel(bestChannel);
    }
    JudgeDbacWithP2p(apConfig);
    WIFI_LOGI("ap config: ssid=%{public}s, inst_id=%{public}d, band=%{public}d, channel=%{public}d",
        SsidAnonymize(apConfig.GetSsid()).c_str(), m_id, static_cast<int>(apConfig.GetBand()), apConfig.GetChannel());
}

void ApConfigUse::JudgeConflictBand(HotspotConfig &apConfig)
{
    BandType apBand = apConfig.GetBand();
    BandType p2pBand = BandType::BAND_NONE;
    BandType staBand = BandType::BAND_NONE;
    WifiLinkedInfo wifiLinkedInfo;
    WifiSettings::GetInstance().GetLinkedInfo(wifiLinkedInfo, DEFAULT_STA_INSTANCE_ID);
    if (wifiLinkedInfo.connState == ConnState::CONNECTED) {
        switch (wifiLinkedInfo.band) {
            case static_cast<int>(BandType::BAND_2GHZ):
                staBand = BandType::BAND_2GHZ;
                break;
            case static_cast<int>(BandType::BAND_5GHZ):
                staBand = BandType::BAND_5GHZ;
                break;
            default:
                // BAND_6GHZ, BAND_60GHZ do nothing
                break;
        }
    }
    WifiP2pLinkedInfo p2pLinkedInfo;
    WifiSettings::GetInstance().GetP2pInfo(p2pLinkedInfo);
    if (p2pLinkedInfo.GetConnectState() == P2pConnectedState::P2P_CONNECTED) {
        WifiP2pGroupInfo group = WifiSettings::GetInstance().GetCurrentP2pGroupInfo();
        p2pBand = TransformFreqToBand(group.GetFrequency());
    }
    if (p2pBand == BandType::BAND_NONE || staBand == BandType::BAND_NONE || p2pBand != staBand) {
        WIFI_LOGI("no need judge conflict, p2pBandType=%{public}d, staBandType=%{public}d",
            static_cast<int>(p2pBand), static_cast<int>(staBand));
        return;
    }

    // handling scenes with the same band
    if (apBand >= BandType::BAND_5GHZ && p2pBand == BandType::BAND_5GHZ) {
        apBand = BandType::BAND_2GHZ;
    } else if (apBand == BandType::BAND_2GHZ && p2pBand == BandType::BAND_2GHZ) {
        apBand = BandType::BAND_5GHZ;
    }
    apConfig.SetBand(apBand);
    WIFI_LOGI("judge conflict result, p2pBandType=%{public}d, staBandType=%{public}d, apBandType=%{public}d",
        static_cast<int>(p2pBand), static_cast<int>(staBand), static_cast<int>(apBand));
}

int ApConfigUse::GetBestChannelFor2G()
{
    std::vector<int> channels = GetChannelFromDrvOrXmlByBand(BandType::BAND_2GHZ);
    if (channels.empty()) {
        WIFI_LOGI("GetBestChannelFor2G is empty");
        return AP_CHANNEL_INVALID;
    }

    // randomly select a channel as the best channel
    return channels[GetRandomInt(0, channels.size() - 1)];
}

int ApConfigUse::GetBestChannelFor5G()
{
    std::vector<int> channels = GetChannelFromDrvOrXmlByBand(BandType::BAND_5GHZ);
    FilterIndoorChannel(channels);
    Filter165Channel(channels);
    if (channels.empty()) {
        WIFI_LOGI("GetBestChannelFor5G is empty");
        return AP_CHANNEL_INVALID;
    }
    for (auto item : channels) {
        if (AP_CHANNEL_5G_DEFAULT == item) {
            return AP_CHANNEL_5G_DEFAULT;  // channel 149 is preferred for 5G
        }
    }

    // randomly select a channel as the best channel
    return channels[GetRandomInt(0, channels.size() - 1)];
}

std::vector<int> ApConfigUse::GetChannelFromDrvOrXmlByBand(const BandType &bandType)
{
    CHECK_NULL_AND_RETURN(m_softapChannelPolicyPtr, {});
    std::vector<int> preferredChannels = m_softapChannelPolicyPtr->GetPreferredChannels(bandType);
    if (!preferredChannels.empty()) {
        WIFI_LOGI("get freqs from xml success, bandType=%{public}d, channel size=%{public}lu",
            static_cast<int>(bandType), preferredChannels.size());
        return preferredChannels;
    }
    std::vector<int> freqs;
    WifiErrorNo ret = WifiApHalInterface::GetInstance().GetFrequenciesByBand(static_cast<int>(bandType), freqs);
    if (ret != WifiErrorNo::WIFI_IDL_OPT_OK) {
        WIFI_LOGI("get freqs from drv fail, use default, bandType=%{public}d", static_cast<int>(bandType));
        WifiSettings::GetInstance().SetDefaultFrequenciesByCountryBand(bandType, freqs);
    } else {
        WIFI_LOGI("get freqs from drv success, bandType=%{public}d, size=%{public}lu",
            static_cast<int>(bandType), freqs.size());
    }
    std::vector<int> channels;
    TransformFrequencyIntoChannel(freqs, channels);
    return channels;
}

void ApConfigUse::FilterIndoorChannel(std::vector<int> &channels)
{
    if (channels.empty()) {
        return;
    }
    std::string wifiCountryCode;
    WifiCountryCodeManager::GetInstance().GetWifiCountryCode(wifiCountryCode);
    CHECK_NULL_AND_RETURN_NULL(m_softapChannelPolicyPtr);
    std::set<int> indoorChannels = m_softapChannelPolicyPtr->GetIndoorChannels(wifiCountryCode);
    if (indoorChannels.empty()) {
        WIFI_LOGI("indoor channel is empty");
        return;
    }
    std::stringstream filteredChannels;
    std::vector<int> tempChannels;
    for (auto item : channels) {
        if (indoorChannels.find(item) == indoorChannels.end()) {
            tempChannels.push_back(item);
            continue;
        }
        filteredChannels << item << " ";
    }
    WIFI_LOGI("filter indoor channels=%{public}s", filteredChannels.str().c_str());
    channels = std::move(tempChannels);
}

/* Channel 165 cannot be combined with a channel with a bandwidth of 40 MHz or higher.
   Therefore, channel 165 is not recommended */
void ApConfigUse::Filter165Channel(std::vector<int> &channels)
{
    if (channels.empty()) {
        return;
    }
    auto iter = channels.begin();
    while (iter != channels.end()) {
        if (AP_CHANNEL_5G_NOT_RECOMMEND == *iter) {
            channels.erase(iter);
            WIFI_LOGI("filter 165 channel");
            return;
        }
        iter++;
    }
}

void ApConfigUse::JudgeDbacWithP2p(HotspotConfig &apConfig)
{
    WifiP2pLinkedInfo p2pLinkedInfo;
    WifiSettings::GetInstance().GetP2pInfo(p2pLinkedInfo);
 
    // When playing the go role on the local end, the P2P and AP channels must be consistent,
    // but the GC can be inconsistent. If consistency is required, the underlying layer will switch spontaneously.
    if (p2pLinkedInfo.GetConnectState() != P2pConnectedState::P2P_CONNECTED || !p2pLinkedInfo.IsGroupOwner()) {
        return;
    }
    WifiP2pGroupInfo group = WifiSettings::GetInstance().GetCurrentP2pGroupInfo();
    int p2pChannel = TransformFrequencyIntoChannel(group.GetFrequency());
    int apChannel = apConfig.GetChannel();
    if (IsChannelDbac(p2pChannel, apChannel) && TransformChannelToBand(p2pChannel) != BandType::BAND_NONE) {
        WIFI_LOGI("dbac, follow the p2p band and channel, p2pChannel=%{public}d", p2pChannel);
        apConfig.SetBand(TransformChannelToBand(p2pChannel));
        apConfig.SetChannel(p2pChannel);
    }
}

std::set<int> ApConfigUse::SoftapChannelPolicyParser::GetIndoorChannels(const std::string &countryCode)
{
    std::set<int> indoorChannelByCode;
    if (countryCode.empty() || m_indoorChannels.find(countryCode) == m_indoorChannels.end()) {
        return indoorChannelByCode;
    }
    indoorChannelByCode = m_indoorChannels.find(countryCode)->second;
    return indoorChannelByCode;
}

std::vector<int> ApConfigUse::SoftapChannelPolicyParser::GetPreferredChannels(const BandType &bandType)
{
    std::vector<int> preferredChannelByBand;
    if (m_preferredChannels.find(bandType) == m_preferredChannels.end()) {
        return preferredChannelByBand;
    }
    preferredChannelByBand = m_preferredChannels.find(bandType)->second;
    return preferredChannelByBand;
}

ApConfigUse::SoftapChannelPolicyParser::SoftapChannelPolicyParser()
{
    g_softapChannelsPolicyMap = {
        {SoftapChannelPolicyParser::XML_TAG_COUNTRY_CODE,
            SoftapChannelPolicyParser::SoftapChannelsPolicyType::COUNTRY_CODE},
        {SoftapChannelPolicyParser::XML_TAG_INDOOR_CHANNELS,
            SoftapChannelPolicyParser::SoftapChannelsPolicyType::INDOOR_CHANNELS}
    };
    g_bandTypeMap = {
        {SoftapChannelPolicyParser::XML_TAG_CHANNEL_2G_LIST, BandType::BAND_2GHZ},
        {SoftapChannelPolicyParser::XML_TAG_CHANNEL_5G_LIST, BandType::BAND_5GHZ},
        {SoftapChannelPolicyParser::XML_TAG_CHANNEL_6G_LIST, BandType::BAND_6GHZ},
        {SoftapChannelPolicyParser::XML_TAG_CHANNEL_60G_LIST, BandType::BAND_60GHZ}
    };
    InitParser();
}

ApConfigUse::SoftapChannelPolicyParser::~SoftapChannelPolicyParser()
{
    m_indoorChannels.clear();
    m_preferredChannels.clear();
}

bool ApConfigUse::SoftapChannelPolicyParser::InitParser()
{
    if (!std::filesystem::exists(SOFTAP_CHANNELS_POLICY_FILE_PATH)) {
        LOGI("softap_channels_policy_file.xml not exists, filtering indoor channels is not required");
        return false;
    }
    bool ret = LoadConfiguration(SOFTAP_CHANNELS_POLICY_FILE_PATH);
    if (!ret) {
        LOGE("load softap_channels_policy_file fail");
        return false;
    }
    ret = Parse();  // the parent class invokes ParseInternal
    if (!ret) {
        WIFI_LOGE("parse softap_channels_policy_file failed");
        return ret;
    }
    return ret;
}

bool ApConfigUse::SoftapChannelPolicyParser::ParseInternal(xmlNodePtr node)
{
    if (node == nullptr || xmlStrcmp(node->name, BAD_CAST(XML_TAG_SOFTAP_CHANNELS_POLICY)) != 0) {
        WIFI_LOGE("softap_channels_policy_file doc invalid");
        return false;
    }
    ParseCountryPolicyList(node);
    ParsePreferredChannelsList(node);
    return true;
}

void ApConfigUse::SoftapChannelPolicyParser::ParseCountryPolicyList(xmlNodePtr innode)
{
    if (innode == nullptr) {
        WIFI_LOGE("parse CountryPolicy node is null");
        return;
    }
    xmlNodePtr policyNodeList = GotoCountryPolicy(innode);
    for (xmlNodePtr node = policyNodeList->children; node != nullptr; node = node->next) {
        if (xmlStrcmp(node->name, BAD_CAST(XML_TAG_POLICY_ITEM)) != 0) {
            continue;
        }
        std::string code;
        std::set<int> channels;
        for (xmlNodePtr item = node->children; item != nullptr; item = item->next) {
            switch (GetPolicyItem(item)) {
                case SoftapChannelsPolicyType::COUNTRY_CODE:
                    code = GetStringValue(item);
                    break;
                case SoftapChannelsPolicyType::INDOOR_CHANNELS:
                    channels = ParseChannels(item);
                    break;
                default:
                    break;
            }
        }
        if (IsValidCountryCode(code) && channels.size() > 0) {
            m_indoorChannels.insert({code, channels});
        }
    }
    WIFI_LOGI("parse CountryPolicy final");
}

std::set<int> ApConfigUse::SoftapChannelPolicyParser::ParseChannels(xmlNodePtr innode)
{
    if (innode == nullptr) {
        WIFI_LOGE("parse channels node is null");
        return {};
    }
    std::string channelsStr = GetStringValue(innode);
    if (channelsStr.empty()) {
        return {};
    }
    std::vector<int> channelsVector = SplitStringToIntVector(channelsStr, ",");
    std::set<int> channelsSet(channelsVector.begin(), channelsVector.end());
    return channelsSet;
}

xmlNodePtr ApConfigUse::SoftapChannelPolicyParser::GotoCountryPolicy(xmlNodePtr innode)
{
    if (innode == nullptr) {
        WIFI_LOGE("goto SoftapChannelsPolicy node is null");
        return nullptr;
    }
    for (xmlNodePtr node = innode->children; node != nullptr; node = node->next) {
        if (xmlStrcmp(node->name, BAD_CAST(XML_TAG_CHANNELS_POLICY)) == 0) {
            return node;
        }
    }
    return nullptr;
}

ApConfigUse::SoftapChannelPolicyParser::SoftapChannelsPolicyType
    ApConfigUse::SoftapChannelPolicyParser::GetPolicyItem(xmlNodePtr node)
{
    if (node == nullptr) {
        WIFI_LOGE("GetPolicyItem node is null");
        return SoftapChannelsPolicyType::UNVALID;
    }
    std::string tagName = GetNodeValue(node);
    if (g_softapChannelsPolicyMap.find(tagName) != g_softapChannelsPolicyMap.end()) {
        return g_softapChannelsPolicyMap.at(tagName);
    }
    return SoftapChannelsPolicyType::UNVALID;
}

void ApConfigUse::SoftapChannelPolicyParser::ParsePreferredChannelsList(xmlNodePtr innode)
{
    if (innode == nullptr) {
        WIFI_LOGE("parse SoftapSupportChannels node is null");
        return;
    }
    xmlNodePtr policyNode = GotoSoftapSupportChannels(innode);
    for (xmlNodePtr item = policyNode->children; item != nullptr; item = item->next) {
        std::vector<int> channels;
        switch (GetSupportChannelsItem(item)) {
            case BandType::BAND_2GHZ:
                channels = ParseSupportChannels(item, XML_TAG_CHANNEL_2G_LIST);
                m_preferredChannels.insert({BandType::BAND_2GHZ, channels});
                break;
            case BandType::BAND_5GHZ:
                channels = ParseSupportChannels(item, XML_TAG_CHANNEL_5G_LIST);
                m_preferredChannels.insert({BandType::BAND_5GHZ, channels});
                break;
            case BandType::BAND_6GHZ:
                channels = ParseSupportChannels(item, XML_TAG_CHANNEL_6G_LIST);
                m_preferredChannels.insert({BandType::BAND_6GHZ, channels});
                break;
            case BandType::BAND_60GHZ:
                channels = ParseSupportChannels(item, XML_TAG_CHANNEL_60G_LIST);
                m_preferredChannels.insert({BandType::BAND_60GHZ, channels});
                break;
            default:
                break;
        }
    }
    WIFI_LOGI("parse SoftapSupportChannels final");
}

xmlNodePtr ApConfigUse::SoftapChannelPolicyParser::GotoSoftapSupportChannels(xmlNodePtr innode)
{
    if (innode == nullptr) {
        WIFI_LOGE("goto SoftapSupportChannels node is null");
        return nullptr;
    }
    for (xmlNodePtr node = innode->children; node != nullptr; node = node->next) {
        if (xmlStrcmp(node->name, BAD_CAST(XML_TAG_SOFTAP_SUPPORT_CHANNELS)) == 0) {
            return node;
        }
    }
    return nullptr;
}

BandType ApConfigUse::SoftapChannelPolicyParser::GetSupportChannelsItem(xmlNodePtr node)
{
    if (node == nullptr) {
        WIFI_LOGE("GetSupportChannelsItem node is null");
        return BandType::BAND_NONE;
    }
    std::string tagName = GetNodeValue(node);
    if (g_bandTypeMap.find(tagName) != g_bandTypeMap.end()) {
        return g_bandTypeMap.at(tagName);
    }
    return BandType::BAND_NONE;
}

std::vector<int> ApConfigUse::SoftapChannelPolicyParser::ParseSupportChannels(xmlNodePtr innode, const char* const &bandXml)
{
    if (innode == nullptr) {
        WIFI_LOGE("parse channels node is null");
        return {};
    }
    std::string channelsStr = GetStringValue(innode);
    if (channelsStr.empty()) {
        return {};
    }
    return SplitStringToIntVector(channelsStr, ",");
}
}  // namespace Wifi
}  // namespace OHOS