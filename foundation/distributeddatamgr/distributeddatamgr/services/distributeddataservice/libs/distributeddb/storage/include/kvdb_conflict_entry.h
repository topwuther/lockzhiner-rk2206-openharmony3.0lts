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

#ifndef KVDB_CONFLICT_ENTRY_H
#define KVDB_CONFLICT_ENTRY_H

#include "db_types.h"

namespace DistributedDB {
struct ConflictData {
    Value value;
    bool isDeleted = false;
    bool isLocal = false;
};

struct KvDBConflictEntry {
    int type = 1;
    Key key;
    ConflictData oldData;
    ConflictData newData;
};
} // namespace DistributedDB

#endif // KVDB_CONFLICT_ENTRY_H
