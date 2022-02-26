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
#include "distributeddb_nb_cursor_testcase.h"

#include <gtest/gtest.h>
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>
#include "types.h"
#include "types_export.h"
#include "kv_store_delegate.h"
#include "kv_store_nb_delegate.h"
#include "kv_store_delegate_manager.h"
#include "distributed_test_tools.h"
#include "distributeddb_nb_test_tools.h"
#include "distributeddb_data_generator.h"

using namespace std;
using namespace std::chrono;
using namespace std::placeholders;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

namespace {
const unsigned int FIFTEEN_RECORDS = 15;
const unsigned int CURSOR_DIFFERENT_RECORDS = 102;
const unsigned long ONE_TENTH_M_LONG_STRING = 104858;
const unsigned int FOUR_BYTE_KEY = 4;
const unsigned int FIVE_BYTE_KEY = 5;
const unsigned int THREAD_NUM_START = 1;
const unsigned int THREAD_NUM_END = 4;
const unsigned int OPEN_DB_TIMES = 3;
const unsigned int DELEGATE_NUM = 3;
const unsigned int WAIT_FOR_OBSERVER_REKEY = 75000;
const int CURSOR_ORIGINAL_POSITION_NEGATIVE10 = -10;
const int CURSOR_ORIGINAL_POSITION_NEGATIVE1 = -1;
const int CURSOR_ORIGINAL_POSITION_0 = 0;
const int CURSOR_ORIGINAL_POSITION_1 = 1;
const int CURSOR_ORIGINAL_POSITION_2 = 2;
const int CURSOR_ORIGINAL_POSITION_40 = 40;
const int CURSOR_ORIGINAL_POSITION_50 = 50;
const int CURSOR_ORIGINAL_POSITION_60 = 60;
const int CURSOR_ORIGINAL_POSITION_99 = 99;
const int CURSOR_ORIGINAL_POSITION_100 = 100;
const int CURSOR_ORIGINAL_POSITION_120 = 120;

const int CURSOR_POSITION_NEGATIVE5 = -5;
const int CURSOR_POSITION_NEGATIVE1 = -1;
const int CURSOR_POSITION_0 = 0;
const int CURSOR_POSITION_1 = 1;
const int CURSOR_POSITION_2 = 2;
const int CURSOR_POSITION_40 = 40;
const int CURSOR_POSITION_44 = 44;
const int CURSOR_POSITION_49 = 49;
const int CURSOR_POSITION_50 = 50;
const int CURSOR_POSITION_55 = 55;
const int CURSOR_POSITION_60 = 60;
const int CURSOR_POSITION_99 = 99;
const int CURSOR_POSITION_100 = 100;

const int CURSOR_OFFSET_NEGATIVE65 = -65;
const int CURSOR_OFFSET_NEGATIVE60 = -60;
const int CURSOR_OFFSET_NEGATIVE50 = -50;
const int CURSOR_OFFSET_NEGATIVE45 = -45;
const int CURSOR_OFFSET_NEGATIVE2 = -2;
const int CURSOR_OFFSET_NEGATIVE1 = -1;
const int CURSOR_OFFSET_0 = 0;
const int CURSOR_OFFSET_1 = 1;
const int CURSOR_OFFSET_2 = 2;
const int CURSOR_OFFSET_5 = 5;
const int CURSOR_OFFSET_45 = 45;
const int CURSOR_OFFSET_50 = 50;
const int CURSOR_OFFSET_55 = 55;
const int CURSOR_OFFSET_60 = 60;
const int CURSOR_OFFSET_65 = 65;
DistributedDB::CipherPassword g_passwd1;

struct PositionStatus01 {
    DBStatus isGetSuccess = OK;
    bool isFirst = true;
    bool isLast = false;
    bool isBeforeFirst = false;
    bool isAfterLast = false;
    int position = 0;
};

void SetResultSetCacheMode(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    if (isRowIdMode) {
        int cacheMode = static_cast<int>(ResultSetCacheMode::CACHE_ENTRY_ID_ONLY);
        PragmaData data = static_cast<PragmaData>(&cacheMode);
        EXPECT_EQ(delegate->Pragma(RESULT_SET_CACHE_MODE, data), OK);
    } else {
        int cacheMode = static_cast<int>(ResultSetCacheMode::CACHE_FULL_ENTRY);
        PragmaData data = static_cast<PragmaData>(&cacheMode);
        EXPECT_EQ(delegate->Pragma(RESULT_SET_CACHE_MODE, data), OK);
    }
}

bool JudgePosition(KvStoreResultSet *&resultSet, const PositionStatus01 &currentStatus)
{
    Entry entry;
    bool bRes = true;
    DBStatus status = resultSet->GetEntry(entry);
    bRes = bRes && (status == currentStatus.isGetSuccess);
    bool result = resultSet->IsFirst();
    bRes = bRes && (result == currentStatus.isFirst);
    result = resultSet->IsLast();
    bRes = bRes && (result == currentStatus.isLast);
    result = resultSet->IsBeforeFirst();
    bRes = bRes && (result == currentStatus.isBeforeFirst);
    result = resultSet->IsAfterLast();
    bRes = bRes && (result == currentStatus.isAfterLast);
    int position = resultSet->GetPosition();
    bRes = bRes && (position == currentStatus.position);
    return bRes;
}
}

void DistributedNbCursorTestcase::ResultSetDb001(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);

    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);

    /**
     * @tc.steps: step3. call GetPosition and MoveToPrevious interface.
     * @tc.expected: step3. when the Current position is -1, can't do MoveToPrevious, and if do, it can't effect at all.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_TRUE(resultSet->MoveToPrevious() == false);

    /**
     * @tc.steps: step4. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast and GetPostion interface.
     * @tc.expected: step4. when the Current position is -1, other position judge interface can return right result.
     */
    PositionStatus01 currentStatus1 = {NOT_FOUND, false, false, true, false, CURSOR_POSITION_NEGATIVE1};
    EXPECT_TRUE(JudgePosition(resultSet, currentStatus1));

    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToFirst() == true);
    /**
     * @tc.steps: step6. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step6. when the Current position is 0, other position judge interface can return right result.
     */
    PositionStatus01 currentStatus2 = {OK, true, false, false, false, CURSOR_POSITION_0};
    EXPECT_TRUE(JudgePosition(resultSet, currentStatus2));
    /**
     * @tc.steps: step7. call MoveToNext interface.
     * @tc.expected: step7. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == true);
    /**
     * @tc.steps: step8. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step8. when the Current position is 1, other position judge interface can return right result.
     */
    PositionStatus01 currentStatus3 = {OK, false, false, false, false, CURSOR_POSITION_1};
    EXPECT_TRUE(JudgePosition(resultSet, currentStatus3));
    /**
     * @tc.steps: step9. call MoveToLast interface.
     * @tc.expected: step9. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToLast() == true);
    /**
     * @tc.steps: step10. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step10. when the Current position is the last,
     *    other position judge interface can return right result.
     */
    PositionStatus01 currentStatus4 = {OK, false, true, false, false, CURSOR_POSITION_99};
    EXPECT_TRUE(JudgePosition(resultSet, currentStatus4));
    /**
     * @tc.steps: step11. call MoveToNext interface.
     * @tc.expected: step11. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == false);
    /**
     * @tc.steps: step10. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step10. if call MoveToNext interface when the Current position is the last,
     *    other position judge interface can also return right result.
     */
    PositionStatus01 currentStatus5 = {NOT_FOUND, false, false, false, true, CURSOR_POSITION_100};
    EXPECT_TRUE(JudgePosition(resultSet, currentStatus5));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb002(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);

    sort(entries.begin(), entries.end(), DistributedTestTools::CompareKey);

    /**
     * @tc.steps: step2. set the current position is -1, call MoveToNext, GetPostion and GetEntry interface looply.
     * @tc.expected: step2. return values are all right.
     */
    EXPECT_TRUE(DistributedDBNbTestTools::MoveToNextFromBegin(*resultSet, entries, CURSOR_POSITION_100));
    /**
     * @tc.steps: step2. set the current position is 100, call MoveToPrevious, GetPostion, GetEntry looply.
     * @tc.expected: step2. return values are all right.
     */
    int positionGot = CURSOR_POSITION_NEGATIVE1;
    Entry entry;
    bool result = true;
    for (int position = CURSOR_POSITION_100; position > CURSOR_POSITION_NEGATIVE1; --position) {
        result = resultSet->MoveToPrevious();
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(result == true);
        } else {
            EXPECT_TRUE(result == false);
        }
        positionGot = resultSet->GetPosition();
        EXPECT_TRUE(positionGot == (position - CURSOR_POSITION_1));
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet->GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entries[position - 1].key));
            EXPECT_TRUE(CompareVector(entry.value, entries[position - 1].value));
        } else {
            EXPECT_TRUE(resultSet->GetEntry(entry) == NOT_FOUND);
        }
    }
    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}
