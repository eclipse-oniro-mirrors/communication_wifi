/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_HDI_DEFINE_H
#define OHOS_HDI_DEFINE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define HDI_UUID_LEN 16
#define HDI_PIN_LEN 9
#define HDI_MAC_SUB_LEN 2

#define HDI_PROTO_DEFAULT HDI_BIT(0)
#define HDI_PROTO_ONE HDI_BIT(1)
#define HDI_PROTO_THREE HDI_BIT(3)

#define HDI_MAX__VENDOR_EXT 10

#define HDI_MAX_IES_SUPPORTED 5

#define HDI_P2P_CARD_SSID "DIRECT-"
#define HDI_P2P_CARD_SSID_LEN 7

#define HDI_IE_VENDOR_TYPE 0x0050f201
#define HDI_P2P_IE_VENDOR_TYPE 0x506f9a09
#define HDI_WFD_TYPE 10
#define HDI_HS20_IE_VENDOR_TYPE 0x506f9a10
#define HDI_OSEN_IE_VENDOR_TYPE 0x506f9a12
#define HDI_MBO_OUI_TYPE 22
#define HDI_OWE_VENDOR_TYPE 0x506f9a1c
#define HDI_OWE_OUI_TYPE 28
#define HDI_AP_OUI_TYPE 0x1B

#define HDI_WMM_OUI_TYPE 2
#define HDI_WMM_OUI_SUBTYPE_INFORMATION 0
#define HDI_WMM_OUI_SUBTYPE_PARAMETER 1
#define HDI_WMM_OUI_SUBTYPE_ELEMENT 2


#define HDI_SELECTOR_LEN 4
#define HDI_VERSION 1

#define HDI_CIPHER_NONE HDI_BIT(0)
#define HDI_CIPHER_TKIP HDI_BIT(3)
#define HDI_CIPHER_CCMP HDI_BIT(4)
#define HDI_CIPHER_AES_128_CMAC HDI_BIT(5)
#define HDI_CIPHER_GCMP HDI_BIT(6)
#define HDI_CIPHER_GCMP_256 HDI_BIT(8)
#define HDI_CIPHER_CCMP_256 HDI_BIT(9)
#define HDI_CIPHER_BIP_GMAC_128 HDI_BIT(11)
#define HDI_CIPHER_BIP_GMAC_256 HDI_BIT(12)
#define HDI_CIPHER_BIP_CMAC_256 HDI_BIT(13)
#define HDI_CIPHER_GTK_NOT_USED HDI_BIT(14)

#define HDI_KEY_MGMT HDI_BIT(0)
#define HDI_KEY_MGMT_PSK HDI_BIT(1)
#define HDI_KEY_MGMT_HDI_NONE HDI_BIT(4)
#define HDI_KEY_MGMT_FT_IEEE8021X HDI_BIT(5)
#define HDI_KEY_MGMT_FT_PSK HDI_BIT(6)
#define HDI_KEY_MGMT_IEEE8021X_SHA256 HDI_BIT(7)
#define HDI_KEY_MGMT_PSK_SHA256 HDI_BIT(8)
#define HDI_KEY_MGMT_SAE HDI_BIT(10)
#define HDI_KEY_MGMT_FT_SAE HDI_BIT(11)
#define HDI_KEY_MGMT_OSEN HDI_BIT(15)
#define HDI_KEY_MGMT_SUITE_B HDI_BIT(16)
#define HDI_KEY_MGMT_SUITE_B_192 HDI_BIT(17)
#define HDI_KEY_MGMT_FILS_SHA256 HDI_BIT(18)
#define HDI_KEY_MGMT_FILS_SHA384 HDI_BIT(19)
#define HDI_KEY_MGMT_FT_FILS_SHA256 HDI_BIT(20)
#define HDI_KEY_MGMT_FT_FILS_SHA384 HDI_BIT(21)
#define HDI_KEY_MGMT_OWE HDI_BIT(22)
#define HDI_KEY_MGMT_DPP HDI_BIT(23)
#define HDI_KEY_MGMT_FT_SHA384 HDI_BIT(24)

#define HDI_PMKID_LEN 16

#define HDI_EID_RSNX 244
#define HDI_RSNX_CAPAB_SAE_H2E 5
#define HDI_RSNX_CAPAB_SAE_PK 6

#define HDI_CAPABILITY_PREAUTH HDI_BIT(0)

