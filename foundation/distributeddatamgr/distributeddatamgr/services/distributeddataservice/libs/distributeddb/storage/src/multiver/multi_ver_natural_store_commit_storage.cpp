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
#include "multi_ver_natural_store_commit_storage.h"

#include <stack>

#include "db_errno.h"
#include "db_constant.h"
#include "log_print.h"
#include "multi_ver_commit.h"
#include "ikvdb_factory.h"
#include "parcel.h"
#include "db_common.h"
#include "sqlite_local_kvdb.h"
#include "kvdb_utils.h"

namespace DistributedDB {
using std::string;
using std::vector;
using std::list;
using std::map;
using std::make_pair;
using std::stack;

namespace {
    const size_t MAX_COMMIT_ST_LENGTH = 4096;
    const Version VERSION_MAX = 0xFFFFFFFFFFFFFFFF;
    const string MULTI_VER_COMMIT_DB_NAME = "commit_logs.db";
}

const string MultiVerNaturalStoreCommitStorage::HEADER_KEY = "header commit";

MultiVerNaturalStoreCommitStorage::MultiVerNaturalStoreCommitStorage()
    : commitStorageDatabase_(nullptr),
      commitStorageDBConnection_(nullptr)
{}

MultiVerNaturalStoreCommitStorage::~MultiVerNaturalStoreCommitStorage()
{
    Close();
}

int MultiVerNaturalStoreCommitStorage::CheckVersion(const Property &property, bool &isDbExist) const
{
    int dbVer = 0;
    int errCode = GetVersion(property, dbVer, isDbExist);
    if (errCode != E_OK) {
        LOGE("[CommitStorage][CheckVer] GetVersion failed, errCode=%d.", errCode);
        return errCode;
    }
    if (!isDbExist) {
        return E_OK;
    }
    LOGD("[CommitStorage][CheckVer] DbVersion=%d, CurVersion=%d.", dbVer, MULTI_VER_COMMIT_STORAGE_VERSION_CURRENT);
    if (dbVer > MULTI_VER_COMMIT_STORAGE_VERSION_CURRENT) {
        LOGE("[CommitStorage][CheckVer] Version Not Support!");
        return -E_VERSION_NOT_SUPPORT;
    }
    return E_OK;
}

int MultiVerNaturalStoreCommitStorage::GetVersion(const IKvDBCommitStorage::Property &property,
    int &version, bool &isDbExisted)
{
    SQLiteLocalKvDB *localKvdb = new (std::nothrow) SQLiteLocalKvDB();
    if (localKvdb == nullptr) {
        return -E_INVALID_DB;
    }

    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, property.isNeedCreate);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.path);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_COMMIT_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);

    int errCode = localKvdb->GetVersion(dbProperties, version, isDbExisted);
    RefObject::DecObjRef(localKvdb);
    localKvdb = nullptr;
    return errCode;
}

int MultiVerNaturalStoreCommitStorage::Open(const IKvDBCommitStorage::Property &property)
{
    if (commitStorageDatabase_ != nullptr && commitStorageDBConnection_ != nullptr) {
        return E_OK;
    }
    IKvDBFactory *factory = IKvDBFactory::GetCurrent();
    if (factory == nullptr) {
        LOGE("Failed to open IKvDB! Get factory failed.");
        return -E_INVALID_DB;
    }
    int errCode = E_OK;
    commitStorageDatabase_ = factory->CreateCommitStorageDB(errCode);
    if (commitStorageDatabase_ == nullptr) {
        LOGE("Failed to create commit storage database:%d", errCode);
        return -E_INVALID_DB;
    }

    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, property.isNeedCreate);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.path);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_COMMIT_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);

    errCode = commitStorageDatabase_->Open(dbProperties);
    if (errCode != E_OK) {
        LOGE("Failed to open commit storage database! err:%d", errCode);
        RefObject::KillAndDecObjRef(commitStorageDatabase_);
        commitStorageDatabase_ = nullptr;
        return errCode;
    }
    commitStorageDBConnection_ = commitStorageDatabase_->GetDBConnection(errCode);
    if (commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get connection for commit storage! err:%d", errCode);
        RefObject::KillAndDecObjRef(commitStorageDatabase_);
        commitStorageDatabase_ = nullptr;
        return errCode;
    }
    // Need to refactor in the future
    errCode = static_cast<SQLiteLocalKvDB *>(commitStorageDatabase_)->SetVersion(dbProperties,
        MULTI_VER_COMMIT_STORAGE_VERSION_CURRENT);
    if (errCode != E_OK) {
        LOGE("[CommitStorage][Open] SetVersion fail, errCode=%d.", errCode);
        Close();
        return errCode;
    }
    return E_OK;
}