namespace {
struct MoveStatus {
    int offSet = 0;
    bool moveResult = true;
    int currentPosition = 0;
    DBStatus status = OK;
};

bool MoveAndCheck(KvStoreResultSet *&resultSet, const MoveStatus status)
{
    bool result = true;
    Entry entry;
    bool mRes = resultSet->Move(status.offSet);
    result = result && (mRes == status.moveResult);
    int position = resultSet->GetPosition();
    result = result && (position == status.currentPosition);
    DBStatus statusGot = resultSet->GetEntry(entry);
    result = result && (statusGot == status.status);
    return result;
}
}

void DistributedNbCursorTestcase::ResultSetDb003(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);
    /**
     * @tc.steps: step2. call Move interface move to offset 50 and check the result.
     * @tc.expected: step2. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status1 = {CURSOR_OFFSET_50, true, CURSOR_POSITION_49, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step3. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status2 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_49, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call Move interface move to offset 60 by upstairs and check the result.
     * @tc.expected: step4. Move failed, and the position is 100, and can't get entry.
     */
    MoveStatus status3 = {CURSOR_OFFSET_60, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status3));
    /**
     * @tc.steps: step5. call Move interface move to offset -50 by upstairs and check the result.
     * @tc.expected: step5. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status4 = {CURSOR_OFFSET_NEGATIVE50, true, CURSOR_POSITION_50, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status4));
    /**
     * @tc.steps: step6. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step6. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status5 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_50, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status5));
    /**
     * @tc.steps: step7. call Move interface move to offset -60 by upstairs and check the result.
     * @tc.expected: step7. Move failed, and the position is -1, and can't get entry.
     */
    MoveStatus status6 = {CURSOR_OFFSET_NEGATIVE60, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status6));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}
