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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <cstring>

#include "p2p_monitor.h"
#include "mock_wifi_p2p_hal_interface.h"
#include "wifi_logger.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ext::TestSize;
const char *g_pIface = "test0";

namespace OHOS {
namespace Wifi {
class P2pMonitorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    virtual void SetUp()
    {
        const int testDevCap = 10;
        const int testGroupCap = 15;
        const int testCfgMethod = 6;
        pP2pMonitor.reset(new P2pMonitor);
        testDevice.SetDeviceName(std::string("UnitTestDeviceName"));
        testDevice.SetDeviceAddress(std::string("ff:ff:ff:ff:ff:ff"));
        testDevice.SetPrimaryDeviceType(std::string("1-111111-1"));
        testDevice.SetDeviceCapabilitys(testDevCap);
        testDevice.SetGroupCapabilitys(testGroupCap);
        testDevice.SetWpsConfigMethod(testCfgMethod);
        testDevice.SetP2pDeviceStatus(P2pDeviceStatus::PDS_AVAILABLE);
    }
    virtual void TearDown()
    {
        EXPECT_CALL(WifiP2PHalInterface::GetInstance(), RegisterP2pCallback(_)).WillRepeatedly(Return(WIFI_IDL_OPT_OK));
        pP2pMonitor.reset();
    }

public:
    void WrapMethodSendMessage(
        const std::string &iface, P2P_STATE_MACHINE_CMD msgName, int param1, int param2, const std::any &anyObj) const
    {
        pP2pMonitor->MessageToStateMachine(iface, msgName, param1, param2, anyObj);
    }

    P2pStatus WrapMethodIntStatusToP2pStatus(int status) const
    {
        return pP2pMonitor->IntStatusToP2pStatus(status);
    }

    void WrapMethodBroadcast2SmConnectSupplicant(const std::string &iface, int status) const
    {
        pP2pMonitor->Broadcast2SmConnectSupplicant(iface, status);
    }

    void WrapMethodBroadcast2SmDeviceFound(const std::string &iface, const WifiP2pDevice &device) const
    {
        pP2pMonitor->Broadcast2SmDeviceFound(iface, device);
    }

    void WrapMethodBroadcast2SmDeviceLost(const std::string &iface, const WifiP2pDevice &device) const
    {
        pP2pMonitor->Broadcast2SmDeviceLost(iface, device);
    }

    void WrapMethodBroadcast2SmGoNegRequest(const std::string &iface, const WifiP2pConfig &config) const
    {
        pP2pMonitor->Broadcast2SmGoNegRequest(iface, config);
    }

    void WrapMethodBroadcast2SmGoNegSuccess(const std::string &iface) const
    {
        pP2pMonitor->Broadcast2SmGoNegSuccess(iface);
    }

    void WrapMethodBroadcast2SmGoNegFailure(const std::string &iface, P2pStatus p2pStatus) const
    {
        pP2pMonitor->Broadcast2SmGoNegFailure(iface, p2pStatus);
    }

    void WrapMethodBroadcast2SmInvitationReceived(const std::string &iface, const WifiP2pGroupInfo &group) const
    {
        pP2pMonitor->Broadcast2SmInvitationReceived(iface, group);
    }

    void WrapMethodBroadcast2SmInvitationResult(const std::string &iface, P2pStatus p2pStatus) const
    {
        pP2pMonitor->Broadcast2SmInvitationResult(iface, p2pStatus);
    }

    void WrapMethodBroadcast2SmGroupFormationSuccess(const std::string &iface) const
    {
        pP2pMonitor->Broadcast2SmGroupFormationSuccess(iface);
    }

    void WrapMethodBroadcast2SmGroupFormationFailure(const std::string &iface, const std::string &reason) const
    {
        pP2pMonitor->Broadcast2SmGroupFormationFailure(iface, reason);
    }

    void WrapMethodBroadcast2SmGroupStarted(const std::string &iface, const WifiP2pGroupInfo &group) const
    {
        pP2pMonitor->Broadcast2SmGroupStarted(iface, group);
    }

    void WrapMethodBroadcast2SmGroupRemoved(const std::string &iface, const WifiP2pGroupInfo &group) const
    {
        pP2pMonitor->Broadcast2SmGroupRemoved(iface, group);
    }

    void WrapMethodBroadcast2SmProvDiscPbcReq(const std::string &iface, const WifiP2pTempDiscEvent &event) const
    {
        pP2pMonitor->Broadcast2SmProvDiscPbcReq(iface, event);
    }

