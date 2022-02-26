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

#include "common_func.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../common/log.h"
#include "../common/napi/n_class.h"
#include "../common/napi/n_func_arg.h"
#include "../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, void *> GetActualBuf(napi_env env, void *rawBuf, int64_t bufLen, NVal op)
{
    bool succ = false;
    void *realBuf = nullptr;

    if (op.HasProp("offset")) {
        int64_t opOffset;
        tie(succ, opOffset) = op.GetProp("offset").ToInt64();
        if (!succ || opOffset < 0) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.offset, positive integer is desired");
            return { false, nullptr };
        } else if (opOffset > bufLen) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.offset, buffer limit exceeded");
            return { false, nullptr };
        } else {
            realBuf = static_cast<uint8_t *>(rawBuf) + opOffset;
        }
    } else {
        realBuf = rawBuf;
    }

    return { true, realBuf };
}

static tuple<bool, size_t> GetActualLen(napi_env env, int64_t bufLen, int64_t bufOff, NVal op)
{
    bool succ = false;
    int64_t retLen;

    if (op.HasProp("length")) {
        int64_t opLength;
        tie(succ, opLength) = op.GetProp("length").ToInt64();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.length, expect integer");
            return { false, 0 };
        }
        if (opLength < 0) {
            retLen = bufLen - bufOff;
        } else if (opLength + bufOff > bufLen) {
            UniError(EINVAL).ThrowErr(env, "Invalid option.length, buffer limit exceeded");
            return { false, 0 };
        } else {
            retLen = opLength;
        }
    } else {
        retLen = bufLen - bufOff;
    }

    return { true, retLen };
}

tuple<bool, void *, int64_t, bool, int64_t> CommonFunc::GetReadArg(napi_env env, napi_value readBuf, napi_value option)
{
    bool succ = false;
    void *retBuf = nullptr;
    int64_t retLen;
    bool posAssigned = false;
    int64_t position;

    NVal txt(env, readBuf);
    void *buf = nullptr;
    int64_t bufLen;
    tie(succ, buf, bufLen) = txt.ToArraybuffer();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid read buffer, expect arraybuffer");
        return { false, nullptr, 0, posAssigned, position };
    }

    NVal op = NVal(env, option);
    tie(succ, retBuf) = GetActualBuf(env, buf, bufLen, op);
    if (!succ) {
        return { false, nullptr, 0, posAssigned, position };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    tie(succ, retLen) = GetActualLen(env, bufLen, bufOff, op);
    if (!succ) {
        return { false, nullptr, 0, posAssigned, position };
    }

    tie(succ, position) = op.GetProp("position").ToInt64();
    if (succ && position >= 0) {
        posAssigned = true;
    }
    return { true, retBuf, retLen, posAssigned, position };
}

static tuple<bool, unique_ptr<char[]>, int64_t> DecodeString(napi_env env, NVal jsStr, NVal encoding)
{
    unique_ptr<char[]> buf;
    if (!jsStr.TypeIs(napi_string)) {
        return { false, nullptr, 0 };
    }

    bool succ = false;
    if (!encoding) {
        return jsStr.ToUTF8String();
    }

    unique_ptr<char[]> encodingBuf;
    tie(succ, encodingBuf, ignore) = encoding.ToUTF8String();
    if (!succ) {
        return { false, nullptr, 0 };
    }
    string encodingStr(encodingBuf.release());
    if (encodingStr == "utf-8") {
        return jsStr.ToUTF8String();
    } else if (encodingStr == "utf-16") {
        return jsStr.ToUTF16String();
    } else {
        return { false, nullptr, 0 };
    }
}

// Is everthing ok? Do we need to free memory? What's the three args required by fwrite? Where to start writing?
tuple<bool, unique_ptr<char[]>, void *, int64_t, bool, int64_t> CommonFunc::GetWriteArg(napi_env env,
                                                                                        napi_value argWBuf,
                                                                                        napi_value argOption)
{
    void *retBuf = nullptr;
    int64_t retLen;
    bool hasPos = false;
    int64_t retPos;

    /* To get write buffer */
    bool succ = false;
    void *buf = nullptr;
    int64_t bufLen;
    NVal op(env, argOption);
    NVal jsBuffer(env, argWBuf);
    unique_ptr<char[]> bufferGuard;
    tie(succ, bufferGuard, bufLen) = DecodeString(env, jsBuffer, op.GetProp("encoding"));
    if (!succ) {
        tie(succ, buf, bufLen) = NVal(env, argWBuf).ToArraybuffer();
        if (!succ) {
            UniError(EINVAL).ThrowErr(env, "Illegal write buffer or encoding");
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
    } else {
        buf = bufferGuard.get();
    }

    tie(succ, retBuf) = GetActualBuf(env, buf, bufLen, op);
    if (!succ) {
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    int64_t bufOff = static_cast<uint8_t *>(retBuf) - static_cast<uint8_t *>(buf);
    tie(succ, retLen) = GetActualLen(env, bufLen, bufOff, op);
    if (!succ) {
        return { false, nullptr, nullptr, 0, hasPos, retPos };
    }

    /* To parse options - Where to begin writing */
    if (op.HasProp("position")) {
        int32_t position = 0;
        tie(succ, position) = op.GetProp("position").ToInt32();
        if (!succ || position < 0) {
            UniError(EINVAL).ThrowErr(env, "option.position shall be positive number");
            return { false, nullptr, nullptr, 0, hasPos, retPos };
        }
        hasPos = true;
        retPos = position;
    } else {
        retPos = INVALID_POSITION;
    }
    return { true, move(bufferGuard), retBuf, retLen, hasPos, retPos };
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
