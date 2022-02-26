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

#ifndef FBE_SDP_CODE_NUM
#define FBE_SDP_CODE_NUM

enum ErrorCode {
    RET_SDP_NOT_SUPPORT_ATTR = -13,
    RET_SDP_NOT_SET_ERROR = -12,
    RET_SDP_CODE_FAILED_ERROR = -11,
    RET_SDP_CONTEXT_ERROR = -10,
    RET_SDP_LABEL_HAS_BEEN_SET = -9,
    RET_SDP_GENERIC_ERROR = -8,
    RET_SDP_FILE_OPEN_ERROR = -7,
    RET_SDP_GET_DESC_ERROR = -6,
    RET_SDP_SUPPORT_IUDF_ERROR = -5,
    RET_SDP_IOCTL_ERROR = -4,
    RET_SDP_OPEN_ERROR = -3,
    RET_SDP_MEMORY_ERROR = -2,
    RET_SDP_PARAM_ERROR = -1,
    RET_SDP_OK = 0,
};

enum FsCryptType {
    FSCRYPT_NO_ECE_OR_SECE_CLASS = 1,
    FSCRYPT_SDP_ECE_CLASS = 2,
    FSCRYPT_SDP_SECE_CLASS = 3,
    FSCRYPT_SDP_GET_FEB_VER = 10,
};

enum FbeVesion {
    FBE_VER_NO_2 = 2,
    FBE_VER_NO_3 = 3,
};

enum FbeLockState {
    FLAG_LOCAL_STATE = 0x01,
};

enum FbeLockErrorCode {
    RET_LOCK_IUDF_SERVICE_NO_SUPPORT = -7,
    RET_LOCK_CALLBACK_NOT_REGIST = -6,
    RET_LOCK_CALLBACK_HAS_BEEN_REGIST = -5,
    RET_LOCK_REMOTE_EXCEPTION = -4,
    RET_LOCK_INVALID_PARAM_ERROR = -3,
    RET_LOCK_SERVICE_NOT_FOUND = -2,
    RET_LOCK_PARAM_ERROR = -1,
    RET_LOCK_OK = 0,
};
#endif