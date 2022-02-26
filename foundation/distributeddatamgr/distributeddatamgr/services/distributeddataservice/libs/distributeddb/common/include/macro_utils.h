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

#ifndef MACRO_UTILS_H
#define MACRO_UTILS_H

namespace DistributedDB {
#define DISABLE_COPY_ASSIGN_MOVE(ClassName) \
    ClassName(const ClassName &) = delete; \
    ClassName(ClassName &&) = delete; \
    ClassName& operator=(const ClassName &) = delete; \
    ClassName& operator=(ClassName &&) = delete

#define DECLARE_OBJECT_TAG(ClassName) \
    virtual std::string GetObjectTag() const override; \
    constexpr static const char * const classTag = "Class-"#ClassName

#define DEFINE_OBJECT_TAG_FACILITIES(ClassName) \
    std::string ClassName::GetObjectTag() const \
    { \
        return ClassName::classTag; \
    }

#define BYTE_8_ALIGN(x) (((x) + (8 - 1)) & ~(8 - 1))

#define BITX(x) (1 << (x))

#define ULL(x) (static_cast<unsigned long long>(x))

// Convert var or enum to variable name for printf
#define VNAME(name) (#name)
}
#endif // MACRO_UTILS_H
