/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is: distributed on an "AS is:"BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "time_service_proxy.h"
#include "iremote_broker.h"
#include "time_common.h"
#include "time_service_interface.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::HiviewDFX;

TimeServiceProxy::TimeServiceProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<ITimeService>(object)
{
}

int32_t TimeServiceProxy::SetTime(const int64_t time)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteInt64(time)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }
    int32_t result = Remote()->SendRequest(SET_TIME, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "SetTime failed, error code is: %{public}d", result);
        return result;
    }
    return result;
}

uint64_t TimeServiceProxy::CreateTimer(int32_t type, bool repeat, uint64_t interval, sptr<IRemoteObject> &timerCallback)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return 0;
    }
    
    if (!data.WriteInt32(type)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return 0;
    }
    
    if (!data.WriteBool(repeat)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return 0;
    }

    if (!data.WriteUint64(interval)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return 0;
    }
    
    if (!data.WriteRemoteObject(timerCallback)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return 0;
    }
    int32_t result = Remote()->SendRequest(CREATE_TIMER, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "CreateTimer failed, error code is: %{public}d", result);
        return 0;
    }
    auto TimerId = reply.ReadUint64();

    return TimerId;
}

bool TimeServiceProxy::StartTimer(uint64_t timerId, uint64_t triggerTimes) 
{
    MessageParcel data, reply;
    MessageOption option;
    
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteUint64(timerId)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteUint64(triggerTimes)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }
    int32_t result = Remote()->SendRequest(START_TIMER, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Start failed, error code is: %{public}d", result);
        return false;
    }
    return true;
}

bool TimeServiceProxy::StopTimer(uint64_t  timerId) 
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteUint64(timerId)) { 
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }
    int32_t result = Remote()->SendRequest(STOP_TIMER, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Stop failed, error code is: %{public}d", result);
        return false;
    }
    
    return true;
}
bool TimeServiceProxy::DestroyTimer(uint64_t  timerId) 
{
    MessageParcel data, reply;
    MessageOption option;
    
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteUint64(timerId)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }
    int32_t result = Remote()->SendRequest(DESTORY_TIMER, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return false;
    }
    
    return true;
}

int32_t TimeServiceProxy::SetTimeZone(const std::string timezoneId)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteString(timezoneId)) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(SET_TIME_ZONE, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "SetTimeZone failed, error code is: %{public}d", result);
        return result;
    }
    return result;
}

int32_t TimeServiceProxy::GetTimeZone(std::string &timezoneId)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_TIME_ZONE, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetTimeZone failed, error code is: %{public}d", result);
        return result;
    }
    timezoneId = reply.ReadString();
    return result;
}

int32_t TimeServiceProxy::GetWallTimeMs(int64_t &times) 
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_WALL_TIME_MILLI, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetWallTimeMs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetWallTimeNs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_WALL_TIME_NANO, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetWallTimeNs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetBootTimeMs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_BOOT_TIME_MILLI, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetBootTimeMs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetBootTimeNs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_BOOT_TIME_MILLI, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetBootTimeNs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetMonotonicTimeMs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_MONO_TIME_MILLI, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetMonotonicTimeMs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetMonotonicTimeNs(int64_t &times) 
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_MONO_TIME_NANO, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetMonotonicTimeNs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetThreadTimeMs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_THREAD_TIME_MILLI, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetThreadTimeMs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}

int32_t TimeServiceProxy::GetThreadTimeNs(int64_t &times)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "Failed to write parcelable");
        return E_TIME_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(GET_THREAD_TIME_NANO, data, reply, option);
    if (result != ERR_NONE) {
        TIME_HILOGE(TIME_MODULE_CLIENT, "GetThreadTimeNs failed, error code is: %{public}d", result);
        return result;
    }
    times = reply.ReadInt64();
    return result;
}
} // namespace MiscServices
} // namespace OHOS