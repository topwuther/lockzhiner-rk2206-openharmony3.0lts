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

#ifndef MULTI_VER_NATURAL_STORE_TRANSFER_DATA_H
#define MULTI_VER_NATURAL_STORE_TRANSFER_DATA_H

#ifndef OMIT_MULTI_VER
#include "db_types.h"
#include "macro_utils.h"

namespace DistributedDB {
class MultiVerNaturalStoreTransferData {
public:
    MultiVerNaturalStoreTransferData() {};
    ~MultiVerNaturalStoreTransferData() {};

    DISABLE_COPY_ASSIGN_MOVE(MultiVerNaturalStoreTransferData);

    int SegmentAndTransferValueToHash(const Value &oriValue, std::vector<Value> &partValues) const;

private:
    size_t sliceLengthThreshold_ = 4194304; // 4MB
    size_t blockSizeByte_ = 4194304; // 4MB
};
} // namespace DistributedDB

#endif  // MULTI_VER_NATURAL_STORE_CONNECTION_H
#endif