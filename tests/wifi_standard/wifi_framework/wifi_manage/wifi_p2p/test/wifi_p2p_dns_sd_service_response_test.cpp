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
#include <gtest/gtest.h>

#include "wifi_p2p_dns_sd_service_response.h"
using ::testing::ext::TestSize;

namespace OHOS {
namespace Wifi {
class WifiP2pDnsSdServiceResponseTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    virtual void SetUp()
    {
        pWifiP2pDnsSdServiceResponse.reset(new WifiP2pDnsSdServiceResponse(resp));
    }
    virtual void TearDown()
    {
        pWifiP2pDnsSdServiceResponse.reset();
    }

public:
    WifiP2pServiceResponse resp;
    std::unique_ptr<WifiP2pDnsSdServiceResponse> pWifiP2pDnsSdServiceResponse;
    void WarpReadTxtData()
    {
        std::istringstream istream("name");
        pWifiP2pDnsSdServiceResponse->FetchTxtData(istream);
    }
};

HWTEST_F(WifiP2pDnsSdServiceResponseTest, GetQueryName, TestSize.Level1)
{
    EXPECT_EQ("", pWifiP2pDnsSdServiceResponse->GetQueryName());
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, GetInstanceName, TestSize.Level1)
{
    EXPECT_EQ("", pWifiP2pDnsSdServiceResponse->GetInstanceName());
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, GetDnsType, TestSize.Level1)
{
    EXPECT_EQ(-1, pWifiP2pDnsSdServiceResponse->GetDnsType());
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, GetVersion, TestSize.Level1)
{
    EXPECT_EQ(-1, pWifiP2pDnsSdServiceResponse->GetVersion());
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, GetTxtRecord, TestSize.Level1)
{
    pWifiP2pDnsSdServiceResponse->GetTxtRecord();
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, ParseData, TestSize.Level1)
{
    EXPECT_EQ(true, pWifiP2pDnsSdServiceResponse->ParseData());
    std::vector<unsigned char> data;
    data.push_back(1);
    pWifiP2pDnsSdServiceResponse->SetData(data);
    EXPECT_EQ(false, pWifiP2pDnsSdServiceResponse->ParseData());

    data.clear();
    data.push_back(0x00);
    pWifiP2pDnsSdServiceResponse->SetData(data);
    EXPECT_EQ(false, pWifiP2pDnsSdServiceResponse->ParseData());

    data.clear();
    data.push_back(0xc0);
    pWifiP2pDnsSdServiceResponse->SetData(data);
    EXPECT_EQ(false, pWifiP2pDnsSdServiceResponse->ParseData());
}

HWTEST_F(WifiP2pDnsSdServiceResponseTest, FetchTxtData, TestSize.Level1)
{
    WarpReadTxtData();
}
}  // namespace Wifi
}  // namespace OHOS