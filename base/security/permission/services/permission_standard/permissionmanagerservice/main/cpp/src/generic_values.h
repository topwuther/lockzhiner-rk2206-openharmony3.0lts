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

#ifndef GENERIC_VALUES_H
#define GENERIC_VALUES_H

#include <map>
#include <vector>
#include <string>

#include "variant_value.h"

namespace OHOS {
namespace Security {
namespace Permission {
class GenericValues final {
public:
    GenericValues() = default;
    virtual ~GenericValues() = default;

    void Put(const std::string& key, int value);

    void Put(const std::string& key, const std::string& value);

    void Put(const std::string& key, const VariantValue& value);

    std::vector<std::string> GetAllKeys() const;

    VariantValue Get(const std::string& key) const;

    int GetInt(const std::string& key) const;

    std::string GetString(const std::string& key) const;

private:
    std::map<std::string, VariantValue> map_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // GENERIC_VALUES_H
