/*
 * Copyright (c) Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef OHOS_WIFI_MSDP_STATE_LISTENER_H
#define OHOS_WIFI_MSDP_STATE_LISTENER_H

#include "movement_client.h"
#include "movement_callback_stub.h"

namespace OHOS {
namespace Wifi {
class DeviceMovementCallback : public Msdp::MovementCallbackStub {
public:
    DeviceMovementCallback() = default;
    ~DeviceMovementCallback() = default;
    void OnMovementChanged(const Msdp::MovementDataUtils::MovementData &movementData) override;
};

} // namespace Wifi
} // namespace OHOS
#endif