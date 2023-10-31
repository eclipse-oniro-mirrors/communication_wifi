/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitation under the License.
*/

#ifndef WIFICONTROLLER_WIFICONTROLLERMACHINE_H
#define WIFICONTROLLER_WIFICONTROLLERMACHINE_H

#include <string>
#include <vector>
#include "state.h"
#include "state_machine.h"
#include "wifi_logger.h"
#include "wifi_errcode.h"
#include "concrete_clientmode_manager.h"
#include "softap_manager.h"

namespace OHOS {
namespace Wifi {
class WifiControllerMachine : public StateMachine {
public:
    WifiControllerMachine();
    ~WifiControllerMachine();

    class DisableState : public State {
    public:
        explicit DisableState(WifiControllerMachine *wifiControllerMachine);
        ~DisableState() override;
        void GoInState() override;
        void GoOutState() override;
        bool ExecuteStateMsg(InternalMessage *msg) override;

    private:
        WifiControllerMachine *pWifiControllerMachine;
    };

    class EnableState : public State {
    public:
        explicit EnableState(WifiControllerMachine *wifiControllerMachine);
        ~EnableState() override;
        void GoInState() override;
        void GoOutState() override;
        bool ExecuteStateMsg(InternalMessage *msg) override;

    private:
        void HandleApStart(int id);
        void HandleWifiToggleChangeInEnabledState(InternalMessage *msg);
        void HandleSoftapToggleChangeInEnabledState(InternalMessage *msg);
        WifiControllerMachine *pWifiControllerMachine;
    };

    class DefaultState : public State {
    public:
        explicit DefaultState(WifiControllerMachine *wifiControllerMachine);
        ~DefaultState() override;
        void GoInState() override;
        void GoOutState() override;
        bool ExecuteStateMsg(InternalMessage *msg) override;

    private:
        WifiControllerMachine *pWifiControllerMachine;
    };

public:
    ErrCode InitWifiControllerMachine();

    void RmoveConcreteManager(int id);
    void RmoveSoftapManager(int id);
    void HandleStaClose(int id);
    void HandleStaStart(int id);
    void HandleStaStartFailure(int id);
    void HandleConcreteStop(int id);
    void HandleSoftapStop(int id);

private:
    template <typename T>
    inline void ParsePointer(T *&pointer)
    {
        if (pointer != nullptr) {
            delete pointer;
            pointer = nullptr;
        }
    }

    template <typename T>
    inline ErrCode JudgmentEmpty(T *&pointer)
    {
        if (pointer == nullptr) {
            return WIFI_OPT_FAILED;
        }
        return WIFI_OPT_SUCCESS;
    }

    void BuildStateTree();
    ErrCode InitWifiStates();
    bool HasAnyConcreteManager();
    bool HasAnySoftApManager();
    bool HasAnyManager();
    bool SoftApIdExit(int id);
    bool ConcreteIdExit(int id);
    void MakeConcreteManager(ConcreteManagerRole role, int id);
    void MakeSoftapManager(SoftApManager::Role role, int id);
    bool ShouldEnableWifi();
    ConcreteManagerRole GetWifiRole();
    void StopAllConcreteManagers();
    void StopAllSoftapManagers();
    void StopSoftapManager(int id);
    void SwitchRole(ConcreteManagerRole role);
    void HandleAirplaneOpen();
    void HandleAirplaneClose();
    static bool IsWifiEnable();
    static bool IsScanOnlyEnable();

    int mApidStopWifi;
    EnableState *pEnableState;
    DisableState *pDisableState;
    DefaultState *pDefaultState;
    std::vector<ConcreteClientModeManager *> concreteManagers;
    std::vector<SoftApManager *> softapManagers;
    ConcreteModeCallback mConcreteCallback;
    SoftApModeCallback mSoftapCallback;
    mutable std::mutex concreteManagerMutex;
    mutable std::mutex softapManagerMutex;
};
}  // namespace Wifi
}  // namespace OHOS
#endif // WIFICONTROLLER_WIFICONTROLLERMACHINE_H
