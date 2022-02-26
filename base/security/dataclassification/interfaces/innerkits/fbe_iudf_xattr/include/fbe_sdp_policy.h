/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef FBE_IUDF_H
#define FBE_IUDF_H

#include "fbe_sdp_code_num.h"
#include <functional>

#define LABEL_VALUE_S0 "S0"
#define LABEL_VALUE_S1 "S1"
#define LABEL_VALUE_S2 "S2"
#define LABEL_VALUE_S3 "S3"
#define LABEL_VALUE_S4 "S4"
#define LABEL_NAME_SECURITY_LEVEL "SecurityLevel"
#define FLAG_FILE_PROTECTION_COMPLETE 0
#define FLAG_FILE_PROTECTION_COMPLETE_UNLESS_OPEN 1

extern "C" __attribute (()) bool IsSupportIudf();
extern "C" __attribute (()) int SetEcePathPolicy(int userId, const char *path);
extern "C" __attribute (()) int SetSecePathPolicy(int userId, const char *path);
extern "C" __attribute (()) int GetPathPolicy(const char *path);
extern "C" __attribute (()) int SetLabel(int userId, const char *filePath,
    const char *labelName, const char *labelValue, int flag);
extern "C" __attribute (()) int GetLabel(int userId, const char *filePath,
    const char *labelName, char *labelValue, const int valueLen);
extern "C" __attribute (()) int GetFlag(int userId, const char *filePath, const char *labelName);
__attribute (()) int GetLockState(int userId, int flag);

__attribute (()) int RegisterLockStateChangeCallback(int flag,
    std::function<int32_t(int32_t, int32_t)> &lockStateChangedListener);

__attribute (()) int UnRegisterLockStateChangeCallback(
    std::function<int32_t(int32_t, int32_t)> &lockStateChangedListener);
#endif