    void WrapMethodBroadcast2SmProvDiscPbcResp(const std::string &iface, const WifiP2pTempDiscEvent &event) const
    {
        pP2pMonitor->Broadcast2SmProvDiscPbcResp(iface, event);
    }

    void WrapMethodBroadcast2SmProvDiscEnterPin(const std::string &iface, const WifiP2pTempDiscEvent &event) const
    {
        pP2pMonitor->Broadcast2SmProvDiscEnterPin(iface, event);
    }

    void WrapMethodBroadcast2SmProvDiscShowPin(const std::string &iface, const WifiP2pTempDiscEvent &event) const
    {
        pP2pMonitor->Broadcast2SmProvDiscShowPin(iface, event);
    }

    void WrapMethodBroadcast2SmProvDiscFailure(const std::string &iface) const
    {
        pP2pMonitor->Broadcast2SmProvDiscFailure(iface);
    }

    void WrapMethodBroadcast2SmFindStopped(const std::string &iface) const
    {
        pP2pMonitor->Broadcast2SmFindStopped(iface);
    }
    void WrapMethodBroadcast2SmServDiscReq(const std::string &iface, const WifiP2pServiceRequestList &reqList) const
    {
        pP2pMonitor->Broadcast2SmServDiscReq(iface, reqList);
    }
    void WrapMethodBroadcast2SmServDiscResp(const std::string &iface, const WifiP2pServiceResponseList &respList) const
    {
        pP2pMonitor->Broadcast2SmServDiscResp(iface, respList);
    }

    void WrapMethodBroadcast2SmApStaDisconnected(const std::string &iface, const WifiP2pDevice &device) const
    {
        pP2pMonitor->Broadcast2SmApStaDisconnected(iface, device);
    }

    void WrapMethodBroadcast2SmApStaConnected(const std::string &iface, const WifiP2pDevice &device) const
    {
        pP2pMonitor->Broadcast2SmApStaConnected(iface, device);
    }

    void WrapMethodBroadcast2SmConnectSupplicantFailed(const std::string &iface) const
    {
        pP2pMonitor->Broadcast2SmConnectSupplicantFailed(iface);
    }

    void WrapMethodOnConnectSupplicant(int status)
    {
        pP2pMonitor->OnConnectSupplicant(status);
    }

    void WrapMethodOnDeviceFound(IdlP2pDeviceFound deviceInfo)
    {
        pP2pMonitor->WpaEventDeviceFound(deviceInfo);
    }

    void WrapMethodOnDeviceLost(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventDeviceLost(p2pDeviceAddress);
    }

    void WrapMethodOnGoNegRequest(const std::string &srcAddress, short passwordId)
    {
        pP2pMonitor->WpaEventGoNegRequest(srcAddress, passwordId);
    }

    void WrapMethodOnGoNegSuccess()
    {
        pP2pMonitor->WpaEventGoNegSuccess();
    }

    void WrapMethodOnGoNegFailure(int status)
    {
        pP2pMonitor->WpaEventGoNegFailure(status);
    }

    void WrapMethodOnInvitationReceived(IdlP2pInvitationInfo recvInfo)
    {
        pP2pMonitor->WpaEventInvitationReceived(recvInfo);
    }

    void WrapMethodOnInvitationResult(const std::string &bssid, int status)
    {
        pP2pMonitor->WpaEventInvitationResult(bssid, status);
    }

    void WrapMethodOnGroupFormationSuccess()
    {
        pP2pMonitor->WpaEventGroupFormationSuccess();
    }

    void WrapMethodOnGroupFormationFailure(const std::string &failureReason)
    {
        pP2pMonitor->WpaEventGroupFormationFailure(failureReason);
    }

    void WrapMethodOnGroupStarted(IdlP2pGroupInfo groupInfo)
    {
        pP2pMonitor->WpaEventGroupStarted(groupInfo);
    }

    void WrapMethodOnGroupRemoved(const std::string &groupIfName, bool isGo)
    {
        pP2pMonitor->WpaEventGroupRemoved(groupIfName, isGo);
    }

    void WrapMethodOnProvDiscPbcReq(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventProvDiscPbcReq(p2pDeviceAddress);
    }

    void WrapMethodOnProvDiscPbcResp(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventProvDiscPbcResp(p2pDeviceAddress);
    }

    void WrapMethodOnProvDiscEnterPin(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventProvDiscEnterPin(p2pDeviceAddress);
    }

    void WrapMethodOnProvDiscShowPin(const std::string &p2pDeviceAddress, const std::string &generatedPin)
    {
        pP2pMonitor->WpaEventProvDiscShowPin(p2pDeviceAddress, generatedPin);
    }

