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

#ifndef RES_FINALIZER_H
#define RES_FINALIZER_H

#include <functional>
#include "macro_utils.h"

namespace DistributedDB {
// RAII style resource finalizer for using in functions where the resource should be finalized before each return after
// the resource had been allocated. Just create an instance as function local stack variable and provide finalizer
// function after where the resource allocated. Suggest using this RAII style instead of using goto statement.
class ResFinalizer {
public:
    explicit ResFinalizer(const std::function<void(void)> &inFinalizer) : finalizer_(inFinalizer) {}
    ~ResFinalizer()
    {
        if (finalizer_) {
            finalizer_();
        }
    }

    DISABLE_COPY_ASSIGN_MOVE(ResFinalizer);
private:
    std::function<void(void)> finalizer_;
};
} // namespace DistributedDB
#endif // RES_FINALIZER_H