namespace {
struct PositionStatus02 {
    int originalPosition = 0;
    bool moveResult = true;
    int currentPosition = 0;
    DBStatus status = OK;
};

bool MoveToPositionAndCheck(KvStoreResultSet *&resultSet, const PositionStatus02 status)
{
    bool result = true;
    Entry entry;
    bool mRes = resultSet->MoveToPosition(status.originalPosition);
    result = result && (mRes == status.moveResult);
    int position = resultSet->GetPosition();
    result = result && (position == status.currentPosition);
    DBStatus statusGot = resultSet->GetEntry(entry);
    result = result && (statusGot == status.status);
    return result;
}
}
void DistributedNbCursorTestcase::ResultSetDb004(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_ONE_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);
    /**
     * @tc.steps: step2. call MoveToPostion interface move to position 50 and check the result.
     * @tc.expected: step2. MoveToPostion ok, and the position is 50, and can get entry.
     */
    PositionStatus02 status1 = {CURSOR_ORIGINAL_POSITION_50, true, CURSOR_POSITION_50, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call MoveToPostion interface move to position 60 and check the result.
     * @tc.expected: step3. MoveToPostion ok, and the position is 60, and can get entry.
     */
    PositionStatus02 status2 = {CURSOR_ORIGINAL_POSITION_60, true, CURSOR_POSITION_60, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call MoveToPostion interface move to position -10 and check the result.
     * @tc.expected: step4. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status3 = {CURSOR_ORIGINAL_POSITION_NEGATIVE10, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status3));
    /**
     * @tc.steps: step5. call MoveToPostion interface move to position 120 and check the result.
     * @tc.expected: step5. Move failed, and the position is 100, and can't get entry.
     */
    PositionStatus02 status4 = {CURSOR_ORIGINAL_POSITION_120, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status4));
    /**
     * @tc.steps: step6. call MoveToPostion interface move to position 100 and check the result.
     * @tc.expected: step6. Move failed, and the position is 100, and can't get entry.
     */
    PositionStatus02 status5 = {CURSOR_ORIGINAL_POSITION_100, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status5));
    /**
     * @tc.steps: step7. call MoveToPostion interface move to position 0 and check the result.
     * @tc.expected: step7. move OK, and the position is 0, and can get entry.
     */
    PositionStatus02 status6 = {CURSOR_ORIGINAL_POSITION_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status6));
    /**
     * @tc.steps: step8. call MoveToPostion interface move to position 99 and check the result.
     * @tc.expected: step8. move OK, and the position is 99, and can get entry.
     */
    PositionStatus02 status7 = {CURSOR_ORIGINAL_POSITION_99, true, CURSOR_POSITION_99, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status7));
    /**
     * @tc.steps: step9. call MoveToPostion interface move to position 0 and check the result.
     * @tc.expected: step9. move OK, and the position is 0, and can get entry.
     */
    PositionStatus02 status8 = {CURSOR_ORIGINAL_POSITION_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status8));
    /**
     * @tc.steps: step10. call MoveToPostion interface move to position -1 and check the result.
     * @tc.expected: step10. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status9 = {CURSOR_ORIGINAL_POSITION_NEGATIVE1, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status9));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb005(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_RECORD);
    EXPECT_TRUE(delegate->Put(entries[0].key, entries[0].value) == OK);

    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success and returned 1 records.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_RECORD);
    /**
     * @tc.steps: step3. call GetPosition, MoveToPrevious and GetPosition interface and check the result.
     * @tc.expected: step3. GetPosition returned -1, MoveToPrevious returned false, and GetPosition still returned -1.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_TRUE(resultSet->MoveToPrevious() == false);
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    /**
     * @tc.steps: step4. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast and GetPostion interface.
     * @tc.expected: step4. when the Current position is -1, other position judge interface can return right result.
     */
    PositionStatus01 positionStatus = {NOT_FOUND, false, false, true, false, CURSOR_POSITION_NEGATIVE1};
    EXPECT_TRUE(JudgePosition(resultSet, positionStatus));
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToFirst() == true);
    /**
     * @tc.steps: step6. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step6. when the Current position is 0, other position judge interface can return right result.
     */
    positionStatus = {OK, true, true, false, false, CURSOR_POSITION_0};
    EXPECT_TRUE(JudgePosition(resultSet, positionStatus));
    /**
     * @tc.steps: step7. call MoveToNext interface.
     * @tc.expected: step7. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == false);
    /**
     * @tc.steps: step8. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step8. when the Current position is 1, other position judge interface can return right result.
     */
    positionStatus = {NOT_FOUND, false, false, false, true, CURSOR_POSITION_1};
    EXPECT_TRUE(JudgePosition(resultSet, positionStatus));
    /**
     * @tc.steps: step9. call MoveToLast interface.
     * @tc.expected: step9. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToLast() == true);
    /**
     * @tc.steps: step10. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step10. when the Current position is 1, other position judge interface can return right result.
     */
    positionStatus = {OK, true, true, false, false, CURSOR_POSITION_0};
    EXPECT_TRUE(JudgePosition(resultSet, positionStatus));
    /**
     * @tc.steps: step11. call MoveToNext interface.
     * @tc.expected: step11. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == false);
    /**
     * @tc.steps: step12. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step12. when the Current position is 1, other position judge interface can return right result.
     */
    positionStatus = {NOT_FOUND, false, false, false, true, CURSOR_POSITION_1};
    EXPECT_TRUE(JudgePosition(resultSet, positionStatus));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb006(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_RECORD);
    EXPECT_TRUE(delegate->Put(entries[0].key, entries[0].value) == OK);
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);

    /**
     * @tc.steps: step2. set the current position is -1, call MoveToNext, GetPostion and GetEntry interface looply.
     * @tc.expected: step2. return values are all right.
     */
    int currentPosition = CURSOR_POSITION_NEGATIVE1;
    Entry entry;
    bool result = true;
    for (int position = CURSOR_POSITION_NEGATIVE1; position < ONE_RECORD; ++position) {
        result = resultSet->MoveToNext();
        if (position < (ONE_RECORD - CURSOR_POSITION_1)) {
            EXPECT_TRUE(result == true);
        } else {
            EXPECT_TRUE(result == false);
        }
        currentPosition = resultSet->GetPosition();
        EXPECT_TRUE(currentPosition == (position + CURSOR_POSITION_1));
        if (position < (ONE_RECORD - CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet->GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entries[position + CURSOR_POSITION_1].key));
            EXPECT_TRUE(CompareVector(entry.value, entries[position + CURSOR_POSITION_1].value));
        } else {
            EXPECT_TRUE(resultSet->GetEntry(entry) == NOT_FOUND);
        }
    }
    /**
     * @tc.steps: step2. set the current position is 100, call MoveToPrevious, GetPostion, GetEntry looply.
     * @tc.expected: step2. return values are all right.
     */
    for (int position = ONE_RECORD; position > CURSOR_POSITION_NEGATIVE1; --position) {
        result = resultSet->MoveToPrevious();
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(result == true);
        } else {
            EXPECT_TRUE(result == false);
        }
        currentPosition = resultSet->GetPosition();
        EXPECT_TRUE(currentPosition == (position - CURSOR_POSITION_1));
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet->GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entries[position - CURSOR_POSITION_1].key));
            EXPECT_TRUE(CompareVector(entry.value, entries[position - CURSOR_POSITION_1].value));
        } else {
            EXPECT_TRUE(resultSet->GetEntry(entry) == NOT_FOUND);
        }
    }
    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb007(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_RECORD);
    EXPECT_TRUE(delegate->Put(entries[0].key, entries[0].value) == OK);
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);
    /**
     * @tc.steps: step2. call Move interface move to offset 50 and check the result.
     * @tc.expected: step2. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status1 = {CURSOR_OFFSET_1, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step3. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status2 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call Move interface move to offset 60 by upstairs and check the result.
     * @tc.expected: step4. Move failed, and the position is 100, and can't get entry.
     */
    MoveStatus status3 = {CURSOR_OFFSET_2, false, CURSOR_POSITION_1, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status3));
    /**
     * @tc.steps: step5. call Move interface move to offset -50 by upstairs and check the result.
     * @tc.expected: step5. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status4 = {CURSOR_OFFSET_NEGATIVE1, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status4));
    /**
     * @tc.steps: step6. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step6. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status5 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status5));
    /**
     * @tc.steps: step7. call Move interface move to offset -60 by upstairs and check the result.
     * @tc.expected: step7. Move failed, and the position is -1, and can't get entry.
     */
    MoveStatus status6 = {CURSOR_OFFSET_NEGATIVE2, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status6));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb008(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, FOUR_M_LONG_STRING};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_RECORD);
    EXPECT_TRUE(delegate->Put(entries[0].key, entries[0].value) == OK);
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_K, resultSet) == OK);
    /**
     * @tc.steps: step2. call MoveToPostion interface move to position 0 and check the result.
     * @tc.expected: step2. MoveToPostion ok, and the position is 50, and can get entry.
     */
    PositionStatus02 status1 = {CURSOR_ORIGINAL_POSITION_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call MoveToPostion interface move to position 1 and check the result.
     * @tc.expected: step3. MoveToPostion false, and the position is 1, and can't get entry.
     */
    PositionStatus02 status2 = {CURSOR_ORIGINAL_POSITION_1, false, CURSOR_POSITION_1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call MoveToPostion interface move to position -1 and check the result.
     * @tc.expected: step4. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status3 = {CURSOR_ORIGINAL_POSITION_NEGATIVE1, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status3));
    /**
     * @tc.steps: step5. call MoveToPostion interface move to position 2 and check the result.
     * @tc.expected: step5. Move failed, and the position is 1, and can't get entry.
     */
    PositionStatus02 status4 = {CURSOR_ORIGINAL_POSITION_2, false, CURSOR_POSITION_1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status4));
    /**
     * @tc.steps: step6. call MoveToPostion interface move to position 1 and check the result.
     * @tc.expected: step6. Move failed, and the position is 1, and can't get entry.
     */
    PositionStatus02 status5 = {CURSOR_ORIGINAL_POSITION_1, false, CURSOR_POSITION_1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status5));
    /**
     * @tc.steps: step7. call MoveToPostion interface move to position 0 and check the result.
     * @tc.expected: step7. move OK, and the position is 0, and can get entry.
     */
    PositionStatus02 status6 = {CURSOR_ORIGINAL_POSITION_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status6));
    /**
     * @tc.steps: step8. call MoveToPostion interface move to position 1 and check the result.
     * @tc.expected: step8. Move failed, and the position is 1, and can't get entry.
     */
    PositionStatus02 status7 = {CURSOR_ORIGINAL_POSITION_1, false, CURSOR_POSITION_1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status7));
    /**
     * @tc.steps: step9. call MoveToPostion interface move to position -1 and check the result.
     * @tc.expected: step9. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status8 = {CURSOR_ORIGINAL_POSITION_NEGATIVE1, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status8));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb009(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }

    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSet) == OK);
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success and returned 10 records.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. call GetPosition, MoveToPrevious and GetPosition interface and check the result.
     * @tc.expected: step3. GetPosition returned -1, MoveToPrevious returned false, and GetPosition still returned -1.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_TRUE(resultSet->MoveToPrevious() == false);
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    /**
     * @tc.steps: step4. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast and GetPostion interface.
     * @tc.expected: step4. when the Current position is -1, other position judge interface can return right result.
     */
    PositionStatus01 position = {NOT_FOUND, false, false, true, false, CURSOR_POSITION_NEGATIVE1};
    EXPECT_TRUE(JudgePosition(resultSet, position));
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToFirst() == true);
    /**
     * @tc.steps: step6. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step6. when the Current position is 0, other position judge interface can return right result.
     */
    position = {OK, true, false, false, false, CURSOR_POSITION_0};
    EXPECT_TRUE(JudgePosition(resultSet, position));
    /**
     * @tc.steps: step7. call MoveToNext interface.
     * @tc.expected: step7. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == true);
    /**
     * @tc.steps: step8. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step8. when the Current position is 1, other position judge interface can return right result.
     */
    position = {OK, false, false, false, false, CURSOR_POSITION_1};
    EXPECT_TRUE(JudgePosition(resultSet, position));
    /**
     * @tc.steps: step9. call MoveToLast interface.
     * @tc.expected: step9. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToLast() == true);
    /**
     * @tc.steps: step10. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step10. when the Current position is 9, other position judge interface can return right result.
     */
    position = {OK, false, true, false, false, CURSOR_POSITION_99};
    EXPECT_TRUE(JudgePosition(resultSet, position));
    /**
     * @tc.steps: step11. call MoveToNext interface.
     * @tc.expected: step11. Move success.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == false);
    /**
     * @tc.steps: step12. call GetEntry, IsFirst, IsLast, IsBeforeFirst, IsAfterLast, GetPostion interface.
     * @tc.expected: step12. when the Current position is 10, other position judge interface can return right result.
     */
    position = {NOT_FOUND, false, false, false, true, CURSOR_POSITION_100};
    EXPECT_TRUE(JudgePosition(resultSet, position));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb010(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }

    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSet) == OK);
    sort(entries.begin(), entries.end(), DistributedTestTools::CompareKey);
    /**
     * @tc.steps: step2. set the current position is -1, call MoveToNext, GetPostion and GetEntry interface looply.
     * @tc.expected: step2. return values are all right.
     */
    EXPECT_TRUE(DistributedDBNbTestTools::MoveToNextFromBegin(*resultSet, entries, CURSOR_POSITION_100));
    /**
     * @tc.steps: step3. set the current position is 100, call MoveToPrevious, GetPostion, GetEntry looply.
     * @tc.expected: step3. return values are all right.
     */
    int currentPosition = CURSOR_POSITION_NEGATIVE1;
    Entry entry;
    bool result = true;
    for (int position = ONE_HUNDRED_RECORDS; position > CURSOR_POSITION_NEGATIVE1; --position) {
        result = resultSet->MoveToPrevious();
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(result == true);
        } else {
            EXPECT_TRUE(result == false);
        }
        currentPosition = resultSet->GetPosition();
        EXPECT_TRUE(currentPosition == (position - CURSOR_POSITION_1));
        if (position > (CURSOR_POSITION_NEGATIVE1 + CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet->GetEntry(entry) == OK);
            EXPECT_TRUE(CompareVector(entry.key, entries[position - CURSOR_POSITION_1].key));
            EXPECT_TRUE(CompareVector(entry.value, entries[position - CURSOR_POSITION_1].value));
        } else {
            EXPECT_TRUE(resultSet->GetEntry(entry) == NOT_FOUND);
        }
    }
    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb011(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSet) == OK);
    /**
     * @tc.steps: step2. call Move interface move to offset 45 and check the result.
     * @tc.expected: step2. move ok, and the position is 44, and can get entry.
     */
    MoveStatus status1 = {CURSOR_OFFSET_45, true, CURSOR_POSITION_44, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step3. move ok, and the position is 44, and can get entry.
     */
    MoveStatus status2 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_44, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call Move interface move to offset 65 by upstairs and check the result.
     * @tc.expected: step4. Move failed, and the position is 100, and can't get entry.
     */
    MoveStatus status3 = {CURSOR_OFFSET_55, true, CURSOR_POSITION_99, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status3));
    MoveStatus status4 = {CURSOR_OFFSET_65, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status4));
    /**
     * @tc.steps: step5. call Move interface move to offset -45 by upstairs and check the result.
     * @tc.expected: step5. move ok, and the position is 55, and can get entry.
     */
    MoveStatus status5 = {CURSOR_OFFSET_NEGATIVE45, true, CURSOR_POSITION_55, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status5));
    /**
     * @tc.steps: step6. call Move interface move to offset 0 by upstairs and check the result.
     * @tc.expected: step6. move ok, and the position is 50, and can get entry.
     */
    MoveStatus status6 = {CURSOR_OFFSET_0, true, CURSOR_POSITION_55, OK};
    EXPECT_TRUE(MoveAndCheck(resultSet, status6));
    /**
     * @tc.steps: step7. call Move interface move to offset -65 by upstairs and check the result.
     * @tc.expected: step7. Move failed, and the position is -1, and can't get entry.
     */
    MoveStatus status7 = {CURSOR_OFFSET_NEGATIVE65, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveAndCheck(resultSet, status7));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb012(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSet) == OK);
    /**
     * @tc.steps: step2. call MoveToPostion interface move to position 40 and check the result.
     * @tc.expected: step2. Move OK, and the position is 40, and can get entry.
     */
    PositionStatus02 status1 = {CURSOR_ORIGINAL_POSITION_40, true, CURSOR_POSITION_40, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status1));
    /**
     * @tc.steps: step3. call MoveToPostion interface move to position 60 and check the result.
     * @tc.expected: step3. Move failed, and the position is 60, and can't get entry.
     */
    PositionStatus02 status2 = {CURSOR_ORIGINAL_POSITION_60, true, CURSOR_POSITION_60, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status2));
    /**
     * @tc.steps: step4. call MoveToPostion interface move to position -1 and check the result.
     * @tc.expected: step4. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status3 = {CURSOR_ORIGINAL_POSITION_NEGATIVE1, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status3));
    /**
     * @tc.steps: step5. call MoveToPostion interface move to position 120 and check the result.
     * @tc.expected: step5. Move failed, and the position is 100, and can't get entry.
     */
    PositionStatus02 status4 = {CURSOR_ORIGINAL_POSITION_120, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status4));
    /**
     * @tc.steps: step6. call MoveToPostion interface move to position 100 and check the result.
     * @tc.expected: step6. Move failed, and the position is 100, and can't get entry.
     */
    PositionStatus02 status5 = {CURSOR_ORIGINAL_POSITION_100, false, CURSOR_POSITION_100, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status5));
    /**
     * @tc.steps: step7. call MoveToPostion interface move to position 0 and check the result.
     * @tc.expected: step7. move OK, and the position is 0, and can get entry.
     */
    PositionStatus02 status6 = {CURSOR_ORIGINAL_POSITION_0, true, CURSOR_POSITION_0, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status6));
    /**
     * @tc.steps: step8. call MoveToPostion interface move to position 99 and check the result.
     * @tc.expected: step8. move OK, and the position is 99, and can get entry.
     */
    PositionStatus02 status7 = {CURSOR_ORIGINAL_POSITION_99, true, CURSOR_POSITION_99, OK};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status7));
    /**
     * @tc.steps: step9. call MoveToPostion interface move to position -1 and check the result.
     * @tc.expected: step9. Move failed, and the position is -1, and can't get entry.
     */
    PositionStatus02 status8 = {CURSOR_ORIGINAL_POSITION_NEGATIVE1, false, CURSOR_POSITION_NEGATIVE1, NOT_FOUND};
    EXPECT_TRUE(MoveToPositionAndCheck(resultSet, status8));

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb013(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    Entry entry4M, entry2M;
    entry4M.key.assign(KEY_SIX_BYTE, 'k');
    entry4M.value.assign(FOUR_M_LONG_STRING, 'v');
    entry2M.key.assign(KEY_THIRTYTWO_BYTE, 'k');
    entry2M.value.assign(TWO_M_LONG_STRING, 'v');
    entries.push_back(entry4M);
    entries.push_back(entry2M);
    for (unsigned int index = 0; index < CURSOR_DIFFERENT_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSet) == OK);
    /**
     * @tc.steps: step2. call GetCount interface get number of records of cursor.
     * @tc.expected: step2. the number is 102.
     */
    EXPECT_TRUE(resultSet->GetCount() == CURSOR_DIFFERENT_RECORDS);
    unsigned int position = 0;
    while (position < CURSOR_DIFFERENT_RECORDS) {
        /**
         * @tc.steps: step3. call MoveToNext interface and check the result.
         * @tc.expected: step3. move ok.
         */
        if (position != (CURSOR_DIFFERENT_RECORDS - CURSOR_POSITION_1)) {
            EXPECT_TRUE(resultSet->MoveToNext());
            position = resultSet->GetPosition();
            /**
             * @tc.steps: step4. call Move interface by 1 offset and check the result.
             * @tc.expected: step4. move ok.
             */
            EXPECT_TRUE(resultSet->Move(CURSOR_OFFSET_1));
            /**
             * @tc.steps: step5. call GetPosition interface by 1 offset and check the result.
             * @tc.expected: step5. GetPosition ok and the position increased by 1 each loop.
             */
            position = resultSet->GetPosition();
            EXPECT_TRUE(resultSet->MoveToPosition(++position));
        } else {
            EXPECT_FALSE(resultSet->MoveToNext());
            /**
             * @tc.steps: step4. call Move interface by 1 offset and check the result.
             * @tc.expected: step4. move ok.
             */
            EXPECT_FALSE(resultSet->Move(CURSOR_OFFSET_1));
            /**
             * @tc.steps: step5. call GetPosition interface by 1 offset and check the result.
             * @tc.expected: step5. GetPosition ok and the position increased by 1 each loop.
             */
            position = resultSet->GetPosition();
            EXPECT_FALSE(resultSet->MoveToPosition(++position));
        }
    }

    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb014(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet with the prefix = { 'a' }.
     * @tc.expected: step1. get KvStoreResultSet success.
     */
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_A, resultSet), OK);
    /**
     * @tc.steps: step2. call GetCount interface get number of records of cursor.
     * @tc.expected: step2. the number is 0.
     */
    EXPECT_TRUE(resultSet->GetCount() == NO_RECORD);
    /**
     * @tc.steps: step3. call IsFirst interface check whether the current position is first.
     * @tc.expected: step3. return false.
     */
    EXPECT_TRUE(resultSet->IsFirst() == false);
    /**
     * @tc.steps: step4. call IsLast interface check whether the current position is last.
     * @tc.expected: step4. return false.
     */
    EXPECT_TRUE(resultSet->IsLast() == false);
    /**
     * @tc.steps: step5. call MoveToFirst interface check whether it can MoveToFirst.
     * @tc.expected: step5. return false.
     */
    EXPECT_TRUE(resultSet->MoveToFirst() == false);
    /**
     * @tc.steps: step6. call MoveToLast interface check whether it can MoveToLast.
     * @tc.expected: step6. return false.
     */
    EXPECT_TRUE(resultSet->MoveToLast() == false);
    /**
     * @tc.steps: step7. call IsBeforeFirst interface check whether it can MoveToLast.
     * @tc.expected: step7. return false.
     */
    EXPECT_TRUE(resultSet->IsBeforeFirst() == true);
    /**
     * @tc.steps: step8. call MoveToNext first and then call IsBeforeFirst interface check the result.
     * @tc.expected: step8. MoveToNext can returns ok, but IsBeforeFirst still returns false.
     */
    EXPECT_TRUE(resultSet->MoveToNext() == false);
    EXPECT_TRUE(resultSet->IsBeforeFirst() == true);
    /**
     * @tc.steps: step9. call IsAfterLast interface check whether it can MoveToLast.
     * @tc.expected: step9. return false.
     */
    EXPECT_TRUE(resultSet->IsAfterLast() == true);
    /**
     * @tc.steps: step10. call MoveToPrevious first and then call IsAfterLast interface check the result.
     * @tc.expected: step10. MoveToPrevious can returns ok, but IsAfterLast still returns false.
     */
    EXPECT_TRUE(resultSet->MoveToPrevious() == false);
    EXPECT_TRUE(resultSet->IsAfterLast() == true);

    /**
     * @tc.steps: step11. close KvStoreResultSet.
     * @tc.expected: step11. close success.
     */
    EXPECT_TRUE(delegate->CloseResultSet(resultSet) == OK);
}