    void WrapMethodOnProvDiscFailure()
    {
        pP2pMonitor->WpaEventProvDiscFailure();
    }

    void WrapMethodOnFindStopped()
    {
        pP2pMonitor->WpaEventFindStopped();
    }
    void WrapMethodOnServDiscReq(IdlP2pServDiscReqInfo reqInfo)
    {
        pP2pMonitor->WpaEventServDiscReq(reqInfo);
    }

    void WrapMethodOnServDiscResp(
        const std::string &srcAddress, short updateIndicator, const std::vector<unsigned char> &tlvs)
    {
        pP2pMonitor->WpaEventServDiscResp(srcAddress, updateIndicator, tlvs);
    }

    void WrapMethodOnApStaDisconnected(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventApStaDisconnected(p2pDeviceAddress);
    }

    void WrapMethodOnApStaConnected(const std::string &p2pDeviceAddress)
    {
        pP2pMonitor->WpaEventApStaConnected(p2pDeviceAddress);
    }

    void WrapMethodOnConnectSupplicantFailed()
    {
        pP2pMonitor->OnConnectSupplicantFailed();
    }

    const std::set<std::string> &WrapDataSetMonitorIface()
    {
        return pP2pMonitor->setMonitorIface;
    }

public:
    std::unique_ptr<P2pMonitor> pP2pMonitor;
    WifiP2pDevice testDevice;
};

HWTEST_F(P2pMonitorTest, Initialize_SUCCESS, TestSize.Level1)
{
    pP2pMonitor->Initialize();
}
HWTEST_F(P2pMonitorTest, RegisterHandler_SUCCESS, TestSize.Level1)
{
    const std::string iface = g_pIface;
    const std::function<HandlerMethod> mapHandler;
    pP2pMonitor->RegisterIfaceHandler(iface, mapHandler);
}
HWTEST_F(P2pMonitorTest, UnregisterHandler_SUCCESS, TestSize.Level1)
{
    const std::string iface = g_pIface;
    pP2pMonitor->UnregisterHandler(iface);
}

HWTEST_F(P2pMonitorTest, WrapMethod_test, TestSize.Level1)
{
    const std::string iface = g_pIface;
    constexpr P2P_STATE_MACHINE_CMD msgName = P2P_STATE_MACHINE_CMD::WPA_CONNECTED_EVENT;
    constexpr int param1 = 0;
    constexpr int param2 = 1;
    const std::any anyObj = std::string("test_any");
    WrapMethodSendMessage(iface, msgName, param1, param2, anyObj);
}

HWTEST_F(P2pMonitorTest, WrapMethod_data, TestSize.Level1)
{
    const std::string iface = g_pIface;
    const std::set<std::string> &setMonitorIface = WrapDataSetMonitorIface();
    EXPECT_CALL(WifiP2PHalInterface::GetInstance(), RegisterP2pCallback(_));
    pP2pMonitor->MonitorEnds(iface);
    EXPECT_TRUE(setMonitorIface.count(iface) == 0);

    EXPECT_CALL(WifiP2PHalInterface::GetInstance(), RegisterP2pCallback(_));
    pP2pMonitor->MonitorBegins(iface);
    EXPECT_TRUE(setMonitorIface.count(iface) == 1);
}

HWTEST_F(P2pMonitorTest, IntStatusToP2pStatus, TestSize.Level1)
{
    EXPECT_TRUE(WrapMethodIntStatusToP2pStatus(0) == P2pStatus::SUCCESS);

    EXPECT_TRUE(WrapMethodIntStatusToP2pStatus(5) == P2pStatus::UNABLE_TO_ACCOMMODATE_REQUEST);

    EXPECT_TRUE(WrapMethodIntStatusToP2pStatus(11) == P2pStatus::REJECTED_BY_USER);

    EXPECT_TRUE(WrapMethodIntStatusToP2pStatus(-1) == P2pStatus::UNKNOWN);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmConnectSupplicant, TestSize.Level1)
{
    const std::string iface = g_pIface;
    int status = 1;
    WrapMethodBroadcast2SmConnectSupplicant(iface, status);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmDeviceFound, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmDeviceFound(iface, testDevice);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmDeviceLost, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmDeviceLost(iface, testDevice);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGoNegRequest, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pConfig testConfig;
    WrapMethodBroadcast2SmGoNegRequest(iface, testConfig);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGoNegSuccess, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmGoNegSuccess(iface);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGoNegFailure, TestSize.Level1)
{
    const std::string iface = g_pIface;
    P2pStatus testStatus = P2pStatus::SUCCESS;
    WrapMethodBroadcast2SmGoNegFailure(iface, testStatus);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmInvitationReceived, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pGroupInfo testGroup;
    WrapMethodBroadcast2SmInvitationReceived(iface, testGroup);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmInvitationResult, TestSize.Level1)
{
    const std::string iface = g_pIface;
    P2pStatus testStatus = P2pStatus::SUCCESS;
    WrapMethodBroadcast2SmInvitationResult(iface, testStatus);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGroupFormationSuccess, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmGroupFormationSuccess(iface);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGroupFormationFailure, TestSize.Level1)
{
    const std::string iface = g_pIface;
    const std::string reason("P2pUnitTest");
    WrapMethodBroadcast2SmGroupFormationFailure(iface, reason);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGroupStarted, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pGroupInfo testGroup;
    WrapMethodBroadcast2SmGroupStarted(iface, testGroup);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmGroupRemoved, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pGroupInfo testGroup;
    WrapMethodBroadcast2SmGroupRemoved(iface, testGroup);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmProvDiscPbcReq, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pTempDiscEvent testEvent;
    WrapMethodBroadcast2SmProvDiscPbcReq(iface, testEvent);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmProvDiscPbcResp, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pTempDiscEvent testEvent;
    WrapMethodBroadcast2SmProvDiscPbcResp(iface, testEvent);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmProvDiscEnterPin, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pTempDiscEvent testEvent;
    WrapMethodBroadcast2SmProvDiscEnterPin(iface, testEvent);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmProvDiscShowPin, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pTempDiscEvent testEvent;
    WrapMethodBroadcast2SmProvDiscShowPin(iface, testEvent);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmProvDiscFailure, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmProvDiscFailure(iface);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmFindStopped, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmFindStopped(iface);
}
HWTEST_F(P2pMonitorTest, Broadcast2SmServDiscReq, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pServiceRequestList testRequeList;
    WrapMethodBroadcast2SmServDiscReq(iface, testRequeList);
}
HWTEST_F(P2pMonitorTest, Broadcast2SmServDiscResp, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WifiP2pServiceResponseList testRespList;
    WrapMethodBroadcast2SmServDiscResp(iface, testRespList);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmApStaDisconnected, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmApStaDisconnected(iface, testDevice);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmApStaConnected, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmApStaConnected(iface, testDevice);
}

HWTEST_F(P2pMonitorTest, Broadcast2SmConnectSupplicantFailed, TestSize.Level1)
{
    const std::string iface = g_pIface;
    WrapMethodBroadcast2SmConnectSupplicantFailed(iface);
}

HWTEST_F(P2pMonitorTest, OnConnectSupplicant, TestSize.Level1)
{
    int status = 1;
    WrapMethodOnConnectSupplicant(status);
}

HWTEST_F(P2pMonitorTest, WpaEventDeviceFound, TestSize.Level1)
{
    IdlP2pDeviceFound deviceInfo;
    deviceInfo.srcAddress = "ff:ff:ff:ff:ff:ff";
    deviceInfo.p2pDeviceAddress = "ff:ff:ff:ff:ff:ff";
    deviceInfo.primaryDeviceType = "1-111111-1";
    deviceInfo.deviceName = "P2pUnitTest";
    deviceInfo.configMethods = 10;
    deviceInfo.deviceCapabilities = 8;
    deviceInfo.groupCapabilities = 12;
    deviceInfo.wfdDeviceInfo.push_back('1');
    deviceInfo.wfdDeviceInfo.push_back('5');
    deviceInfo.wfdDeviceInfo.push_back('a');
    deviceInfo.wfdDeviceInfo.push_back('3');
    deviceInfo.wfdDeviceInfo.push_back('6');
    deviceInfo.wfdDeviceInfo.push_back('e');
    deviceInfo.wfdDeviceInfo.push_back('8');
    deviceInfo.wfdDeviceInfo.push_back('8');
    WrapMethodOnDeviceFound(deviceInfo);
}

HWTEST_F(P2pMonitorTest, WpaEventDeviceLost, TestSize.Level1)
{
    WrapMethodOnDeviceLost("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, WpaEventGoNegRequest, TestSize.Level1)
{
    short testPasswordId = 8;
    WrapMethodOnGoNegRequest("ff:ff:ff:ff:ff:ff", testPasswordId);
}

HWTEST_F(P2pMonitorTest, WpaEventGoNegSuccess, TestSize.Level1)
{
    WrapMethodOnGoNegSuccess();
}

HWTEST_F(P2pMonitorTest, WpaEventGoNegFailure, TestSize.Level1)
{
    int testStatus = 1;
    WrapMethodOnGoNegFailure(testStatus);
}

HWTEST_F(P2pMonitorTest, WpaEventInvitationReceived, TestSize.Level1)
{
    IdlP2pInvitationInfo testRecvInfo;
    testRecvInfo.persistentNetworkId = 1;
    testRecvInfo.operatingFrequency = 6;
    testRecvInfo.srcAddress = "ff:ff:ff:ff:ff:ff";
    testRecvInfo.goDeviceAddress = "ff:ff:ff:ff:ff:fe";
    testRecvInfo.bssid = "ff:ff:ff:ff:ff:ef";
    WrapMethodOnInvitationReceived(testRecvInfo);
}

HWTEST_F(P2pMonitorTest, WpaEventInvitationResult, TestSize.Level1)
{
    int testStatus = 1;
    WrapMethodOnInvitationResult("ff:ff:ff:ff:ff:ff", testStatus);
}

HWTEST_F(P2pMonitorTest, WpaEventGroupFormationSuccess, TestSize.Level1)
{
    WrapMethodOnGroupFormationSuccess();
}

HWTEST_F(P2pMonitorTest, WpaEventGroupFormationFailure, TestSize.Level1)
{
    WrapMethodOnGroupFormationFailure("P2pUnitTestReason");
}

HWTEST_F(P2pMonitorTest, WpaEventGroupStarted, TestSize.Level1)
{
    IdlP2pGroupInfo testGroupInfo;
    testGroupInfo.isGo = true;
    testGroupInfo.isPersistent = true;
    testGroupInfo.frequency = 6;
    testGroupInfo.groupName =  "P2pUnitTestWlan";
    testGroupInfo.ssid = "P2pUnitTestGroup";
    testGroupInfo.psk = "123456789";
    testGroupInfo.passphrase =  "TestPassphrase";
    testGroupInfo.goDeviceAddress = "ff:ff:ff:ff:ff:ff";
    WrapMethodOnGroupStarted(testGroupInfo);
}

HWTEST_F(P2pMonitorTest, WpaEventGroupRemoved, TestSize.Level1)
{
    bool isGo = true;
    WrapMethodOnGroupRemoved("P2pUnitTestWlan", isGo);
}

HWTEST_F(P2pMonitorTest, WpaEventProvDiscPbcReq, TestSize.Level1)
{
    WrapMethodOnProvDiscPbcReq("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, WpaEventProvDiscPbcResp, TestSize.Level1)
{
    WrapMethodOnProvDiscPbcResp("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, WpaEventProvDiscEnterPin, TestSize.Level1)
{
    WrapMethodOnProvDiscEnterPin("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, WpaEventProvDiscShowPin, TestSize.Level1)
{
    WrapMethodOnProvDiscShowPin("ff:ff:ff:ff:ff:ff", "TestGeneratedPin");
}

HWTEST_F(P2pMonitorTest, WpaEventProvDiscFailure, TestSize.Level1)
{
    WrapMethodOnProvDiscFailure();
}

HWTEST_F(P2pMonitorTest, WpaEventFindStopped, TestSize.Level1)
{
    WrapMethodOnFindStopped();
}

HWTEST_F(P2pMonitorTest, WpaEventServDiscReq, TestSize.Level1)
{
    EXPECT_CALL(WifiP2PHalInterface::GetInstance(), RegisterP2pCallback(_));
    const std::string iface = g_pIface;
    pP2pMonitor->MonitorBegins(iface);
    IdlP2pServDiscReqInfo info;
    std::vector<unsigned char> tList;
    info.tlvList = tList;
    info.mac = "111";
    WrapMethodOnServDiscReq(info);
}

HWTEST_F(P2pMonitorTest, WpaEventServDiscResp, TestSize.Level1)
{
    EXPECT_CALL(WifiP2PHalInterface::GetInstance(), RegisterP2pCallback(_));
    const std::string iface = g_pIface;
    pP2pMonitor->MonitorBegins(iface);
    short testUpdateIndicator = 5;
    const std::vector<unsigned char> testTlvs;
    WrapMethodOnServDiscResp("ff:ff:ff:ff:ff:ff", testUpdateIndicator, testTlvs);
}

HWTEST_F(P2pMonitorTest, WpaEventApStaDisconnected, TestSize.Level1)
{
    WrapMethodOnApStaDisconnected("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, WpaEventApStaConnected, TestSize.Level1)
{
    WrapMethodOnApStaConnected("ff:ff:ff:ff:ff:ff");
}

HWTEST_F(P2pMonitorTest, OnConnectSupplicantFailed, TestSize.Level1)
{
    WrapMethodOnConnectSupplicantFailed();
}
}  // namespace Wifi
}  // namespace OHOS