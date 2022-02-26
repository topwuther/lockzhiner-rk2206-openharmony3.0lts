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

#ifndef OBJECTHOLDERTYPED_H
#define OBJECTHOLDERTYPED_H

#include "object_holder.h"

namespace DistributedDB {
template<typename T>
class ObjectHolderTyped : public ObjectHolder {
public:
    // Accept a heap object
    ObjectHolderTyped(T *inObject)
    {
        objectPtr_ = inObject;
    }

    ~ObjectHolderTyped() override
    {
        if (objectPtr_ != nullptr) {
            delete objectPtr_;
            objectPtr_ = nullptr;
        }
    }

    const T *GetObject() const
    {
        return objectPtr_;
    }
private:
    T *objectPtr_ = nullptr;
};
} // namespace DistributedDB

#endif // OBJECTHOLDERTYPED_H