void DistributedNbCursorTestcase::ResultSetDb015(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet with the prefix = { 'a' }.
     * @tc.expected: step1. get KvStoreResultSet success.
     */
    Entry entry;
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_A, resultSet), OK);
    /**
     * @tc.steps: step2. call GetCount interface get number of records of cursor.
     * @tc.expected: step2. the number is 0.
     */
    EXPECT_TRUE(resultSet->GetCount() == NO_RECORD);
    /**
     * @tc.steps: step3. call Move interface with the offset is 0.
     * @tc.expected: step3. return false.
     */
    EXPECT_TRUE(resultSet->Move(CURSOR_OFFSET_0) == false);
    /**
     * @tc.steps: step4. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step4. GetPosition returns -1, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_TRUE(resultSet->GetEntry(entry) == NOT_FOUND);
    /**
     * @tc.steps: step5. call Move interface with the offset is -1.
     * @tc.expected: step5. return false.
     */
    EXPECT_TRUE(resultSet->Move(CURSOR_OFFSET_NEGATIVE1) == false);
    /**
     * @tc.steps: step6. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step6. GetPosition returns -1, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_EQ(resultSet->GetEntry(entry), NOT_FOUND);
    /**
     * @tc.steps: step7. call Move interface with the offset is 5.
     * @tc.expected: step7. return false.
     */
    EXPECT_TRUE(resultSet->Move(CURSOR_OFFSET_5) == false);
    /**
     * @tc.steps: step8. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step8. GetPosition returns 0, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_0);
    EXPECT_EQ(resultSet->GetEntry(entry), NOT_FOUND);

    /**
     * @tc.steps: step11. close KvStoreResultSet.
     * @tc.expected: step11. close success.
     */
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);
}

