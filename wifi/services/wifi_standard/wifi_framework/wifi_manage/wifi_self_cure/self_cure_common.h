/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_WIFI_SELF_CURE_DEFINE_H
#define OHOS_WIFI_SELF_CURE_DEFINE_H

namespace OHOS {
namespace Wifi {
#define FRIEND_GTEST(test_typename) friend class test_typename##Test

#define WIFI_CURE_NOTIFY_NETWORK_CONNECTED_RCVD 104
#define WIFI_CURE_NOTIFY_NETWORK_DISCONNECTED_RCVD 108
#define WIFI_CURE_NOTIFY_RSSI_LEVEL_CHANGED_EVENT 109
#define WIFI_CURE_CMD_NETWORK_ROAMING_DETECT 110
#define WIFI_CURE_CMD_INTERNET_FAILED_SELF_CURE 112
#define WIFI_CURE_CMD_INTERNET_RECOVERY_CONFIRM 113
#define WIFI_CURE_CMD_SELF_CURE_WIFI_LINK 114
#define WIFI_CURE_CMD_GATEWAY_CHANGED_DETECT 115
#define WIFI_CURE_CMD_IP_CONFIG_TIMEOUT 116
#define WIFI_CURE_CMD_IP_CONFIG_COMPLETED 117
#define WIFI_CURE_CMD_RESETUP_SELF_CURE_MONITOR 118
#define WIFI_CURE_CMD_UPDATE_CONN_SELF_CURE_HISTORY 119
#define WIFI_CURE_CMD_INTERNET_FAILURE_DETECTED 122
#define WIFI_CURE_CMD_DNS_FAILED_MONITOR 123
#define WIFI_CURE_CMD_P2P_DISCONNECTED_EVENT 128
#define WIFI_CURE_CMD_INVALID_IP_CONFIRM 129
#define WIFI_CURE_CMD_INVALID_DHCP_OFFER_EVENT 130
#define WIFI_CURE_CMD_ARP_FAILED_DETECTED 139
#define WIFI_CURE_CMD_WIFI6_SELFCURE 140
#define WIFI_CURE_CMD_WIFI6_BACKOFF_SELFCURE 141
#define WIFI_CURE_CMD_WIFI6_WITHOUT_HTC_PERIODIC_ARP_DETECTED 307
#define WIFI_CURE_CMD_WIFI6_WITH_HTC_PERIODIC_ARP_DETECTED 308
#define WIFI_CURE_CMD_WIFI6_WITH_HTC_ARP_FAILED_DETECTED 309
#define WIFI_CURE_CMD_WIFI6_WITHOUT_HTC_ARP_FAILED_DETECTED 310

#define EVENT_AX_BLA_LIST 131
#define EVENT_AX_CLOSE_HTC 132
#define WIFI_CURE_RESET_LEVEL_IDLE 200
#define WIFI_CURE_RESET_LEVEL_LOW_1_DNS 201
#define WIFI_CURE_RESET_LEVEL_LOW_2_RENEW_DHCP 202
#define WIFI_CURE_RESET_LEVEL_LOW_3_STATIC_IP 203
#define WIFI_CURE_RESET_LEVEL_MIDDLE_REASSOC 204
#define WIFI_CURE_RESET_LEVEL_HIGH_RESET 205
#define WIFI_CURE_RESET_REJECTED_BY_STATIC_IP_ENABLED 206
#define WIFI_CURE_RESET_LEVEL_RECONNECT_4_INVALID_IP 207
#define WIFI_CURE_RESET_LEVEL_DEAUTH_BSSID 208
#define WIFI_CURE_RESET_LEVEL_RAND_MAC_REASSOC 209

#define WIFI_CURE_INTERNET_FAILED_TYPE_ROAMING 301
#define WIFI_CURE_INTERNET_FAILED_TYPE_GATEWAY 302
#define WIFI_CURE_INTERNET_FAILED_TYPE_DNS 303
#define WIFI_CURE_INTERNET_FAILED_TYPE_TCP 304
#define WIFI_CURE_INTERNET_FAILED_INVALID_IP 305
#define WIFI_CURE_CMD_PERIODIC_ARP_DETECTED 306
#define WIFI_CURE_INTERNET_FAILED_RAND_MAC 307

#define NET_ERR_HTTP_REDIRECTED 302

#define HISTORY_TYPE_INTERNET 100
#define HISTORY_TYPE_PORTAL 102
#define HISTORY_TYPE_EMPTY 103
#define HISTORY_TYPE_HAS_INTERNET_EVER 104

#define HISTORY_ITEM_UNCHECKED (-1)
#define HISTORY_ITEM_NO_INTERNET 0
#define HISTORY_ITEM_INTERNET 1
#define HISTORY_ITEM_PORTAL 2

#define SELFCURE_FAIL_LENGTH 12
#define SELFCURE_HISTORY_LENGTH 18

#define SIGNAL_LEVEL_1 1
#define SIGNAL_LEVEL_2 2
#define SIGNAL_LEVEL_3 3
#define FAC_MAC_REASSOC 2
#define RAND_MAC_REASSOC 3
#define DEAUTH_BSSID_CNT 3
#define DEFAULT_SLOW_NUM_ARP_PINGS 3
#define MULTI_BSSID_NUM 2
#define ACTION_TYPE_HTC 0
#define ACTION_TYPE_WIFI6 1

#define ARP_DETECTED_FAILED_COUNT 5
#define SELF_CURE_RAND_MAC_MAX_COUNT 20
#define SELF_CURE_RAND_MAC_CONNECT_FAIL_MAX_COUNT 3

#define SELF_CURE_WIFI_OFF_TIMEOUT 2000
#define SELF_CURE_WIFI_ON_TIMEOUT 5000
#define MAX_ARP_DNS_CHECK_TIME 300
#define SELF_CURE_DELAYED_MS 100
#define INTERNET_RECOVERY_TIME 300
#define WIFI6_HTC_ARP_DETECTED_MS 300
#define FAST_ARP_DETECTED_MS (10 * 1000)
#define DEFAULT_ARP_DETECTED_MS (60 * 1000)
#define SELF_CURE_MONITOR_DELAYED_MS (2 * 1000)
#define DHCP_RENEW_TIMEOUT_MS (6 * 1000)
#define DNS_UPDATE_CONFIRM_DELAYED_MS (1 * 1000)
#define IP_CONFIG_CONFIRM_DELAYED_MS (2 * 1000)
#define DELAYED_DAYS_LOW (24 * 60 * 60 * 1000)
#define DELAYED_DAYS_MID (3 * DELAYED_DAYS_LOW)
#define DELAYED_DAYS_HIGH (5 * DELAYED_DAYS_LOW)
#define RAND_MAC_FAIL_EXPIRATION_AGE_MILLIS (30 * 1000)
#define SET_STATIC_IP_TIMEOUT_MS (3 * 1000)
#define INTERNET_DETECT_INTERVAL_MS (6 * 1000)
#define WIFI6_BLA_LIST_TIME_EXPIRED (2 * 24 * 60 * 60 * 1000)

#define MIN_VAL_LEVEL_2_24G (-82)
#define MIN_VAL_LEVEL_2_5G (-79)
#define MIN_VAL_LEVEL_3 (-75)
#define MIN_VAL_LEVEL_3_5 (-70)
#define MIN_VAL_LEVEL_3_24G (-75)
#define MIN_VAL_LEVEL_3_5G (-72)
#define MIN_VAL_LEVEL_4 (-65)
} //namespace Wifi
} //namespace OHOS
#endif