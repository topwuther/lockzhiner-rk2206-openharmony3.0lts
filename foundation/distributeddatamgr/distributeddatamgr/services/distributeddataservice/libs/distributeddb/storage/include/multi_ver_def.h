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

#ifndef MULTI_VER_DEF_H
#define MULTI_VER_DEF_H

#include <list>
#include <string>
#include <vector>

#include "db_types.h"

namespace DistributedDB {
using CommitID = std::vector<uint8_t>;
using Version = uint64_t;
static const size_t MULTI_VER_TAG_SIZE = 8;

struct MultiVerCommitNode {
    static const uint64_t LOCAL_FLAG = 1;
    static const uint64_t NON_LOCAL_FLAG = 0;
    std::vector<uint8_t> commitId;
    std::vector<uint8_t> leftParent;
    std::vector<uint8_t> rightParent;
    uint64_t timestamp = 0;
    uint64_t version = 0; // version for storage
    uint64_t isLocal = 0; // merge node or native node
    std::string deviceInfo; // device name
};

struct MultiVerEntryAuxData {
    uint64_t operFlag = 0;
    uint64_t timestamp = 0;
    uint64_t oriTimestamp = 0;
};

struct MultiVerEntryData {
    Key key;
    Value value;
    MultiVerEntryAuxData auxData; // auxiliaries
};

struct MultiVerTrimedVersionData {
    Key key; // hash key
    uint64_t operFlag = 0;
    uint64_t version = 0;
};

struct MultiVerDiffData {
    std::list<Entry> inserted;
    std::list<Entry> updated;
    std::list<Entry> deleted;
    bool isCleared = false;
    void Reset()
    {
        inserted.clear();
        updated.clear();
        deleted.clear();
        isCleared = false;
    }
};

enum class MultiVerDataType {
    NATIVE_TYPE,
    ALL_TYPE,
};
}

#endif // MULTI_VER_DEF_H