void DistributedNbCursorTestcase::ResultSetDb016(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    std::vector<DistributedDB::Entry> entries;
    EntrySize entrySize = {KEY_SIX_BYTE, VALUE_HUNDRED_K_BYTE};
    GenerateAppointPrefixAndSizeRecords(entries, entrySize, ONE_HUNDRED_RECORDS);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegate->Put(entries[index].key, entries[index].value) == OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface to get KvStoreResultSet with the prefix = { 'a' }.
     * @tc.expected: step1. get KvStoreResultSet success.
     */
    Entry entryGot;
    KvStoreResultSet *resultSet = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_A, resultSet), OK);
    /**
     * @tc.steps: step2. call GetCount interface get number of records of cursor.
     * @tc.expected: step2. the number is 0.
     */
    EXPECT_TRUE(resultSet->GetCount() == NO_RECORD);
    /**
     * @tc.steps: step3. call MoveToPosition interface to move to position 0.
     * @tc.expected: step3. return false.
     */
    EXPECT_TRUE(resultSet->MoveToPosition(CURSOR_POSITION_0) == false);
    /**
     * @tc.steps: step4. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step4. GetPosition returns -1, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_0);
    EXPECT_EQ(resultSet->GetEntry(entryGot), NOT_FOUND);
    /**
     * @tc.steps: step5. call MoveToPosition interface to move to position 2.
     * @tc.expected: step5. return false.
     */
    EXPECT_TRUE(resultSet->MoveToPosition(CURSOR_POSITION_2) == false);
    /**
     * @tc.steps: step6. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step6. GetPosition returns 0, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_0);
    EXPECT_EQ(resultSet->GetEntry(entryGot), NOT_FOUND);
    /**
     * @tc.steps: step7. call MoveToPosition interface to move to position -5.
     * @tc.expected: step7. return false.
     */
    EXPECT_TRUE(resultSet->MoveToPosition(CURSOR_POSITION_NEGATIVE5) == false);
    /**
     * @tc.steps: step8. call GetPosition interface to check the current position and GetEntry to check the Entry.
     * @tc.expected: step8. GetPosition returns -1, and GetEntry returns NOT_FOUND.
     */
    EXPECT_TRUE(resultSet->GetPosition() == CURSOR_POSITION_NEGATIVE1);
    EXPECT_EQ(resultSet->GetEntry(entryGot), NOT_FOUND);

    /**
     * @tc.steps: step11. close KvStoreResultSet.
     * @tc.expected: step11. close success.
     */
    EXPECT_EQ(delegate->CloseResultSet(resultSet), OK);
}

void DistributedNbCursorTestcase::ResultSetDb017(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entries;
    vector<Key> allKey;
    GenerateRecords(ONE_HUNDRED_RECORDS, DEFAULT_START, allKey, entries);
    for (const auto &iter : entries) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call CloseResultSet interface with nullptr parameter.
     * @tc.expected: step2. return INVALID_ARGS.
     */
    KvStoreResultSet *resultSetAllptr = nullptr;
    EXPECT_EQ(delegate->CloseResultSet(resultSetAllptr), INVALID_ARGS);
    /**
     * @tc.steps: step3. call CloseResultSet interface with resultSetAll.
     * @tc.expected: step3. return OK.
     */
    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
}

