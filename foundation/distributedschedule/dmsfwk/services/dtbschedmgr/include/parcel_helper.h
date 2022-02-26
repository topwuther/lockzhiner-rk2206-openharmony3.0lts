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

#ifndef OHOS_DISTRIBUTED_SCHEDULE_PARCEL_HELPER_H
#define OHOS_DISTRIBUTED_SCHEDULE_PARCEL_HELPER_H

#include <cinttypes>

#include "dtbschedmgr_log.h"

namespace OHOS {
namespace DistributedSchedule {
#define PARCEL_WRITE_HELPER(parcel, type, value) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            HILOGE("%{public}s write value failed!", __func__); \
            return ERR_FLATTEN_OBJECT; \
        } \
    } while (0)

#define PARCEL_WRITE_HELPER_NORET(parcel, type, value) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            HILOGE("%{public}s write value failed!", __func__); \
            return; \
        } \
    } while (0)

#define PARCEL_WRITE_HELPER_RET(parcel, type, value, failRet) \
    do { \
        bool ret = parcel.Write##type((value)); \
        if (!ret) { \
            HILOGE("%{public}s write value failed!", __func__); \
            return failRet; \
        } \
    } while (0)

#define PARCEL_READ_HELPER(parcel, type, out) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            HILOGE("%{public}s read value failed!", __func__); \
            return ERR_FLATTEN_OBJECT; \
        } \
    } while (0)

#define PARCEL_READ_HELPER_RET(parcel, type, out, failRet) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            HILOGE("%{public}s read value failed!", __func__); \
            return failRet; \
        } \
    } while (0)

#define PARCEL_READ_HELPER_NORET(parcel, type, out) \
    do { \
        bool ret = parcel.Read##type((out)); \
        if (!ret) { \
            HILOGW("%{public}s read value failed!", __func__); \
        } \
    } while (0)

#define PARCEL_TRANSACT_SYNC_RET_INT(remote, code, data, reply) \
    do { \
        MessageOption option; \
        int32_t error = remote->SendRequest(code, data, reply, option); \
        if (error != ERR_NONE) { \
            HILOGE("%{public}s transact failed, error: %{public}d", __func__, error); \
            return error; \
        } \
        int32_t result = reply.ReadInt32(); \
        HILOGD("%{public}s get result from server data = %{public}d", __func__, result); \
        return result; \
    } while (0)

#define PARCEL_TRANSACT_SYNC_NORET(remote, code, data, reply) \
    do { \
        MessageOption option; \
        int32_t error = remote->SendRequest(code, data, reply, option); \
        if (error != ERR_NONE) { \
            HILOGE("%{public}s transact failed, error: %{public}d", __func__, error); \
            return; \
        } \
        HILOGD("%{public}s transact success!", __func__); \
    } while (0)

#define PARCEL_WRITE_REPLY_NOERROR(reply, type, result) \
    do { \
        bool ret = reply.Write##type(result); \
        if (!ret) { \
            HILOGW("%{public}s write reply failed.", __func__); \
        } \
        return ERR_NONE; \
    } while (0)
} // namespace DistributedSchedule
} // namespace OHOS

#endif /* OHOS_DISTRIBUTED_SCHEDULE_PARCEL_HELPER_H */
