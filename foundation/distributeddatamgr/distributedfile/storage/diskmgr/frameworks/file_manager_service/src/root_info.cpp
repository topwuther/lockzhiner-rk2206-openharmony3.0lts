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
#include "../include/root_info.h"

#include <cstring>

#include "string_ex.h"

namespace OHOS {
namespace FileManager {
bool RootInfo::ReadFromParcel(Parcel &parcel)
{
    path = Str16ToStr8(parcel.ReadString16());
    id = Str16ToStr8(parcel.ReadString16());
    mountFlags = parcel.ReadInt32();
    diskId = Str16ToStr8(parcel.ReadString16());
    return true;
}

RootInfo *RootInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) RootInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool RootInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(path)) && !parcel.WriteString16(Str8ToStr16(id)) &&
        !parcel.WriteInt32(mountFlags) && !parcel.WriteString16(Str8ToStr16(diskId))) {
        return false;
    }
    return true;
}
} // namespace FileManager
} // namespace OHOS