void MultiVerNaturalStoreCommitStorage::Close()
{
    if (commitStorageDatabase_ != nullptr && commitStorageDBConnection_ != nullptr) {
        commitStorageDBConnection_->Close();
        commitStorageDBConnection_ = nullptr;
    }
    if (commitStorageDatabase_ != nullptr) {
        IKvDB::DecObjRef(commitStorageDatabase_);
        commitStorageDatabase_ = nullptr;
    }
}

int MultiVerNaturalStoreCommitStorage::Remove(const IKvDBCommitStorage::Property &property)
{
    if (commitStorageDatabase_ != nullptr && commitStorageDBConnection_ != nullptr) {
        commitStorageDBConnection_->Close();
        commitStorageDBConnection_ = nullptr;
        RefObject::DecObjRef(commitStorageDatabase_);
        commitStorageDatabase_ = nullptr;
    }

    std::string dataDir = property.path + ("/" + property.identifierName + "/" + DBConstant::MULTI_SUB_DIR + "/");
    int errCode = KvDBUtils::RemoveKvDB(dataDir, DBConstant::MULTI_VER_COMMIT_STORE);
    if (errCode != E_OK) {
        LOGE("Failed to remove commit storage database! err:%d", errCode);
        return errCode;
    }
    return E_OK;
}

IKvDBCommit *MultiVerNaturalStoreCommitStorage::AllocCommit(int &errCode) const
{
    auto commit = new (std::nothrow) MultiVerCommit();
    if (commit != nullptr) {
        errCode = E_OK;
    } else {
        errCode = -E_OUT_OF_MEMORY;
        LOGE("Failed to alloc commit! Bad alloc.");
    }
    return commit;
}

IKvDBCommit *MultiVerNaturalStoreCommitStorage::GetCommit(const CommitID &commitId, int &errCode) const
{
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get commit! Commit storage do not open.");
        errCode = -E_INVALID_DB;
        return nullptr;
    }
    Key key;
    TransferCommitIDToKey(commitId, key);
    IOption option;
    Value value;
    errCode = commitStorageDBConnection_->Get(option, key, value);
    if (errCode != E_OK) {
        if (errCode != -E_NOT_FOUND) {
            LOGE("Failed to get the commit:%d", errCode);
        }
        return nullptr;
    }

    IKvDBCommit *commit = AllocCommit(errCode);
    if (commit == nullptr) {
        return nullptr;
    }

    errCode = TransferValueToCommit(value, *commit);
    if (errCode != E_OK) {
        delete commit;
        commit = nullptr;
    }
    return commit;
}

int MultiVerNaturalStoreCommitStorage::StartVacuum()
{
    if (commitStorageDBConnection_ == nullptr) {
        LOGE("commitStorage Connection not existed!");
        return -E_INVALID_CONNECTION;
    }
    return commitStorageDBConnection_->StartTransaction();
}

int MultiVerNaturalStoreCommitStorage::CancelVacuum()
{
    if (commitStorageDBConnection_ == nullptr) {
        LOGE("commitStorage Connection not existed!");
        return -E_INVALID_CONNECTION;
    }
    return commitStorageDBConnection_->RollBack();
}

