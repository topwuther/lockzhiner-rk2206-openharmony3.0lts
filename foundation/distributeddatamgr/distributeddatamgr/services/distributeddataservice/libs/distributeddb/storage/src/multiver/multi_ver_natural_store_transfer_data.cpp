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

#ifndef OMIT_MULTI_VER
#include "multi_ver_natural_store_transfer_data.h"

#include "db_constant.h"
#include "log_print.h"
#include "db_errno.h"

namespace DistributedDB {
int MultiVerNaturalStoreTransferData::SegmentAndTransferValueToHash(const Value &oriValue,
    std::vector<Value> &partValues) const
{
    if (oriValue.size() <= sliceLengthThreshold_) {
        return -E_UNEXPECTED_DATA;
    }

    const uint32_t sizeByte = blockSizeByte_;
    if (sizeByte == 0) {
        return -E_UNEXPECTED_DATA;
    }

    const size_t partNum = oriValue.size() / sizeByte;

    for (size_t i = 0; i < partNum; i++) {
        Value tempValue(sizeByte);
        // When the hash value is combined, the overlapped part is removed. So not need -1 at tail
        std::copy(oriValue.begin() + i * sizeByte, oriValue.begin() + sizeByte * (i + 1), tempValue.begin());
        partValues.push_back(std::move(tempValue));
    }
    Value tailValue(oriValue.size() - partNum * sizeByte);
    std::copy(oriValue.begin() + partNum * sizeByte, oriValue.end(), tailValue.begin());
    if (!tailValue.empty()) {
        partValues.push_back(tailValue);
    }

    return E_OK;
}
} // namespace DistributedDB
#endif