#define HDI_AUTH_KEY_MGMT_NONE HDI_BIT_MULTI(0x00, 0x50, 0xf2, 0)
#define HDI_AUTH_KEY_MGMT_UNSPEC HDI_BIT_MULTI(0x00, 0x50, 0xf2, 1)
#define HDI_AUTH_KEY_MGMT_PSK_OVER HDI_BIT_MULTI(0x00, 0x50, 0xf2, 2)
#define HDI_CIPHER_SUITE_NONE HDI_BIT_MULTI(0x00, 0x50, 0xf2, 0)
#define HDI_CIPHER_SUITE_TKIP HDI_BIT_MULTI(0x00, 0x50, 0xf2, 2)
#define HDI_CIPHER_SUITE_CCMP HDI_BIT_MULTI(0x00, 0x50, 0xf2, 4)

#define HDI_RSN_AUTH_KEY_MGMT_UNSPEC HDI_BIT_MULTI(0x00, 0x0f, 0xac, 1)
#define HDI_RSN_AUTH_KEY_MGMT_PSK_OVER HDI_BIT_MULTI(0x00, 0x0f, 0xac, 2)
#define HDI_RSN_AUTH_KEY_MGMT_FT HDI_BIT_MULTI(0x00, 0x0f, 0xac, 3)
#define HDI_RSN_AUTH_KEY_MGMT_FT_PSK HDI_BIT_MULTI(0x00, 0x0f, 0xac, 4)
#define HDI_RSN_AUTH_KEY_MGMT_SHA256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 5)
#define HDI_RSN_AUTH_KEY_MGMT_PSK_SHA256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 6)
#define HDI_RSN_AUTH_KEY_MGMT_SAE HDI_BIT_MULTI(0x00, 0x0f, 0xac, 8)
#define HDI_RSN_AUTH_KEY_MGMT_FT_SAE HDI_BIT_MULTI(0x00, 0x0f, 0xac, 9)
#define HDI_RSN_AUTH_KEY_MGMT_SUITE_B HDI_BIT_MULTI(0x00, 0x0f, 0xac, 11)
#define HDI_RSN_AUTH_KEY_MGMT_SUITE_B_192 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 12)
#define HDI_RSN_AUTH_KEY_MGMT_FT_SHA384 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 13)
#define HDI_RSN_AUTH_KEY_MGMT_FILS_SHA256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 14)
#define HDI_RSN_AUTH_KEY_MGMT_FILS_SHA384 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 15)
#define HDI_RSN_AUTH_KEY_MGMT_FT_FILS_SHA256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 16)
#define HDI_RSN_AUTH_KEY_MGMT_FT_FILS_SHA384 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 17)
#define HDI_RSN_AUTH_KEY_MGMT_OWE HDI_BIT_MULTI(0x00, 0x0f, 0xac, 18)
#define HDI_RSN_AUTH_KEY_MGMT_OSEN HDI_BIT_MULTI(0x50, 0x6f, 0x9a, 0x01)
#define HDI_RSN_AUTH_KEY_MGMT_DPP HDI_BIT_MULTI(0x50, 0x6f, 0x9a, 0x02)

#define HDI_RSN_CIPHER_SUITE_NONE HDI_BIT_MULTI(0x00, 0x0f, 0xac, 0)
#define HDI_RSN_CIPHER_SUITE_TKIP HDI_BIT_MULTI(0x00, 0x0f, 0xac, 2)

#define HDI_RSN_CIPHER_SUITE_CCMP HDI_BIT_MULTI(0x00, 0x0f, 0xac, 4)
#define HDI_RSN_CIPHER_SUITE_AES_128_CMAC HDI_BIT_MULTI(0x00, 0x0f, 0xac, 6)
#define HDI_RSN_CIPHER_SUITE_NO_GROUP_ADDRESSED HDI_BIT_MULTI(0x00, 0x0f, 0xac, 7)
#define HDI_RSN_CIPHER_SUITE_GCMP HDI_BIT_MULTI(0x00, 0x0f, 0xac, 8)
#define HDI_RSN_CIPHER_SUITE_GCMP_256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 9)
#define HDI_RSN_CIPHER_SUITE_CCMP_256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 10)
#define HDI_RSN_CIPHER_SUITE_BIP_GMAC_128 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 11)
#define HDI_RSN_CIPHER_SUITE_BIP_GMAC_256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 12)
#define HDI_RSN_CIPHER_SUITE_BIP_CMAC_256 HDI_BIT_MULTI(0x00, 0x0f, 0xac, 13)