void DistributedNbCursorTestcase::ResultSetDb018(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entriesKA, entriesKB;
    vector<Key> allKeysKA, allKeysKB;
    std::vector<uint8_t> ka = { 'k', 'a' };
    std::vector<uint8_t> kb = { 'k', 'b' };
    GenerateRecords(ONE_HUNDRED_RECORDS, DEFAULT_START, allKeysKA, entriesKA, ka);
    GenerateRecords(ONE_HUNDRED_RECORDS, DEFAULT_START, allKeysKB, entriesKB, kb);
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_EQ(delegate->Put(entriesKA[index].key, entriesKA[index].value), OK);
        EXPECT_EQ(delegate->Put(entriesKB[index].key, entriesKB[index].value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetEntries interface with "ka" parameter to get KvStoreResultSet.
     * @tc.expected: step2. get success.
     */
    KvStoreResultSet *resultSetKA = nullptr;
    Key keyPrefixKA = ka;
    EXPECT_EQ(delegate->GetEntries(keyPrefixKA, resultSetKA), OK);
    /**
     * @tc.steps: step3. call GetEntries interface with "kb" parameter to get KvStoreResultSet.
     * @tc.expected: step3. get success.
     */
    KvStoreResultSet *resultSetKB = nullptr;
    Key keyPrefixKB = kb;
    EXPECT_EQ(delegate->GetEntries(keyPrefixKB, resultSetKB), OK);
    /**
     * @tc.steps: step4. call GetCount interface of all recordsets.
     * @tc.expected: step4. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == TWO_HUNDREDS_RECORDS);
    EXPECT_TRUE(resultSetKA->GetCount() == ONE_HUNDRED_RECORDS);
    EXPECT_TRUE(resultSetKB->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step5. close resultSetAll.
     * @tc.expected: step5. call success.
     */
    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
    /**
     * @tc.steps: step6. close resultSetAll.
     * @tc.expected: step6. call success.
     */
    EXPECT_TRUE(resultSetKA->GetCount() == ONE_HUNDRED_RECORDS);
    EXPECT_TRUE(resultSetKB->GetCount() == ONE_HUNDRED_RECORDS);

    EXPECT_EQ(delegate->CloseResultSet(resultSetKA), OK);
    EXPECT_EQ(delegate->CloseResultSet(resultSetKB), OK);
}

void DistributedNbCursorTestcase::ResultSetDb019(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, TEN_RECORDS, FOUR_BYTE_KEY, ONE_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetCount interface of resultSetAll and delete k1~k5.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == TEN_RECORDS);
    for (unsigned int delCnt = 0; delCnt < FIVE_RECORDS; ++delCnt) {
        EXPECT_EQ(DistributedDBNbTestTools::Delete(*delegate, entriesBatch[0].key), OK);
        entriesBatch.erase(entriesBatch.begin());
    }
    /**
     * @tc.steps: step3. update k6 and insert another 10 * 1M records.
     * @tc.expected: step3. call success.
     */
    entriesBatch[0].value.push_back('a');
    EXPECT_EQ(delegate->Put(entriesBatch[0].key, entriesBatch[0].value), OK);
    vector<Entry> entriesBatch2;
    vector<Key> allKeys2;
    GenerateFixedRecords(entriesBatch2, allKeys2, TEN_RECORDS, KEY_SIX_BYTE, ONE_M_LONG_STRING);
    for (const auto &iter : entriesBatch2) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step4. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step4. get success.
     */
    KvStoreResultSet *resultSetAll2 = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll2), OK);
    /**
     * @tc.steps: step5. close resultSetAll.
     * @tc.expected: step5. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == TEN_RECORDS);
    EXPECT_TRUE(resultSetAll2->GetCount() == FIFTEEN_RECORDS);

    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
    EXPECT_EQ(delegate->CloseResultSet(resultSetAll2), OK);
}

void DistributedNbCursorTestcase::ResultSetDb020(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entries;
    vector<Key> allKey;
    GenerateRecords(ONE_HUNDRED_RECORDS, DEFAULT_START, allKey, entries);
    for (const auto &iter : entries) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetEntries interface with "ka" parameter to get KvStoreResultSet.
     * @tc.expected: step2. get success.
     */
    KvStoreResultSet *resultSetKA = nullptr;
    std::vector<uint8_t> ka = { 'k', 'a' };
    Key keyPrefixKA = ka;
    EXPECT_EQ(delegate->GetEntries(keyPrefixKA, resultSetKA), OK);
    /**
     * @tc.steps: step3. call GetEntries interface with "kb" parameter to get KvStoreResultSet.
     * @tc.expected: step3. get success.
     */
    KvStoreResultSet *resultSetKB = nullptr;
    std::vector<uint8_t> kb = { 'k', 'b' };
    Key keyPrefixKB = kb;
    EXPECT_EQ(delegate->GetEntries(keyPrefixKB, resultSetKB), OK);
    /**
     * @tc.steps: step3. call GetEntries interface with "kc" parameter to get KvStoreResultSet.
     * @tc.expected: step3. get success.
     */
    KvStoreResultSet *resultSetKC = nullptr;
    std::vector<uint8_t> kc = { 'k', 'c' };
    Key keyPrefixKC = kc;
    EXPECT_EQ(delegate->GetEntries(keyPrefixKC, resultSetKC), OK);
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll2 = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll2), OVER_MAX_LIMITS);

    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
    EXPECT_EQ(delegate->CloseResultSet(resultSetKA), OK);
    EXPECT_EQ(delegate->CloseResultSet(resultSetKB), OK);
    EXPECT_EQ(delegate->CloseResultSet(resultSetKC), OK);
}

void DistributedNbCursorTestcase::ResultSetDb021(KvStoreNbDelegate *delegate,
    KvStoreDelegateManager *manager, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr && manager != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entries;
    vector<Key> allKey;
    GenerateRecords(ONE_HUNDRED_RECORDS, DEFAULT_START, allKey, entries);
    for (const auto &iter : entries) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetCount interface of resultSetAll.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. closeKvStore returns BUSY because resultSet was not closed.
     * @tc.expected: step3. call success.
     */
    EXPECT_EQ(manager->CloseKvStore(delegate), BUSY);

    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
}

void DistributedNbCursorTestcase::ResultSetDb022(bool isRowIdMode)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *delegate = nullptr;
    Option option;
    option.isEncryptedDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS, FOUR_BYTE_KEY, ONE_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_TRUE(delegate->GetEntries(KEY_EMPTY, resultSetAll) == OK);
    /**
     * @tc.steps: step2. call GetCount interface of resultSetAll.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. Rekey with g_passwd1.
     * @tc.expected: step3. call success.
     */
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    EXPECT_EQ(delegate->Rekey(g_passwd1), BUSY);

    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}

void DistributedNbCursorTestcase::ResultSetDb023(bool isRowIdMode)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *delegate = nullptr;
    Option option;
    option.isEncryptedDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, FOUR_RECORDS, FOUR_BYTE_KEY, ONE_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetCount interface of resultSetAll.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == FOUR_RECORDS);
    /**
     * @tc.steps: step3. Rekey with g_passwd1.
     * @tc.expected: step3. return BUSY.
     */
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    EXPECT_EQ(delegate->Rekey(g_passwd1), BUSY);

    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    delete manager;
    manager = nullptr;

    Option option2;
    option2.isEncryptedDb = true;
    option2.passwd = PASSWD_VECTOR_1;
    option2.createIfNecessary = IS_NOT_NEED_CREATE;
    KvStoreNbDelegate *delegate2 = nullptr;
    KvStoreDelegateManager *manager2 = nullptr;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, option2);
    ASSERT_TRUE(manager2 == nullptr && delegate2 == nullptr);

    option2.isEncryptedDb = false;
    delegate2 = DistributedDBNbTestTools::GetNbDelegateSuccess(manager2, g_dbParameter2, option2);
    ASSERT_TRUE(manager2 != nullptr && delegate2 != nullptr);
    EXPECT_EQ(manager2->CloseKvStore(delegate2), OK);
    delegate2 = nullptr;
    EXPECT_EQ(manager2->DeleteKvStore(STORE_ID_2), OK);
    delete manager2;
    manager2 = nullptr;
}

