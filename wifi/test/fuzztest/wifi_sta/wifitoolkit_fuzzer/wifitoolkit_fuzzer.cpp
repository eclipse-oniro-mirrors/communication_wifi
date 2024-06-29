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
#include <mutex>

namespace OHOS {
namespace Wifi {
constexpr int U32_AT_SIZE_ZERO = 4;
constexpr int WIFI_MAX_SSID_LEN = 16;
static bool g_isInsted = false;

class MockXmlParser : public XmlParser {
public:
    bool ParseInternal(xmlNodePtr node)
    {
        return false;
    }
};
static xmlNodePtr root_node;
static std::unique_ptr<NetworkXmlParser> m_networkXmlParser = nullptr;
static std::unique_ptr<AppParser> m_appXmlParser = nullptr;
static std::unique_ptr<MockXmlParser> m_xmlParser = nullptr;
static std::unique_ptr<SoftapXmlParser> m_softapXmlParser = nullptr;

void MyExit()
{
    m_networkXmlParser.reset();
    m_appXmlParser.reset();
    m_xmlParser.reset();
    m_softapXmlParser.reset();
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

void InitParam()
{
    if (!g_isInsted) {
        m_networkXmlParser = std::make_unique<NetworkXmlParser>();
        m_appXmlParser = std::make_unique<AppParser>();
        m_xmlParser = std::make_unique<MockXmlParser>();
        m_softapXmlParser = std::make_unique<SoftapXmlParser>();
        InitAppParserTest();
        if (m_networkXmlParser == nullptr) {
            return;
        }
        atexit(MyExit);
        g_isInsted = true;
    }
    return;
}

void NetworkXmlParserTest(const uint8_t* data, size_t size)
{
    WifiDeviceConfig config;
    config.ssid = std::string(reinterpret_cast<const char*>(data), size);
    config.bssid = std::string(reinterpret_cast<const char*>(data), size);
    config.preSharedKey = std::string(reinterpret_cast<const char*>(data), size);
    config.keyMgmt = std::string(reinterpret_cast<const char*>(data), size);

    m_networkXmlParser->GetIpConfig(root_node);
    m_networkXmlParser->GetConfigNameAsInt(root_node);
    m_networkXmlParser->GotoNetworkList(root_node);
    m_networkXmlParser->GetNodeNameAsInt(root_node);
    m_networkXmlParser->ParseIpConfig(root_node);
    m_networkXmlParser->GetProxyMethod(root_node);
    m_networkXmlParser->ParseProxyConfig(root_node);
    m_networkXmlParser->HasWepKeys(config);
    m_networkXmlParser->GetKeyMgmt(root_node, config);
    m_networkXmlParser->GetRandMacSetting(root_node);
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
    std::string conditionName = std::string(reinterpret_cast<const char*>(data), size);
    char *string = nullptr;
    if (memcpy_s(string, WIFI_MAX_SSID_LEN, data, WIFI_MAX_SSID_LEN - 1) != EOK) {
        return;
    }
    m_appXmlParser->IsWhiteListApp(conditionName);
    m_appXmlParser->IsBlackListApp(conditionName);
    m_appXmlParser->IsChariotApp(conditionName);
    m_appXmlParser->IsHighTempLimitSpeedApp(conditionName);
    m_appXmlParser->InitAppParser(string);

    m_appXmlParser->ParseInternal(root_node);
    m_appXmlParser->GetAppTypeAsInt(root_node);
    m_appXmlParser->ReadPackageCloudFilterConfig();
    m_appXmlParser->IsReadCloudConfig();
    m_appXmlParser->GetCloudPushFileVersion(string);
    m_appXmlParser->GetLocalFileVersion(string);
    m_appXmlParser->GetCloudPushVersionFilePath();
    m_appXmlParser->GetCloudPushJsonFilePath();
    m_xmlParser->LoadConfiguration(string);
    m_xmlParser->LoadConfigurationMemory(string);
}

void AppParserTest(const uint8_t* data, size_t size)
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

void SoftapParserTest(const uint8_t* data, size_t size)
{
    m_softapXmlParser->ParseInternal(root_node);
    m_softapXmlParser->GotoSoftApNode(root_node);
    m_softapXmlParser->ParseSoftap(root_node);
    m_softapXmlParser->GetConfigNameAsInt(root_node);
    m_softapXmlParser->GetBandInfo(root_node);
    m_softapXmlParser->TransBandinfo(root_node);
    m_softapXmlParser->GetSoftapConfigs();
}
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= OHOS::Wifi::U32_AT_SIZE_ZERO)) {
        return 0;
    }
    OHOS::Wifi::InitParam();
    OHOS::Wifi::NetworkXmlParserTest(data, size);
    OHOS::Wifi::AppXmlParserTest(data, size);
    OHOS::Wifi::AppParserTest(data, size);
    OHOS::Wifi::SoftapParserTest(data, size);
    return 0;
}
}
}
