/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BASE_STARTUP_INITLITE_NOTIFY_H
#define BASE_STARTUP_INITLITE_NOTIFY_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define EVENT1             0xf
#define EVENT1_WAITTIME    10000 // 10s = 10*1000 * 1 tick(1ms)

#define EVENT2             0xf0
/* Define EVENT2_WAITTIME 0 means QS_STAGE2 is no used */
#define EVENT2_WAITTIME    0

#define EVENT3             0xf00
/* Define EVENT3_WAITTIME 0 means QS_STAGE3 is no used */
#define EVENT3_WAITTIME    0

/*
 * Notify the event to Init process.
 * All processes can call. Usually called by the listened process.
 * event: There needs to be a consensus between the listener(init process) and the notifier.
 */
extern int NotifyInit(unsigned long event);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // BASE_STARTUP_INITLITE_NOTIFY_H