int MultiVerNaturalStoreCommitStorage::FinishlVacuum()
{
    if (commitStorageDBConnection_ == nullptr) {
        LOGE("commitStorage Connection not existed!");
        return -E_INVALID_CONNECTION;
    }
    return commitStorageDBConnection_->Commit();
}

int MultiVerNaturalStoreCommitStorage::GetAllCommitsInTree(std::list<MultiVerCommitNode> &commits) const
{
    std::map<CommitID, IKvDBCommit *> commitsTable;
    CommitID headerId;
    int errCode = GetAllCommits(commitsTable, headerId);
    if (errCode != E_OK || commitsTable.empty()) { // error or no commit.
        return errCode;
    }

    std::stack<CommitID> commitStack;
    commitStack.push(headerId);
    while (!commitStack.empty()) {
        auto currentCommitIter = commitsTable.find(commitStack.top());
        if (currentCommitIter == commitsTable.end()) {
            // not found the node in the commit tree.
            commits.clear();
            errCode = -E_UNEXPECTED_DATA;
            break;
        }

        commitStack.pop();
        if (currentCommitIter->second == nullptr) {
            // if the node has been released or traveled.
            continue;
        }

        AddParentsToStack(currentCommitIter->second, commitsTable, commitStack);
        MultiVerCommitNode commitNode;
        // Get the current commit info
        commitNode.commitId = currentCommitIter->first;
        commitNode.deviceInfo = currentCommitIter->second->GetDeviceInfo();
        commitNode.isLocal = (currentCommitIter->second->GetLocalFlag() ?
            MultiVerCommitNode::LOCAL_FLAG : MultiVerCommitNode::NON_LOCAL_FLAG);
        commitNode.leftParent = currentCommitIter->second->GetLeftParentId();
        commitNode.rightParent = currentCommitIter->second->GetRightParentId();
        commitNode.timestamp = currentCommitIter->second->GetTimestamp();
        commitNode.version = currentCommitIter->second->GetCommitVersion();
        commits.push_back(commitNode);

        ReleaseCommit(currentCommitIter->second);
        currentCommitIter->second = nullptr; // has been traveled, set to nullptr.
    }

    commits.sort([] (const MultiVerCommitNode &thisNode, const MultiVerCommitNode &thatNode) {
        return (thisNode.version > thatNode.version);
    });
    ReleaseUnusedCommits(commitsTable);

    return errCode;
}

int MultiVerNaturalStoreCommitStorage::AddCommit(const IKvDBCommit &commitEntry, bool isHeader)
{
    int errCode = CheckAddedCommit(commitEntry);
    if (errCode != E_OK) {
        return errCode;
    }

    Key key;
    TransferCommitIDToKey(commitEntry.GetCommitId(), key);
    Value value;
    errCode = TransferCommitToValue(commitEntry, value);
    if (errCode != E_OK) {
        return errCode;
    }
    IOption option;
    errCode = commitStorageDBConnection_->StartTransaction();
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = commitStorageDBConnection_->Put(option, key, value);
    if (errCode != E_OK) {
        goto END;
    }

    if (isHeader) {
        errCode = SetHeaderInner(commitEntry.GetCommitId());
    }
END:
    if (errCode != E_OK) {
        commitStorageDBConnection_->RollBack();
    } else {
        errCode = commitStorageDBConnection_->Commit();
    }

    return errCode;
}

