/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include "state_machine.h"
#include "wifi_log.h"

#undef LOG_TAG
#define LOG_TAG "OHWIFI_STATE_MACHINE"

namespace OHOS {
namespace Wifi {
static const int SM_INIT_CMD = -2;
StateMachine::StateMachine(const std::string &name) : pStateMachineHandler(nullptr), mStateName(name)
{}

StateMachine::~StateMachine()
{
    LOGD("StateMachine::~StateMachine");
    if (pStateMachineHandler != nullptr) {
        delete pStateMachineHandler;
        pStateMachineHandler = nullptr;
    }
}

bool StateMachine::InitialStateMachine(const std::string &name)
{
    LOGI("InitialStateMachine\n");
    pStateMachineHandler = new (std::nothrow) StateMachineHandler(this);
    if (pStateMachineHandler == nullptr) {
        LOGE("pStateMachineHandler alloc failed.\n");
        return false;
    }

    if (!pStateMachineHandler->InitialSmHandler(name)) {
        LOGE("InitialStateMachineHandler failed.\n");
        return false;
    }

    return true;
}

void StateMachine::StartStateMachine()
{
    if (pStateMachineHandler == nullptr) {
        LOGE("StartStateMachine failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->BuildTreeComplete();
    return;
}

void StateMachine::SetHandler(StateMachineHandler *handler)
{
    pStateMachineHandler = handler;
}

void StateMachine::NotExecutedMessage(const InternalMessagePtr msg)
{
    if (msg == nullptr) {
        return;
    }
    LOGD("msg not handled  msg:%{public}d", msg->GetMessageName());
}

void StateMachine::StatePlus(State *state, State *upper)
{
    if (pStateMachineHandler == nullptr) {
        LOGE("StatePlus failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->StatePlus(state, upper);
}

void StateMachine::StateDelete(State *state)
{
    if (pStateMachineHandler == nullptr) {
        LOGE("StateDelete failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->StateDelete(state);
}

void StateMachine::SetFirstState(State *firstState)
{
    if (pStateMachineHandler == nullptr) {
        LOGE("SetFirstState failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->SetFirstState(firstState);
}

void StateMachine::SwitchState(State *targetState)
{
    if (pStateMachineHandler == nullptr) {
        LOGE("SwitchState failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->SwitchState(targetState);
}

void StateMachine::DelayMessage(const InternalMessagePtr msg)
{
    if (pStateMachineHandler == nullptr) {
        LOGE("DelayMessage failed, pStateMachineHandler is nullptr!");
        return;
    }

    pStateMachineHandler->DelayMessage(msg);
}

void StateMachine::StopHandlerThread()
{
    if (pStateMachineHandler == nullptr) {
        LOGE("StopHandlerThread failed, pStateMachineHandler is nullptr!");
        return;
    }
    pStateMachineHandler->StopHandlerThread();
}

InternalMessagePtr StateMachine::CreateMessage()
{
    return MessageManage::GetInstance().CreateMessage();
}

InternalMessagePtr StateMachine::CreateMessage(const InternalMessagePtr orig)
{
    if (orig == nullptr) {
        return nullptr;
    }
    return MessageManage::GetInstance().CreateMessage(orig);
}

InternalMessagePtr StateMachine::CreateMessage(int msgName)
{
    return MessageManage::GetInstance().CreateMessage(msgName);
}

InternalMessagePtr StateMachine::CreateMessage(int msgName, int param1)
{
    return MessageManage::GetInstance().CreateMessage(msgName, param1, 0);
}

InternalMessagePtr StateMachine::CreateMessage(int msgName, int param1, int param2)
{
    return MessageManage::GetInstance().CreateMessage(msgName, param1, param2);
}

InternalMessagePtr StateMachine::CreateMessage(int msgName, const std::any &messageObj)
{
    return MessageManage::GetInstance().CreateMessage(msgName, messageObj);
}

InternalMessagePtr StateMachine::CreateMessage(int msgName, int param1, int param2, const std::any &messageObj)
{
    return MessageManage::GetInstance().CreateMessage(msgName, param1, param2, messageObj);
}

void StateMachine::SendMessage(int msgName)
{
    pStateMachineHandler->SendMessage(CreateMessage(msgName));
    return;
}

void StateMachine::SendMessage(int msgName, int param1)
{
    pStateMachineHandler->SendMessage(CreateMessage(msgName, param1));
    return;
}

void StateMachine::SendMessage(int msgName, int param1, int param2)
{
    pStateMachineHandler->SendMessage(CreateMessage(msgName, param1, param2));
    return;
}

void StateMachine::SendMessage(InternalMessagePtr msg)
{
    if (msg == nullptr) {
        return;
    }
    if (pStateMachineHandler != nullptr) {
        pStateMachineHandler->SendMessage(msg);
    }
    return;
}

void StateMachine::SendMessage(int msgName, const std::any &messageObj)
{
    pStateMachineHandler->SendMessage(CreateMessage(msgName, messageObj));
    return;
}

void StateMachine::SendMessage(int msgName, int param1, int param2, const std::any &messageObj)
{
    pStateMachineHandler->SendMessage(CreateMessage(msgName, param1, param2, messageObj));
    return;
}

void StateMachine::MessageExecutedLater(int msgName, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(CreateMessage(msgName), delayTimeMs);
    return;
}

void StateMachine::MessageExecutedLater(int msgName, int param1, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(CreateMessage(msgName, param1), delayTimeMs);
    return;
}

void StateMachine::MessageExecutedLater(int msgName, int param1, int param2, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(CreateMessage(msgName, param1, param2), delayTimeMs);
    return;
}

void StateMachine::MessageExecutedLater(InternalMessagePtr msg, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(msg, delayTimeMs);
    return;
}

void StateMachine::MessageExecutedLater(int msgName, const std::any &messageObj, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(CreateMessage(msgName, messageObj), delayTimeMs);
    return;
}

void StateMachine::MessageExecutedLater(
    int msgName, int param1, int param2, const std::any &messageObj, int64_t delayTimeMs)
{
    pStateMachineHandler->MessageExecutedLater(CreateMessage(msgName, param1, param2, messageObj), delayTimeMs);
    return;
}

void StateMachine::SendMessageAtFrontOfQueue(int msgName, int param1)
{
    pStateMachineHandler->PlaceMessageTopOfQueue(CreateMessage(msgName, param1));
    return;
}

void StateMachine::StartTimer(int timerName, int64_t interval)
{
    LOGD("Enter StartTimer, timerName is %{public}d, interval is %" PRId64 ".", timerName, interval);
    MessageExecutedLater(timerName, interval);
    return;
}

void StateMachine::StopTimer(int timerName)
{
    LOGD("Enter StopTimer, timerName is %{public}d.", timerName);
    if (pStateMachineHandler != nullptr) {
        pStateMachineHandler->DeleteMessageFromQueue(timerName);
    }
    return;
}

std::string StateMachine::GetCurStateName()
{
    LOGD("GetCurStateName");
    if (pStateMachineHandler == nullptr) {
        LOGE("GetCurStateName failed, pStateMachineHandler is nullptr!");
        return "";
    }
    return pStateMachineHandler->GetCurStateName();
}

StateMachineHandler::StateMachineHandler(StateMachine *pStateMgr)
{
    mStateInfoMap.clear();
    mStateVector.clear();
    mStateVectorTopIndex = -1;
    mSequenceStateVector.clear();
    mSequenceStateVectorCount = 0;
    mDelayedMessages.clear();
    pStateMachine = pStateMgr;
    pFirstState = nullptr;
    pTargetState = nullptr;
    mQuitFlag = false;
    mBuildCompleteFlag = false;
    mSwitchingStateFlag = false;
    pCurrentMsg = nullptr;
}

StateMachineHandler::~StateMachineHandler()
{
    LOGI("~StateMachineHandler");
    StopHandlerThread();
    ReleaseDelayedMessages();
    ClearWhenQuit();
    return;
}

bool StateMachineHandler::InitialSmHandler(const std::string &name)
{
    if (!InitialHandler(name)) {
        LOGE("InitialHandler failed.");
        return false;
    }
    return true;
}

StateInfo *StateMachineHandler::StatePlus(State *state, State *upper)
{
    LOGD("Enter StateMachineHandler::StatePlus function.");

    StateInfo *upperStateInfo = nullptr;
    StateInfoMap::iterator it = mStateInfoMap.begin();
    if (upper != nullptr) {
        it = mStateInfoMap.find(upper->GetStateName());
        if (it != mStateInfoMap.end()) {
            upperStateInfo = it->second;
        }
        if (upperStateInfo == nullptr) {
            LOGD("upperStateInfo is null, add upper first. upper->GetStateName():%{public}s",
                upper->GetStateName().c_str());
            /* Recursively add our upper as it's not been added yet. */
            StatePlus(upper, nullptr);
        } else {
            LOGD("upperStateInfo is not null, go on.");
        }
    }

    StateInfo *stateInfo = nullptr;
    if (state == nullptr) {
        LOGE("state is null");
        return nullptr;
    } else {
        it = mStateInfoMap.find(state->GetStateName());
    }
    if (it != mStateInfoMap.end()) {
        stateInfo = it->second;
    }
    if (stateInfo == nullptr) {
        stateInfo = new (std::nothrow) StateInfo();
        if (stateInfo == nullptr) {
            LOGE("failed to new StateInfo!");
            return nullptr;
        }
        mStateInfoMap.insert(StateInfoMap::value_type(state->GetStateName(), stateInfo));
    }

    /* Validate that we aren't adding the same state in two different hierarchies. */
    if (stateInfo->upperStateInfo != nullptr && stateInfo->upperStateInfo != upperStateInfo) {
        LOGE("The same state cannot be added to two different hierarchies!");
    }

    stateInfo->state = state;
    stateInfo->upperStateInfo = upperStateInfo;
    stateInfo->active = false;

    LOGD("successfully added a new state!");

    return stateInfo;
}

void StateMachineHandler::StateDelete(State *state)
{
    StateInfoMap::iterator it = mStateInfoMap.find(state->GetStateName());
    StateInfo *stateInfo = nullptr;
    if (it != mStateInfoMap.end()) {
        stateInfo = it->second;
    }
    if (stateInfo == nullptr || stateInfo->active) {
        return;
    }

    it = mStateInfoMap.begin();
    while (it != mStateInfoMap.end()) {
        if (it->second->upperStateInfo == stateInfo) {
            return;
        }
        ++it;
    }

    it = mStateInfoMap.find(state->GetStateName());
    if (it != mStateInfoMap.end()) {
        delete it->second;
        it->second = nullptr;
        mStateInfoMap.erase(it);
    }
}

void StateMachineHandler::SetFirstState(State *firstState)
{
    pFirstState = firstState;
}

void StateMachineHandler::BuildTreeComplete()
{
    /* Determines the maximum depth of the state hierarchy. */
    int maxDepth = 0;
    StateInfoMap::iterator it = mStateInfoMap.begin();
    while (it != mStateInfoMap.end()) {
        int depth = 0;
        StateInfo *tempStateInfo = it->second;
        while (tempStateInfo != nullptr) {
            depth++;
            tempStateInfo = tempStateInfo->upperStateInfo;
        }

        if (maxDepth < depth) {
            maxDepth = depth;
        }

        ++it;
    }

    LOGD("StateMachineHandler::BuildTreeComplete, maxDepth:%{public}d", maxDepth);
    mStateVector.resize(maxDepth);
    mSequenceStateVector.resize(maxDepth);
    BuildStateInitVector();
    MessageExecutedAtTime(pStateMachine->CreateMessage(SM_INIT_CMD), 0);
    return;
}

void StateMachineHandler::BuildStateInitVector()
{
    LOGD("StateMachineHandler::BuildStateInitVector");

    if (pFirstState == nullptr) {
        LOGE("StateMachineHandler::BuildStateInitVector  please set initial state first!");
        return;
    }

    StateInfoMap::iterator it = mStateInfoMap.find(pFirstState->GetStateName());
    StateInfo *startStateInfo = nullptr;
    if (it != mStateInfoMap.end()) {
        startStateInfo = it->second;
    }

    for (mSequenceStateVectorCount = 0; startStateInfo != nullptr; mSequenceStateVectorCount++) {
        mSequenceStateVector[mSequenceStateVectorCount] = startStateInfo;
        startStateInfo = startStateInfo->upperStateInfo;
    }

    /* Clearing the StateVector. */
    mStateVectorTopIndex = -1;
    MoveSequenceToStateVector();
}

StateInfo *StateMachineHandler::BuildSequenceStateVector(State *targetState)
{
    mSequenceStateVectorCount = 0;
    StateInfoMap::iterator it = mStateInfoMap.find(targetState->GetStateName());
    StateInfo *curStateInfo = nullptr;
    if (it != mStateInfoMap.end()) {
        curStateInfo = it->second;
    }

    if (curStateInfo == nullptr) {
        return nullptr;
    }

    do {
        mSequenceStateVector[mSequenceStateVectorCount++] = curStateInfo;
        curStateInfo = curStateInfo->upperStateInfo;
    } while ((curStateInfo != nullptr) && (!curStateInfo->active));

    return curStateInfo;
}

void StateMachineHandler::PlaceDelayedMsgQueueTop()
{
    LOGD("Enter StateMachineHandler::PlaceDelayedMsgQueueTop.");
    if (mDelayedMessages.size() == 0) {
        return;
    }
    for (int i = mDelayedMessages.size() - 1; i >= 0; i--) {
        InternalMessagePtr curMsg = mDelayedMessages[i];
        if (curMsg == nullptr) {
            LOGE("StateMachineHandler::PlaceDelayedMsgQueueTop: curMsg is null.");
            continue;
        }
        PlaceMessageTopOfQueue(curMsg);
    }
    mDelayedMessages.clear();

    return;
}

void StateMachineHandler::ReleaseDelayedMessages()
{
    if (mDelayedMessages.size() == 0) {
        return;
    }
    for (int i = mDelayedMessages.size() - 1; i >= 0; i--) {
        InternalMessagePtr curMsg = mDelayedMessages[i];
        if (curMsg != nullptr) {
            curMsg = nullptr;
        }
    }
    mDelayedMessages.clear();

    return;
}

int StateMachineHandler::MoveSequenceToStateVector()
{
    LOGD("StateMachineHandler::MoveSequenceToStateVector mSequenceStateVectorCount:%{public}d",
        mSequenceStateVectorCount);

    int newIndex = mStateVectorTopIndex + 1;
    int i = mSequenceStateVectorCount - 1;
    int j = newIndex;
    while (i >= 0) {
        mStateVector[j] = mSequenceStateVector[i];
        j += 1;
        i -= 1;
    }

    mStateVectorTopIndex = j - 1;
    return newIndex;
}

void StateMachineHandler::SwitchState(State *targetState)
{
    if (targetState == nullptr) {
        LOGE("targetState is null.");
        return;
    }
    LOGE("SwitchState, Switch to targetState: %{public}s.", targetState->GetStateName().c_str());
    pTargetState = static_cast<State *>(targetState);
}

void StateMachineHandler::ClearWhenQuit()
{
    pStateMachine->SetHandler(nullptr);
    pStateMachine = nullptr;
    pCurrentMsg = nullptr;
    mStateVector.clear();
    mSequenceStateVector.clear();
    mDelayedMessages.clear();
    pFirstState = nullptr;
    pTargetState = nullptr;
    mQuitFlag = true;

    StateInfoMap::iterator it = mStateInfoMap.begin();
    while (it != mStateInfoMap.end()) {
        delete it->second;
        it->second = nullptr;
        it = mStateInfoMap.erase(it);
    }
    mStateInfoMap.clear();
}

void StateMachineHandler::PerformSwitchState(State *msgProcessedState, InternalMessagePtr msg)
{
    if (msgProcessedState == nullptr || msg == nullptr) {
        LOGE("pointer is null.");
    }

    State *targetState = pTargetState;
    if (targetState != nullptr) {
        if (pFirstState != nullptr) {
            LOGD("StateMachineHandler::PerformSwitchState, Switch %{public}s -->> %{public}s",
                pFirstState->GetStateName().c_str(), targetState->GetStateName().c_str());
        }
        while (true) {
            StateInfo *commonStateInfo = BuildSequenceStateVector(targetState);
            mSwitchingStateFlag = true;
            CallTreeStateExits(commonStateInfo);

            int stateListEnteringIndex = MoveSequenceToStateVector();
            CallTreeStateEnters(stateListEnteringIndex);

            PlaceDelayedMsgQueueTop();

            if (targetState != pTargetState) {
                targetState = pTargetState;
            } else {
                break;
            }
        }
        pTargetState = nullptr;
    }

    return;
}

void StateMachineHandler::ExecuteMessage(InternalMessagePtr msg)
{
    if (msg == nullptr) {
        return;
    }
    if (!mQuitFlag) {
        if (pStateMachine != nullptr && msg->GetMessageName() != SM_INIT_CMD) {
        }

        pCurrentMsg = msg;

        State *msgProcessedState = nullptr;
        if (mBuildCompleteFlag) {
            LOGD("StateMachineHandler::ExecuteMessage  ExecuteTreeStateMsg!");
            msgProcessedState = ExecuteTreeStateMsg(msg);
        } else if (!mBuildCompleteFlag && msg->GetMessageName() == SM_INIT_CMD) {
            LOGD("StateMachineHandler::ExecuteMessage  msg: SM_INIT_CMD");
            mBuildCompleteFlag = true;
            CallTreeStateEnters(0);
        } else {
            LOGE("The start method not called!");
        }

        if (pStateMachine != nullptr) {
            PerformSwitchState(msgProcessedState, msg);
        } else {
            LOGE("pointer is null.");
        }

        if (pStateMachine != nullptr && msg->GetMessageName() != SM_INIT_CMD) {
        }
    }

    return;
}

void StateMachineHandler::DelayMessage(const InternalMessagePtr msg)
{
    LOGD("Enter StateMachineHandler::DelayMessage.");
    if (msg == nullptr) {
        return;
    }

    InternalMessagePtr newMsg = pStateMachine->CreateMessage(msg);
    if (newMsg == nullptr) {
        LOGE("StateMachineHandler::DelayMessage: newMsg is null.");
        return;
    }
    mDelayedMessages.push_back(newMsg);
    return;
}

State *StateMachineHandler::ExecuteTreeStateMsg(InternalMessagePtr msg)
{
    LOGD("StateMachineHandler::ExecuteTreeStateMsg mStateVectorTopIndex:%{public}d", mStateVectorTopIndex);
    if (msg == nullptr) {
        LOGE("ExecuteTreeStateMsg, msg is nullptr.");
        return nullptr;
    }

    StateInfo *curStateInfo = mStateVector[mStateVectorTopIndex];
    if (curStateInfo == nullptr) {
        LOGE("ExecuteTreeStateMsg, curStateInfo is nullptr.");
        return nullptr;
    }

    if (curStateInfo->state) {
        LOGD("ExecuteTreeStateMsg, State machine: %{public}s execute Cmd:%{public}d",
            curStateInfo->state->GetStateName().c_str(), msg->GetMessageName());
    }

    while (curStateInfo->state && (!curStateInfo->state->ExecuteStateMsg(msg))) {
        curStateInfo = curStateInfo->upperStateInfo;

        if (curStateInfo == nullptr) {
            pStateMachine->NotExecutedMessage(msg);
            break;
        }
    }

    return (curStateInfo != nullptr) ? curStateInfo->state : nullptr;
}

void StateMachineHandler::CallTreeStateExits(StateInfo *commonStateInfo)
{
    while ((mStateVectorTopIndex >= 0) && (mStateVector[mStateVectorTopIndex] != commonStateInfo)) {
        if (mStateVector[mStateVectorTopIndex] != nullptr) {
            State *curState = mStateVector[mStateVectorTopIndex]->state;
            if (curState != nullptr) {
                curState->GoOutState();
            }
            mStateVector[mStateVectorTopIndex]->active = false;
        }
        mStateVectorTopIndex -= 1;
    }
}

void StateMachineHandler::CallTreeStateEnters(int index)
{
    for (int i = index; i <= mStateVectorTopIndex; i++) {
        if (index == mStateVectorTopIndex) {
            /* Last enter state for transition. */
            mSwitchingStateFlag = false;
        }
        LOGD("StateMachineHandler::CallTreeStateEnters  mStateVectorTopIndex:%{public}d, i: %{public}d",
            mStateVectorTopIndex,
            i);
        if (mStateVector[i] != nullptr && mStateVector[i]->state != nullptr) {
            mStateVector[i]->state->GoInState();
            mStateVector[i]->active = true;
        }
    }
    /* ensure flag set to false if no methods called. */
    mSwitchingStateFlag = false;
}

std::string StateMachineHandler::GetCurStateName()
{
    StateInfo *curStateInfo = mStateVector[mStateVectorTopIndex];
    if (curStateInfo == nullptr) {
        LOGE("StateInfo is null.");
        return "";
    }
    return curStateInfo->state->GetStateName();
}
}  // namespace Wifi
}  // namespace OHOS
