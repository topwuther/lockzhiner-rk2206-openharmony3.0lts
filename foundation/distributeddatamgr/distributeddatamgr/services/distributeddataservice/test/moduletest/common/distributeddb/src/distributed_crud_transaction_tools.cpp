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
#include "distributed_crud_transaction_tools.h"
#include <gtest/gtest.h>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <random>
#include <algorithm>
#include <thread>
#include <cstdio>
#include <chrono>
#include <cmath>

#include "distributed_test_tools.h"
#include "distributeddb_data_generator.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"

using namespace std;
using namespace chrono;
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

DistributedCrudTransactionTools::DistributedCrudTransactionTools(KvStoreDelegate &delegate,
    CrudMode first, CrudMode second, bool preset, bool isLocal)
{
    this->storeDelegate_ = &delegate,
    this->firstMode_ = first;
    this->secondMode_ = second;
    this->needPresetData_ = preset;
    this->isLocal_ = isLocal;
}

bool DistributedCrudTransactionTools::PresetValue()
{
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateRecords(this->presetCount_, DEFAULT_START, allKeys, entriesBatch);

    for (unsigned long i = 0; i < this->presetCount_; ++i) {
        entriesBatch[i].value = GetValueWithInt(this->presetValue_);
    }
    DBStatus status = DistributedTestTools::PutBatch(*storeDelegate_, entriesBatch);
    if (status != DistributedDB::OK) {
        MST_LOG("PresetValue failed, status %d", status);
        return false;
    }
    return true;
}

bool DistributedCrudTransactionTools::CheckFirst()
{
    vector<Entry> values = DistributedTestTools::GetEntries(*storeDelegate_, KEY_SEARCH_4);
    if (firstMode_ == CrudMode::PUT_BATCH) {
        MST_LOG("[CHECK LOG]putbatch first %zu", values.size());
        if (needPresetData_) {
            if (values.size() != 0) {
                return GetIntValue(values[0].value) == presetValue_;
            } else {
                return true;
            }
        } else {
            return (values.size() == NO_RECORD) || (values.size() == presetCount_);
        }
    }
    if (firstMode_ == CrudMode::UPDATE_BATCH) {
        if (values.size() != presetCount_) {
            return false;
        }
        for (unsigned long index = 0; index < values.size(); ++index) {
            if (GetIntValue(values[index].value) != presetValue_) {
                return false;
            }
        }
        return true;
    }
    if (firstMode_ == CrudMode::DELETE_BATCH || firstMode_ == CrudMode::CLEAR) {
        MST_LOG("[CHECK LOG]check clear first %zu", values.size());
        if ((values.size() > NO_RECORD) && (values.size() < presetCount_)) {
            return false;
        }
        return (values.size() == NO_RECORD) ||
            ((values.size() == presetCount_ && GetIntValue(values[0].value) == presetValue_));
    }
    return false;
}

