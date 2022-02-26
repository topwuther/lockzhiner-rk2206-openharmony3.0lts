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

#ifndef REMOTE_OBSERVER_LIST_H
#define REMOTE_OBSERVER_LIST_H

#include "bluetooth_log.h"
#include <functional>
#include <map>
#include <mutex>

namespace OHOS {
namespace Bluetooth {
template <typename T>
class RemoteObserverList final {
public:
    RemoteObserverList() = default;
    ~RemoteObserverList();

    bool Register(const sptr<T> &observer);
    bool Deregister(const sptr<T> &observer);

    void ForEach(const std::function<void(sptr<T>)> &observer);

    class ObserverDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        ObserverDeathRecipient(const sptr<T> &observer, RemoteObserverList *owner);

        sptr<T> GetObserver() const
        {
            return observer_;
        };

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        sptr<T> observer_{};
        RemoteObserverList<T> *owner_{};
    };

    using ObserverMap = std::map<sptr<T>, sptr<ObserverDeathRecipient>>;
    std::mutex lock_{};
    ObserverMap observers_{};

    bool UnregisterInternal(typename ObserverMap::iterator iter);

    RemoteObserverList(const RemoteObserverList &) = delete;
    RemoteObserverList &operator=(const RemoteObserverList &) = delete;
};

template <typename T>
RemoteObserverList<T>::~RemoteObserverList()
{
    HILOGI("RemoteObserverList<T>::~RemoteObserverList() called");
    std::lock_guard<std::mutex> lock(lock_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        observers_.erase(it);
    }
    observers_.clear();
}

template <typename T>
bool RemoteObserverList<T>::Register(const sptr<T> &observer)
{
    HILOGI("RemoteObserverList<T>::Register called");
    std::lock_guard<std::mutex> lock(lock_);

    if (observers_.find(observer) != observers_.end()) {
        HILOGI("Observer list already contains given observer");
        return false;
    }

    sptr<ObserverDeathRecipient> dr(new ObserverDeathRecipient(observer, this));

    if (!observer->AsObject()->AddDeathRecipient(dr)) {
        HILOGE("Failed to link death recipient to observer");
        return false;
    }

    observers_[observer] = dr;

    return true;
}

template <typename T>
bool RemoteObserverList<T>::Deregister(const sptr<T> &observer)
{
    HILOGI("RemoteObserverList<T>::Deregister called");
    std::lock_guard<std::mutex> lock(lock_);

    auto iter = observers_.find(observer);
    if (iter == observers_.end()) {
        HILOGW("Given observer not registered with this list");
        return false;
    }
    return UnregisterInternal(iter);
}

template <typename T>
void RemoteObserverList<T>::ForEach(const std::function<void(sptr<T>)> &observer)
{
    HILOGI("RemoteObserverList<T>::ForEach called");
    std::lock_guard<std::mutex> lock(lock_);
    for (const auto &it : observers_) {
        observer(it.first);
    }
}

template <typename T>
RemoteObserverList<T>::ObserverDeathRecipient::ObserverDeathRecipient(
    const sptr<T> &observer, RemoteObserverList<T> *owner)
    : observer_(observer), owner_(owner)
{
    HILOGI("RemoteObserverList<T>::ObserverDeathRecipient::ObserverDeathRecipient called");
}

template <typename T>
void RemoteObserverList<T>::ObserverDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    // Remove the observer but no need to call unlinkToDeath.
    std::lock_guard<std::mutex> lock(owner_->lock_);

    auto iter = owner_->observers_.find(GetObserver());
    owner_->observers_.erase(iter);
    HILOGI("Callback from dead process unregistered");
}

template <typename T>
bool RemoteObserverList<T>::UnregisterInternal(typename ObserverMap::iterator iter)
{
    HILOGI("RemoteObserverList<T>::UnregisterInternal called");
    sptr<ObserverDeathRecipient> dr = iter->second;
    observers_.erase(iter);

    if (!dr->GetObserver()->AsObject()->RemoveDeathRecipient(dr)) {
        HILOGE("Failed to unlink death recipient from binder");
        return false;
    }

    return true;
}

}  // namespace Bluetooth
}  // namespace OHOS

#endif  // REMOTE_OBSERVER_LIST_H