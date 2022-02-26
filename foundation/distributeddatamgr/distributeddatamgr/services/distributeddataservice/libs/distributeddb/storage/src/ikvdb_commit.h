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

#ifndef IKVDB_COMMIT_H
#define IKVDB_COMMIT_H

#include "db_types.h"
#include "multi_ver_def.h"

namespace DistributedDB {
class IKvDBCommit {
public:
    virtual ~IKvDBCommit() {};
    virtual Version GetCommitVersion() const = 0;
    virtual void SetCommitVersion(const Version &versionInfo) = 0;
    virtual CommitID GetCommitId() const = 0;
    virtual void SetCommitId(const CommitID &id) = 0;
    virtual CommitID GetLeftParentId() const = 0;
    virtual void SetLeftParentId(const CommitID &id) = 0;
    virtual CommitID GetRightParentId() const = 0;
    virtual void SetRightParentId(const CommitID &id) = 0;
    virtual TimeStamp GetTimestamp() const = 0;
    virtual void SetTimestamp(TimeStamp timestamp) = 0;
    virtual bool GetLocalFlag() const = 0;
    virtual void SetLocalFlag(bool localFlag) = 0;
    virtual DeviceID GetDeviceInfo() const = 0;
    virtual void SetDeviceInfo(const DeviceID &deviceInfo) = 0;
};
}

#endif
