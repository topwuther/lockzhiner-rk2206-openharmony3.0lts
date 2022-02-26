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

#ifndef OHOS_SERIALIZABLE_H
#define OHOS_SERIALIZABLE_H

#include <json/json.h>

namespace OHOS::DistributedKv {
struct Serializable {
    static inline std::string GetVal(const Json::Value &value, const std::string &def)
    {
        if (value.isNull()) {
            return def;
        }
        return value.asString();
    }

    static inline uint32_t GetVal(const Json::Value &value, const uint32_t def)
    {
        if (value.isNull()) {
            return def;
        }
        return value.asUInt();
    }

#ifndef GET_NAME
#define GET_NAME(value) #value
#endif
};
}

#endif // OHOS_SERIALIZABLE_H
