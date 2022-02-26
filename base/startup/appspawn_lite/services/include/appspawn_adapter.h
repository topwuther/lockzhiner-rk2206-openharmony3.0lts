/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef BASE_STARTUP_APPSPAWN_ADAPTER_H
#define BASE_STARTUP_APPSPAWN_ADAPTER_H
#ifdef __LINUX__
#include <linux/capability.h>
#else
#include <sys/capability.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef __LINUX__
/* Control the ambient capability set */
#ifndef PR_CAP_AMBIENT
#define PR_CAP_AMBIENT          47
#endif
#ifndef PR_CAP_AMBIENT_IS_SET
#define PR_CAP_AMBIENT_IS_SET      1
#endif
#ifndef PR_CAP_AMBIENT_RAISE
#define PR_CAP_AMBIENT_RAISE       2
#endif
#ifndef PR_CAP_AMBIENT_LOWER
#define PR_CAP_AMBIENT_LOWER       3
#endif
#ifndef PR_CAP_AMBIENT_CLEAR_ALL
#define PR_CAP_AMBIENT_CLEAR_ALL   4
#endif
extern int capset(void *a, void *b);
#endif

int KeepCapability();
int SetAmbientCapability(int cap);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // BASE_STARTUP_APPSPAWN_ADAPTER_H
