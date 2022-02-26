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

#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <vector>
#include <map>

#include "generic_values.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DataStorage {
public:
    enum DataType { PERMISSION_DEF, PERMISSIONS_STAT_USER_GRANTED, PERMISSIONS_STAT_SYSTEM_GRANTED };

    static DataStorage& GetRealDataStorage();

    virtual ~DataStorage() = default;

    virtual int Add(const DataType type, const std::vector<GenericValues>& values) = 0;

    virtual int Remove(const DataType type, const GenericValues& conditions) = 0;

    virtual int Modify(const DataType type, const GenericValues& modifyValues, const GenericValues& conditions) = 0;

    virtual int Find(const DataType type, std::vector<GenericValues>& results) = 0;

    virtual int RefreshAll(const DataType type, const std::vector<GenericValues>& values) = 0;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS

#endif // DATA_STORAGE_H