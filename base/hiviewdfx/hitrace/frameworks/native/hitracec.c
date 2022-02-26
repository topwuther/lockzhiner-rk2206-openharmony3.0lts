/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "hitrace/hitracec.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "securec.h"
#include "hilog/log.h"
#include "hilog_trace.h"
#include "hitrace_inner.h"

#undef LOG_DOMAIN
#undef LOG_TAG
static const unsigned int LOG_DOMAIN = 0xD002D03;
static const char* LOG_TAG = "HiTraceC";

typedef struct HiTraceChainIdStruct {
    union {
#if __BYTE_ORDER == __LITTLE_ENDIAN
        struct {
            uint64_t reserved : 4;
            uint64_t usecond : 20;
            uint64_t second : 16;
            uint64_t cpuId : 4;
            uint64_t deviceId : 20;
        };
        struct {
            uint64_t padding : 4;
            uint64_t chainId : 60;
        };
#elif __BYTE_ORDER == __BIG_ENDIAN
        struct {
            uint64_t deviceId : 20;
            uint64_t cpuId : 4;
            uint64_t second : 16;
            uint64_t usecond : 20;
            uint64_t reserved : 4;
        };
        struct {
            uint64_t chainId : 60;
            uint64_t padding : 4;
        };
#else
#error "ERROR: No BIG_LITTLE_ENDIAN defines."
#endif
    };
} HiTraceChainIdStruct;

typedef struct HiTraceIdStructExtra {
    uint32_t setTls : 1;
    uint32_t reserved : 31;
} HiTraceIdStructExtra;

typedef struct HiTraceIdStructInner {
    HiTraceIdStruct id;
    HiTraceIdStructExtra extra;
} HiTraceIdStructInner;

static __thread HiTraceIdStructInner g_hiTraceId = {{0, 0, 0, 0, 0, 0}, {0, 0}};

static inline HiTraceIdStructInner* GetThreadIdInner()
{
    return &g_hiTraceId;
}

HiTraceIdStruct HiTraceGetId()
{
    HiTraceIdStructInner* pThreadId = GetThreadIdInner();
    return pThreadId->id;
}

void HiTraceSetId(const HiTraceIdStruct* pId)
{
    if (!HiTraceIsValid(pId)) {
        return;
    }

    HiTraceIdStructInner* pThreadId = GetThreadIdInner();
    pThreadId->id = *pId;
    return;
}

void HiTraceClearId()
{
    HiTraceIdStructInner* pThreadId = GetThreadIdInner();
    HiTraceInitId(&(pThreadId->id));
    return;
}

static inline int HiTraceGetDeviceId()
{
    // save device id and use it later
    static int deviceId = 0;

    if (deviceId == 0) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        srand(tv.tv_sec);
        deviceId = random();
    }
    return deviceId;
}

static inline unsigned int HiTraceGetCpuId()
{
    // Using vdso call will make get_cpu_id faster: sched_getcpu()
    static unsigned int cpuId = 0;
    cpuId++;

    return cpuId;
}

static inline uint64_t HiTraceCreateChainId()
{
    // get timestamp. Using vdso call(no system call)
    struct timeval tv;
    gettimeofday(&tv, NULL);

    HiTraceChainIdStruct chainId = {
        .padding = 0,
        .chainId = 0
    };
    chainId.deviceId = HiTraceGetDeviceId();
    chainId.cpuId = HiTraceGetCpuId();
    chainId.second = tv.tv_sec;
    chainId.usecond = tv.tv_usec;

    return chainId.chainId;
}

HiTraceIdStruct HiTraceBegin(const char* name, int flags)
{
    HiTraceIdStruct id;
    HiTraceInitId(&id);

    if ((flags < HITRACE_FLAG_MIN) || (flags > HITRACE_FLAG_MAX)) {
        return id;
    }

    HiTraceIdStructInner* pThreadId = GetThreadIdInner();
    if (HiTraceIsValid(&(pThreadId->id))) {
        return id;
    }

    id.valid = HITRACE_ID_VALID;
    id.ver = HITRACE_VER_1;
    id.chainId = HiTraceCreateChainId();
    id.flags = flags;
    id.spanId = 0;
    id.parentSpanId = 0;

    pThreadId->id = id;

    if (!HiTraceIsFlagEnabled(&id, HITRACE_FLAG_NO_BE_INFO)) {
        HILOG_INFO(LOG_CORE, "HiTraceBegin name:%{public}s flags:%{public}x.", name ? name : "", (int)id.flags);
    }
    return id;
}

void HiTraceEnd(const HiTraceIdStruct* pId)
{
    if (!HiTraceIsValid(pId)) {
        HILOG_ERROR(LOG_CORE, "HiTraceEnd error: invalid end id.");
        return;
    }

    HiTraceIdStructInner* pThreadId = GetThreadIdInner();
    if (!HiTraceIsValid(&(pThreadId->id))) {
        HILOG_ERROR(LOG_CORE, "HiTraceEnd error: invalid thread id.");
        return;
    }

    if (HiTraceGetChainId(pId) != HiTraceGetChainId(&(pThreadId->id))) {
        HILOG_ERROR(LOG_CORE, "HiTraceEnd error: end id(%{public}llx) != thread id(%{public}llx).",
                    (unsigned long long)pId->chainId, (unsigned long long)pThreadId->id.chainId);
        return;
    }

    if (!HiTraceIsFlagEnabled(&(pThreadId->id), HITRACE_FLAG_NO_BE_INFO)) {
        HILOG_INFO(LOG_CORE, "HiTraceEnd.");
    }

    HiTraceInitId(&(pThreadId->id));
    return;
}

