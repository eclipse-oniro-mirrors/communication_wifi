/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

/**
 * @addtogroup wifiservice
 * @{
 *
 * @brief Provides functions for the Wi-Fi station and hotspot modes.
 *
 * You can use this module to enable and disable the Wi-Fi station or hotspot mode, connect to and disconnect from a
 * station or hotspot, query the station or hotspot status, and listen for events. \n
 *
 * @since 7
 */

/**
 * @file station_info.h
 *
 * @brief Defines the structure and macro of the station information contained in the event information.
 *
 * The station information is passed to {@link OnHotspotStaJoin} or {@link OnHotspotStaLeave} invoked when a specified
 * event occurs.
 *
 * @since 7
 */

#ifndef WIFI_LITE_WIFI_STATION_INFO_H
#define WIFI_LITE_WIFI_STATION_INFO_H
#include "wifi_device_config.h"

/**
 * @brief Represents the station information.
 *
 * The station information is returned when {@link OnHotspotStaJoin} or {@link OnHotspotStaLeave} is called. \n
 *
 * @since 7
 */
typedef struct {
    /** Network name of the station. Notice: please pre-allocate memory for "name" */
    char *name;
    /** MAC address. For its length, see {@link WIFI_MAC_LEN}. */
    unsigned char macAddress[WIFI_MAC_LEN];
    /* bssid type. */
    int macAddressType;
    /** IP address of the station */
    unsigned int ipAddress;
    unsigned short disconnectedReason;
} StationInfo;

#endif // WIFI_LITE_WIFI_STATION_INFO_H
/** @} */