bool DistributedCrudTransactionTools::CheckSecond()
{
    if (secondMode_ == CrudMode::PUT) {
        vector<Entry> values = DistributedTestTools::GetEntries(*storeDelegate_, KEY_SEARCH_4);
        return values.size() == NO_RECORD || GetIntValue(values[0].value) == SMALL_VALUE_SIZE ||
            GetIntValue(values[0].value) == presetValue_;
    }
    if (secondMode_ == CrudMode::DELETE) {
        Key key0 = { 'k', '0' };
        Value value = DistributedTestTools::Get(*storeDelegate_, key0);
        return value.size() == NO_RECORD || GetIntValue(value) == presetValue_;
    }
    if (secondMode_ == CrudMode::PUT_BATCH) {
        vector<Entry> values = DistributedTestTools::GetEntries(*storeDelegate_, KEY_SEARCH_4);
        if ((values.size() != NO_RECORD) && (values.size() != presetCount_)) {
            return false;
        }
        for (unsigned long index = 0; index < values.size(); ++index) {
            if (GetIntValue(values[0].value) != presetValue_) {
                return false;
            }
        }
        return true;
    }
    if (secondMode_ == CrudMode::DELETE_BATCH || secondMode_ == CrudMode::CLEAR) {
        vector<Entry> values = DistributedTestTools::GetEntries(*storeDelegate_, KEY_SEARCH_4);
        if ((values.size() != NO_RECORD) && (values.size() != presetCount_)) {
            return false;
        }
        for (unsigned long index = 0; index < values.size(); ++index) {
            if (GetIntValue(values[0].value) != presetValue_) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void DistributedCrudTransactionTools::Check()
{
    while (!secondComplete_) {
        if (!firstComplete_) {
            bool result = CheckFirst();
            if (!result) {
                MST_LOG("[CHECK LOG]check first failed;%d", success_);
            }
            MST_LOG("[CHECK LOG]firstComplete_ failed %d;", success_);
            success_ = result;
        } else if (firstComplete_ && !secondComplete_) {
            bool result = CheckSecond();
            if (!result) {
                MST_LOG("[CHECK LOG]check second failed;%d", success_);
            }
            MST_LOG("[CHECK LOG]secondComplete_ failed %d;", success_);
            success_ = result;
        }
    }
}

bool DistributedCrudTransactionTools::Action1(KvStoreDelegate &delegate)
{
    MST_LOG("firstmode %d", static_cast<int>(firstMode_));
    if (firstMode_ == CrudMode::PUT_BATCH) {
        return DBStatus::OK == delegate.PutBatch(entriesBatch_);
    } else if (firstMode_ == CrudMode::DELETE_BATCH) {
        return DBStatus::OK == delegate.DeleteBatch(allKeys_) ||
            DBStatus::NOT_FOUND == delegate.DeleteBatch(allKeys_);
    } else if (firstMode_ == CrudMode::CLEAR) {
        return DBStatus::OK == delegate.Clear();
    } else {
        MST_LOG("unknown first %d", static_cast<int>(firstMode_));
        return false;
    }
}

bool DistributedCrudTransactionTools::Action2(KvStoreDelegate &delegate)
{
    MST_LOG("secondmode %d", static_cast<int>(secondMode_));
    if (secondMode_ == CrudMode::PUT) {
        entriesBatch_[0].value = GetValueWithInt(SMALL_VALUE_SIZE);
        return DBStatus::OK == delegate.Put(entriesBatch_[0].key, entriesBatch_[0].value);
    } else if (secondMode_ == CrudMode::DELETE) {
        return DBStatus::OK == delegate.Delete(entriesBatch_[0].key) ||
            DBStatus::NOT_FOUND == delegate.DeleteBatch(allKeys_);
    } else if (secondMode_ == CrudMode::CLEAR) {
        return DBStatus::OK == delegate.Clear();
    } else if (secondMode_ == CrudMode::PUT_BATCH) {
        return DBStatus::OK == delegate.PutBatch(entriesBatch_);
    } else {
        MST_LOG("unknown secondmode %d", static_cast<int>(secondMode_));
        return false;
    }
}

void SleepOneSecond()
{
    std::this_thread::sleep_for(std::chrono::duration<int>(1));
}

bool DeleteDataBase(bool success_, KvStoreDelegate *delegate1, KvStoreDelegate *delegate2,
    KvStoreDelegateManager *delegateManager1, KvStoreDelegateManager *delegateManager2)
{
    SleepOneSecond();
    if ((delegateManager1->CloseKvStore(delegate1) != OK) ||
        (delegateManager2->CloseKvStore(delegate2) != OK)) {
        MST_LOG("closed failed!");
    }
    delegate1 = nullptr;
    delegate2 = nullptr;
    delete delegateManager1;
    delegateManager1 = nullptr;
    delete delegateManager2;
    delegateManager2 = nullptr;
    MST_LOG("[CHECK LOG]check result %d", success_);
    return success_;
}

bool DistributedCrudTransactionTools::testCrudTransaction()
{
    if (storeDelegate_ == nullptr) {
        return false;
    }

    DistributedTestTools::Clear(*storeDelegate_);
    if (this->needPresetData_) {
        if (!PresetValue()) {
            return false;
        }
    }
    GenerateRecords(this->presetCount_, DEFAULT_START, allKeys_, entriesBatch_);
    for (unsigned long i = 0; i < this->presetCount_; ++i) {
        entriesBatch_[i].value = GetValueWithInt(this->presetValue_);
    }

    KvStoreDelegate *delegate1 = nullptr;
    KvStoreDelegateManager *delegateManager1 = nullptr;
    delegate1 = DistributedTestTools::GetDelegateSuccess(delegateManager1,
        g_kvdbParameter1, g_kvOption);
    if (delegateManager1 == nullptr || delegate1 == nullptr) {
        MST_LOG("[testCrudTransaction] delegateManager1 or delegate1 is nullptr");
        return false;
    }

    KvStoreDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *delegateManager2 = nullptr;
    delegate2 = DistributedTestTools::GetDelegateSuccess(delegateManager2,
        g_kvdbParameter1, g_kvOption);
    if (delegateManager2 == nullptr || delegate2 == nullptr) {
        MST_LOG("[testCrudTransaction] delegateManager2 or delegate2 is nullptr");
        return false;
    }

    std::thread th(&DistributedCrudTransactionTools::Check, this);
    th.detach();

    if (!Action1(*delegate1)) {
        MST_LOG("action1 failed");
        goto ERROR;
    }
    firstComplete_ = true;
    MST_LOG("firstComplete_");

    if (!Action2(*delegate2)) {
        MST_LOG("action2 failed");
        goto ERROR;
    }
    secondComplete_ = true;
    return DeleteDataBase(success_, delegate1, delegate2, delegateManager1, delegateManager2);
ERROR:
    secondComplete_ = true;
    return false;
}