void DistributedNbCursorTestcase::ResultSetDb024(bool isRowIdMode)
{
    KvStoreDelegateManager *manager = nullptr;
    KvStoreNbDelegate *delegate = nullptr;
    Option option;
    option.isEncryptedDb = false;
    delegate = DistributedDBNbTestTools::GetNbDelegateSuccess(manager, g_dbParameter2, option);
    ASSERT_TRUE(manager != nullptr && delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);

    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS, FOUR_BYTE_KEY, FOUR_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }
    /**
     * @tc.steps: step1. Rekey STORE_ID_SYNC_2 with g_passwd1.
     * @tc.expected: step1. operate successfully or BUSY(if the rekey is later executed).
     */
    std::mutex mtx;
    std::condition_variable conditionRekeyVar;
    bool rekeyFlag = false;
    (void)g_passwd1.SetValue(PASSWD_VECTOR_1.data(), PASSWD_VECTOR_1.size());
    thread subThread([&]() {
        auto status = delegate->Rekey(g_passwd1);
        EXPECT_EQ(((status == OK) || (status == BUSY)), true);
        std::unique_lock<std::mutex> lck(mtx);
        conditionRekeyVar.notify_all();
        rekeyFlag = true;
    });
    subThread.detach();
    /**
     * @tc.steps: step2. call GetEntries interface to get KvStoreResultSet.
     * @tc.expected: step2. return BUSY or OK(if the rekey is later executed).
     */
    KvStoreResultSet *resultSetK = nullptr;
    Key keyPrefix = { 'k' };
    std::this_thread::sleep_for(std::chrono::microseconds(WAIT_FOR_OBSERVER_REKEY)); // wait the rekey operation.
    DBStatus status = delegate->GetEntries(keyPrefix, resultSetK);
    EXPECT_EQ(((status == OK) || (status == BUSY)), true);

    std::unique_lock<std::mutex> lck(mtx);
    conditionRekeyVar.wait(lck, [&] { return rekeyFlag; });
    if (resultSetK != nullptr) {
        delegate->CloseResultSet(resultSetK);
    }
    EXPECT_EQ(manager->CloseKvStore(delegate), OK);
    delegate = nullptr;
    EXPECT_EQ(manager->DeleteKvStore(STORE_ID_2), OK);
    delete manager;
    manager = nullptr;
}
namespace {
void CursorOperThread(KvStoreNbDelegate *&nbCursorDelegate)
{
    ASSERT_TRUE(nbCursorDelegate != nullptr);
    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(nbCursorDelegate->GetEntries(KEY_EMPTY, resultSetAll), OK);
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSetAll->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. call IsFirst interface.
     * @tc.expected: step3. call success.
     */
    EXPECT_TRUE(resultSetAll->IsFirst() == false);
    /**
     * @tc.steps: step4. call IsLast interface.
     * @tc.expected: step4. call success.
     */
    EXPECT_TRUE(resultSetAll->IsLast() == false);
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. call success.
     */
    EXPECT_TRUE(resultSetAll->MoveToFirst() == true);
    /**
     * @tc.steps: step6. call MoveToLast interface.
     * @tc.expected: step6. call success.
     */
    EXPECT_TRUE(resultSetAll->MoveToLast() == true);
    /**
     * @tc.steps: step7. call IsBeforeFirst interface.
     * @tc.expected: step7. call success.
     */
    EXPECT_TRUE(resultSetAll->IsBeforeFirst() == false);
    /**
     * @tc.steps: step8. call MoveToNext interface.
     * @tc.expected: step8. call success.
     */
    EXPECT_TRUE(resultSetAll->MoveToNext() == false);
    /**
     * @tc.steps: step9. call IsAfterLast interface.
     * @tc.expected: step9. call success.
     */
    EXPECT_TRUE(resultSetAll->IsAfterLast() == true);
    /**
     * @tc.steps: step10. call MoveToPrevious and then call IsAfterLast interface.
     * @tc.expected: step10. call success.
     */
    EXPECT_TRUE(resultSetAll->MoveToPrevious() == true);
    EXPECT_TRUE(resultSetAll->IsAfterLast() == false);
    /**
     * @tc.steps: step11. close recordset.
     * @tc.expected: step11. call success.
     */
    EXPECT_EQ(nbCursorDelegate->CloseResultSet(resultSetAll), OK);
}
}
void DistributedNbCursorTestcase::ResultSetDb025(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS,
        FOUR_BYTE_KEY, ONE_TENTH_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }

    /**
     * @tc.steps: step1. Call resultSet interfaces.
     * @tc.expected: step1. operate successfully.
     */
    std::vector<std::thread> threads;
    for (unsigned int threadId = THREAD_NUM_START; threadId <= THREAD_NUM_END; ++threadId) {
        threads.push_back(std::thread(CursorOperThread, std::ref(delegate)));
    }
    for (auto& th : threads) {
        th.join();
    }
}
namespace {
void CursorRandOperThread1(KvStoreResultSet *&resultSet)
{
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. call IsFirst interface.
     * @tc.expected: step3. no crash.
     */
    resultSet->IsFirst();
    /**
     * @tc.steps: step4. call IsLast interface.
     * @tc.expected: step4. call success.
     */
    resultSet->IsLast();
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. call success.
     */
    resultSet->MoveToFirst();
    /**
     * @tc.steps: step6. call MoveToLast interface.
     * @tc.expected: step6. call success.
     */
    resultSet->MoveToLast();
    /**
     * @tc.steps: step7. call IsBeforeFirst interface.
     * @tc.expected: step7. call success.
     */
    resultSet->IsBeforeFirst();
    /**
     * @tc.steps: step8. call MoveToNext interface.
     * @tc.expected: step8. call success.
     */
    resultSet->MoveToNext();
    /**
     * @tc.steps: step9. call IsAfterLast interface.
     * @tc.expected: step9. call success.
     */
    resultSet->IsAfterLast();
    /**
     * @tc.steps: step10. call MoveToPrevious and then call IsAfterLast interface.
     * @tc.expected: step10. call success.
     */
    resultSet->MoveToPrevious();
    resultSet->IsAfterLast();
}

void CursorRandOperThread2(KvStoreResultSet *&resultSet)
{
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. call IsFirst interface.
     * @tc.expected: step3. call success.
     */
    resultSet->IsFirst();
    /**
     * @tc.steps: step7. call IsBeforeFirst interface.
     * @tc.expected: step7. call success.
     */
    resultSet->IsBeforeFirst();
    /**
     * @tc.steps: step8. call MoveToNext interface.
     * @tc.expected: step8. call success.
     */
    resultSet->MoveToNext();
    /**
     * @tc.steps: step9. call IsAfterLast interface.
     * @tc.expected: step9. call success.
     */
    resultSet->IsAfterLast();
    /**
     * @tc.steps: step4. call IsLast interface.
     * @tc.expected: step4. call success.
     */
    resultSet->IsLast();
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. call success.
     */
    resultSet->MoveToFirst();
    /**
     * @tc.steps: step6. call MoveToLast interface.
     * @tc.expected: step6. call success.
     */
    resultSet->MoveToLast();
    /**
     * @tc.steps: step10. call MoveToPrevious and then call IsAfterLast interface.
     * @tc.expected: step10. call success.
     */
    resultSet->MoveToPrevious();
    resultSet->IsAfterLast();
}

void CursorRandOperThread3(KvStoreResultSet *&resultSet)
{
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);
    /**
     * @tc.steps: step3. call IsFirst interface.
     * @tc.expected: step3. call success.
     */
    resultSet->IsFirst();
    /**
     * @tc.steps: step6. call MoveToLast interface.
     * @tc.expected: step6. call success.
     */
    resultSet->MoveToLast();
    /**
     * @tc.steps: step7. call IsBeforeFirst interface.
     * @tc.expected: step7. call success.
     */
    resultSet->IsBeforeFirst();
    /**
     * @tc.steps: step4. call IsLast interface.
     * @tc.expected: step4. call success.
     */
    resultSet->IsLast();
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. call success.
     */
    resultSet->MoveToFirst();
    /**
     * @tc.steps: step8. call MoveToNext interface.
     * @tc.expected: step8. call success.
     */
    resultSet->MoveToNext();
    /**
     * @tc.steps: step9. call IsAfterLast interface.
     * @tc.expected: step9. call success.
     */
    resultSet->IsAfterLast();
    /**
     * @tc.steps: step10. call MoveToPrevious and then call IsAfterLast interface.
     * @tc.expected: step10. call success.
     */
    resultSet->MoveToPrevious();
    resultSet->IsAfterLast();
}

void CursorRandOperThread4(KvStoreResultSet *&resultSet)
{
    /**
     * @tc.steps: step10. call MoveToPrevious and then call IsAfterLast interface.
     * @tc.expected: step10. call success.
     */
    resultSet->IsAfterLast();
    resultSet->MoveToPrevious();
    /**
     * @tc.steps: step9. call IsAfterLast interface.
     * @tc.expected: step9. call success.
     */
    resultSet->IsAfterLast();
    /**
     * @tc.steps: step8. call MoveToNext interface.
     * @tc.expected: step8. call success.
     */
    resultSet->MoveToNext();
    /**
     * @tc.steps: step7. call IsBeforeFirst interface.
     * @tc.expected: step7. call success.
     */
    resultSet->IsBeforeFirst();
    /**
     * @tc.steps: step6. call MoveToLast interface.
     * @tc.expected: step6. call success.
     */
    resultSet->MoveToLast();
    /**
     * @tc.steps: step5. call MoveToFirst interface.
     * @tc.expected: step5. call success.
     */
    resultSet->MoveToFirst();
    /**
     * @tc.steps: step4. call IsLast interface.
     * @tc.expected: step4. call success.
     */
    resultSet->IsLast();
    /**
     * @tc.steps: step3. call IsFirst interface.
     * @tc.expected: step3. call success.
     */
    resultSet->IsFirst();
    /**
     * @tc.steps: step2. call GetCount interface.
     * @tc.expected: step2. call success.
     */
    EXPECT_TRUE(resultSet->GetCount() == ONE_HUNDRED_RECORDS);
}
}

