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

#ifndef MULTI_VER_KVDB_SYNC_INTERFACE_H
#define MULTI_VER_KVDB_SYNC_INTERFACE_H

#include <map>
#include <vector>

#include "multi_ver_def.h"
#include "multi_ver_kv_entry.h"
#include "ikvdb_sync_interface.h"

namespace DistributedDB {
class MultiVerKvDBSyncInterface : public IKvDBSyncInterface {
public:
    // Judge whether the commit existed.
    virtual bool IsCommitExisted(const MultiVerCommitNode &commit) const = 0;

    // Get the latest commits of all devices in the current device.
    virtual int GetDeviceLatestCommit(std::map<std::string, MultiVerCommitNode> &commitMap) const = 0;

    // Get the commit tree and exclude the existed commits from the remote device.
    virtual int GetCommitTree(const std::map<std::string, MultiVerCommitNode> &commitMap,
        std::vector<MultiVerCommitNode> &commits) const = 0;

    // Get all the data from one commit.
    virtual int GetCommitData(const MultiVerCommitNode &commit, std::vector<MultiVerKvEntry *> &entries) const = 0;

    // Create one kv entry from the serialized data from  remote device.
    virtual MultiVerKvEntry *CreateKvEntry(const std::vector<uint8_t> &data) = 0;

    // Release the kv entry created from the interface of CreateKvEntry.
    virtual void ReleaseKvEntry(const MultiVerKvEntry *entry) = 0;

    // Judge whether the slice hash-value existed.
    virtual bool IsValueSliceExisted(const ValueSliceHash &value) const = 0;

    // Get the value according the slice hash value, and push the value to the remote.
    virtual int GetValueSlice(const ValueSliceHash &hashValue, ValueSlice &sliceValue) const = 0;

    // Put the value when put the remote data into the local database.
    virtual int PutValueSlice(const ValueSliceHash &hashValue, const ValueSlice &sliceValue) const = 0;

    // Put all the kv entries of one commit received from the remote.
    virtual int PutCommitData(const MultiVerCommitNode &commit, const std::vector<MultiVerKvEntry *> &entries,
        const std::string &deviceName) = 0;

    // Merge the remote commit into the local tree.
    virtual int MergeSyncCommit(const MultiVerCommitNode &commit, const std::vector<MultiVerCommitNode> &commits) = 0;

    virtual void NotifyStartSyncOperation() = 0;

    virtual void NotifyFinishSyncOperation() = 0;

    virtual int TransferSyncCommitDevInfo(MultiVerCommitNode &commit, const std::string &devId,
        bool isSyncedIn) const = 0;
};
}

#endif // MULTI_VER_KVDB_SYNC_INTERFACE_H
