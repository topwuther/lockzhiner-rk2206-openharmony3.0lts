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

#include "refbase.h"

namespace OHOS {

WeakRefCounter::WeakRefCounter(RefCounter *counter, void *cookie)
    : atomicWeak_(0), refCounter_(counter), cookie_(cookie)
{
    if (refCounter_ != nullptr) {
        refCounter_->IncRefCount();
    }
}

WeakRefCounter::~WeakRefCounter()
{
    if (refCounter_ != nullptr) {
        refCounter_->DecRefCount();
    }
}

void* WeakRefCounter::GetRefPtr()
{
    if ((cookie_ != nullptr) && (!refCounter_->IsRefPtrValid())){
        cookie_ = nullptr;
    }
    return cookie_;
}

void WeakRefCounter::IncWeakRefCount(const void *objectId)
{
    if (atomicWeak_.fetch_add(1, std::memory_order_relaxed) == 0) {
        refCounter_->IncWeakRefCount(objectId);
    }
}

void WeakRefCounter::DecWeakRefCount(const void *objectId)
{
    if (atomicWeak_.fetch_sub(1, std::memory_order_release) == 1) {
        refCounter_->DecWeakRefCount(objectId);
        delete this;
    }
}

bool WeakRefCounter::AttemptIncStrongRef(const void *objectId)
{
    int unuse = 0;
    return refCounter_->AttemptIncStrongRef(objectId, unuse);
}

RefCounter::RefCounter()
    : atomicStrong_(INITIAL_PRIMARY_VALUE), atomicWeak_(0), atomicRefCount_(0), atomicFlags_(0), atomicAttempt_(0)
{
}
int RefCounter::GetRefCount()
{
    return atomicRefCount_.load(std::memory_order_relaxed);
}

void RefCounter::IncRefCount()
{
    atomicRefCount_.fetch_add(1, std::memory_order_relaxed);
}

void RefCounter::DecRefCount()
{
    if (atomicRefCount_.load(std::memory_order_relaxed) > 0) {
        if (atomicRefCount_.fetch_sub(1, std::memory_order_release) == 1) {
            delete (this);
        }
    }
}

void RefCounter::SetCallback(const RefPtrCallback& callback)
{
    callback_ = callback;
}

void RefCounter::RemoveCallback()
{
    callback_ = nullptr;
}

bool RefCounter::IsRefPtrValid()
{
    return callback_ != nullptr;
}

RefCounter::~RefCounter()
{
}

int RefCounter::IncStrongRefCount(const void* /*objectId*/)
{
    int curCount = atomicStrong_.load(std::memory_order_relaxed);

    if (curCount >= 0) {
        curCount = atomicStrong_.fetch_add(1, std::memory_order_relaxed);
        if (curCount == INITIAL_PRIMARY_VALUE) {
            atomicStrong_.fetch_sub(INITIAL_PRIMARY_VALUE, std::memory_order_release);
        }
    }

    return curCount;
}

int RefCounter::DecStrongRefCount(const void* /*objectId*/)
{
    int curCount = GetStrongRefCount();
    if (curCount == INITIAL_PRIMARY_VALUE) {
        // unexpected case: there had never a strong reference.
    } else if (curCount > 0) {
        // we should update the current count here.
        // it may be changed after last operation.
        curCount = atomicStrong_.fetch_sub(1, std::memory_order_release);
    }

    return curCount;
}

int RefCounter::GetStrongRefCount()
{
    return atomicStrong_.load(std::memory_order_relaxed);
}

int RefCounter::IncWeakRefCount(const void* /*objectId*/)
{
    return atomicWeak_.fetch_add(1, std::memory_order_relaxed);
}

int RefCounter::DecWeakRefCount(const void* /*objectId*/)
{
    int curCount = GetWeakRefCount();
    if (curCount > 0) {
        curCount = atomicWeak_.fetch_sub(1, std::memory_order_release);
    }

    int strongRefCount = GetStrongRefCount();
    if ((curCount == 1) || (strongRefCount == 0 && !IsLifeTimeExtended())) {
        if (callback_) {
            callback_();
        }
    }
    return curCount;
}

int RefCounter::GetWeakRefCount()
{
    return atomicWeak_.load(std::memory_order_relaxed);
}

void RefCounter::SetAttemptAcquire()
{
    (void)atomicAttempt_.fetch_add(1, std::memory_order_relaxed);
}

bool RefCounter::IsAttemptAcquireSet()
{
    return static_cast<bool>(atomicAttempt_.load(std::memory_order_relaxed) > 0);
}

void RefCounter::ClearAttemptAcquire()
{
    atomicAttempt_.fetch_sub(1, std::memory_order_relaxed);
}

void RefCounter::ExtendObjectLifetime()
{
    atomicFlags_.fetch_or(FLAG_EXTEND_LIFE_TIME, std::memory_order_relaxed);
}

bool RefCounter::IsLifeTimeExtended()
{
    return static_cast<bool>(atomicFlags_.load(std::memory_order_relaxed) & FLAG_EXTEND_LIFE_TIME);
}

bool RefCounter::AttemptIncStrongRef(const void *objectId, int &outCount)
{
    int curCount = GetStrongRefCount();
    IncWeakRefCount(objectId);

    // if the object already had strong references.just promoting it.
    while ((curCount > 0) && (curCount != INITIAL_PRIMARY_VALUE)) {
        if (atomicStrong_.compare_exchange_weak(curCount, curCount + 1, std::memory_order_relaxed)) {
            goto ATTEMPT_SUCCESS;
        }
        // someone else changed the counter.re-acquire the counter value.
        curCount = atomicStrong_.load(std::memory_order_relaxed);
    }

    if ((curCount == INITIAL_PRIMARY_VALUE) && !IsLifeTimeExtended()) {
        // this object has a "normal" life-time,
        while (curCount > 0) {
            if (atomicStrong_.compare_exchange_weak(curCount, curCount + 1, std::memory_order_relaxed)) {
                goto ATTEMPT_SUCCESS;
            }
            curCount = atomicStrong_.load(std::memory_order_relaxed);
        }
    }

    if (IsLifeTimeExtended()) {
        curCount = atomicStrong_.fetch_add(1, std::memory_order_relaxed);
    }

ATTEMPT_SUCCESS:
    if (curCount >= INITIAL_PRIMARY_VALUE) {
        outCount = curCount;
        atomicStrong_.fetch_sub(INITIAL_PRIMARY_VALUE, std::memory_order_release);
        return true;
    }

    if (curCount < 0 || (!IsLifeTimeExtended() && curCount == 0)) {
        // the object destroyed on strong reference count reduce to zero.
        DecWeakRefCount(objectId);
        return false;
    }

    return true;
}

RefBase::RefBase() : refs_(new RefCounter())
{
    refs_->IncRefCount();
    refs_->SetCallback(std::bind(&RefBase::RefPtrCallback, this));
}

RefBase::RefBase(const RefBase &/*other*/)
{
    refs_ = new RefCounter();
    if (refs_ != nullptr) {
        refs_->IncRefCount();
        refs_->SetCallback(std::bind(&RefBase::RefPtrCallback, this));
    }
}

void RefBase::RefPtrCallback()
{
    delete this;
}

/*
 * The two ends of the assignment are two independent and exclusive,
 * and the application should not share the reference counter.
 * RISK: If there is a reference count on the left of the equal sign,
 * it may cause a reference count exception
 */
RefBase &RefBase::operator=(const RefBase &/*other*/)
{
    if (refs_ != nullptr) {
        refs_->RemoveCallback();
        refs_->DecRefCount();
    }

    refs_ = new RefCounter();
    if (refs_ != nullptr) {
        refs_->IncRefCount();
        refs_->SetCallback(std::bind(&RefBase::RefPtrCallback, this));
    }

    return *this;
}

RefBase::RefBase(RefBase &&other) noexcept
{
    refs_ = other.refs_;
    if (other.refs_ != nullptr) {
        other.refs_ = nullptr;
    }
}

RefBase &RefBase::operator=(RefBase &&other) noexcept
{
    if (refs_ == other.refs_) {
        return *this;
    }

    if (refs_ != nullptr) {
        refs_->RemoveCallback();
        refs_->DecRefCount();
    }

    refs_ = other.refs_;
    if (other.refs_ != nullptr) {
        other.refs_ = nullptr;
    }
    return *this;
}

RefBase::~RefBase()
{
    if (refs_ != nullptr) {
        refs_->RemoveCallback();
        refs_->DecRefCount();
        refs_ = nullptr;
    }
}

void RefBase::ExtendObjectLifetime()
{
    refs_->ExtendObjectLifetime();
}

void RefBase::IncStrongRef(const void *objectId)
{
    if (refs_ == nullptr) {
        return;
    }

    const int curCount = refs_->IncStrongRefCount(objectId);
    IncWeakRef(objectId);
    if (curCount == INITIAL_PRIMARY_VALUE) {
        OnFirstStrongRef(objectId);
    }
    if (refs_->IsAttemptAcquireSet()) {
        refs_->ClearAttemptAcquire();
        refs_->DecStrongRefCount(objectId);
        refs_->DecWeakRefCount(objectId);
    }
}

void RefBase::DecStrongRef(const void *objectId)
{
    if (refs_ == nullptr) {
        return;
    }

    const int curCount = refs_->DecStrongRefCount(objectId);
    if (curCount == 1) {
        OnLastStrongRef(objectId);
    }
    DecWeakRef(objectId);
}

int RefBase::GetSptrRefCount()
{
    if (refs_ == nullptr) {
        return 0;
    }
    return refs_->GetStrongRefCount();
}

WeakRefCounter *RefBase::CreateWeakRef(void *cookie)
{
    if (refs_ != nullptr) {
        return new WeakRefCounter(refs_, cookie);
    }
    return nullptr;
}

void RefBase::IncWeakRef(const void *objectId)
{
    if (refs_ != nullptr) {
        refs_->IncWeakRefCount(objectId);
    }
}

void RefBase::DecWeakRef(const void *objectId)
{
    if (refs_ != nullptr) {
        refs_->DecWeakRefCount(objectId);
    }
}

int RefBase::GetWptrRefCount()
{
    if (refs_ == nullptr) {
        return 0;
    }
    return refs_->GetWeakRefCount();
}

bool RefBase::AttemptAcquire(const void *objectId)
{
    if (refs_ == nullptr) {
        return false;
    }

    int count = 0;
    if (refs_->AttemptIncStrongRef(objectId, count)) {
        refs_->SetAttemptAcquire();
        if (count == INITIAL_PRIMARY_VALUE) {
            OnFirstStrongRef(objectId);
        }

        return true;
    }
    return false;
}

bool RefBase::AttemptIncStrongRef(const void *objectId)
{
    if ((refs_ != nullptr) && (OnAttemptPromoted(objectId))) {
        int count = 0;
        bool ret = refs_->AttemptIncStrongRef(objectId, count);
        if (count == INITIAL_PRIMARY_VALUE) {
            OnFirstStrongRef(objectId);
        }
        return ret;
    }

    return false;
}

bool RefBase::IsAttemptAcquireSet()
{
    if (refs_ == nullptr) {
        return false;
    }
    return refs_->IsAttemptAcquireSet();
}

bool RefBase::IsExtendLifeTimeSet()
{
    if (refs_ == nullptr) {
        return false;
    }
    return refs_->IsLifeTimeExtended();
}

void RefBase::OnFirstStrongRef(const void* /*objectId*/)
{}

void RefBase::OnLastStrongRef(const void* /*objectId*/)
{}

void RefBase::OnLastWeakRef(const void* /*objectId*/)
{}

bool RefBase::OnAttemptPromoted(const void* /*objectId*/)
{
    return true;
}

}  // namespace OHOS
