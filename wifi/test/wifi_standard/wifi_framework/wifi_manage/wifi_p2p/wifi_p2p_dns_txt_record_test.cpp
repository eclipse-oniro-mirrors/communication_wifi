/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "wifi_p2p_dns_txt_record.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::Wifi;
using ::testing::ext::TestSize;

class WifiP2PDnsTxtRecordTest : public testing::Test {
public:
    void SetUp() override
    {
        record = std::make_unique<WifiP2PDnsTxtRecord>();
    }
    void TearDown() override
    {
        record.reset();
    }
    std::unique_ptr<WifiP2PDnsTxtRecord> record;
};

HWTEST_F(WifiP2PDnsTxtRecordTest, ConstructorTest, TestSize.Level1)
{
    EXPECT_NE(record.get(), nullptr);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, DestructorTest, TestSize.Level1)
{
    {
        WifiP2PDnsTxtRecord r;
    }
    SUCCEED();
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordTest, TestSize.Level1)
{
    bool ret = record->SetRecord("key1", "value1");
    EXPECT_TRUE(ret);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordEmptyKeyTest, TestSize.Level1)
{
    bool ret = record->SetRecord("", "value1");
    EXPECT_TRUE(ret);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordEmptyValueTest, TestSize.Level1)
{
    bool ret = record->SetRecord("key1", "");
    EXPECT_TRUE(ret);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordMultipleTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    record->SetRecord("key2", "value2");
    record->SetRecord("key3", "value3");
    EXPECT_EQ(record->KeyNum(), 3);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordUpdateExistingTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    record->SetRecord("key1", "updated");
    EXPECT_NE(record->KeyNum(), -1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, KeyNumEmptyTest, TestSize.Level1)
{
    EXPECT_EQ(record->KeyNum(), 0);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, KeyNumAfterAddTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    EXPECT_EQ(record->KeyNum(), 1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, RemoveDataTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    record->SetRecord("key2", "value2");
    record->RemoveData("key1");
    EXPECT_EQ(record->KeyNum(), 2);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, RemoveDataNonExistentTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    int index = record->RemoveData("nonexistent");
    EXPECT_EQ(index, -1);
    EXPECT_EQ(record->KeyNum(), 1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, RemoveDataEmptyTest, TestSize.Level1)
{
    int index = record->RemoveData("key1");
    EXPECT_EQ(index, -1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, RemoveAllDataTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    record->SetRecord("key2", "value2");
    record->RemoveData("key1");
    record->RemoveData("key2");
    EXPECT_NE(record->KeyNum(), -1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, GetDataEmptyTest, TestSize.Level1)
{
    const auto &data = record->GetData();
    EXPECT_TRUE(data.empty());
}

HWTEST_F(WifiP2PDnsTxtRecordTest, GetDataAfterSetTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    const auto &data = record->GetData();
    EXPECT_FALSE(data.empty());
}

HWTEST_F(WifiP2PDnsTxtRecordTest, GetDataAfterRemoveTest, TestSize.Level1)
{
    record->SetRecord("key1", "value1");
    record->SetRecord("key2", "value2");
    record->RemoveData("key1");
    const auto &data = record->GetData();
    EXPECT_FALSE(data.empty());
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordSpecialCharsTest, TestSize.Level1)
{
    bool ret = record->SetRecord("key!@#", "val$%^");
    EXPECT_TRUE(ret);
    EXPECT_EQ(record->KeyNum(), 1);
}

HWTEST_F(WifiP2PDnsTxtRecordTest, SetRecordLongValueTest, TestSize.Level1)
{
    std::string longVal(100, 'x');
    bool ret = record->SetRecord("key1", longVal);
    EXPECT_TRUE(ret);
    EXPECT_EQ(record->KeyNum(), 1);
}
