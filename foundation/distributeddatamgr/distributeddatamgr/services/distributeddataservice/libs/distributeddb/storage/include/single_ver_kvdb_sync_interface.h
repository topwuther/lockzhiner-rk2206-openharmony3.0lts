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

#ifndef SINGLE_VER_KVDB_SYNC_INTERFACE_H
#define SINGLE_VER_KVDB_SYNC_INTERFACE_H

#include "ikvdb_sync_interface.h"
#include "single_ver_kv_entry.h"
#include "iprocess_system_api_adapter.h"

namespace DistributedDB {
class SingleVerKvDBSyncInterface : public IKvDBSyncInterface {
public:
    ~SingleVerKvDBSyncInterface() override {};

    // Get the data which would be synced to other devices according the timestamp.
    // if the data size is over than the blockSize, It would alloc one token and assign to continueStmtToken,
    // it should be released when the read operation terminate.
    virtual int GetSyncData(TimeStamp begin, TimeStamp end, std::vector<DataItem> &dataItems,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const = 0;

    virtual int GetSyncData(TimeStamp begin, TimeStamp end, std::vector<SingleVerKvEntry *> &entries,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const = 0;

    // Get the data using the token allocated by GetSyncData, the token would be release automatically when finished.
    virtual int GetSyncDataNext(std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const = 0;

    virtual int GetSyncDataNext(std::vector<SingleVerKvEntry *> &entries, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const = 0;

    // Release the continue token of getting data.
    virtual void ReleaseContinueToken(ContinueToken &continueStmtToken) const = 0;

    // Put synced data from remote devices.
    virtual int PutSyncData(std::vector<DataItem> &dataItems, const std::string &deviceName) = 0;

    virtual int PutSyncData(const std::vector<SingleVerKvEntry *> &entries, const std::string &deviceName) = 0;

    virtual void ReleaseKvEntry(const SingleVerKvEntry *entry) = 0;

    virtual int RemoveDeviceData(const std::string &deviceName, bool isNeedNotify) = 0;

    virtual SchemaObject GetSchemaInfo() const = 0;

    virtual bool CheckCompatible(const std::string &schema) const = 0;

    virtual int GetSecurityOption(SecurityOption &option) const = 0;

    virtual bool IsReadable() const = 0;

    virtual void NotifyRemotePushFinished(const std::string &targetId) const = 0;
};
}

#endif // SINGLE_VER_KVDB_SYNC_INTERFACE_H
