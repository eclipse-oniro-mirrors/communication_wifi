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
#ifndef MOCK_AUTHORIZING_NEGOTIATION_REQUEST_STATE
#define MOCK_AUTHORIZING_NEGOTIATION_REQUEST_STATE
#include <gmock/gmock.h>
#include "authorizing_negotiation_request_state.h"
namespace OHOS {
namespace Wifi {
class MockAuthorizingNegotiationRequestState : public AuthorizingNegotiationRequestState {
public:
    MockAuthorizingNegotiationRequestState(
        P2pStateMachine &p2pStateMachine, WifiP2pGroupManager &groupManager, WifiP2pDeviceManager &deviceManager)
        : AuthorizingNegotiationRequestState(p2pStateMachine, groupManager, deviceManager)
    {}
    ~MockAuthorizingNegotiationRequestState() = default;
    MOCK_METHOD0(GoInState, void());
    MOCK_METHOD0(GoOutState, void());
    MOCK_METHOD1(ExecuteStateMsg, bool(InternalMessage *msg));
};
}  // namespace Wifi
}  // namespace OHOS
#endif