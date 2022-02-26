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

#ifndef KV_DB_REF_OBJECT_H
#define KV_DB_REF_OBJECT_H

#include <atomic>
#include <string>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "macro_utils.h"

namespace DistributedDB {
class RefObject {
public:
    class AutoLock final {
    public:
        AutoLock(const RefObject *obj, bool unlocked = true);
        ~AutoLock();
        void Lock();
        void Unlock();

    private:
        DISABLE_COPY_ASSIGN_MOVE(AutoLock);
        const RefObject *refObj_;
        bool IsLocked_;
    };

    RefObject();

    /* Invoked before this object deleted. */
    void OnLastRef(const std::function<void(void)> &callback) const;

    /* Invoked when kill object, with lock held. */
    void OnKill(const std::function<void(void)> &callback);

    bool IsKilled() const;
    void KillObj();
    void LockObj() const;
    void UnlockObj() const;
    bool WaitLockedUntil(std::condition_variable &cv,
        const std::function<bool(void)> &condition, int seconds = 0);

    /* Work as static members, avoid to 'delete this' */
    static void IncObjRef(const RefObject *obj);
    static void DecObjRef(const RefObject *obj);
    static void KillAndDecObjRef(RefObject *obj);

protected:
    virtual ~RefObject();
    virtual std::string GetObjectTag() const;

private:
    constexpr static const char * const classTag = "Class-RefObject";

    DISABLE_COPY_ASSIGN_MOVE(RefObject);

    /* A const object can also be locked/unlocked/ref()/unref() */
    mutable std::atomic<int> refCount_;
    mutable std::mutex objLock_;
    std::atomic<bool> isKilled_;
    mutable std::function<void(void)> onLast_;
    std::function<void(void)> onKill_;
};
} // namespace DistributedDB

#endif // KV_DB_REF_OBJECT_H
