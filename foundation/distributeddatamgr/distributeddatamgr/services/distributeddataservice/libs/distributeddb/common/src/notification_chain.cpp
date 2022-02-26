/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "notification_chain.h"

#include <algorithm>
#include <functional>

#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
NotificationChain::Listener *NotificationChain::RegisterListener(
    EventType type, const Listener::OnEvent &onEvent, const Listener::OnFinalize &onFinalize, int &errCode)
{
    errCode = E_OK;
    if (!onEvent) {
        LOGE("[NotificationChain] Register listener failed, 'onEvent()' is null!");
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }

    NotificationChain::ListenerChain *listenerChain = FindAndGetListenerChainLocked(type);
    if (listenerChain == nullptr) {
        LOGE("[NotificationChain] Register listener failed, no event type %u found!", type);
        errCode = -E_NOT_REGISTER;
        return nullptr;
    }

    NotificationChain::Listener *listener = new (std::nothrow)
        NotificationChain::Listener(onEvent, onFinalize);
    if (listener == nullptr) {
        listenerChain->DecObjRef(listenerChain);
        listenerChain = nullptr;
        errCode = -E_OUT_OF_MEMORY;
        return nullptr;
    }

    errCode = listenerChain->RegisterListener(listener);
    if (errCode != E_OK) {
        LOGE("[NotificationChain] Register listener failed, event type %u has been unregistered!", type);
        listener->DecObjRef(listener);
        listener = nullptr;
        listenerChain->DecObjRef(listenerChain);
        listenerChain = nullptr;
        return nullptr;
    }

    listenerChain->DecObjRef(listenerChain);
    listenerChain = nullptr;
    return listener;
}

int NotificationChain::RegisterEventType(EventType type)
{
    AutoLock lockGuard(this);
    if (IsKilled()) {
        LOGI("Register event failed, the notification chain has been killed!");
        return -E_STALE;
    }

    ListenerChain *listenerChain = FindListenerChain(type);
    if (listenerChain != nullptr) {
        LOGE("[NotificationChain] Register event failed, event type %u has been registered!", type);
        return -E_ALREADY_REGISTER;
    }

    listenerChain = new (std::nothrow) ListenerChain();
    if (listenerChain == nullptr) {
        LOGE("[NotificationChain] Register event failed, OOM!");
        return -E_OUT_OF_MEMORY;
    }

    listenerChain->OnKill([listenerChain] {
        listenerChain->ClearListeners();
    });
    eventChains_.insert(std::pair<EventType, ListenerChain *>(type, listenerChain));
    IncObjRef(this);
    return E_OK;
}

int NotificationChain::UnRegisterEventType(EventType type)
{
    NotificationChain::ListenerChain *listenerChain = nullptr;
    {
        AutoLock lockGuard(this);
        listenerChain = FindListenerChain(type);
        if (listenerChain == nullptr) {
            LOGE("[NotificationChain] UnRegister event failed, event %u is not registered!", type);
            return -E_NOT_FOUND;
        }
        eventChains_.erase(type);
    }

    listenerChain->KillAndDecObjRef(listenerChain);
    listenerChain = nullptr;
    DecObjRef(this);
    return E_OK;
}

void NotificationChain::NotifyEvent(EventType type, void *arg)
{
    NotificationChain::ListenerChain *listenerChain = FindAndGetListenerChainLocked(type);
    if (listenerChain == nullptr) {
        return;
    }
    listenerChain->NotifyListeners(arg);
    listenerChain->DecObjRef(listenerChain);
    listenerChain = nullptr;
}

NotificationChain::ListenerChain::ListenerChain() {}

NotificationChain::ListenerChain::~ListenerChain() {}

NotificationChain::ListenerChain *NotificationChain::FindAndGetListenerChainLocked(EventType type)
{
    AutoLock lockGuard(this);
    ListenerChain *listenerChain = FindListenerChain(type);
    if (listenerChain == nullptr) {
        return nullptr;
    }
    listenerChain->IncObjRef(listenerChain);
    return listenerChain;
}

NotificationChain::ListenerChain *NotificationChain::FindListenerChain(EventType type) const
{
    auto iter = eventChains_.find(type);
    if (iter != eventChains_.end()) {
        return iter->second;
    }
    return nullptr;
}

int NotificationChain::ListenerChain::RegisterListener(Listener *listener)
{
    AutoLock lockGuard(this);
    if (IsKilled()) {
        return -E_STALE;
    }
    if (listenerSet_.find(listener) != listenerSet_.end()) {
        return -E_ALREADY_REGISTER;
    }
    listenerSet_.insert(listener);
    listener->SetOwner(this);
    return E_OK;
}