int MultiVerNaturalStoreCommitStorage::RemoveCommit(const CommitID &commitId)
{
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get commit! Commit storage do not open.");
        return -E_INVALID_DB;
    }
    int errCode = commitStorageDBConnection_->StartTransaction();
    if (errCode != E_OK) {
        LOGE("Failed to remove commit when start transaction! err:%d", errCode);
        return errCode;
    }
    Key key;
    IOption option;
    CommitID header = GetHeader(errCode);
    if (header == commitId) {
        IKvDBCommit *commit = GetCommit(commitId, errCode);
        if (commit == nullptr) {
            LOGE("Failed to remove commit when get header commit! err:%d", errCode);
            goto ERROR;
        }
        errCode = SetHeader(commit->GetLeftParentId());
        ReleaseCommit(commit);
        commit = nullptr;
        if (errCode != E_OK) {
            LOGE("Failed to remove commit when set header commit! err:%d", errCode);
            goto ERROR;
        }
    } else {
        LOGE("Failed to remove commit! The commit is not the header.");
        errCode = -E_UNEXPECTED_DATA;
        goto ERROR;
    }
    TransferCommitIDToKey(commitId, key);
    errCode = commitStorageDBConnection_->Delete(option, key);
    if (errCode != E_OK) {
        LOGI("Failed to remove commit when remove commit! err:%d", errCode);
        goto ERROR;
    }
    errCode = commitStorageDBConnection_->Commit();
    if (errCode != E_OK) {
        LOGE("Failed to remove commit when commit! err:%d", errCode);
        goto ERROR;
    }
    return E_OK;
ERROR:
    (void)commitStorageDBConnection_->RollBack();
    return errCode;
}

void MultiVerNaturalStoreCommitStorage::ReleaseCommit(const IKvDBCommit *commit) const
{
    if (commit != nullptr) {
        delete commit;
        commit = nullptr;
    }
}

int MultiVerNaturalStoreCommitStorage::SetHeader(const CommitID &commitId)
{
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get commit! Commit storage do not open.");
        return -E_INVALID_DB;
    }

    if (commitId.size() != 0) {
        int errCode = E_OK;
        if (!CommitExist(commitId, errCode)) {
            LOGE("Failed to set header! The commit does not exist.");
            return errCode;
        }
    }

    return SetHeaderInner(commitId);
}

CommitID MultiVerNaturalStoreCommitStorage::GetHeader(int &errCode) const
{
    CommitID headerCommitID;
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get commit for uninitialized store");
        errCode = -E_INVALID_DB;
        return headerCommitID;
    }
    Key key;
    TransferStringToKey(HEADER_KEY, key);
    IOption option;
    Value value;
    errCode = commitStorageDBConnection_->Get(option, key, value);
    if (errCode != E_OK) {
        if (errCode == -E_NOT_FOUND) { // not find the header, means no header.
            LOGI("Not find the header.");
            errCode = E_OK;
        } else {
            LOGE("Get the commit header failed:%d", errCode);
            return headerCommitID;
        }
    }
    TransferValueToCommitID(value, headerCommitID);
    return headerCommitID;
}

bool MultiVerNaturalStoreCommitStorage::CommitExist(const CommitID &commitId, int &errCode) const
{
    IKvDBCommit *commit = GetCommit(commitId, errCode);
    if (commit == nullptr) {
        return false;
    } else {
        ReleaseCommit(commit);
        commit = nullptr;
        return true;
    }
}

void MultiVerNaturalStoreCommitStorage::ReleaseUnusedCommits(
    std::map<CommitID, IKvDBCommit *> &commits) const
{
    // need release the unmerged commits
    for (auto &item : commits) {
        if (item.second != nullptr) {
            ReleaseCommit(item.second);
            item.second = nullptr;
        }
    }
    commits.clear();
}

void MultiVerNaturalStoreCommitStorage::ReleaseLatestCommits(
    std::map<DeviceID, IKvDBCommit *> &latestCommits) const
{
    // need release the commits for exception.
    for (auto &item : latestCommits) {
        if (item.second != nullptr) {
            ReleaseCommit(item.second);
            item.second = nullptr;
        }
    }
    latestCommits.clear();
}

void MultiVerNaturalStoreCommitStorage::ReleaseCommitList(list<IKvDBCommit *> &commits) const
{
    for (auto &item : commits) {
        if (item != nullptr) {
            ReleaseCommit(item);
            item = nullptr;
        }
    }
    commits.clear();
}

