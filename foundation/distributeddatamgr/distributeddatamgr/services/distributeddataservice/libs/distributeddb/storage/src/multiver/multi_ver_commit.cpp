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

#ifndef OMIT_MULTI_VER
#include "log_print.h"
#include "multi_ver_commit.h"

namespace DistributedDB {
MultiVerCommit::MultiVerCommit()
    : versionInfo_(0),
      timestamp_(0),
      localFlag_(true)
{}

MultiVerCommit::~MultiVerCommit()
{}

Version MultiVerCommit::GetCommitVersion() const
{
    return versionInfo_;
}

void MultiVerCommit::SetCommitVersion(const Version &versionInfo)
{
    versionInfo_ = versionInfo;
    return;
}

CommitID MultiVerCommit::GetCommitId() const
{
    return commitID_;
}

void MultiVerCommit::SetCommitId(const CommitID &id)
{
    commitID_ = id;
    return;
}

CommitID MultiVerCommit::GetLeftParentId() const
{
    return leftParentID_;
}

void MultiVerCommit::SetLeftParentId(const CommitID &id)
{
    leftParentID_ = id;
    return;
}

CommitID MultiVerCommit::GetRightParentId() const
{
    return rightParentID_;
}

void MultiVerCommit::SetRightParentId(const CommitID &id)
{
    rightParentID_ = id;
    return;
}

TimeStamp MultiVerCommit::GetTimestamp() const
{
    return timestamp_;
}

void MultiVerCommit::SetTimestamp(TimeStamp timestamp)
{
    timestamp_ = timestamp;
    return;
}

bool MultiVerCommit::GetLocalFlag() const
{
    return localFlag_;
}

void MultiVerCommit::SetLocalFlag(bool localFlag)
{
    localFlag_ = localFlag;
    return;
}

DeviceID MultiVerCommit::GetDeviceInfo() const
{
    return deviceInfo_;
}

void MultiVerCommit::SetDeviceInfo(const DeviceID &deviceInfo)
{
    deviceInfo_ = deviceInfo;
    return;
}

bool MultiVerCommit::CheckCommit() const
{
    if (commitID_.size() == 0 || commitID_.size() > MAX_COMMIT_ID_LENGTH ||
        leftParentID_.size() > MAX_COMMIT_ID_LENGTH || rightParentID_.size() > MAX_COMMIT_ID_LENGTH ||
        deviceInfo_.size() > MAX_COMMIT_DEV_LENGTH) {
        LOGE("Check commit failed! Error length of commit ID.");
        return false;
    }
    // commitId should not equal to the parentId; the left parent should not equal to the right
    if (commitID_ == leftParentID_ || commitID_ == rightParentID_ ||
        (leftParentID_ == rightParentID_ && leftParentID_.size() != 0)) {
        LOGE("Check commit failed! Wrong commit ID.");
        return false;
    }
    return true;
}
}
#endif