int NotificationChain::ListenerChain::UnRegisterListener(Listener *listener, bool wait)
{
    if (listener == nullptr) {
        return -E_INVALID_ARGS;
    }

    {
        AutoLock lockGuard(this);
        auto result = listenerSet_.find(listener);
        if (result != listenerSet_.end()) {
            if (wait) {
                listener->OnKill([listener]() {
                    listener->KillWait();
                });
            }
            listenerSet_.erase(result);
        }
    }

    listener->KillAndDecObjRef(listener);
    listener = nullptr;
    return E_OK;
}

void NotificationChain::ListenerChain::BackupListenerSet(std::set<Listener *> &backupSet) const
{
    for (auto listener : listenerSet_) {
        listener->IncObjRef(listener);
        backupSet.insert(listener);
    }
}

void NotificationChain::ListenerChain::NotifyListeners(void *arg)
{
    std::set<Listener *> tmpSet;
    {
        AutoLock lockGuard(this);
        if (IsKilled()) {
            return;
        }
        BackupListenerSet(tmpSet);
    }

    for (auto listener : tmpSet) {
        if (listener != nullptr) {
            listener->NotifyListener(arg);
            listener->DecObjRef(listener);
            listener = nullptr;
        }
    }
}

void NotificationChain::ListenerChain::ClearListeners()
{
    std::set<Listener *> tmpSet;
    BackupListenerSet(tmpSet);
    listenerSet_.clear();
    // Enter this function with lock held(OnKill() is invoked with object lock held), so drop it.
    UnlockObj();

    for (auto listener : tmpSet) {
        // Drop the ref 1 which increased in 'BackupListenerSet()',
        // the origal 1 will be dropped when user call listener->Drop();
        listener->KillAndDecObjRef(listener);
        listener = nullptr;
    }

    // Lock it again before leaving.
    LockObj();
}

void NotificationChain::Listener::NotifyListener(void *arg)
{
    if (onEvent_ && !IsKilled()) {
        if (EnterEventAction()) {
            onEvent_(arg);
            LeaveEventAction();
        }
    }
}

void NotificationChain::Listener::Finalize() const
{
    if (onFinalize_) {
        onFinalize_();
    }
}

bool NotificationChain::Listener::EnterEventAction()
{
    AutoLock lockGuard(this);
    if (IsKilled()) {
        return false;
    }
    // We never call onEvent() of the same listener in parallel with 2 or more threads.
    eventRunningThread_ = std::this_thread::get_id();
    return true;
}

void NotificationChain::Listener::LeaveEventAction()
{
    AutoLock lockGuard(this);
    eventRunningThread_ = std::thread::id();
    safeKill_.notify_one();
}

void NotificationChain::Listener::KillWait()
{
    // We entered with object lock held.
    if ((eventRunningThread_ == std::thread::id()) ||
        (eventRunningThread_ == std::this_thread::get_id())) {
        return;
    }

    LOGW("[NotificationChain] Try to kill a active event listener, now wait.");
    bool noDeadLock = WaitLockedUntil(safeKill_, [this]() {
            if (eventRunningThread_ == std::thread::id()) {
                return true;
            }
            return false;
        }, KILL_WAIT_SECONDS);
    if (!noDeadLock) {
        LOGE("[NotificationChain] Dead lock maybe happen, we stop waiting the listener.");
    } else {
        LOGW("[NotificationChain] Wait the active event listener ok.");
    }
}

void NotificationChain::Listener::SetOwner(ListenerChain *listenerChain)
{
    if (listenerChain_ != nullptr) {
        listenerChain_->DecObjRef(listenerChain_);
    }
    listenerChain_ = listenerChain;
    if (listenerChain_ != nullptr) {
        listenerChain_->IncObjRef(listenerChain_);
    }
}

int NotificationChain::Listener::Drop(bool wait)
{
    if (listenerChain_ == nullptr) {
        LOGE("[NotificationChain] Drop listener failed, lost the chain!");
        return -E_INTERNAL_ERROR;
    }
    return listenerChain_->UnRegisterListener(this, wait);
}

NotificationChain::Listener::Listener(const OnEvent &onEvent, const OnFinalize &onFinalize)
    : onEvent_(onEvent),
      onFinalize_(onFinalize),
      listenerChain_(nullptr)
{
    OnLastRef([this]() {
        this->Finalize();
    });
}

NotificationChain::Listener::~Listener()
{
    SetOwner(nullptr);
}

NotificationChain::~NotificationChain()
{
    for (auto &iter : eventChains_) {
        iter.second->KillAndDecObjRef(iter.second);
        iter.second = nullptr;
    }
    eventChains_.clear();
}

DEFINE_OBJECT_TAG_FACILITIES(NotificationChain)
DEFINE_OBJECT_TAG_FACILITIES(NotificationChain::Listener)
DEFINE_OBJECT_TAG_FACILITIES(NotificationChain::ListenerChain)
} // namespace DistributedDB