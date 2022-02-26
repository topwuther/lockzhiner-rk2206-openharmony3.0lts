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

#include "system_ability_info.h"

#include "sam_log.h"

namespace OHOS {
bool SystemAbilityInfo::ReadFromParcel(Parcel& parcel)
{
    if (!parcel.ReadInt32(systemAbilityId)) {
        HILOGW("read systemAbilityId from parcel failed");
        return false;
    }
    if (!parcel.ReadString(deviceId)) {
        HILOGW("read deviceId from parcel failed");
        return false;
    }
    return true;
}

SystemAbilityInfo* SystemAbilityInfo::Unmarshalling(Parcel& parcel)
{
    SystemAbilityInfo* info = new SystemAbilityInfo();
    if (info != nullptr && !info->ReadFromParcel(parcel)) {
        HILOGW("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool SystemAbilityInfo::Marshalling(Parcel& parcel) const
{
    if (!parcel.WriteInt32(systemAbilityId)) {
        HILOGW("write systemAbilityId failed");
        return false;
    }
    if (!parcel.WriteString(deviceId)) {
        HILOGW("write deviceId failed");
        return false;
    }
    return true;
}
} // namespace OHOS