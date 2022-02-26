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

#ifndef DEVICE_KVSTORE_RESULTSET_IMPL_H
#define DEVICE_KVSTORE_RESULTSET_IMPL_H

#include "kvstore_resultset_impl.h"

namespace OHOS::DistributedKv {
class DeviceKvStoreResultSetImpl : public KvStoreResultSetImpl {
public:
    DeviceKvStoreResultSetImpl(DistributedDB::Key tmpKeyPrefix,
        DistributedDB::KvStoreResultSet *kvStoreResultSet, bool deviceCoordinate);
    DeviceKvStoreResultSetImpl(DistributedDB::KvStoreResultSet *kvStoreResultSet,
        bool deviceCoordinate);
    ~DeviceKvStoreResultSetImpl();
    Status GetEntry(Entry &entry) override;
private:
    bool deviceCoordinate_;
};
}

#endif // DEVICE_KVSTORE_RESULTSET_IMPL_H