int MultiVerNaturalStoreCommitStorage::GetLatestCommits(std::map<DeviceID, IKvDBCommit *> &latestCommits) const
{
    latestCommits.clear();
    map<CommitID, IKvDBCommit *> commits;
    CommitID headerId;
    int errCode = GetAllCommits(commits, headerId);
    if (errCode != E_OK || commits.empty()) { // error or no commit.
        return errCode;
    }

    std::stack<CommitID> commitStack;
    commitStack.push(headerId);
    while (!commitStack.empty()) {
        CommitID frontId = commitStack.top();
        auto currentCommitIter = commits.find(frontId);
        if (currentCommitIter == commits.end()) {
            // not found the node in the commit tree.
            LOGE("Not found the commit for the latest commits!");
            ReleaseLatestCommits(latestCommits);
            errCode = -E_UNEXPECTED_DATA;
            break;
        }

        commitStack.pop();
        if (currentCommitIter->second == nullptr) {
            // if the node has been released or traveled.
            continue;
        }

        AddParentsToStack(currentCommitIter->second, commits, commitStack);

        // Get the current commit info
        DeviceID deviceInfo = currentCommitIter->second->GetDeviceInfo();
        auto latestCommit = latestCommits.find(deviceInfo);
        if (latestCommit == latestCommits.end()) {
            // not found any node of the device in the commit tree.
            latestCommits.insert(make_pair(deviceInfo, currentCommitIter->second));
        } else if (CompareCommit(latestCommit->second, currentCommitIter->second)) {
            // if the current commit version is bigger than the stored.
            ReleaseCommit(latestCommit->second);
            latestCommit->second = currentCommitIter->second;
        } else {
            ReleaseCommit(currentCommitIter->second);
        }
        currentCommitIter->second = nullptr; // has been traveled, set to nullptr.
    }

    ReleaseUnusedCommits(commits);
    return errCode;
}

void MultiVerNaturalStoreCommitStorage::GetLocalVersionThredForLatestCommits(
    const map<CommitID, IKvDBCommit *> &allCommits, const map<DeviceID, CommitID> &latestCommits,
    map<DeviceID, Version> &latestCommitVersions)
{
    for (const auto &latestCommit : latestCommits) {
        auto commitIter = allCommits.find(latestCommit.second);
        if (commitIter != allCommits.end()) {
            // found in the local store, just set the threshold.
            latestCommitVersions.insert(make_pair(latestCommit.first, commitIter->second->GetCommitVersion()));
        } else {
            // not found in the local store, means that newer than local.
            latestCommitVersions.insert(make_pair(latestCommit.first, VERSION_MAX));
        }
    }
}

void MultiVerNaturalStoreCommitStorage::AddParentsToStack(const IKvDBCommit *commit,
    const std::map<CommitID, IKvDBCommit *> &allCommits, std::stack<CommitID> &commitStack)
{
    if (commit == nullptr) {
        return;
    }

    auto leftParentId = commit->GetLeftParentId();
    auto rightParentId = commit->GetRightParentId();
    if (!rightParentId.empty()) {
        auto iter = allCommits.find(rightParentId);
        if (iter != allCommits.end() && iter->second != nullptr) {
            // if the right parent has not been traveled, just push into the stack.
            commitStack.push(rightParentId);
        }
    }
    if (!leftParentId.empty()) {
        auto iter = allCommits.find(leftParentId);
        if (iter != allCommits.end() && iter->second != nullptr) {
            // if the left parent has not been traveled, just push into the stack.
            commitStack.push(leftParentId);
        }
    }
}

