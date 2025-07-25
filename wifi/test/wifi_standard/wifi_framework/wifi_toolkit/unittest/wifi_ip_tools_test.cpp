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

#include "wifi_ip_tools_test.h"
#include <net/if.h>
#include "securec.h"
#include "ip_tools.h"
#include "network_interface.h"

using namespace testing::ext;

namespace OHOS {
namespace Wifi {
HWTEST_F(WifiIpToolsTest, ConvertIpv4AddressTest, TestSize.Level1)
{
    EXPECT_TRUE(IpTools::ConvertIpv4Address(0) == "");
    EXPECT_TRUE(IpTools::ConvertIpv4Address(0xc0a80001) == "192.168.0.1");
    EXPECT_TRUE(IpTools::ConvertIpv4Address("") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("192:168:0:1") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("192.168:0.1") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("289.168.0.1") == 0);
    EXPECT_FALSE(IpTools::ConvertIpv4Address("-1.168.0.1") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("192.168.0.289") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("192.168.0.-1") == 0);
    EXPECT_TRUE(IpTools::ConvertIpv4Address("192.168.0.1") == 0xc0a80001);
}

HWTEST_F(WifiIpToolsTest, ConvertIpv6AddressTest, TestSize.Level1)
{
    std::vector<unsigned char> tmp;
    EXPECT_TRUE(IpTools::ConvertIpv6Address(tmp) == "");
    // 2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b
    tmp.push_back(0x20);
    tmp.push_back(0x01);
    tmp.push_back(0x0d);
    tmp.push_back(0xb8);
    tmp.push_back(0x3c);
    tmp.push_back(0x4d);
    tmp.push_back(0x00);
    tmp.push_back(0x15);
    tmp.push_back(0x00);
    tmp.push_back(0x00);
    tmp.push_back(0x00);
    tmp.push_back(0x00);
    tmp.push_back(0x1a);
    tmp.push_back(0x2f);
    tmp.push_back(0x1a);
    EXPECT_TRUE(IpTools::ConvertIpv6Address(tmp) == "");
    tmp.push_back(0x2b);
    EXPECT_TRUE(IpTools::ConvertIpv6Address(tmp) == "2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b");

    std::vector<unsigned char> addr;
    IpTools::ConvertIpv6Address("", addr);
    EXPECT_TRUE(addr.size() == 0);
    std::string ipv6Str = "2001:0db8:3c4d:15:0000:0000:1a2f:1a2b";
    IpTools::ConvertIpv6Address(ipv6Str, addr);
    EXPECT_TRUE(addr.size() == 0);
    ipv6Str = "2001:0db8:3c4d:0000:0000:1a2f:1a2b";
    IpTools::ConvertIpv6Address(ipv6Str, addr);
    EXPECT_TRUE(addr.size() == 0);
    ipv6Str = "2001:0db8:3c4d:0015:0000:0000:1a2f:1";
    IpTools::ConvertIpv6Address(ipv6Str, addr);
    ipv6Str = "2001:0db8:3c4d:0015:0000:0000:1a2f:1a2b";
    IpTools::ConvertIpv6Address(ipv6Str, addr);
    EXPECT_TRUE(addr == tmp);
}

HWTEST_F(WifiIpToolsTest, ConvertIpv4MaskTest, TestSize.Level1)
{
    EXPECT_TRUE(IpTools::ConvertIpv4Mask(-1) == "255.255.255.0");
    EXPECT_TRUE(IpTools::ConvertIpv4Mask(33) == "255.255.255.0");
    EXPECT_TRUE(IpTools::ConvertIpv4Mask(0) == "255.255.255.0");
    EXPECT_TRUE(IpTools::ConvertIpv4Mask(32) == "255.255.255.255");
    EXPECT_TRUE(IpTools::ConvertIpv4Mask(11) == "255.224.0.0");
}

HWTEST_F(WifiIpToolsTest, ConvertIpv6MaskTest, TestSize.Level1)
{
    EXPECT_TRUE(IpTools::ConvertIpv6Mask(-1) == "");
    EXPECT_TRUE(IpTools::ConvertIpv6Mask(129) == "");
    EXPECT_TRUE(IpTools::ConvertIpv6Mask(0) == "::");
    EXPECT_TRUE(IpTools::ConvertIpv6Mask(128) == "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
}

HWTEST_F(WifiIpToolsTest, GetMaskLengthTest, TestSize.Level1)
{
    EXPECT_EQ(IpTools::GetMaskLength("255.0.0.0"), 8);
}

HWTEST_F(WifiIpToolsTest, GetIPV6MaskLengthTest, TestSize.Level1)
{
    EXPECT_EQ(IpTools::GetIPV6MaskLength(""), 0);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("ff00:0000:0000:0000:0000:0000:0000:0000"), 8);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("fe00:0000:0000:0000:0000:0000:0000:0000"), 7);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("fc00:0000:0000:0000:0000:0000:0000:0000"), 6);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("f800:0000:0000:0000:0000:0000:0000:0000"), 5);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("f000:0000:0000:0000:0000:0000:0000:0000"), 4);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("e000:0000:0000:0000:0000:0000:0000:0000"), 3);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("c000:0000:0000:0000:0000:0000:0000:0000"), 2);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("8000:0000:0000:0000:0000:0000:0000:0000"), 1);
    EXPECT_EQ(IpTools::GetIPV6MaskLength("0000:0000:0000:0000:0000:0000:0000:0000"), 0);
}

HWTEST_F(WifiIpToolsTest, GetExclusionObjectListTest, TestSize.Level1)
{
    std::string str = "a,b,c";
    std::vector<std::string> vec;
    IpTools::GetExclusionObjectList(str, vec);
    EXPECT_TRUE(vec.size() == 3);
    str = "abc";
    vec.clear();
    IpTools::GetExclusionObjectList(str, vec);
    EXPECT_TRUE(vec.size() == 1);
}

HWTEST_F(WifiIpToolsTest, ConvertIpv6AddressToCompletedTest, TestSize.Level1)
{
    // 2001:0db8:1234:5678:0000:0000:0000:0001
    std::string tmp = "2001:0db8:1234:5678:0000:0000:0000:0001";
    std::string ipv6Str = "2001:0db8:1234:5678::0001";
    EXPECT_TRUE(IpTools::ConvertIpv6AddressToCompleted(ipv6Str) == tmp);
    ipv6Str = "2001:0db8:1234:5678::1";
    EXPECT_TRUE(IpTools::ConvertIpv6AddressToCompleted(ipv6Str) == tmp);
    ipv6Str = "2001:db8:1234:5678::1";
    EXPECT_TRUE(IpTools::ConvertIpv6AddressToCompleted(ipv6Str) == tmp);
    ipv6Str = "123";
    EXPECT_FALSE(IpTools::ConvertIpv6AddressToCompleted(ipv6Str) == "1230:0000:0000:0000:0000:0000:0000:0000");
}
}  // namespace Wifi
}  // namespace OHOS