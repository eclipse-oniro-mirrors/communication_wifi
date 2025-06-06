/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_MOCK_AP_ROOT_STATE_H
#define OHOS_MOCK_AP_ROOT_STATE_H

#include <gmock/gmock.h>
#include "wifi_msg.h"
#include "ap_root_state.h"

namespace OHOS {
namespace Wifi {
class ApStateMachine;
class MockApRootState : public ApRootState {
public:
    MOCK_METHOD0(GoInState, void());
    MOCK_METHOD0(GoOutState, void());
    MOCK_METHOD1(ExecuteStateMsg, bool(InternalMessagePtr msg));
}; /* ApRootState */
} // namespace Wifi
} // namespace OHOS
#endif