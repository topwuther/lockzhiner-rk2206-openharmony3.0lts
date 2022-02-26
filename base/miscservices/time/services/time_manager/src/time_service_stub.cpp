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

#include "time_service_stub.h"
#include <cinttypes>

#include "time_common.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::HiviewDFX;

TimeServiceStub::TimeServiceStub()
{
    memberFuncMap_[SET_TIME] = &TimeServiceStub::OnSetTime;
    memberFuncMap_[SET_TIME_ZONE] = &TimeServiceStub::OnSetTimeZone;
    memberFuncMap_[GET_TIME_ZONE] = &TimeServiceStub::OnGetTimeZone;
    memberFuncMap_[GET_WALL_TIME_MILLI] = &TimeServiceStub::OnGetWallTimeMs;
    memberFuncMap_[GET_WALL_TIME_NANO] = &TimeServiceStub::OnGetWallTimeNs;

    memberFuncMap_[GET_BOOT_TIME_MILLI] = &TimeServiceStub::OnGetBootTimeMs;
    memberFuncMap_[GET_BOOT_TIME_NANO] = &TimeServiceStub::OnGetBootTimeNs;
    memberFuncMap_[GET_MONO_TIME_MILLI] = &TimeServiceStub::OnGetMonotonicTimeMs;
    memberFuncMap_[GET_MONO_TIME_NANO] = &TimeServiceStub::OnGetMonotonicTimeMs;
    memberFuncMap_[GET_THREAD_TIME_MILLI] = &TimeServiceStub::OnGetThreadTimeMs;
    memberFuncMap_[GET_THREAD_TIME_NANO] = &TimeServiceStub::OnGetThreadTimeNs;
    memberFuncMap_[CREATE_TIMER] = &TimeServiceStub::OnCreateTimer;
    memberFuncMap_[START_TIMER] = &TimeServiceStub::OnStartTimer;
    memberFuncMap_[STOP_TIMER] = &TimeServiceStub::OnStopTimer;
    memberFuncMap_[DESTORY_TIMER] = &TimeServiceStub::OnDestoryTimer;
}

TimeServiceStub::~TimeServiceStub()
{
    memberFuncMap_.clear();
}

int32_t TimeServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start##code = %{public}u", code);
    std::u16string myDescripter = TimeServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t p = IPCSkeleton::GetCallingPid();
    pid_t p1 = IPCSkeleton::GetCallingUid();
    TIME_HILOGI(TIME_MODULE_SERVICE,
                "CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u", p, p1, code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
    return ret;
}

int32_t TimeServiceStub::OnSetTime(MessageParcel& data, MessageParcel& reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t time = data.ReadInt64();

    int32_t ret = SetTime(time);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
    return ret;
}

int32_t TimeServiceStub::OnSetTimeZone(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    std::string timeZoneId = data.ReadString();

    int32_t ret = SetTimeZone(timeZoneId);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
    return ret;
}

int32_t TimeServiceStub::OnGetTimeZone(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    std::string timeZoneId;

    int32_t ret = GetTimeZone(timeZoneId);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteString(timeZoneId);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetWallTimeMs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetWallTimeMs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetWallTimeNs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetWallTimeNs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetBootTimeMs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetBootTimeMs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetBootTimeNs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetBootTimeNs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetMonotonicTimeMs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetMonotonicTimeMs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetMonotonicTimeNs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetMonotonicTimeNs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetThreadTimeMs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;

    int32_t ret = GetThreadTimeMs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnGetThreadTimeNs(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, " start.");
    int64_t times;
    int32_t ret = GetThreadTimeNs(times);
    if (ret != ERR_OK) {
        TIME_HILOGE(TIME_MODULE_SERVICE, " end##ret = %{public}d", ret);
        return ret;
    }
    reply.WriteInt64(times);
    TIME_HILOGI(TIME_MODULE_SERVICE, " end.");
    return ret;
}

int32_t TimeServiceStub::OnCreateTimer(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "start.");
 
    auto type = data.ReadInt32();
    auto repeat = data.ReadBool();
    auto interval = data.ReadUint64();

    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    if (obj == nullptr) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Input nullptr");
        return E_TIME_PARAMETERS_INVALID;
    }
    auto timerId = CreateTimer(type, repeat, interval, obj);
    if (timerId == 0 ) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Create timer failed");
        return E_TIME_DEAL_FAILED;
    }
    if (!reply.WriteUint64(timerId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t TimeServiceStub::OnStartTimer(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "start.");
    auto timerId = data.ReadUint64();
    auto triggerTime = data.ReadUint64();
    if (!StartTimer(timerId, triggerTime)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Failed to start timer");
        return E_TIME_DEAL_FAILED;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t TimeServiceStub::OnStopTimer(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "start.");
    auto timerId = data.ReadUint64();
    if (!StopTimer(timerId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Failed to stop timer");
        return E_TIME_DEAL_FAILED;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t TimeServiceStub::OnDestoryTimer(MessageParcel &data, MessageParcel &reply)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "start.");
    auto timerId = data.ReadUint64();
    if (!DestroyTimer(timerId)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Failed to destory timer");
        return E_TIME_DEAL_FAILED;
    }
    TIME_HILOGI(TIME_MODULE_SERVICE, "end.");
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS