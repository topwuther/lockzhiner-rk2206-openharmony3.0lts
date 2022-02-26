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

#ifndef I_KV_DB_H
#define I_KV_DB_H

#include <functional>
#include <string>

#include "ref_object.h"
#include "macro_utils.h"
#include "kvdb_properties.h"
#include "ikvdb_connection.h"

namespace DistributedDB {
class IKvDB : public virtual RefObject {
public:
    IKvDB() = default;
    ~IKvDB() override {}
    DISABLE_COPY_ASSIGN_MOVE(IKvDB);

    // Open the database.
    virtual int Open(const KvDBProperties &kvDBProp) = 0;

    // Get the properties object of this database.
    virtual const KvDBProperties &GetMyProperties() const = 0;

    // Create a db connection.
    virtual IKvDBConnection *GetDBConnection(int &errCode) = 0;

    // Register callback invoked when all connections released.
    virtual void OnClose(const std::function<void(void)> &func) = 0;

    virtual void OpenPerformanceAnalysis() = 0;

    virtual void ClosePerformanceAnalysis() = 0;

    virtual void WakeUpSyncer() = 0;

    virtual void SetCorruptHandler(const DatabaseCorruptHandler &handler) = 0;

    virtual int RemoveKvDB(const KvDBProperties &properties) = 0;
    virtual int GetKvDBSize(const KvDBProperties &properties, uint64_t &size) const = 0;

    virtual void EnableAutonomicUpgrade() = 0;
};
} // namespace DistributedDB

#endif // I_KV_DB_H
