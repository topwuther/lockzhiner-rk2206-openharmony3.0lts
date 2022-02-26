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

#ifndef MULTI_VER_SYNC_TASK_CONTEXT_H
#define MULTI_VER_SYNC_TASK_CONTEXT_H

#ifndef OMIT_MULTI_VER
#include "sync_task_context.h"
#include "multi_ver_kvdb_sync_interface.h"

namespace DistributedDB {
class MultiVerSyncTaskContext final : public SyncTaskContext {
public:
    MultiVerSyncTaskContext() {};

    DISABLE_COPY_ASSIGN_MOVE(MultiVerSyncTaskContext);

    // Init the MultiVerSyncTaskContext
    int Initialize(const std::string &deviceId, IKvDBSyncInterface *syncInterface, std::shared_ptr<Metadata> &metadata,
        ICommunicator *communicator) override;

    // Add a sync task target with the operation to the queue
    int AddSyncOperation(SyncOperation *operation) override;

    int GetCommitIndex() const;

    void SetCommitIndex(int index);

    int GetEntriesIndex() const;

    void SetEntriesIndex(int index);

    int GetValueSlicesIndex() const;

    void SetValueSlicesIndex(int index);

    void GetCommits(std::vector<MultiVerCommitNode> &commits);

    void SetCommits(const std::vector<MultiVerCommitNode> &commits);

    void GetCommit(int index, MultiVerCommitNode &commit) const;

    void SetCommit(int index, const MultiVerCommitNode &commit);

    void SetEntries(const std::vector<MultiVerKvEntry *> &entries);

    void ReleaseEntries(void);

    void GetEntries(std::vector<MultiVerKvEntry *> &entries) const;

    void GetEntry(int index, MultiVerKvEntry *&entry);

    void SetCommitsSize(int commitsSize);

    int GetCommitsSize() const;

    void SetEntriesSize(int entriesSize);

    int GetEntriesSize() const;

    void SetValueSlicesSize(int valueSlicesSize);

    int GetValueSlicesSize() const;

    void GetValueSliceHashNode(int index, ValueSliceHash &hashNode) const;

    void SetValueSliceHashNodes(const std::vector<ValueSliceHash> &valueSliceHashNodes);

    void GetValueSliceHashNodes(std::vector<ValueSliceHash> &valueSliceHashNodes) const;

    void Clear() override;

protected:
    ~MultiVerSyncTaskContext() override;

    void CopyTargetData(const ISyncTarget *target) override;

private:
    DECLARE_OBJECT_TAG(MultiVerSyncTaskContext);

    std::vector<MultiVerCommitNode> commits_;
    std::vector<MultiVerKvEntry *> entries_;
    std::vector<ValueSliceHash> valueSliceHashNodes_;
    int commitsIndex_ = 0;
    int commitsSize_ = 0;
    int entriesIndex_ = 0;
    int entriesSize_ = 0;
    int valueSlicesIndex_ = 0;
    int valueSlicesSize_ = 0;
};
} // namespace DistributedDB

#endif // MULTI_VER_SYNC_TASK_CONTEXT_H
#endif