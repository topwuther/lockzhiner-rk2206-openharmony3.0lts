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

#ifndef I_KV_DB_RAW_CURSOR_H
#define I_KV_DB_RAW_CURSOR_H

#include "macro_utils.h"
#include "db_types.h"

namespace DistributedDB {
class IKvDBRawCursor {
public:
    IKvDBRawCursor() = default;
    virtual ~IKvDBRawCursor() {}

    DISABLE_COPY_ASSIGN_MOVE(IKvDBRawCursor);

    // Open the raw cursor.
    virtual int Open() = 0;

    // Close the raw cursor before invoking operator delete.
    virtual void Close() = 0;

    // Reload all data.
    virtual int Reload() = 0;

    // Get total entries count.
    virtual int GetCount() const = 0;

    // Get next entry, return errCode if it fails.
    virtual int GetNext(Entry &entry, bool isCopy) const = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_RAW_CURSOR_H
