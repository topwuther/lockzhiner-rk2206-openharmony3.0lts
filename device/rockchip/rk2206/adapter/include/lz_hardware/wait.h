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
 * @file wait.h
 *
 */

#ifndef LZ_HARDWARE_WAIT_H
#define LZ_HARDWARE_WAIT_H

/**
 * @brief sleep until a condition gets true or a timeout elapses.
 *
 *
 *
 * @param condition Indicates a C expression for the event to wait for.
 * @param timeout Indicates timeout, in ticks.
 * @return Returns:
 * 0 if the @condition evaluated to %false after the @timeout elapsed,
 * 1 if the @condition evaluated to %true after the @timeout elapsed,
 * or the remaining jiffies (at least 1) if the @condition evaluated
 * to %true before the @timeout elapsed.
 */
#define WaitEventTimeout(condition, timeout)				        \
({                                                                  \
    int __ret = timeout;                                            \
    while (--__ret && !(condition)) {                               \
        ToyMsleep(1);                                               \
    }                                                               \
    if ((__ret == 0) && (condition))                                \
        __ret = 1;                                                  \
    __ret;                                                          \
})

/**
 * @brief sleep until a condition gets true.
 *
 *
 *
 * @param condition Indicates a C expression for the event to wait for.
 * @return Returns:
 */
#define WaitEvent(condition)          				                \
({                                                                  \
    while (!(condition)) {                                          \
        ToyMsleep(1);                                               \
    }                                                               \
})

#endif
/** @} */
