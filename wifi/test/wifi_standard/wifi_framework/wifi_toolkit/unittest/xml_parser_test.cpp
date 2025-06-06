/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <iostream>
#include <map>
#include <memory>
#include "wifi_log.h"
#include "wifi_logger.h"
#include "wifi_internal_msg.h"
#include "wifi_msg.h"
#include "wifi_errcode.h"
#include "xml_parser.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::StrEq;
using ::testing::TypedEq;
using ::testing::ext::TestSize;
  
namespace OHOS {
namespace Wifi {
static std::string g_errLog;
void XmlParserLogCallback(const LogType type, const LogLevel level,
                          const unsigned int domain, const char *tag,
                          const char *msg)
{
    g_errLog = msg;
}

DEFINE_WIFILOG_LABEL("XmlParserTest");

class MockXmlParser : public XmlParser {
public:
    bool ParseInternal(xmlNodePtr node)
    {
        return false;
    }
};

class XmlParserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp()
    {
        m_xmlParser  = std::make_unique<MockXmlParser>();
        LOG_SetCallback(XmlParserLogCallback);
    }
    virtual void TearDown() {}
private:
    std::unique_ptr<MockXmlParser> m_xmlParser;
};

HWTEST_F(XmlParserTest, DestroyTest, TestSize.Level1)
{
    WIFI_LOGI("DestroyTest enter");
    m_xmlParser->Destroy();
    EXPECT_FALSE(g_errLog.find("service is null") != std::string::npos);
}

HWTEST_F(XmlParserTest, LoadConfigurationTest, TestSize.Level1)
{
    WIFI_LOGI("LoadConfigurationTest enter");
    EXPECT_FALSE(m_xmlParser->LoadConfiguration(nullptr));
}

HWTEST_F(XmlParserTest, LoadConfigurationMemoryTest, TestSize.Level1)
{
    WIFI_LOGI("LoadConfigurationMemoryTest enter");
    EXPECT_FALSE(m_xmlParser->LoadConfigurationMemory(nullptr));
}

HWTEST_F(XmlParserTest, ParseTest, TestSize.Level1)
{
    WIFI_LOGI("ParseTest enter");
    EXPECT_FALSE(m_xmlParser->Parse());
}

HWTEST_F(XmlParserTest, GetNameValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetNameValueTest enter");
    EXPECT_EQ("", m_xmlParser->GetNameValue(nullptr));
}

HWTEST_F(XmlParserTest, GetNodeValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetNodeValueTest enter");
    EXPECT_EQ("", m_xmlParser->GetNodeValue(nullptr));
}

HWTEST_F(XmlParserTest, GetStringValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetStringValueTest enter");
    EXPECT_EQ("", m_xmlParser->GetStringValue(nullptr));
}

HWTEST_F(XmlParserTest, GetStringArrValue_ValidChildNodesTest, TestSize.Level1)
{
    WIFI_LOGI("GetStringArrValue_ValidChildNodesTest enter");
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "TestNode");
    xmlNewProp(node, BAD_CAST "num", BAD_CAST "2");

    xmlNodePtr child1 = xmlNewChild(node, nullptr, BAD_CAST "item", nullptr);
    xmlNewProp(child1, BAD_CAST "value", BAD_CAST "value1");

    xmlNodePtr child2 = xmlNewChild(node, nullptr, BAD_CAST "item", nullptr);
    xmlNewProp(child2, BAD_CAST "value", BAD_CAST "value2");

    std::vector<std::string> result = m_xmlParser->GetStringArrValue(node);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "value1");
    EXPECT_EQ(result[1], "value2");

    xmlFreeNode(node);
}

HWTEST_F(XmlParserTest, GetStringArrValue_MissingValuePropertyTest, TestSize.Level1)
{
    WIFI_LOGI("GetStringArrValue_MissingValuePropertyTest enter");
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "TestNode");
    xmlNewProp(node, BAD_CAST "num", BAD_CAST "2");

    xmlNodePtr child1 = xmlNewChild(node, nullptr, BAD_CAST "item", nullptr);
    xmlNewProp(child1, BAD_CAST "value", BAD_CAST "value1");

    xmlNodePtr child2;
    child2 = xmlNewChild(node, nullptr, BAD_CAST "item", nullptr);
    // Missing "value" property for child2

    EXPECT_EQ(m_xmlParser->GetStringArrValue(node).size(), 1);

    xmlFreeNode(node);
}

HWTEST_F(XmlParserTest, GetStringArrValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetStringArrValueTest enter");
    EXPECT_EQ(m_xmlParser->GetStringArrValue(nullptr).size(), 0);
}

HWTEST_F(XmlParserTest, GetByteArrValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetByteArrValueTest enter");
    EXPECT_EQ(m_xmlParser->GetByteArrValue(nullptr).size(), 0);
}

HWTEST_F(XmlParserTest, GetByteArrValue_ValidContentTest, TestSize.Level1)
{
    WIFI_LOGI("GetByteArrValue_ValidContentTest enter");
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "TestNode");
    xmlNewProp(node, BAD_CAST "num", BAD_CAST "2");
    xmlNodeSetContent(node, BAD_CAST "1A2B"); // Valid hex content
    std::vector<unsigned char> result = m_xmlParser->GetByteArrValue(node);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 0x1A);
    EXPECT_EQ(result[1], 0x2B);
    xmlFreeNode(node);
}

HWTEST_F(XmlParserTest, GetStringMapValueTest, TestSize.Level1)
{
    WIFI_LOGI("GetStringMapValueTest enter");
    EXPECT_EQ(m_xmlParser->GetStringMapValue(nullptr).size(), 0);
}

HWTEST_F(XmlParserTest, IsDocValidTest, TestSize.Level1)
{
    WIFI_LOGI("IsDocValidTest enter");
    EXPECT_FALSE(m_xmlParser->IsDocValid(nullptr));
}

HWTEST_F(XmlParserTest, ParseInternalTest, TestSize.Level1)
{
    WIFI_LOGI("ParseInternalTest enter");
    EXPECT_FALSE(m_xmlParser->ParseInternal(nullptr));
}
}
}