#define HDI_OUI_TYPE HDI_BIT_MULTI(0x00, 0x50, 0xf2, 1)

#define SSID_MAX_LEN 32

#define HDI_EID_SSID 0
#define HDI_EID_SUPP_RATES 1
#define HDI_EID_DS_PARAMS 3
#define HDI_EID_CF_PARAMS 4
#define HDI_EID_TIM 5
#define HDI_EID_COUNTRY 7
#define HDI_EID_CHALLENGE 16
#define HDI_EID_PWR_CAPABILITY 33
#define HDI_EID_SUPPORTED_CHANNELS 36
#define HDI_EID_ERP_INFO 42
#define HDI_EID_HT_CAP 45
#define HDI_EID_RSN 48
#define HDI_EID_EXT_SUPP_RATES 50
#define HDI_EID_MOBILITY_DOMAIN 54
#define HDI_EID_FAST_BSS_TRANSITION 55
#define HDI_EID_TIMEOUT_INTERVAL 56
#define HDI_EID_SUPPORTED_OPERATING_CLASSES 59
#define HDI_EID_HT_OPERATION 61
#define HDI_EID_WAPI 68
#define HDI_EID_RRM_ENABLED_CAPABILITIES 70
#define HDI_EID_SSID_LIST 84
#define HDI_EID_BSS_MAX_IDLE_PERIOD 90
#define HDI_EID_LINK_ID 101
#define HDI_EID_INTERWORKING 107
#define HDI_EID_QOS_MAP_SET 110
#define HDI_EID_MESH_CONFIG 113
#define HDI_EID_MESH_ID 114
#define HDI_EID_PEER_MGMT 117
#define HDI_EID_EXT_CAPAB 127
#define HDI_EID_AMPE 139
#define HDI_EID_MIC 140
#define HDI_EID_MULTI_BAND 158
#define HDI_EID_VHT_CAP 191
#define HDI_EID_VHT_OPERATION 192
#define HDI_EID_VHT_OPERATING_MODE_NOTIFICATION 199
#define HDI_EID_VENDOR_SPECIFIC 221
#define HDI_EID_CAG_NUMBER 237
#define HDI_EID_AP_CSN 239
#define HDI_EID_FILS_INDICATION 240
#define HDI_EID_DILS 241
#define HDI_EID_FRAGMENT 242
#define HDI_EID_EXTENSION 255

/* Element ID Extension (EID 255) values */
#define HDI_EID_EXT_ASSOC_DELAY_INFO 1
#define HDI_EID_EXT_FILS_REQ_PARAMS 2
#define HDI_EID_EXT_FILS_KEY_CONFIRM 3
#define HDI_EID_EXT_FILS_SESSION 4
#define HDI_EID_EXT_FILS_HLP_CONTAINER 5
#define HDI_EID_EXT_FILS_IP_ADDR_ASSIGN 6
#define HDI_EID_EXT_KEY_DELIVERY 7
#define HDI_EID_EXT_FILS_WRAPPED_DATA 8
#define HDI_EID_EXT_FILS_PUBLIC_KEY 12
#define HDI_EID_EXT_FILS_NONCE 13
#define HDI_EID_EXT_OWE_DH_PARAM 32
#define HDI_EID_EXT_PASSWORD_IDENTIFIER 33
#define HDI_EID_EXT_HE_CAPABILITIES 35
#define HDI_EID_EXT_HE_OPERATION 36
#define HDI_EID_EXT_OCV_OCI 54
#define HDI_EID_EXT_EHT_CAPABILITIES_80211BE 108

#define HDI_EID_EXT_EDMG_OPERATION 62
#define HDI_EXT_CAPAB_UTF_8_SSID 48

#define HDI_CAP_ESS    0x0001
#define HDI_CAP_IBSS    0x0002
#define HDI_CAP_PRIVACY    0x0010

#define HDI_CAP_DMG_MASK    0x0003
#define HDI_CAP_DMG_IBSS    0x0001
#define HDI_CAP_DMG_PBSS    0x0002
#define HDI_CAP_DMG_AP    0x0003

#define HDI_OUI_MICROSOFT 0x0050f2
#define HDI_OUI_WFA 0x506f9a
#define HDI_P2P_OUI_TYPE 9
#define HDI_HT_CAPAB_OUI_TYPE 0x33
#define HDI_OUI_BROADCOM 0x00904c

