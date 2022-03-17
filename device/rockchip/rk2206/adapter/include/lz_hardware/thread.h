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
 * @file thread.h
 */

#ifndef LZ_HARDWARE_THREAD_H
#define LZ_HARDWARE_THREAD_H

#define LZ_HARDWARE_THREAD_STACK_SIZE              0x1000
#define LZ_HARDWARE_THREAD_PRIO                    6

typedef void (*ThreadFunc)(void *arg);

/**
 * @brief Create a thread.
 *
 *
 *
 * @param threadID Indicates the thread ID.
 * @param func Indicates the thread callback function.
 * @param arg Indicates input parameter of callback function.
 * @param owner Indicates the owner process who create thread.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the thread is created successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values.
 */

int CreateThread(unsigned int *threadID, ThreadFunc func, void *arg, const char *owner);

/**
 * @brief Destroy a thread.
 *
 *
 *
 * @param threadID Indicates the thread ID.
 * @return Returns {@link LZ_HARDWARE_SUCCESS} if the thread is destoried successfully;
 * returns {@link LZ_HARDWARE_FAILURE} otherwise. For details about other return values, see the chip description.
 */

int DestroyThread(unsigned int threadID);

#endif
/** @} */
