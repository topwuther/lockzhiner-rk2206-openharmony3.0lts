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
#include "../include/file_info.h"

#include <cstring>

#include "string_ex.h"

namespace OHOS {
namespace FileManager {
bool FileInfo::ReadFromParcel(Parcel &parcel)
{
    fileName = Str16ToStr8(parcel.ReadString16());
    fileUri = Str16ToStr8(parcel.ReadString16());
    typeDir = parcel.ReadInt32();
    mimeType = Str16ToStr8(parcel.ReadString16());
    lastUseTime = parcel.ReadInt64();
    fileSize = parcel.ReadInt32();
    dirNum = parcel.ReadInt32();
    return true;
}

FileInfo *FileInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) FileInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool FileInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(fileName)) && !parcel.WriteString16(Str8ToStr16(fileUri)) &&
        !parcel.WriteInt32(typeDir) && !parcel.WriteString16(Str8ToStr16(mimeType)) &&
        !parcel.WriteInt64(lastUseTime) && !parcel.WriteInt32(typeDir) && !parcel.WriteInt32(typeDir)) {
        return false;
    }
    return true;
}
} // namespace FileManager
} // namespace OHOS