#define HDI_VHT_TYPE        0x04
#define HDI_VHT_SUBTYPE    0x08
#define HDI_VHT_SUBTYPE2    0x00

#define HDI_HS20_INDICATION_OUI_TYPE 16
#define HDI_HS20_OSEN_OUI_TYPE 18
#define HDI_HS20_ROAMING_CONS_SEL_OUI_TYPE 29

#define HDI_OUI_QCA 0x001374

#define HDI_MOBILITY_DOMAIN_ID_LEN 2

#define HDI_FILS_NONCE_LEN 16
#define HDI_FILS_SESSION_LEN 8

#define HDI_KEY_RSC_LEN 8

#define HDI_NONCE_LEN 32

#define HDI_MAX_CRED_COUNT 10
#define HDI_MAX_REQ_DEV_TYPE_COUNT 10

#define HDI_POS_ZERO 0
#define HDI_POS_FIRST 1
#define HDI_POS_SECOND 2
#define HDI_POS_THIRD 3
#define HDI_POS_FOURTH 4
#define HDI_POS_FIVE 5
#define HDI_POS_SIX 6
#define HDI_POS_EIGHT 8
#define HDI_POS_TEN 10
#define HDI_POS_OT 16
#define HDI_POS_ET 18
#define HDI_POS_TT 32
#define HDI_POS_OTX 126
#define HDI_POS_DMG 45000

#define HDI_MOVE_EIGHT 8
#define HDI_MOVE_SIXTEEN 16
#define HDI_MOVE_TF 24

#define HDI_BSSID_LENGTH 18
#define HDI_SSID_LENGTH 132
#define HDI_SCAN_INFO_CAPABILITY_LENGTH 256
#define HDI_GET_MAX_SCAN_INFO 256 /* Maximum number of scan infos obtained at a time */
#define HDI_COUNTRY_CODE_LENGTH 2
#define HDI_MAC_LENGTH 17

#define HDI_STA_CB_SCAN_FAILED 1
#define HDI_STA_CB_SCAN_OVER_OK 2

#define WIFI_CAPABILITY_DEFAULT 0
#define CMD_GET_FEATURE_CAPAB 101
#define CMD_GET_WIFI_CATEGORY 127
#define HDI_SCAN_RESULTS_MAX_LEN 1024

#define HDI_CAP_WAPI_BIT_OFFSET 9
#define HDI_KEY_MGMT_WAPI_CERT_AKM 0X01
#define HDI_KEY_MGMT_WAPI_PSK_AKM 0X02

typedef long os_time_t;

enum HDIVendorElementId {
    HDI_VENDOR_ELEM_P2P_PREF_CHAN_LIST = 0,
    HDI_VENDOR_ELEM_HE_CAPAB = 1,
    HDI_VENDOR_ELEM_HE_OPER = 2,
    HDI_VENDOR_ELEM_RAPS = 3,
    HDI_VENDOR_ELEM_MU_EDCA_PARAMS = 4,
    HDI_VENDOR_ELEM_BSS_COLOR_CHANGE = 5,
};

typedef enum HdiPortType {
    HDI_PORT_TYPE_STATION    = 0,
    HDI_PORT_TYPE_AP         = 1,
    HDI_PORT_TYPE_P2P_CLIENT = 2,
    HDI_PORT_TYPE_P2P_GO     = 3,
    HDI_PORT_TYPE_P2P_DEVICE = 4,
    HDI_PORT_TYPE_BUTT            // invalid type
} HdiPortType;

typedef enum {
    PROTOCOL_80211_IFTYPE_UNSPECIFIED,
    PROTOCOL_80211_IFTYPE_ADHOC,
    PROTOCOL_80211_IFTYPE_STATION,
    PROTOCOL_80211_IFTYPE_AP,
    PROTOCOL_80211_IFTYPE_AP_VLAN,
    PROTOCOL_80211_IFTYPE_WDS,
    PROTOCOL_80211_IFTYPE_MONITOR,
    PROTOCOL_80211_IFTYPE_MESH_POINT,
    PROTOCOL_80211_IFTYPE_P2P_CLIENT,
    PROTOCOL_80211_IFTYPE_P2P_GO,
    PROTOCOL_80211_IFTYPE_P2P_DEVICE,
    PROTOCOL_80211_IFTYPE_NUM,
} FeatureType;
#ifdef __cplusplus
}
#endif
#endif