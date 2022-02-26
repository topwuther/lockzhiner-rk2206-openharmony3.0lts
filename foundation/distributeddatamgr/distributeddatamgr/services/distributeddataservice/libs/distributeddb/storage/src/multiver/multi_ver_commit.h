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

#ifndef MULTI_VER_COMMIT_H
#define MULTI_VER_COMMIT_H

#ifndef OMIT_MULTI_VER
#include "ikvdb_commit.h"

#include "multi_ver_def.h"
#include "macro_utils.h"

namespace DistributedDB {
class MultiVerCommit final : public IKvDBCommit {
public:
    MultiVerCommit();
    ~MultiVerCommit() override;

    DISABLE_COPY_ASSIGN_MOVE(MultiVerCommit);

    Version GetCommitVersion() const override;
    void SetCommitVersion(const Version &versionInfo) override;

    CommitID GetCommitId() const override;
    void SetCommitId(const CommitID &id) override;

    CommitID GetLeftParentId() const override;
    void SetLeftParentId(const CommitID &id) override;

    CommitID GetRightParentId() const override;
    void SetRightParentId(const CommitID &id) override;

    TimeStamp GetTimestamp() const override;
    void SetTimestamp(TimeStamp timestamp) override;

    bool GetLocalFlag() const override;
    void SetLocalFlag(bool localFlag) override;

    DeviceID GetDeviceInfo() const override;
    void SetDeviceInfo(const DeviceID &deviceInfo) override;

    bool CheckCommit() const;

private:
    static const size_t MAX_VERSION_INFO_LENGTH = 4096;
    static const size_t MAX_COMMIT_ID_LENGTH = 128;
    static const size_t MAX_COMMIT_DEV_LENGTH = 256;
    Version versionInfo_;
    CommitID commitID_;
    CommitID leftParentID_;
    CommitID rightParentID_;
    TimeStamp timestamp_;
    bool localFlag_;
    DeviceID deviceInfo_;
};
}

#endif
#endif