int MultiVerNaturalStoreCommitStorage::GetCommitTree(const map<DeviceID, CommitID> &latestCommits,
    list<IKvDBCommit *> &commits) const
{
    commits.clear();
    CommitID header;
    map<CommitID, IKvDBCommit *> allCommits;
    int errCode = GetAllCommits(allCommits, header);
    // error or no commit.
    if (errCode != E_OK || allCommits.empty()) {
        return errCode;
    }
    map<DeviceID, Version> latestCommitVersions;
    GetLocalVersionThredForLatestCommits(allCommits, latestCommits, latestCommitVersions);
    std::stack<CommitID> commitStack;
    commitStack.push(header);
    while (!commitStack.empty()) {
        CommitID frontId = commitStack.top();
        auto currentCommitIter = allCommits.find(frontId);
        if (currentCommitIter == allCommits.end()) {
            // not found the node in the commit tree.
            LOGE("Not found the commit in the local tree!");
            ReleaseCommitList(commits);
            errCode = -E_UNEXPECTED_DATA;
            break;
        }
        commitStack.pop();
        if (currentCommitIter->second == nullptr) {
            // if the commit has been traveled.
            continue;
        }
        AddParentsToStack(currentCommitIter->second, allCommits, commitStack);
        // Get the current commit info
        DeviceID deviceInfo = currentCommitIter->second->GetDeviceInfo();
        auto latestCommit = latestCommitVersions.find(deviceInfo);
        if (latestCommit == latestCommitVersions.end() ||
            latestCommit->second < currentCommitIter->second->GetCommitVersion()) {
            // not found in the latest commits of the other device,
            // or the current commit version is bigger than the threshold.
            commits.push_back(currentCommitIter->second);
        } else {
            // means that the commit existed in the other device.
            ReleaseCommit(currentCommitIter->second);
        }
        currentCommitIter->second = nullptr;
    }

    ReleaseUnusedCommits(allCommits);
    RefreshCommitTree(commits); // for version ascend.
    return errCode;
}

int MultiVerNaturalStoreCommitStorage::RunRekeyLogic(CipherType type, const CipherPassword &passwd)
{
    int errCode = static_cast<SQLiteLocalKvDB *>(commitStorageDatabase_)->RunRekeyLogic(type, passwd);
    if (errCode != E_OK) {
        LOGE("commit logs rekey failed:%d", errCode);
    }
    return errCode;
}

int MultiVerNaturalStoreCommitStorage::RunExportLogic(CipherType type, const CipherPassword &passwd,
    const std::string &dbDir)
{
    // execute export
    std::string newDbName = dbDir + "/" + MULTI_VER_COMMIT_DB_NAME;
    int errCode = static_cast<SQLiteLocalKvDB *>(commitStorageDatabase_)->RunExportLogic(type, passwd, newDbName);
    if (errCode != E_OK) {
        LOGE("commit logs export failed:%d", errCode);
    }
    return errCode;
}

void MultiVerNaturalStoreCommitStorage::TransferCommitIDToKey(const CommitID &commitID, Key &key)
{
    key = commitID;
}

int MultiVerNaturalStoreCommitStorage::TransferCommitToValue(const IKvDBCommit &commit, Value &value)
{
    // 3 uint64_t members.
    uint32_t totalLength = Parcel::GetUInt64Len() * 3 + Parcel::GetVectorCharLen(commit.GetCommitId()) +
        Parcel::GetVectorCharLen(commit.GetLeftParentId()) + Parcel::GetVectorCharLen(commit.GetRightParentId()) +
        Parcel::GetStringLen(commit.GetDeviceInfo());
    if (totalLength > MAX_COMMIT_ST_LENGTH) {
        LOGE("The commit length is over the max threshold");
        return -E_UNEXPECTED_DATA;
    }

    value.resize(totalLength);
    Parcel parcel(const_cast<uint8_t *>(value.data()), totalLength);

    int errCode = parcel.WriteUInt64(commit.GetTimestamp());
    if (errCode != E_OK) {
        return errCode;
    }

    uint64_t localFlag = static_cast<uint32_t>((commit.GetLocalFlag() == true) ? 1 : 0);
    errCode = parcel.WriteUInt64(localFlag);
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteUInt64(commit.GetCommitVersion());
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteVectorChar(commit.GetCommitId());
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteVectorChar(commit.GetLeftParentId());
    if (errCode != E_OK) {
        return errCode;
    }

    errCode = parcel.WriteVectorChar(commit.GetRightParentId());
    if (errCode != E_OK) {
        return errCode;
    }

    return parcel.WriteString(commit.GetDeviceInfo());
}

