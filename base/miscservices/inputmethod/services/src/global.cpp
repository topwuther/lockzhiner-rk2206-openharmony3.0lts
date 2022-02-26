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

#include <cstdio>
#include "global.h"

namespace OHOS {
namespace MiscServices {
    void LogTimeStamp()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        struct tm nowTime;
        localtime_r(&tv.tv_sec, &nowTime);
        int32_t millSec = 1000;
        printf("%02d-%02d %02d:%02d:%02d.%03d\t", nowTime.tm_mon, nowTime.tm_mday,
            nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec, (int)tv.tv_usec / millSec);
    }

    namespace ErrorCode {
        const char *ToString(int errorCode)
        {
            switch (errorCode) {
                case NO_ERROR: {
                    return "no error";
                }
                case ERROR_NULL_POINTER: {
                    return "null pointer";
                }
                case ERROR_BAD_PARAMETERS: {
                    return "bad parameters";
                }
                case ERROR_SERVICE_START_FAILED: {
                    return "failed to start service";
                }
                case ERROR_USER_NOT_STARTED: {
                    return "user has not started";
                }
                case ERROR_USER_ALREADY_STARTED: {
                    return "user has already started";
                }
                case ERROR_USER_NOT_UNLOCKED: {
                    return "user has not unlocked";
                }
                case ERROR_USER_ALREADY_UNLOCKED: {
                    return "user has already unlocked";
                }
                case ERROR_USER_NOT_LOCKED: {
                    return "user has not locked";
                }
                case ERROR_IME_NOT_AVAILABLE: {
                    return "input method engine is not available";
                }
                case ERROR_SECURITY_IME_NOT_AVAILABLE: {
                    return "security input method engine is not available";
                }
                case ERROR_TOKEN_CREATE_FAILED: {
                    return "failed to create token";
                }
                case ERROR_TOKEN_DESTROY_FAILED: {
                    return "failed to destroy token";
                }
                case ERROR_IME_BIND_FAILED: {
                    return "failed to bind service";
                }
                case ERROR_IME_UNBIND_FAILED: {
                    return "failed to unbind service";
                }
                case ERROR_IME_START_FAILED: {
                    return "failed to start input";
                }
                case ERROR_IME_STOP_FAILED: {
                    return "failed to stop input";
                }
                case ERROR_KBD_SHOW_FAILED: {
                    return "failed to show keyboard";
                }
                case ERROR_KBD_HIDE_FAILED: {
                    return "failed to hide keyboard";
                }
                case ERROR_IME_NOT_STARTED: {
                    return "input method service has not started";
                }
                case ERROR_KBD_IS_OCCUPIED: {
                    return "keyboard is showing by another client";
                }
                case ERROR_KBD_IS_NOT_SHOWING: {
                    return "keyboard is not showing";
                }
                case ERROR_IME_ALREADY_STARTED: {
                    return "input method service has already started";
                }
                case ERROR_NOT_IME_PACKAGE: {
                    return "not an input method engine package";
                }
                case ERROR_IME_PACKAGE_DUPLICATED: {
                    return "duplicated input method engine package";
                }
                case ERROR_SETTING_SAME_VALUE: {
                    return "same setting value";
                }
                case ERROR_NO_NEXT_IME: {
                    return "next input method engine is not available";
                }
                case ERROR_CLIENTWINDOW_NOT_FOCUSED: {
                    return "input client window is not focused";
                }
                case ERROR_CLIENT_NOT_WINDOW: {
                    return "input client is not from a valid window";
                }
                case ERROR_REMOTE_IME_DIED: {
                    return "input method service died";
                }
                case ERROR_RESTART_IME_FAILED: {
                    return "failed to restart input method service";
                }
                case ERROR_REMOTE_CLIENT_DIED: {
                    return "input client died";
                }
                case ERROR_CLIENT_DUPLICATED: {
                    return "duplicated client";
                }
                case ERROR_CLIENT_NOT_FOUND: {
                    return "client is not found";
                }
                case ERROR_STATUS_UNKNOWN_ERROR: {
                    return "unknown error";
                }
                case ERROR_STATUS_NO_MEMORY: {
                    return "no memory";
                }
                case ERROR_STATUS_INVALID_OPERATION: {
                    return "invalid operation";
                }
                case ERROR_STATUS_BAD_VALUE: {
                    return "bad value";
                }
                case ERROR_STATUS_BAD_TYPE: {
                    return "bad type";
                }
                case ERROR_STATUS_NAME_NOT_FOUND: {
                    return "name not found";
                }
                case ERROR_STATUS_PERMISSION_DENIED: {
                    return "permission denied";
                }
                case ERROR_STATUS_NO_INIT: {
                    return "no init";
                }
                case ERROR_STATUS_ALREADY_EXISTS: {
                    return "already exists";
                }
                case ERROR_STATUS_DEAD_OBJECT: {
                    return "dead object";
                }
                case ERROR_STATUS_FAILED_TRANSACTION: {
                    return "failed transaction";
                }
                case ERROR_STATUS_BAD_INDEX: {
                    return "bad index";
                }
                case ERROR_STATUS_NOT_ENOUGH_DATA: {
                    return "not enough data";
                }
                case ERROR_STATUS_WOULD_BLOCK: {
                    return "would block";
                }
                case ERROR_STATUS_TIMED_OUT: {
                    return "time out";
                }
                case ERROR_STATUS_UNKNOWN_TRANSACTION: {
                    return "unknown transaction";
                }
                case ERROR_STATUS_FDS_NOT_ALLOWED: {
                    return "fds not allowed";
                }
                case ERROR_STATUS_UNEXPECTED_NULL: {
                    return "unexpected null";
                }
                case ERROR_EX_ILLEGAL_ARGUMENT: {
                    return "illegal argument exception";
                }
                case ERROR_EX_NULL_POINTER: {
                    return "null pointer exception";
                }
                case ERROR_EX_ILLEGAL_STATE: {
                    return "illegal state exception";
                }
                case ERROR_EX_NETWORK_MAIN_THREAD: {
                    return "network main thread exception";
                }
                case ERROR_EX_UNSUPPORTED_OPERATION: {
                    return "unsupported operation exception";
                }
                case ERROR_EX_SERVICE_SPECIFIC: {
                    return "service specific exception";
                }
                case ERROR_EX_PARCELABLE: {
                    return "parcelable exception";
                }
                default: {
                    return "error is out of definition";
                }
            }
            return "error is out of definition";
        };
    }
}
}

