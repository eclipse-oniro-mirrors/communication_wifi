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

#ifndef OHOS_WIFI_SCAN_MGR_SERVICE_H
#define OHOS_WIFI_SCAN_MGR_SERVICE_H

#ifndef OHOS_ARCH_LITE
#include <map>
#include "iremote_stub.h"
#include "message_parcel.h"
#include "message_option.h"
#include "i_wifi_scan_mgr.h"

namespace OHOS {
namespace Wifi {
class WifiScanMgrStub : public IRemoteStub<IWifiScanMgr> {
public:
    using FuncHandle = int (WifiScanMgrStub::*)(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    using FuncHandleMap = std::map<int, FuncHandle>;

    WifiScanMgrStub();
    virtual ~WifiScanMgrStub();
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;
    int GetWifiRemoteInner(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    static FuncHandleMap funcHandleMap_;
};
}  // namespace Wifi
}  // namespace OHOS
#endif
#endif