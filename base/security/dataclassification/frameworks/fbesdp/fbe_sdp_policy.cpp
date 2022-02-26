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

#include "fbe_sdp_policy.h"

extern "C" __attribute (()) bool IsSupportIudf()
{
    return false;
}

extern "C" __attribute (()) int SetLabel(int userId, const char* filePath,
    const char* labelName, const char* labelValue, int flag)
{
    return RET_SDP_OK;
}

extern "C" __attribute (()) int GetLabel(int userId, const char* filePath,
    const char* labelName, char* labelValue, const int valueLen)
{
    return RET_SDP_OK;
}

extern "C" __attribute (()) int GetFlag(int userId, const char* filePath, const char* labelName)
{
    return RET_SDP_OK;
}

extern "C" __attribute (()) int SetEcePathPolicy(int userId, const char *path)
{
    return RET_SDP_OK;
}

extern "C" __attribute (()) int SetSecePathPolicy(int userId, const char *path)
{
    return RET_SDP_OK;
}

extern "C" __attribute (()) int GetPathPolicy(const char *path)
{
    if (!IsSupportIudf()) {
        return RET_SDP_SUPPORT_IUDF_ERROR;
    }
    return FSCRYPT_NO_ECE_OR_SECE_CLASS;
}

__attribute (()) int GetLockState(int userId, int flag)
{
    return RET_LOCK_IUDF_SERVICE_NO_SUPPORT;
}

__attribute (()) int RegisterLockStateChangeCallback(int flag,
    std::function<int32_t(int32_t, int32_t)> &lockStateChangedListener)
{
    return RET_SDP_OK;
}
__attribute (()) int UnRegisterLockStateChangeCallback(
    std::function<int32_t(int32_t, int32_t)> &lockStateChangedListener)
{
    return RET_SDP_OK;
}
