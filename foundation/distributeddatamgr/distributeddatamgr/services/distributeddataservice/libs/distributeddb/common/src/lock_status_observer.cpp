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

#include "lock_status_observer.h"

#include "log_print.h"

namespace DistributedDB {
LockStatusObserver::LockStatusObserver()
    : lockStatusChangedNotifier_(nullptr),
      isStarted_(false)
{}

LockStatusObserver::~LockStatusObserver()
{
    Stop();
}

int LockStatusObserver::Start()
{
    if (isStarted_) {
        return E_OK;
    }

    int errCode = PrepareNotifierChain();
    if (errCode != E_OK) {
        LOGE("PrepareNotifierChain failed, errorCode = %d", errCode);
        return errCode;
    }
    isStarted_ = true;
    return E_OK;
}

bool LockStatusObserver::IsStarted() const
{
    return isStarted_;
}

void LockStatusObserver::Stop()
{
    if (!isStarted_) {
        return;
    }

    lockStatusChangedNotifier_->UnRegisterEventType(LOCK_STATUS_CHANGE_EVENT);
    RefObject::KillAndDecObjRef(lockStatusChangedNotifier_);
    lockStatusChangedNotifier_ = nullptr;
    isStarted_ = false;
}

int LockStatusObserver::PrepareNotifierChain()
{
    if (lockStatusChangedNotifier_ != nullptr) {
        return E_OK;
    }

    lockStatusChangedNotifier_ = new (std::nothrow) NotificationChain();
    if (lockStatusChangedNotifier_ == nullptr) {
        LOGE("lockStatusChangedNotifier_ is nullptr");
        return -E_OUT_OF_MEMORY;
    }

    int errCode = lockStatusChangedNotifier_->RegisterEventType(LOCK_STATUS_CHANGE_EVENT);
    if (errCode != E_OK) {
        LOGE("RegisterEventType failed, errCode = %d", errCode);
        RefObject::KillAndDecObjRef(lockStatusChangedNotifier_);
        lockStatusChangedNotifier_ = nullptr;
    }
    return errCode;
}

NotificationChain::Listener *LockStatusObserver::RegisterLockStatusChangedLister(const LockStatusNotifier &action,
    int &errCode) const
{
    if (lockStatusChangedNotifier_ == nullptr) {
        LOGE("lockStatusChangedNotifier_ is nullptr");
        errCode = -E_NOT_INIT;
        return nullptr;
    }

    if (!action) {
        LOGE("action is nullptr");
        errCode = -E_INVALID_ARGS;
        return nullptr;
    }

    return lockStatusChangedNotifier_->RegisterListener(LOCK_STATUS_CHANGE_EVENT, action, nullptr, errCode);
}

void LockStatusObserver::OnStatusChange(bool isLocked) const
{
    if (lockStatusChangedNotifier_ == nullptr) {
        LOGE("lockStatusChangedNotifier_ is nullptr");
        return;
    }
    lockStatusChangedNotifier_->NotifyEvent(LOCK_STATUS_CHANGE_EVENT, &isLocked);
}
}
