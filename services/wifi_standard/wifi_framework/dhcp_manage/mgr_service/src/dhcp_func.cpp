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
#include "dhcp_func.h"

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "securec.h"
#include "wifi_logger.h"
#include "dhcp_event_subscriber.h"

DEFINE_WIFILOG_DHCP_LABEL("DhcpFunc");

namespace OHOS {
namespace Wifi {
std::string DhcpFunc::IpToDot(unsigned int nIp)
{
    in_addr addr;
    addr.s_addr = htonl(nIp);
    std::string strip = inet_ntoa(addr);
    return strip;
}

unsigned int DhcpFunc::IPtoInt(const std::string& strIp)
{
    in_addr addr;
    unsigned int intIp = 0;
    if (inet_aton(strIp.c_str(), &addr) == 1) {
        intIp = ntohl(addr.s_addr);
    }
    return intIp;
}

bool DhcpFunc::Ip4StrConToInt(const std::string& strIp, uint32_t& uIp)
{
    if (strIp.empty()) {
        WIFI_LOGE("Ip4StrConToInt error, strIp is empty()!");
        return false;
    }

    struct in_addr addr4;
    int nRet = inet_pton(AF_INET, strIp.c_str(), &addr4);
    if (nRet != 1) {
        WIFI_LOGE("Ip4StrConToInt strIp:%{private}s failed, nRet:%{public}d!", strIp.c_str(), nRet);
        if (nRet == 0) {
            WIFI_LOGE("Ip4StrConToInt strIp:%{private}s not in presentation format!", strIp.c_str());
        } else {
            WIFI_LOGE("Ip4StrConToInt strIp:%{private}s inet_pton not contain a valid address!", strIp.c_str());
        }
        return false;
    }

    uIp = ntohl(addr4.s_addr);
    WIFI_LOGI("Ip4StrConToInt strIp:%{private}s -> uIp:%{private}u.", strIp.c_str(), uIp);

    return true;
}

std::string DhcpFunc::Ip4IntConToStr(uint32_t uIp)
{
    if (uIp == 0) {
        WIFI_LOGE("Ip4IntConToStr uIp is 0!");
        return "";
    }

    std::string strIp = "";
    char bufIp4[INET_ADDRSTRLEN] = {0};
    struct in_addr addr4;
    addr4.s_addr = htonl(uIp);
    if (inet_ntop(AF_INET, &addr4, bufIp4, INET_ADDRSTRLEN) == NULL) {
        WIFI_LOGE("Ip4IntConToStr uIp:%{private}u failed, inet_ntop NULL!", uIp);
    } else {
        strIp = bufIp4;
        WIFI_LOGI("Ip4IntConToStr uIp:%{private}u -> strIp:%{private}s.", uIp, strIp.c_str());
    }

    return strIp;
}

bool DhcpFunc::Ip6StrConToChar(const std::string& strIp, uint8_t chIp[], size_t uSize)
{
    if (strIp.empty()) {
        WIFI_LOGE("Ip6StrConToChar param error, strIp is empty()!");
        return false;
    }

    struct in6_addr addr6;
    if (memset_s(&addr6, sizeof(addr6), 0, sizeof(addr6)) != EOK) {
        return false;
    }
    int nRet = inet_pton(AF_INET6, strIp.c_str(), &addr6);
    if (nRet != 1) {
        WIFI_LOGE("Ip6StrConToChar inet_pton strIp:%{private}s failed, nRet:%{public}d!", strIp.c_str(), nRet);
        if (nRet == 0) {
            WIFI_LOGE("Ip6StrConToChar strIp:%{private}s not in presentation format!", strIp.c_str());
        } else {
            WIFI_LOGE("Ip6StrConToChar strIp:%{private}s inet_pton not contain a valid address!", strIp.c_str());
        }
        return false;
    }

    for (size_t i = 0; i < uSize; i++) {
        chIp[i] = addr6.s6_addr[i];
    }

    return true;
}

std::string DhcpFunc::Ip6CharConToStr(uint8_t chIp[], int size)
{
    if (size <= 0) {
        WIFI_LOGE("Ip6CharConToStr param error, size:%{public}d!", size);
        return "";
    }

    std::string strIp = "";
    char bufIp6[INET6_ADDRSTRLEN] = {0};
    struct in6_addr addr6;
    if (memcpy_s(addr6.s6_addr, sizeof(addr6.s6_addr), &chIp, size) != EOK) {
        return "";
    }
    if (inet_ntop(AF_INET6, &addr6, bufIp6, INET6_ADDRSTRLEN) == NULL) {
        WIFI_LOGE("Ip6CharConToStr chIp failed, inet_ntop NULL!");
    } else {
        strIp = bufIp6;
        WIFI_LOGI("Ip6CharConToStr chIp -> strIp:%{private}s.", strIp.c_str());
    }

    return strIp;
}

bool DhcpFunc::CheckIpStr(const std::string& strIp)
{
    if (strIp.empty()) {
        WIFI_LOGE("CheckIpStr param error, strIp is empty()!");
        return false;
    }

    bool bIp4 = false;
    bool bIp6 = false;
    std::string::size_type idx = strIp.find(IP4_SEPARATOR);
    if (idx != std::string::npos) {
        bIp4 = true;
    }
    idx = strIp.find(IP6_SEPARATOR);
    if (idx != std::string::npos) {
        bIp6 = true;
    }
    if ((!bIp4 && !bIp6) || (bIp4 && bIp6)) {
        WIFI_LOGE("CheckIpStr strIp:%{private}s error, bIp4:%{public}d,bIp6:%{public}d!", strIp.c_str(), bIp4, bIp6);
        return false;
    }

    if (bIp4) {
        uint32_t uIp = 0;
        if (!Ip4StrConToInt(strIp, uIp)) {
            WIFI_LOGE("CheckIpStr Ip4StrConToInt failed, strIp:%{private}s.", strIp.c_str());
            return false;
        }
    } else {
        uint8_t	addr6[sizeof(struct in6_addr)] = {0};
        if (!Ip6StrConToChar(strIp, addr6, sizeof(struct in6_addr))) {
            WIFI_LOGE("CheckIpStr Ip6StrConToChar failed, strIp:%{private}s.", strIp.c_str());
            return false;
        }
    }

    return true;
}

int DhcpFunc::GetLocalIp(const std::string ethInf, std::string& localIp)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd) {
        WIFI_LOGE("GetLocalIp socket ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        return -1;
    }

    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ethInf.c_str(), IFNAMSIZ - 1) != EOK) {
        return -1;
    }
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0) {
        WIFI_LOGE("GetLocalIp ioctl ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        close(sd);
        return -1;
    }

    if (memcpy_s(&sin, sizeof(sin), &ifr.ifr_addr, sizeof(sin)) != EOK) {
        return -1;
    }
    char ip[IP_SIZE] = { 0 };
    if (snprintf_s(ip, IP_SIZE, IP_SIZE - 1, "%s", inet_ntoa(sin.sin_addr)) < 0) {
        WIFI_LOGE("GetLocalIp snprintf_s ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        close(sd);
        return -1;
    }
    localIp = ip;
    close(sd);
    return 0;
}

int DhcpFunc::GetLocalMac(const std::string ethInf, std::string& ethMac)
{
    struct ifreq ifr;
    int sd = 0;

    bzero(&ifr, sizeof(struct ifreq));
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        WIFI_LOGE("GetLocalMac socket ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        return -1;
    }

    if (strncpy_s(ifr.ifr_name, IFNAMSIZ, ethInf.c_str(), IFNAMSIZ - 1) != EOK) {
        return -1;
    }

    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) {
        WIFI_LOGE("GetLocalMac ioctl ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        close(sd);
        return -1;
    }

    char mac[ETH_MAC_ADDR_LEN * ETH_MAC_ADDR_CHAR_NUM] = { 0 };
    int nRes = snprintf_s(mac,
        ETH_MAC_ADDR_LEN * ETH_MAC_ADDR_CHAR_NUM,
        ETH_MAC_ADDR_LEN * ETH_MAC_ADDR_CHAR_NUM - 1,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[ETH_MAC_ADDR_INDEX_5]);
    if (nRes < 0) {
        WIFI_LOGE("GetLocalMac snprintf_s ethInf:%{public}s,strerror:%{public}s!", ethInf.c_str(), strerror(errno));
        close(sd);
        return -1;
    }
    ethMac = mac;
    close(sd);
    return 0;
}

bool DhcpFunc::IsExistFile(const std::string& filename)
{
    bool bExist = false;
    std::fstream ioFile;
    ioFile.open(filename.c_str(), std::ios::in);
    if (ioFile) {
        bExist = true;
    }
    ioFile.close();

    return bExist;
}

bool DhcpFunc::CreateFile(const std::string& filename, const std::string& filedata)
{
    std::ofstream outFile;
    outFile.open(filename.c_str());
    outFile.flush();
    outFile << filedata << std::endl;
    outFile.close();
    return true;
}

bool DhcpFunc::RemoveFile(const std::string& filename)
{
    if (std::remove(filename.c_str()) != 0) {
        WIFI_LOGE("RemoveFile filename:%{public}s failed!", filename.c_str());
        return false;
    }
    WIFI_LOGI("RemoveFile filename:%{public}s success.", filename.c_str());
    return true;
}

bool DhcpFunc::AddFileLineData(const std::string& filename, const std::string& prevdata, const std::string& linedata)
{
    bool bAdd = false;
    std::ifstream inFile;
    inFile.open(filename.c_str());
    std::string strFileData = "";
    std::string strTemp = "";
    char tmpLineData[1024] = {0};
    while (inFile.getline(tmpLineData, sizeof(tmpLineData))) {
        strTemp = tmpLineData;
        strFileData += strTemp;
        strFileData += "\n";
        if (strTemp == prevdata) {
            strFileData += linedata;
            bAdd = true;
        }
    }
    inFile.close();

    if (bAdd) {
        std::ofstream outFile;
        outFile.open(filename.c_str());
        outFile.flush();
        WIFI_LOGI("AddFileLineData Reflush filename:%{public}s, strFileData:%{public}s.",
            filename.c_str(), strFileData.c_str());
        outFile << strFileData;
        outFile.close();
    }
    return true;
}

bool DhcpFunc::DelFileLineData(const std::string& filename, const std::string& linedata)
{
    bool bDel = false;
    std::ifstream inFile;
    inFile.open(filename.c_str());
    std::string strFileData = "";
    std::string strTemp = "";
    char tmpLineData[1024] = {0};
    while (inFile.getline(tmpLineData, sizeof(tmpLineData))) {
        strTemp = tmpLineData;
        if (strTemp != linedata) {
            strFileData += strTemp;
            strFileData += "\n";
        } else {
            bDel = true;
        }
    }
    inFile.close();

    if (bDel) {
        std::ofstream outFile;
        outFile.open(filename.c_str());
        outFile.flush();
        WIFI_LOGI("DelFileLineData Reflush filename:%{public}s, strFileData:%{public}s.",
            filename.c_str(), strFileData.c_str());
        outFile << strFileData;
        outFile.close();
    }
    return true;
}

bool DhcpFunc::ModifyFileLineData(const std::string& filename, const std::string& srcdata, const std::string& dstdata)
{
    bool bModify = false;
    std::ifstream inFile;
    inFile.open(filename.c_str());
    std::string strFileData = "";
    std::string strTemp = "";
    char tmpLineData[1024] = {0};
    while (inFile.getline(tmpLineData, sizeof(tmpLineData))) {
        strTemp = tmpLineData;
        if (strTemp != srcdata) {
            strFileData += strTemp;
            strFileData += "\n";
        } else {
            strFileData += dstdata;
            strFileData += "\n";
            bModify = true;
        }
    }
    inFile.close();

    if (bModify) {
        std::ofstream outFile;
        outFile.open(filename.c_str());
        outFile.flush();
        WIFI_LOGI("ModifyFileLineData Reflush filename:%{public}s, strFileData:%{public}s.",
            filename.c_str(), strFileData.c_str());
        outFile << strFileData;
        outFile.close();
    }
    return true;
}

int DhcpFunc::FormatString(struct DhcpPacketResult &result)
{
    if (strncmp(result.strYiaddr, "*", 1) == 0) {
        if (memset_s(result.strYiaddr, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptServerId, "*", 1) == 0) {
        if (memset_s(result.strOptServerId, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptSubnet, "*", 1) == 0) {
        if (memset_s(result.strOptSubnet, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptDns1, "*", 1) == 0) {
        if (memset_s(result.strOptDns1, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptDns2, "*", 1) == 0) {
        if (memset_s(result.strOptDns2, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptRouter1, "*", 1) == 0) {
        if (memset_s(result.strOptRouter1, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptRouter2, "*", 1) == 0) {
        if (memset_s(result.strOptRouter2, INET_ADDRSTRLEN, 0, INET_ADDRSTRLEN) != EOK) {
            return -1;
        }
    }
    if (strncmp(result.strOptVendor, "*", 1) == 0) {
        if (memset_s(result.strOptVendor, DHCP_FILE_MAX_BYTES, 0, DHCP_FILE_MAX_BYTES) != EOK) {
            return -1;
        }
    }
    return 0;
}

int DhcpFunc::InitPidfile(const std::string& piddir, const std::string& pidfile)
{
    if (piddir.empty() || pidfile.empty()) {
        WIFI_LOGE("InitPidfile() failed, piddir or pidfile is empty!");
        return DHCP_OPT_FAILED;
    }
    WIFI_LOGI("InitPidfile() piddir:%{public}s, pidfile:%{public}s.", piddir.c_str(), pidfile.c_str());
    unlink(pidfile.c_str());

    int fd;
    if ((fd = open(pidfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
        WIFI_LOGE("InitPidfile() failed, open pidfile:%{public}s err:%{public}s!", pidfile.c_str(), strerror(errno));
        return DHCP_OPT_FAILED;
    }

    char buf[PID_MAX_LEN] = {0};
    if (snprintf_s(buf, PID_MAX_LEN, PID_MAX_LEN - 1, "%d", getpid()) < 0) {
        WIFI_LOGE("InitPidfile() %{public}s failed, snprintf_s error:%{public}s!", pidfile.c_str(), strerror(errno));
        close(fd);
        return DHCP_OPT_FAILED;
    }
    ssize_t bytes;
    if ((bytes = write(fd, buf, strlen(buf))) <= 0) {
        WIFI_LOGE("InitPidfile() failed, write pidfile:%{public}s error:%{public}s, bytes:%{public}zu!",
            pidfile.c_str(), strerror(errno), bytes);
        close(fd);
        return DHCP_OPT_FAILED;
    }
    WIFI_LOGI("InitPidfile() pid:%{public}s write %{public}s, bytes:%{public}zu!", buf, pidfile.c_str(), bytes);
    close(fd);

    if (chdir(piddir.c_str()) != 0) {
        WIFI_LOGE("InitPidfile() failed, chdir piddir:%{public}s err:%{public}s!", piddir.c_str(), strerror(errno));
        return DHCP_OPT_FAILED;
    }

    /* Set default permissions for the specified client process id files and directories. */
    umask(DEFAULT_UMASK);

    /* Change attribs to the specified client process id files: 644 (user=rw, group=r, other=r). */
    chmod(pidfile.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    return DHCP_OPT_SUCCESS;
}

pid_t DhcpFunc::GetPID(const std::string& pidfile)
{
    /* Check pidfile is or not exists. */
    struct stat sb;
    if (stat(pidfile.c_str(), &sb) != 0) {
        WIFI_LOGW("GetPID() pidfile:%{public}s stat:%{public}s!", pidfile.c_str(), strerror(errno));
        return -1;
    }
    WIFI_LOGI("GetPID() pidfile:%{public}s stat st_size:%{public}d.", pidfile.c_str(), (int)sb.st_size);

    int fd;
    if ((fd = open(pidfile.c_str(), O_RDONLY)) < 0) {
        WIFI_LOGE("GetPID() failed, open pidfile:%{public}s error!", pidfile.c_str());
        return -1;
    }

    lseek(fd, 0, SEEK_SET);

    char buf[PID_MAX_LEN] = {0};
    ssize_t bytes;
    if ((bytes = read(fd, buf, sb.st_size)) < 0) {
        WIFI_LOGE("GetPID() failed, read pidfile:%{public}s error, bytes:%{public}zu!", pidfile.c_str(), bytes);
        close(fd);
        return -1;
    }
    WIFI_LOGI("GetPID() read pidfile:%{public}s, buf:%{public}s, bytes:%{public}zu.", pidfile.c_str(), buf, bytes);
    close(fd);

    return atoi(buf);
}

int DhcpFunc::CreateDirs(const std::string dirs, int mode)
{
    if (dirs.empty() || (dirs.size() >= DIR_MAX_LEN)) {
        WIFI_LOGE("CreateDirs() dirs:%{public}s error!", dirs.c_str());
        return DHCP_OPT_FAILED;
    }

    int nSrcLen = (int)dirs.size();
    char strDir[DIR_MAX_LEN] = {0};
    if (strncpy_s(strDir, sizeof(strDir), dirs.c_str(), dirs.size()) != EOK) {
        WIFI_LOGE("CreateDirs() strncpy_s dirs:%{public}s failed!", dirs.c_str());
        return DHCP_OPT_FAILED;
    }
    if (strDir[nSrcLen - 1] != '/') {
        if (nSrcLen == (DIR_MAX_LEN - 1)) {
            WIFI_LOGE("CreateDirs() dirs:%{public}s len:%{public}d error!", dirs.c_str(), nSrcLen);
            return DHCP_OPT_FAILED;
        }
        if (strcat_s(strDir, sizeof(strDir), "/") != EOK) {
            WIFI_LOGE("CreateDirs() strcat_s strDir:%{public}s failed!", strDir);
            return DHCP_OPT_FAILED;
        }
        nSrcLen++;
    }

    int i = (strDir[0] == '/') ? 1 : 0;
    for (; i <= nSrcLen - 1; i++) {
        if (strDir[i] == '/') {
            strDir[i] = 0;
            if ((access(strDir, F_OK) != 0) && (mkdir(strDir, mode) != 0)) {
                WIFI_LOGE("CreateDirs() mkdir %{public}s %{public}.4o %{public}s!", strDir, mode, strerror(errno));
                return DHCP_OPT_FAILED;
            }
            strDir[i] = '/';
        }
    }
    WIFI_LOGI("CreateDirs() %{public}s %{public}.4o success.", dirs.c_str(), mode);
    return DHCP_OPT_SUCCESS;
}

bool DhcpFunc::SplitString(
    const std::string src, const std::string delim, const int count, std::vector<std::string> &splits)
{
    if (src.empty() || delim.empty()) {
        WIFI_LOGE("SplitString() error, src or delim is empty!");
        return false;
    }

    splits.clear();

    std::string strData(src);
    int nDelim = 0;
    char *pSave = NULL;
    char *pTok = strtok_r(const_cast<char *>(strData.c_str()), delim.c_str(), &pSave);
    while (pTok != NULL) {
        splits.push_back(std::string(pTok));
        nDelim++;
        pTok = strtok_r(NULL, delim.c_str(), &pSave);
    }
    if (nDelim != count) {
        WIFI_LOGE("SplitString() %{public}s failed, nDelim:%{public}d,count:%{public}d!", src.c_str(), nDelim, count);
        return false;
    }
    WIFI_LOGI("SplitString() %{private}s success, delim:%{public}s, count:%{public}d, splits.size():%{public}d.",
        src.c_str(), delim.c_str(), count, (int)splits.size());
    return true;
}

bool DhcpFunc::SubscribeDhcpCommonEvent(
    const std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> &subscriber)
{
    return OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
}

int DhcpFunc::SubscribeDhcpEvent(const std::string action, int timeouts)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(action);
    EventFwk::CommonEventSubscribeInfo subInfo(matchingSkills);
    std::shared_ptr<OHOS::Wifi::DhcpEventSubscriber> dhcpSubscriber =
        std::make_shared<OHOS::Wifi::DhcpEventSubscriber>(subInfo);
    if (!DhcpFunc::SubscribeDhcpCommonEvent(dhcpSubscriber)) {
        WIFI_LOGE("SubscribeDhcpEvent() SubscribeDhcpCommonEvent %{public}s failed!", action.c_str());
        return DHCP_OPT_FAILED;
    }
    WIFI_LOGI("SubscribeDhcpEvent() SubscribeDhcpCommonEvent %{public}s success.", action.c_str());
    return DHCP_OPT_SUCCESS;
}

bool DhcpFunc::UnsubscribeDhcpCommonEvent(
    const std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> &subscriber)
{
    return OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
}

int DhcpFunc::UnsubscribeDhcpEvent(const std::string action, int timeouts)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(action);
    EventFwk::CommonEventSubscribeInfo subInfo(matchingSkills);
    std::shared_ptr<OHOS::Wifi::DhcpEventSubscriber> dhcpSubscriber =
        std::make_shared<OHOS::Wifi::DhcpEventSubscriber>(subInfo);
    if (!DhcpFunc::UnsubscribeDhcpCommonEvent(dhcpSubscriber)) {
        WIFI_LOGE("UnsubscribeDhcpEvent() UnsubscribeDhcpCommonEvent %{public}s failed!", action.c_str());
        return DHCP_OPT_FAILED;
    }
    WIFI_LOGI("UnsubscribeDhcpEvent() UnsubscribeDhcpCommonEvent %{public}s success.", action.c_str());
    return DHCP_OPT_SUCCESS;
}

bool DhcpFunc::PublishDhcpEvent(const std::string action, const int code, const std::string data)
{
    OHOS::EventFwk::Want want;
    want.SetAction(action);
    OHOS::EventFwk::CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    if (!OHOS::EventFwk::CommonEventManager::PublishCommonEvent(commonData)) {
        WIFI_LOGE("PublishDhcpEvent() PublishCommonEvent failed, action:%{public}s, code:%{public}d, data:%{public}s.",
            action.c_str(), code, data.c_str());
        return DHCP_OPT_FAILED;
    }
    WIFI_LOGI("PublishDhcpEvent() PublishCommonEvent success, action:%{public}s, code:%{public}d, data:%{private}s.",
        action.c_str(), code, data.c_str());
    return DHCP_OPT_SUCCESS;
}
}  // namespace Wifi
}  // namespace OHOS