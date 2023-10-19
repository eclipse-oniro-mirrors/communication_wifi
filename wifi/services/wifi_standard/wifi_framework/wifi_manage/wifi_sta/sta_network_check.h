/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_NET_CHECK_H
#define OHOS_WIFI_NET_CHECK_H

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <fstream>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <vector>
#include "http_request.h"
#include "sta_define.h"
#include "wifi_errcode.h"
#include "wifi_log.h"
#include "arp_checker.h"
#include "dns_checker.h"
#include "wifi_internal_msg.h"
#ifndef OHOS_ARCH_LITE
#include "http_client_request.h"
#include "http_client.h"
#endif

#define HTTP_DETECTION_TIMEOUT 3000
namespace OHOS {
namespace Wifi {
class StaNetworkCheck {
    FRIEND_GTEST(StaNetworkCheck);
public:
    StaNetworkCheck(NetStateHandler nethandle, ArpStateHandler arpHandle, DnsStateHandler dnsHandle, int instId = 0);
    virtual ~StaNetworkCheck();
    /**
     * @Description : Start NetCheck thread
     *
     * @Return success : WIFI_OPT_SUCCESS  failed : WIFI_OPT_FAILED
     */
    virtual ErrCode InitNetCheckThread();
    /**
     * @Description : wake up the DHCP processing thread.
     *
     * @param ipType - Type of IP to be obtained [in]
     */
    virtual void SignalNetCheckThread();
    /**
     * @Description : stop the NetCheck processing thread.
     *
     */
    virtual void StopNetCheckThread();

    /**
     * @Description : Exit the NetCheck thread.
     *
     */
    virtual void ExitNetCheckThread();

private:
    std::thread *pDealNetCheckThread;
    NetStateHandler netStateHandler;
    ArpStateHandler arpStateHandler;
    DnsStateHandler dnsStateHandler;
    std::string httpUrl;
    std::string httpsUrl;
    int httpCodeNum;
    int httpsCodeNum;
    int httpResultLen;
    int httpsResultLen;
    std::atomic<StaNetState> lastNetState;
#ifndef OHOS_ARCH_LITE
    /**
     * @Description : Detect Internet ability
     *
     */
    int HttpPortalDetection(const std::string& url);

    void RegistHttpCallBack(std::shared_ptr<NetStack::HttpClient::HttpClientTask> task);
#endif
    /**
     * @Description : NetCheck thread function
     *
     */
    void RunNetCheckThreadFunc();
    /**
     * @Description : check networktype by code function
     *
     */
    void CheckResponseCode(std::string url, int codeNum, int codeLenNum);

    void SetHttpResultInfo(std::string url, int codeNum, int codeLenNum);

    void DnsDetection(std::string url);
private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    std::condition_variable mCondition_timeout;
    std::atomic<bool> isStopNetCheck;
    std::atomic<bool> isExitNetCheckThread;
    std::atomic<bool> isExited;
    std::atomic<int> detectResultNum;
    ArpChecker arpChecker;
    DnsChecker dnsChecker;
    std::chrono::steady_clock::time_point lastArpDnsCheckTime;
    WifiPortalConf mUrlInfo;
    int m_instId;
};
}  // namespace Wifi
}  // namespace OHOS
#endif