int MultiVerNaturalStoreCommitStorage::TransferValueToCommit(const Value &value, IKvDBCommit &commit)
{
    size_t valueLength = value.size();
    if (valueLength == 0 || valueLength >= MAX_COMMIT_ST_LENGTH) {
        LOGE("Failed to transfer value to commit struct! invalid value length:%ul.", valueLength);
        return -E_UNEXPECTED_DATA;
    }

    TimeStamp timestamp = 0;
    uint64_t localFlag = 1;
    Version versionInfo;

    CommitID commitID;
    CommitID leftParentID;
    CommitID rightParentID;
    DeviceID deviceInfo;

    Parcel parcel(const_cast<uint8_t *>(value.data()), valueLength);
    parcel.ReadUInt64(timestamp);
    parcel.ReadUInt64(localFlag);
    parcel.ReadUInt64(versionInfo);
    parcel.ReadVectorChar(commitID);
    parcel.ReadVectorChar(leftParentID);
    parcel.ReadVectorChar(rightParentID);
    parcel.ReadString(deviceInfo);
    if (parcel.IsError()) {
        return -E_PARSE_FAIL;
    }

    // set commit value
    commit.SetCommitVersion(versionInfo);
    commit.SetCommitId(commitID);
    commit.SetLeftParentId(leftParentID);
    commit.SetRightParentId(rightParentID);
    commit.SetTimestamp(timestamp);
    commit.SetLocalFlag((localFlag == 1) ? true : false);
    commit.SetDeviceInfo(deviceInfo);
    return E_OK;
}

void MultiVerNaturalStoreCommitStorage::TransferStringToKey(const string &str, Key &key)
{
    key.assign(str.begin(), str.end());
}

void MultiVerNaturalStoreCommitStorage::TransferCommitIDToValue(const CommitID &commitID, Value &value)
{
    value = commitID;
}

void MultiVerNaturalStoreCommitStorage::TransferValueToCommitID(const Value &value, CommitID &commitID)
{
    commitID = value;
}

bool MultiVerNaturalStoreCommitStorage::CompareCommit(const IKvDBCommit *first,
    const IKvDBCommit *second)
{
    if (first == nullptr || second == nullptr) {
        return false;
    }
    return first->GetCommitVersion() < second->GetCommitVersion();
}

int MultiVerNaturalStoreCommitStorage::GetAllCommits(map<CommitID, IKvDBCommit *> &commits,
    CommitID &headerId) const
{
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get all commits for uninitialized store");
        return -E_INVALID_DB;
    }
    IOption option;
    Key keyPrefix;
    vector<Entry> entries;
    int errCode = commitStorageDBConnection_->GetEntries(option, keyPrefix, entries);
    if (errCode != E_OK) {
        if (errCode == -E_NOT_FOUND) {
            errCode = E_OK;
        } else {
            LOGE("Failed to get commit entries from DB:%d", errCode);
        }

        return errCode;
    }

    Key header;
    TransferStringToKey(HEADER_KEY, header);

    for (const auto &entry : entries) {
        if (entry.key == header) {
            headerId = entry.value; // get the header.
            continue;
        }
        IKvDBCommit *commit = new (std::nothrow) MultiVerCommit();
        if (commit == nullptr) {
            ReleaseUnusedCommits(commits);
            LOGE("Failed to alloc commit! Bad alloc.");
            return -E_OUT_OF_MEMORY;
        }
        errCode = TransferValueToCommit(entry.value, *commit);
        if (errCode != E_OK) {
            delete commit;
            commit = nullptr;
            ReleaseUnusedCommits(commits);
            return errCode;
        }
        commits.insert(make_pair(commit->GetCommitId(), commit));
    }
    return E_OK;
}

