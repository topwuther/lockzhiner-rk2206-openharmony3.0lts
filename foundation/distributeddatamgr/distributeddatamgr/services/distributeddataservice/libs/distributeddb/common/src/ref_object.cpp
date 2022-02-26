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

#include "ref_object.h"
#include "log_print.h"

namespace DistributedDB {
constexpr static int MAX_REF_COUNT = 1024;

RefObject::AutoLock::AutoLock(const RefObject *obj, bool unlocked)
    : refObj_(obj),
      IsLocked_(false)
{
    if (refObj_ != nullptr) {
        if (unlocked) {
            refObj_->LockObj();
        }
        IsLocked_ = true;
    }
}

void RefObject::AutoLock::Lock()
{
    if (refObj_ != nullptr) {
        if (!IsLocked_) {
            refObj_->LockObj();
            IsLocked_ = true;
        } else {
            LOGE("RefObject-AutoLock: obj' acquires lock more than once.");
        }
    }
}

void RefObject::AutoLock::Unlock()
{
    if (refObj_ != nullptr) {
        if (IsLocked_) {
            refObj_->UnlockObj();
            IsLocked_ = false;
        } else {
            LOGE("RefObject-AutoLock: obj releases lock more than once.");
        }
    }
}

RefObject::AutoLock::~AutoLock()
{
    if (refObj_ != nullptr) {
        if (IsLocked_) {
            refObj_->UnlockObj();
            IsLocked_ = false;
        }
        refObj_ = nullptr;
    }
}

RefObject::RefObject()
    : refCount_(1),
      isKilled_(false)
{}

RefObject::~RefObject()
{
    int refCount = refCount_.load(std::memory_order_seq_cst);
    if (refCount > 0) {
        LOGF("object is destructed with ref-count > 0., refCount = %d", refCount);
    }
}

void RefObject::OnLastRef(const std::function<void(void)> &callback) const
{
    if (onLast_) {
        std::string tag = GetObjectTag();
        LOGW("%s object set 'OnLastRef()' callback twice.", tag.c_str());
        return;
    }
    onLast_ = callback;
}

void RefObject::OnKill(const std::function<void(void)> &callback)
{
    if (onKill_) {
        std::string tag = GetObjectTag();
        LOGW("%s object set 'OnKill()' callback twice.", tag.c_str());
        return;
    }
    onKill_ = callback;
}

bool RefObject::IsKilled() const
{
    return isKilled_;
}

void RefObject::KillObj()
{
    std::lock_guard<std::mutex> lockGuard(objLock_);
    if (!IsKilled()) {
        isKilled_ = true;
        if (onKill_) {
            onKill_();
        }
    }
}

void RefObject::LockObj() const
{
    objLock_.lock();
}

void RefObject::UnlockObj() const
{
    objLock_.unlock();
}

bool RefObject::WaitLockedUntil(std::condition_variable &cv,
    const std::function<bool(void)> &condition, int seconds)
{
    // Enter with lock held.
    if (!condition) {
        return false;
    }

    bool waitOk = true;
    {
        std::unique_lock<std::mutex> lock(objLock_, std::adopt_lock_t());
        while (!condition()) {
            if (seconds > 0) {
                cv.wait_for(lock, std::chrono::seconds(seconds));
                waitOk = condition();
                break;
            } else {
                cv.wait(lock);
            }
        }
    }

    // Lock has just been dropped in unique_lock::~unique_lock(),
    // so we lock it again.
    LockObj();
    return waitOk;
}

void RefObject::IncObjRef(const RefObject *obj)
{
    if (obj == nullptr) {
        return;
    }
    int refCount = obj->refCount_.fetch_add(1, std::memory_order_seq_cst);
    if ((refCount <= 0) || (refCount >= MAX_REF_COUNT)) {
        std::string tag = obj->GetObjectTag();
        LOGF("%s object is refed with ref-count=%d.", tag.c_str(), refCount);
    }
}

void RefObject::DecObjRef(const RefObject *obj)
{
    if (obj == nullptr) {
        return;
    }
    int refCount = obj->refCount_.fetch_sub(1, std::memory_order_seq_cst);
    if (refCount <= 0) {
        std::string tag = obj->GetObjectTag();
        LOGF("%s object is unrefed with ref-count(%d) <= 0.", tag.c_str(), refCount);
    } else {
        if (refCount == 1) {
            if (obj->onLast_) {
                obj->onLast_();
            }
            delete obj;
        }
    }
}

void RefObject::KillAndDecObjRef(RefObject *obj)
{
    if (obj == nullptr) {
        return;
    }
    obj->KillObj();
    obj->DecObjRef(obj);
    obj = nullptr;
}

DEFINE_OBJECT_TAG_FACILITIES(RefObject)
} // namespace DistributedDB
