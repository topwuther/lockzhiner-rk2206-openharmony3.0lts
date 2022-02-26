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

#ifndef DISTRIBUTEDDB_DATA_GENERATE_UNIT_H
#define DISTRIBUTEDDB_DATA_GENERATE_UNIT_H

#include <string>
#include <vector>
#include "distributeddb_tools_unit_test.h"
#include "types.h"

namespace DistributedDBUnitTest {
// define some variables to init a KvStoreDelegateManager object.
const std::string APP_ID = "app0";
const std::string USER_ID = "user0";

const std::string STORE_ID_LOCAL = "distributed_local_db_test";
const std::string STORE_ID_SYNC = "distributed_sync_db_test";
const std::string STORE_ID_1 = "distributed_db_test1";
const std::string STORE_ID_2 = "distributed_db_test2";
const std::string STORE_ID_3 = "distributed_db_test3";
const std::string STORE_ID_4 = "distributed_db_test4";
const std::string STORE_ID_5 = "distributed_db_test5";
const std::string STORE_ID_6 = "distributed_db_test6";
const std::string STORE_ID_7 = "distributed_db_test7";
const std::string STORE_ID_8 = "distributed_db_test8";

const int NUM_LENGTH = 10;
const int LETTER_LENGTH = 52;
const uint8_t KEY_NUM[NUM_LENGTH] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const uint8_t VALUE_LETTER[LETTER_LENGTH] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'L', 'M', 'L', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U',
    'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
    'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'l', 'm', 'l', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w',
    'x', 'y', 'z'
};

const DistributedDB::Key KEY_1 = {'1'};
const DistributedDB::Value VALUE_1 = {'a'};
const DistributedDB::Key KEY_2 = {'2'};
const DistributedDB::Value VALUE_2 = {'b'};
const DistributedDB::Key KEY_3 = {'3'};
const DistributedDB::Value VALUE_3 = {'c'};
const DistributedDB::Key KEY_4 = {'4'};
const DistributedDB::Value VALUE_4 = {'d'};
const DistributedDB::Key KEY_5 = {'5'};
const DistributedDB::Value VALUE_5 = {'e'};
const DistributedDB::Key KEY_6 = {'6'};
const DistributedDB::Value VALUE_6 = {'f'};
const DistributedDB::Key KEY_7 = {'7'};
const DistributedDB::Value VALUE_7 = {'g'};

const DistributedDB::Key NULL_KEY_1;
const DistributedDB::Value NULL_VALUE_1;

const DistributedDB::Entry ENTRY_1 = {KEY_1, VALUE_1};
const DistributedDB::Entry ENTRY_2 = {KEY_2, VALUE_2};
const DistributedDB::Entry NULL_ENTRY_1 = {NULL_KEY_1, VALUE_1};
const DistributedDB::Entry NULL_ENTRY_2 = {KEY_1, NULL_VALUE_1};

const DistributedDB::Entry ENTRY_3 = {KEY_3, VALUE_3};
const DistributedDB::Entry ENTRY_4 = {KEY_4, VALUE_4};

const DistributedDB::Entry KV_ENTRY_1 = {KEY_1, VALUE_1};
const DistributedDB::Entry KV_ENTRY_2 = {KEY_2, VALUE_2};
const DistributedDB::Entry KV_ENTRY_3 = {KEY_3, VALUE_3};
const DistributedDB::Entry KV_ENTRY_4 = {KEY_4, VALUE_4};

const std::vector<DistributedDB::Entry> ENTRY_VECTOR = {ENTRY_1, ENTRY_2};

const int DEFAULT_NB_KEY_VALUE_SIZE = 10;

// generate a key, has keyCount chars, from KEY_NUM[startPosition], return keyTest
void GenerateKey(int keyCount, int startPosition, DistributedDB::Key &keyTest);

// generate a value, has valueCount chars, from VALUE_LETTER[startPosition], return valueTest
void GenerateValue(int valueCount, int startPosition, DistributedDB::Value &valueTest);

/*
 * generate an entry, entry.key and entry.value have valueCount chars,
 * from KEY_NUM[startPosition] and VALUE_LETTER[startPosition], return entryTest
 */
void GenerateEntry(int entryCount, int startPosition, DistributedDB::Entry &entryTest);

/*
 * generate a vector<entry>, vector.size() is entryVectorCount, entry.key and entry.value have valueCount chars,
 * from KEY_NUM[startPosition] and VALUE_LETTER[startPosition], return entrysTest
 */
void GenerateEntryVector(int entryVectorCount, int entryCount, std::vector<DistributedDB::Entry> &entrysTest);

void GenerateRecords(int recordNum, std::vector<DistributedDB::Entry> &entries, std::vector<DistributedDB::Key> &keys,
    int keySize = DEFAULT_NB_KEY_VALUE_SIZE, int valSize = DEFAULT_NB_KEY_VALUE_SIZE);
} // namespace DistributedDBUnitTest

#endif // DISTRIBUTEDDB_DATA_GENERATE_UNIT_H
