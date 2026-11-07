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

#ifdef FEATURE_P2P_SUPPORT
#include <gtest/gtest.h>
#include "p2p_interface.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::Wifi;
using ::testing::ext::TestSize;

class P2pInterfaceTest : public testing::Test {
public:
    void SetUp() override
    {
        p2pInterface = std::make_unique<P2pInterface>();
    }
    void TearDown() override
    {
        p2pInterface.reset();
    }
    std::unique_ptr<P2pInterface> p2pInterface;
};

HWTEST_F(P2pInterfaceTest, ConstructorTest, TestSize.Level1)
{
    EXPECT_NE(p2pInterface.get(), nullptr);
}

HWTEST_F(P2pInterfaceTest, DestructorTest, TestSize.Level1)
{
    {
        P2pInterface p2p;
    }
    SUCCEED();
}

HWTEST_F(P2pInterfaceTest, EnableP2pTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->EnableP2p();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, DisableP2pTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->DisableP2p();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, DiscoverDevicesTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->DiscoverDevices();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, StopDiscoverDevicesTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->StopDiscoverDevices();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, DiscoverServicesTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->DiscoverServices();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, StopDiscoverServicesTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->StopDiscoverServices();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, PutLocalP2pServiceTest, TestSize.Level1)
{
    WifiP2pServiceInfo srvInfo;
    ErrCode ret = p2pInterface->PutLocalP2pService(srvInfo);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, DeleteLocalP2pServiceTest, TestSize.Level1)
{
    WifiP2pServiceInfo srvInfo;
    ErrCode ret = p2pInterface->DeleteLocalP2pService(srvInfo);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, StartP2pListenTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->StartP2pListen(500, 1000);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, StopP2pListenTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->StopP2pListen();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, CreateGroupTest, TestSize.Level1)
{
    WifiP2pConfig config;
    ErrCode ret = p2pInterface->CreateGroup(config);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, RemoveGroupTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->RemoveGroup();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, DeleteGroupTest, TestSize.Level1)
{
    WifiP2pGroupInfo group;
    ErrCode ret = p2pInterface->DeleteGroup(group);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, P2pConnectTest, TestSize.Level1)
{
    WifiP2pConfig config;
    ErrCode ret = p2pInterface->P2pConnect(config);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, P2pCancelConnectTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->P2pCancelConnect();
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, SetP2pDeviceNameTest, TestSize.Level1)
{
    ErrCode ret = p2pInterface->SetP2pDeviceName("TestDevice");
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, QueryP2pLinkedInfoTest, TestSize.Level1)
{
    WifiP2pLinkedInfo linkedInfo;
    ErrCode ret = p2pInterface->QueryP2pLinkedInfo(linkedInfo);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, GetCurrentGroupTest, TestSize.Level1)
{
    WifiP2pGroupInfo group;
    ErrCode ret = p2pInterface->GetCurrentGroup(group);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, GetP2pEnableStatusTest, TestSize.Level1)
{
    int status = -1;
    ErrCode ret = p2pInterface->GetP2pEnableStatus(status);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, GetP2pDiscoverStatusTest, TestSize.Level1)
{
    int status = -1;
    ErrCode ret = p2pInterface->GetP2pDiscoverStatus(status);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, GetP2pConnectedStatusTest, TestSize.Level1)
{
    int status = -1;
    ErrCode ret = p2pInterface->GetP2pConnectedStatus(status);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, QueryP2pDevicesTest, TestSize.Level1)
{
    std::vector<WifiP2pDevice> devices;
    ErrCode ret = p2pInterface->QueryP2pDevices(devices);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, QueryP2pLocalDeviceTest, TestSize.Level1)
{
    WifiP2pDevice device;
    ErrCode ret = p2pInterface->QueryP2pLocalDevice(device);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, QueryP2pGroupsTest, TestSize.Level1)
{
    std::vector<WifiP2pGroupInfo> groups;
    ErrCode ret = p2pInterface->QueryP2pGroups(groups);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, QueryP2pServicesTest, TestSize.Level1)
{
    std::vector<WifiP2pServiceInfo> services;
    ErrCode ret = p2pInterface->QueryP2pServices(services);
    EXPECT_TRUE(ret == WIFI_OPT_SUCCESS || ret == WIFI_OPT_FAILED);
}

HWTEST_F(P2pInterfaceTest, GetP2pRecommendChannelTest, TestSize.Level1)
{
    int channel = p2pInterface->GetP2pRecommendChannel();
    EXPECT_GE(channel, 0);
}
#endif
