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
 * @file mutex.h
 */

#ifndef LZ_HARDWARE_MUTEX_H
#define LZ_HARDWARE_MUTEX_H

typedef struct _ToyMutex {
    unsigned int muxHandle;
} ToyMutex;

/**
 * @par Description:
 * This API is used to initialize the mutex.
 * Return LZ_HARDWARE_SUCCESS on creating successful, return specific error code otherwise.
 *
 * @param mutex   the mutex to be initialized
 *
 */
unsigned int ToyMutexInit(ToyMutex *lock);

/**
 * @par Description:
 * This API is used to destroy the mutex.
 * Return LZ_HARDWARE_SUCCESS on destroying successful, return specific error code otherwise.
 *
 * @param mutex   the mutex to be destroied
 *
 */
unsigned int ToyMutexDestroy(ToyMutex *lock);

/**
 * @par Description:
 * This API is used to lock the mutex.
 * Return LZ_HARDWARE_SUCCESS on destroying successful, return specific error code otherwise.
 *
 * @param mutex   the mutex to be locked
 *
 */
unsigned int ToyMutexLock(ToyMutex *lock);

/**
 * @par Description:
 * This API is used to try to lock the mutex.
 * Return LZ_HARDWARE_SUCCESS on locking successful, return specific error code otherwise.
 *
 * @param mutex   the mutex to be locked
 *
 */
unsigned int ToyMutexTryLock(ToyMutex *lock);

/**
 * @par Description:
 * This API is used to unlock the mutex.
 * Return LZ_HARDWARE_SUCCESS on unlocking successful, return specific error code otherwise.
 *
 * @param mutex   the mutex to be unlocked
 *
 */
unsigned int ToyMutexUnlock(ToyMutex *lock);

#endif
/** @} */