int MultiVerNaturalStoreCommitStorage::SetHeaderInner(const CommitID &commitId)
{
    Key key;
    Value value;
    TransferStringToKey(HEADER_KEY, key);
    TransferCommitIDToValue(commitId, value);
    IOption option;
    int errCode = commitStorageDBConnection_->Put(option, key, value);
    if (errCode != E_OK) {
        LOGE("Failed to set header! err:%d", errCode);
    }
    return errCode;
}

int MultiVerNaturalStoreCommitStorage::CheckAddedCommit(const IKvDBCommit &commitEntry) const
{
    if (commitStorageDatabase_ == nullptr || commitStorageDBConnection_ == nullptr) {
        LOGE("Failed to get commit! Commit storage do not open.");
        return -E_INVALID_DB;
    }
    // Parameter check
    if (!((static_cast<const MultiVerCommit&>(commitEntry)).CheckCommit())) {
        LOGE("Failed to add commit! Commit is invalid.");
        return -E_UNEXPECTED_DATA;
    }
    int errCode = E_OK;
    if (commitEntry.GetLeftParentId().size() != 0) {
        if (!CommitExist(commitEntry.GetLeftParentId(), errCode)) {
            LOGE("Failed to add commit! The left parent commit does not exist.");
            return errCode;
        }
    }
    if (commitEntry.GetRightParentId().size() != 0) {
        if (!CommitExist(commitEntry.GetRightParentId(), errCode)) {
            LOGE("Failed to add commit! The right parent commit does not exist.");
            return errCode;
        }
    }

    return E_OK;
}

void MultiVerNaturalStoreCommitStorage::RefreshCommitTree(std::list<IKvDBCommit *> &commits)
{
    if (commits.empty()) {
        return;
    }
    commits.sort(CompareCommit);
}

Version MultiVerNaturalStoreCommitStorage::GetMaxCommitVersion(int &errCode) const
{
    std::map<CommitID, IKvDBCommit *> commits;
    CommitID headerId;
    errCode = GetAllCommits(commits, headerId);
    if (errCode != E_OK || commits.empty()) { // means no commit or error.
        return 0;
    }

    Version maxVersion = 0;
    for (const auto &item : commits) {
        if (item.second != nullptr) {
            Version itemVersion = item.second->GetCommitVersion();
            maxVersion = (maxVersion < itemVersion) ? itemVersion : maxVersion;
        }
    }
    ReleaseUnusedCommits(commits);
    errCode = E_OK;
    return maxVersion;
}

int MultiVerNaturalStoreCommitStorage::BackupCurrentDatabase(const Property &property, const std::string &dir)
{
    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.path);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_COMMIT_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);
    int errCode = SQLiteLocalKvDB::BackupCurrentDatabase(dbProperties, dir);
    return errCode;
}

int MultiVerNaturalStoreCommitStorage::ImportDatabase(const Property &property, const std::string &dir,
    const CipherPassword &passwd)
{
    KvDBProperties dbProperties;
    dbProperties.SetBoolProp(KvDBProperties::CREATE_IF_NECESSARY, true);
    dbProperties.SetStringProp(KvDBProperties::DATA_DIR, property.path);
    dbProperties.SetStringProp(KvDBProperties::FILE_NAME, DBConstant::MULTI_VER_COMMIT_STORE);
    dbProperties.SetStringProp(KvDBProperties::IDENTIFIER_DIR, property.identifierName);
    dbProperties.SetIntProp(KvDBProperties::DATABASE_TYPE, KvDBProperties::MULTI_VER_TYPE);
    dbProperties.SetPassword(property.cipherType, property.passwd);
    int errCode = SQLiteLocalKvDB::ImportDatabase(dbProperties, dir, passwd);
    return errCode;
}
}  // namespace DistributedDB
#endif
