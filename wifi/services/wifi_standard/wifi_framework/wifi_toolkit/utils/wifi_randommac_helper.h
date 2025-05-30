/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_WIFI_RANDOM_MAC_HELPER_H
#define OHOS_WIFI_RANDOM_MAC_HELPER_H
#include <string>
#include <random>
#include <fcntl.h>
#include <unistd.h>
#include "wifi_common_util.h"
#include "wifi_msg.h"

namespace OHOS {
namespace Wifi {

class WifiRandomMacHelper {
public:

    /**
     * @Description Generate random number
     *
     * @return long int
     */
    static long int GetRandom();

    /**
     * @Description generate a MAC address
     *
     * @param randomMacAddr - random MAC address[out]
     */
    static void GenerateRandomMacAddress(std::string &randomMacAddr);

    /**
     * @Description generate a MAC address
     *
     * @param peerBssid - real MAC address[in]
     * @param randomMacAddr - random MAC address[out]
     */
    static void GenerateRandomMacAddressByBssid(std::string peerBssid, std::string &randomMacAddr);

    /**
     * @Description generate a MAC address
     *
     * @param random - unsigned long long value mac address[in]
     * @param randomMacAddr - random MAC address[out]
     * @return 0 - success
     */
    static int GenerateRandomMacAddressByLong(unsigned long long random, std::string &randomMacAddr);

#ifdef SUPPORT_LOCAL_RANDOM_MAC
    /**
     * @Description generate a MAC address
     *
     * @param content - content for generate MAC address[in]
     * @param randomMacAddr - random MAC address[out]
     * @return 0 - success
     */
    static int CalculateRandomMacForWifiDeviceConfig(const std::string &content, std::string &randomMacAddr);
#endif

    /**
     * @Description generate a MAC address
     *
     * @param peerBssid - real MAC address[in]
     * @param randomMacAddr - random MAC address[out]
     */
    static void LongLongToBytes(unsigned long long value, std::vector<uint8_t> &outPlant);

    /**
     * @Description convert uint8_t array to unsigned long long value
     *
     * @param bytes - uint8_t array bytes[in]
     * @return unsigned long long value
     */
    static unsigned long long BytesToLonglong(const std::vector<uint8_t> &bytes);

    /**
     * @Description convert uint8_t array to unsigned long long mac address
     *
     * @param bytes - uint8_t array bytes[in]
     * @return unsigned long long value of mac address
     */
    static unsigned long long LongAddrFromByteAddr(std::vector<uint8_t> &bytes);

    /**
     * @Description generate a MAC address from unsigned long long value
     *
     * @param addr - addr[in]
     * @param randomMacAddr - random MAC address[out]
     * @return 0 - success
     */
    static int StringAddrFromLongAddr(unsigned long long addr, std::string &randomMacAddr);
    /**
     * @Description a debug tool for BytesArrayToString
     *
     * @param bytes - uint8_t array
     * @return std::string - formated string
     */
    static std::string BytesArrayToString(const std::vector<uint8_t> &bytes);

    /**
    * @Description  change random mac for wifi2
    *for example, input mac is 00:1a:2b:3c:4d:5e, this func will change pos 3 byte mac from "3c" to "bc" by
    *performing an XOR operation with 0x80
    * @param wifi2RandomMac - wifi2RandomMac
    */
    static bool GetWifi2RandomMac(std::string &wifi2RandomMac);
};
}
}
#endif