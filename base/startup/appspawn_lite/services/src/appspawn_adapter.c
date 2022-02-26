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
#include "appspawn_adapter.h"
#include <stdio.h>
#include <sys/prctl.h>
#ifdef __LINUX__
#include <linux/securebits.h>
#endif

int KeepCapability()
{
#ifdef __LINUX__
    if (prctl(PR_SET_SECUREBITS, SECBIT_NO_SETUID_FIXUP | SECBIT_NO_SETUID_FIXUP_LOCKED)) {
        printf("prctl failed\n");
        return -1;
    }
#endif
    return 0;
}

int SetAmbientCapability(int cap)
{
#ifdef __LINUX__
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0)) {
        printf("[Init] prctl PR_CAP_AMBIENT failed\n");
        return -1;
    }
#endif
    return 0;
}

