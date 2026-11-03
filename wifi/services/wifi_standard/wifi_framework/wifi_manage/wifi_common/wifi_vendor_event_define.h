/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef WIFI_VENDOR_EVENT_DEFINE_H
#define WIFI_VENDOR_EVENT_DEFINE_H

namespace OHOS {
namespace Wifi {
/* Vendor netlink event ids reported by the WLAN driver and dispatched inside the wifi service. */
constexpr int VDR_VOWIFI_SYNC_REPORT = 302;
/* Vendor netlink event for a MIMO/SISO spatial-stream switch; payload carries nssNum and smpsMode. */
constexpr int VDR_MIMO_SISO_SWITCH_EVENT = 105;
/* Sentinel for WifiLinkedInfo.nssState: -1 = not learned yet for this link, 0 = SISO, >=1 = MIMO.
   The first MIMO/SISO event always passes the dedup gate because of this value. */
constexpr int INVALID_NSS_STATE = -1;
}  // namespace Wifi
}  // namespace OHOS
#endif  // WIFI_VENDOR_EVENT_DEFINE_H