// BKDRHash
static uint32_t HashFunc(const void* pData, uint32_t dataLen)
{
    const uint32_t seed = 131;

    if ((!pData) || dataLen == 0) {
        return 0;
    }

    uint32_t hash = 0;
    uint32_t len = dataLen;
    char* p = (char*)pData;

    for (; len > 0; --len) {
        hash = (hash * seed) + (*p++);
    }

    return hash;
}

HiTraceIdStruct HiTraceCreateSpan()
{
    static const uint32_t hashDataNum = 5;

    HiTraceIdStruct id = HiTraceGetId();
    if (!HiTraceIsValid(&id)) {
        return id;
    }

    if (HiTraceIsFlagEnabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN)) {
        return id;
    }

    // create child span id
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint32_t hashData[hashDataNum];
    hashData[0] = HiTraceGetDeviceId(); // 0: device id
    hashData[1] = id.parentSpanId;      // 1: parent span id
    hashData[2] = id.spanId;            // 2: span id
    hashData[3] = tv.tv_sec;            // 3: second
    hashData[4] = tv.tv_usec;           // 4: usecond

    uint32_t hash = HashFunc(hashData, hashDataNum * sizeof(uint32_t));

    id.parentSpanId = id.spanId;
    id.spanId = hash;
    return id;
}

void HiTraceTracepointInner(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceIdStruct* pId,
    const char* fmt, va_list args)
{
    static const int tpBufferSize = 2048;
    static const char* hiTraceTypeStr[] = { "CS", "CR", "SS", "SR", "GENERAL", };
    static const char* hiTraceModeStr[] = { "DEFAULT", "THREAD", "PROCESS", "DEVICE", };

    if (mode < HITRACE_CM_MIN || mode > HITRACE_CM_MAX) {
        return;
    }
    if (type < HITRACE_TP_MIN || type > HITRACE_TP_MAX) {
        return;
    }

    if (!HiTraceIsValid(pId)) {
        return;
    }

    if (!HiTraceIsFlagEnabled(pId, HITRACE_FLAG_TP_INFO) && !HiTraceIsFlagEnabled(pId, HITRACE_FLAG_D2D_TP_INFO)) {
        // Both tp and d2d-tp flags are disabled.
        return;
    } else if (!HiTraceIsFlagEnabled(pId, HITRACE_FLAG_TP_INFO) && (mode != HITRACE_CM_DEVICE)) {
        // Only d2d-tp flag is enabled. But the communication mode is not device-to-device.
        return;
    }

    char buff[tpBufferSize];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
    // if using privacy parameter: vsnprintf => hilog_vsnprintf
    int ret = vsnprintf_s(buff, tpBufferSize, tpBufferSize - 1, fmt, args);
#pragma clang diagnostic pop
    if (ret == -1) { // -1: vsnprintf_s copy string fail
        return;
    }
    buff[tpBufferSize - 1] = 0;

    HILOG_INFO(LOG_CORE, "<%{public}s,%{public}s,[%{public}llx,%{public}llx,%{public}llx]> %{public}s",
               hiTraceModeStr[mode], hiTraceTypeStr[type], (unsigned long long)pId->chainId,
               (unsigned long long)pId->spanId, (unsigned long long)pId->parentSpanId, buff);
    return;
}

void HiTraceTracepointWithArgs(HiTraceTracepointType type, const HiTraceIdStruct* pId, const char* fmt, va_list args)
{
    HiTraceTracepointInner(HITRACE_CM_DEFAULT, type, pId, fmt, args);
}

void HiTraceTracepointExWithArgs(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceIdStruct* pId,
    const char* fmt, va_list args)
{
    HiTraceTracepointInner(mode, type, pId, fmt, args);
}

void HiTraceTracepoint(HiTraceTracepointType type, const HiTraceIdStruct* pId, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HiTraceTracepointInner(HITRACE_CM_DEFAULT, type, pId, fmt, args);
    va_end(args);
    return;
}

void HiTraceTracepointEx(HiTraceCommunicationMode mode, HiTraceTracepointType type, const HiTraceIdStruct* pId,
    const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HiTraceTracepointInner(mode, type, pId, fmt, args);
    va_end(args);
    return;
}

// return: -1 -- fail; 0 -- all valid; 1 -- all valid except span
int HiTraceGetInfo(uint64_t* pChainId, uint32_t* pFlags, uint64_t* pSpanId, uint64_t* pParentSpanId)
{
    if (!pChainId || !pFlags || !pSpanId || !pParentSpanId) {
        return -1;
    }

    HiTraceIdStruct id = HiTraceGetId();
    if (!HiTraceIsValid(&id)) {
        return -1;
    }

    if (HiTraceIsFlagEnabled(&id, HITRACE_FLAG_DONOT_ENABLE_LOG)) {
        return -1;
    }

    *pChainId = HiTraceGetChainId(&id);
    *pFlags = HiTraceGetFlags(&id);

    if (HiTraceIsFlagEnabled(&id, HITRACE_FLAG_DONOT_CREATE_SPAN)) {
        *pSpanId = 0;
        *pParentSpanId = 0;
        return 1;
    }

    *pSpanId = HiTraceGetSpanId(&id);
    *pParentSpanId = HiTraceGetParentSpanId(&id);
    return 0;
}

static void __attribute__((constructor)) HiTraceInit()
{
    // Call HiLog Register Interface
    HiLogRegisterGetIdFun(HiTraceGetInfo);
}

static void __attribute__((destructor)) HiTraceFini()
{
    HiLogUnregisterGetIdFun(HiTraceGetInfo);
}
