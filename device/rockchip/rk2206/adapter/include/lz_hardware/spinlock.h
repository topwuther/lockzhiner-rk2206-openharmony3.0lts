/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

/**
 * @addtogroup Lockzhiner
 *
 * @file spinlock.h
 */

#ifndef LZ_HARDWARE_SPINLOCK_H
#define LZ_HARDWARE_SPINLOCK_H

typedef struct {
    unsigned int flags;
} Spinlock;

#define SPIN_LOCK_INITIALIZER           \
{                                       \
    .flags    = 0,                      \
}

#define SPIN_LOCK_INIT(lock)  Spinlock lock = SPIN_LOCK_INITIALIZER

void SpinLockInit(Spinlock *lock);

/**
 * @par Description:
 * This API is used to lock the spin lock.
 *
 * @param lock   the spin lock to be locked
 *
 */
void SpinLock(Spinlock *lock);

/**
 * @par Description:
 * This API is used to unlock the spin lock.
 *
 * @param lock   the spin lock to be unlocked
 *
 */
void SpinUnlock(Spinlock *lock);

#endif
/** @} */
