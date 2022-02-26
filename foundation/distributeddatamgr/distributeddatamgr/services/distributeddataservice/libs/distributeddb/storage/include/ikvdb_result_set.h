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

#ifndef I_KV_DB_RESULT_SET_H
#define I_KV_DB_RESULT_SET_H

#include "macro_utils.h"
#include "db_types.h"

namespace DistributedDB {
class IKvDBResultSet {
public:
    IKvDBResultSet() = default;
    virtual ~IKvDBResultSet() {}

    DISABLE_COPY_ASSIGN_MOVE(IKvDBResultSet);

    // Initialize logic
    virtual int Open(bool isMemDb) = 0;

    // Get total entries count.
    // >= 0: count, < 0: errCode.
    virtual int GetCount() const = 0;

    // Get current read position.
    // >= 0: position, < 0: errCode
    virtual int GetPosition() const = 0;

    // Move the read position to an absolute position value.
    virtual int MoveTo(int position) const = 0;

    // Get the entry of current position.
    virtual int GetEntry(Entry &entry) const = 0;

    // Finalize logic
    virtual void Close() = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_RESULT_SET_H
