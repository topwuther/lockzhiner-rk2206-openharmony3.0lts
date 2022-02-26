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

#ifndef NOTIFICATION_CHAIN_H
#define NOTIFICATION_CHAIN_H

#include <map>
#include <set>
#include <thread>
#include <condition_variable>

#include "ref_object.h"

namespace DistributedDB {
using EventType = unsigned int;

class NotificationChain final : public RefObject {
private:
    class ListenerChain;

public:
    class Listener final : public RefObject {
    public:
        using OnEvent = std::function<void(void *)>;
        using OnFinalize = std::function<void(void)>;

        // Called by ListenerChain.callbackListeners, it will call the OnEvent
        void NotifyListener(void *arg);

        // Drop this listener. after call this function, the listener will be destroy
        int Drop(bool wait = false);

        // Enter kill-waiting state if 'onEvent()' is invoking when 'KillObj()'.
        void KillWait();

        // Set the listener chain we belong to.
        void SetOwner(ListenerChain *listenerChain);

        Listener(const OnEvent &onEvent, const OnFinalize &onFinalize);

        // Delete the copy and assign constructors
        DISABLE_COPY_ASSIGN_MOVE(Listener);

    protected:
        ~Listener() override;

    private:
        // will be call when this listener destroy
        void Finalize() const;
        bool EnterEventAction();
        void LeaveEventAction();

        DECLARE_OBJECT_TAG(Listener);

        constexpr static int KILL_WAIT_SECONDS = 5; // wait only 5 seconds when killing to avoid dead-lock.
        OnEvent onEvent_;
        OnFinalize onFinalize_;
        ListenerChain *listenerChain_;
        std::thread::id eventRunningThread_;
        std::condition_variable safeKill_;
    };

    // Add a listener from the NotificationChain. it will return a Listener handle
    // The param type should match the RegisterEventsType
    // The param onEvent will be call when events happened.
    // The param onFinalize will be call when this listener destroy
    Listener *RegisterListener(EventType type, const Listener::OnEvent &onEvent,
        const Listener::OnFinalize &onFinalize, int &errCode);

    // User to register an events type to the NotificationChain, needs to call at init
    int RegisterEventType(EventType type);

    // User to unregister an events type.
    int UnRegisterEventType(EventType type);

    // Should be call when events happened.
    void NotifyEvent(EventType type, void *arg);

    NotificationChain() = default;

    // Delete the copy and assign constructors
    DISABLE_COPY_ASSIGN_MOVE(NotificationChain);

protected:
    ~NotificationChain() override;

private:
    class ListenerChain final : public RefObject {
    public:
        // Add a listener to the ListenerChain
        int RegisterListener(Listener *listener);

        // Remove a listener to the ListenerChain
        int UnRegisterListener(Listener *listener, bool wait = false);

        // Callback all the listeners
        void NotifyListeners(void *arg);

        // Clear all listeners
        void ClearListeners();

        ListenerChain();

        // Delete the copy and assign constructors
        DISABLE_COPY_ASSIGN_MOVE(ListenerChain);
    protected:
        ~ListenerChain() override;

    private:
        // Used to back up listenerSet_, need to lock
        void BackupListenerSet(std::set<Listener *> &backupSet) const;

        DECLARE_OBJECT_TAG(ListenerChain);

        std::set<Listener *> listenerSet_;
    };

    // Find a ListenerChain from  the eventChains_ with given type,
    // this function needs to lock.
    ListenerChain *FindAndGetListenerChainLocked(EventType type);

    // Find a ListenerChain from  the eventChains_ with given type,
    ListenerChain *FindListenerChain(EventType type) const;

    DECLARE_OBJECT_TAG(NotificationChain);

    std::map<EventType, ListenerChain *> eventChains_;
};
} // namespace DistributedDB

#endif // NOTIFICATION_CHAIN_H
