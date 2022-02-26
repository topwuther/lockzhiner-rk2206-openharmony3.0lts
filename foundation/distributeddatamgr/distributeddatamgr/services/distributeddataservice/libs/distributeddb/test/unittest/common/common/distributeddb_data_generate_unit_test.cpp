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

#include "distributeddb_data_generate_unit_test.h"
#include "types.h"

using namespace DistributedDB;

namespace DistributedDBUnitTest {
void GenerateKey(int keyCount, int startPosition, Key &keyTest)
{
    if (keyCount <= 0) {
        return;
    }
    int i;
    for (i = 0; i < keyCount; i++) {
        keyTest.push_back(KEY_NUM[(i + startPosition) % NUM_LENGTH]);
    }
}

void GenerateValue(int valueCount, int startPosition, Value &valueTest)
{
    if (valueCount <= 0) {
        return;
    }
    int i;
    for (i = 0; i < valueCount; i++) {
        valueTest.push_back(VALUE_LETTER[(i + startPosition) % LETTER_LENGTH]);
    }
}

void GenerateEntry(int entryCount, int startPosition, Entry &entryTest)
{
    if (entryCount <= 0) {
        return;
    }
    GenerateKey(entryCount, startPosition, entryTest.key);
    GenerateValue(entryCount, startPosition, entryTest.value);
}

void GenerateEntryVector(int entryVectorCount, int entryCount, std::vector<Entry> &entrysTest)
{
    if (entryVectorCount <= 0 || entryCount <= 0) {
        return;
    }
    int i;
    for (i = 0; i < entryVectorCount; i++) {
        Entry entry;
        GenerateEntry(entryCount, i, entry);
        entrysTest.push_back(entry);
    }
}

void GenerateRecords(int recordNum, std::vector<Entry> &entries, std::vector<Key> &keys, int keySize, int valSize)
{
    Entry entry;
    // start from index 1
    for (int recordIndex = 1; recordIndex <= recordNum; ++recordIndex) {
        std::string cntStr = std::to_string(recordIndex);
        int len = cntStr.length();
        if (keySize <= len) {
            break;
        }
        if (valSize <= len) {
            break;
        }

        entry.key.assign((keySize - len), '0');
        entry.value.assign((valSize - len), 'v');
        for (auto item = cntStr.begin(); item != cntStr.end(); ++item) {
            entry.key.push_back(*item);
            entry.value.push_back(*item);
        }
        entries.push_back(entry);
        keys.push_back(entry.key);

        entry.key.clear();
        entry.value.clear();
    }
}
} // namespace DistributedDBUnitTest