void DistributedNbCursorTestcase::ResultSetDb026(KvStoreNbDelegate *delegate, bool isRowIdMode)
{
    ASSERT_TRUE(delegate != nullptr);
    SetResultSetCacheMode(delegate, isRowIdMode);
    vector<Entry> entriesBatch;
    vector<Key> allKeys;
    GenerateFixedRecords(entriesBatch, allKeys, ONE_HUNDRED_RECORDS,
        FOUR_BYTE_KEY, ONE_TENTH_M_LONG_STRING);
    for (const auto &iter : entriesBatch) {
        EXPECT_EQ(delegate->Put(iter.key, iter.value), OK);
    }

    /**
     * @tc.steps: step1. call GetEntries interface with "" parameter to get KvStoreResultSet.
     * @tc.expected: step1. get success.
     */
    KvStoreResultSet *resultSetAll = nullptr;
    EXPECT_EQ(delegate->GetEntries(KEY_EMPTY, resultSetAll), OK);

    /**
     * @tc.steps: step2. Call resultSet interfaces.
     * @tc.expected: step2. operate successfully.
     */
    std::vector<std::thread> threads;
    threads.push_back(std::thread(CursorRandOperThread1, std::ref(resultSetAll)));
    threads.push_back(std::thread(CursorRandOperThread2, std::ref(resultSetAll)));
    threads.push_back(std::thread(CursorRandOperThread3, std::ref(resultSetAll)));
    threads.push_back(std::thread(CursorRandOperThread4, std::ref(resultSetAll)));

    for (auto& th : threads) {
        th.join();
    }

    /**
     * @tc.steps: step11. close recordset.
     * @tc.expected: step11. call success.
     */
    EXPECT_EQ(delegate->CloseResultSet(resultSetAll), OK);
}
namespace {
void VerifyResultSetInterfaces(KvStoreNbDelegate **delegates, unsigned long delegateCount)
{
    ASSERT_TRUE(delegates != nullptr);
    Key keyPrefixKA = { 'k', 'a' };
    Key keyPrefixKB = { 'k', 'b' };
    Key keyPrefixKK = { 'k', 'k' };
    KvStoreResultSet *resultSetAlls[OPEN_DB_TIMES] = { nullptr };
    KvStoreResultSet *resultSetKAs[OPEN_DB_TIMES] = { nullptr };
    KvStoreResultSet *resultSetKBs[OPEN_DB_TIMES] = { nullptr };
    KvStoreResultSet *resultSetKKs[OPEN_DB_TIMES] = { nullptr };
    unsigned long delegateCnt = 0;
    for (delegateCnt = 0; delegateCnt < delegateCount; ++delegateCnt) {
        /**
         * @tc.steps: step2. check GetEntries of interfaces of every delegate.
         * @tc.expected: step2. success.
         */
        EXPECT_EQ(delegates[delegateCnt]->GetEntries(KEY_EMPTY, resultSetAlls[delegateCnt]), OK);
        EXPECT_EQ(delegates[delegateCnt]->GetEntries(keyPrefixKA, resultSetKAs[delegateCnt]), OK);
        EXPECT_EQ(delegates[delegateCnt]->GetEntries(keyPrefixKB, resultSetKBs[delegateCnt]), OK);
        EXPECT_EQ(delegates[delegateCnt]->GetEntries(keyPrefixKK, resultSetKKs[delegateCnt]), OK);
        /**
         * @tc.steps: step3. check GetCount of interfaces of every delegate.
         * @tc.expected: step3. success.
         */
        EXPECT_TRUE(resultSetAlls[delegateCnt]->GetCount() == TWO_HUNDREDS_RECORDS);
        EXPECT_TRUE(resultSetKAs[delegateCnt]->GetCount() == ONE_HUNDRED_RECORDS);
        EXPECT_TRUE(resultSetKBs[delegateCnt]->GetCount() == ONE_HUNDRED_RECORDS);
        EXPECT_TRUE(resultSetKKs[delegateCnt]->GetCount() == 0);
    }

    for (delegateCnt = 0; delegateCnt < delegateCount; ++delegateCnt) {
        /**
         * @tc.steps: step4. check GetCount of interfaces of every delegate.
         * @tc.expected: step4. success.
         */
        EXPECT_TRUE(delegates[delegateCnt]->CloseResultSet(resultSetAlls[delegateCnt]) == OK);
        EXPECT_TRUE(delegates[delegateCnt]->CloseResultSet(resultSetKAs[delegateCnt]) == OK);
        EXPECT_TRUE(delegates[delegateCnt]->CloseResultSet(resultSetKBs[delegateCnt]) == OK);
        EXPECT_TRUE(delegates[delegateCnt]->CloseResultSet(resultSetKKs[delegateCnt]) == OK);
    }
}
}

void DistributedNbCursorTestcase::ResultSetDb027(bool isRowIdMode)
{
    KvStoreDelegateManager *managers[OPEN_DB_TIMES] = {nullptr};
    KvStoreNbDelegate *delegates[OPEN_DB_TIMES] = {nullptr};
    Option option;
    unsigned long delegateCnt = 0;
    /**
     * @tc.steps: step1. open STORE_ID_2 for three times.
     * @tc.expected: step1. success.
     */
    delegates[INDEX_ZEROTH] = DistributedDBNbTestTools::GetNbDelegateSuccess(managers[INDEX_ZEROTH],
        g_dbParameter3, option);
    ASSERT_TRUE(managers[INDEX_ZEROTH] != nullptr && delegates[INDEX_ZEROTH] != nullptr);
    SetResultSetCacheMode(delegates[INDEX_ZEROTH], isRowIdMode);

    vector<Entry> entriesKA, entriesKB;
    vector<Key> allKeysKA, allKeysKB;
    std::vector<uint8_t> ka = { 'k', 'a' };
    std::vector<uint8_t> kb = { 'k', 'b' };
    EntrySize entrySizeKA = { FIVE_BYTE_KEY, ONE_K_LONG_STRING };
    EntrySize entrySizeKB = { FIVE_BYTE_KEY, ONE_TENTH_M_LONG_STRING };
    GenerateAppointPrefixAndSizeRecords(entriesKA, entrySizeKA, ONE_HUNDRED_RECORDS, ka, {'v'});
    GenerateAppointPrefixAndSizeRecords(entriesKB, entrySizeKB, ONE_HUNDRED_RECORDS, kb, {'v'});
    for (int index = 0; index < ONE_HUNDRED_RECORDS; index++) {
        EXPECT_TRUE(delegates[INDEX_ZEROTH]->Put(entriesKA[index].key, entriesKA[index].value) == OK);
        EXPECT_TRUE(delegates[INDEX_ZEROTH]->Put(entriesKB[index].key, entriesKB[index].value) == OK);
    }

    option.createIfNecessary = false;
    for (delegateCnt = INDEX_FIRST; delegateCnt < DELEGATE_NUM; ++delegateCnt) {
        delegates[delegateCnt] = DistributedDBNbTestTools::GetNbDelegateSuccess(managers[delegateCnt],
            g_dbParameter3, option);
        ASSERT_TRUE(managers[delegateCnt] != nullptr && delegates[delegateCnt] != nullptr);
        SetResultSetCacheMode(delegates[delegateCnt], isRowIdMode);
    }

    VerifyResultSetInterfaces(delegates, DELEGATE_NUM);
    for (unsigned long operCnt = INDEX_ZEROTH; operCnt < OPEN_DB_TIMES; ++operCnt) {
        EXPECT_EQ(managers[operCnt]->CloseKvStore(delegates[operCnt]), OK);
        delegates[operCnt] = nullptr;
        if (operCnt == OPEN_DB_TIMES - 1) {
            EXPECT_EQ(managers[operCnt]->DeleteKvStore(STORE_ID_3), OK);
        }
        delete managers[operCnt];
        managers[operCnt] = nullptr;
    }
}