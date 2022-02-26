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

#include "ikvdb_factory.h"

namespace DistributedDB {
IKvDBFactory *IKvDBFactory::factory_ = nullptr;
std::mutex IKvDBFactory::instanceLock_;

// Get current factory object.
IKvDBFactory *IKvDBFactory::GetCurrent()
{
    std::lock_guard<std::mutex> lockGuard(IKvDBFactory::instanceLock_);
    return IKvDBFactory::factory_;
}

// Set the factory object to 'current'
void IKvDBFactory::Register(IKvDBFactory *factory)
{
    std::lock_guard<std::mutex> lockGuard(IKvDBFactory::instanceLock_);
    IKvDBFactory::factory_ = factory;
}
} // namespace DistributedDB