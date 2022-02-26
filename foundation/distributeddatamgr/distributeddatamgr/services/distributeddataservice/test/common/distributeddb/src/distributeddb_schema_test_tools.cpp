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
#include "distributeddb_schema_test_tools.h"

#include <gtest/gtest.h>
#include <fstream>
#include "distributeddb_nb_test_tools.h"

using namespace std;
using namespace DistributedDB;
using namespace DistributedDBDataGenerator;

DistributedDB::Entry DistributedDBSchemaTestTools::GenerateFixedLenSchemaRecord(const unsigned long serialNo,
    const EntrySize &entrySize, const uint8_t keyFilledChr, const uint8_t valueFilledChr)
{
    DistributedDB::Entry entry;
    std::string serialNoStr = std::to_string(serialNo);
    entry.key.assign(entrySize.keySize - serialNoStr.length(), keyFilledChr);
    for (unsigned long index = 0; index < serialNoStr.size(); ++index) {
        entry.key.push_back(serialNoStr[index]);
    }

    string val = string("{") + "\"field" + std::to_string(FIRST_FIELD) + "\":" + std::to_string(serialNo) + ",";
    for (unsigned int fieldIndex = SECOND_FIELD; fieldIndex <= THIRTIETH_FIELD; fieldIndex++) {
        if (fieldIndex == SIXTH_FIELD) {
            val += "\"field" + std::to_string(fieldIndex) + "\":" + to_string(static_cast<double>(serialNo)) + ",";
        } else {
            val += "\"field" + std::to_string(fieldIndex) + "\":" +
                "\"SchemaPerfTest" + std::to_string(fieldIndex) + "_" + std::to_string(serialNo) + "\",";
        }
    }
    val.back() = '}';
    size_t found = val.rfind("PerfTest");
    if (found != string::npos) {
        string insert = "PerfTest";
        insert += string(entrySize.valSize - val.length(), valueFilledChr);
        val.replace(found, strlen("PerfTest"), insert);
    }
    string scmVal = "a" + val;
    DistributedDB::Value schemaVal(scmVal.begin(), scmVal.end());
    entry.value = schemaVal;
    return entry;
}

vector<DistributedDB::Entry> DistributedDBSchemaTestTools::GenerateFixedSchemaRecords(
    vector<DistributedDB::Key> &allKeys, const int recordNum, const EntrySize &entrySize,
    const uint8_t keyFilledChr, const uint8_t valueFilledChr)
{
    vector<DistributedDB::Entry> entries;
    DistributedDB::Entry entry;
    for (int serialNo = TEST_START_CNT; serialNo <= recordNum; ++serialNo) {
        entry = GenerateFixedLenSchemaRecord(serialNo, entrySize, keyFilledChr, valueFilledChr);
        allKeys.push_back(entry.key);
        entries.push_back(entry);
        entry.key.clear();
        entry.value.clear();
    }
    return entries;
}

DistributedDB::Entry DistributedDBSchemaTestTools::GenerateFixedLenSchemaPerfRecord(
    const uint64_t presetRecordsCnt, const uint64_t serialNo, const RecordInfo &recordInfo,
    const string &valueSkipString)
{
    DistributedDB::Entry entry;
    std::string serialNoStr = std::to_string(serialNo);
    entry.key.assign(recordInfo.keyLength - serialNoStr.length(), recordInfo.keyFilledChr);
    for (unsigned long index = 0; index < serialNoStr.size(); ++index) {
        entry.key.push_back(serialNoStr[index]);
    }

    string val = string("{") + "\"field" + std::to_string(FIRST_FIELD) + "\":" + std::to_string(serialNo) + ",";
    unsigned int fieldIndex;
    for (fieldIndex = SECOND_FIELD; fieldIndex <= THIRD_FIELD; fieldIndex++) {
        val += "\"field" + std::to_string(fieldIndex) + "\":" +
            "\"SchemaPerfTest" + std::to_string(fieldIndex) + "_" + serialNoStr + "\",";
    }
    if (serialNo <= static_cast<uint64_t>(presetRecordsCnt) / 2) { // 2 is an half of records.
        for (fieldIndex = FOURTH_FIELD; fieldIndex <= FIFTH_FIELD; fieldIndex++) {
            val += "\"field" + std::to_string(fieldIndex) + "\":" +
                "\"SchemaPerfTest" + std::to_string(fieldIndex) + "_" + serialNoStr + "\",";
        }
    }
    for (; fieldIndex <= THIRTIETH_FIELD; fieldIndex++) {
        if (fieldIndex == SIXTH_FIELD) {
            val += "\"field" + std::to_string(fieldIndex) + "\":" + to_string(static_cast<double>(serialNo)) + ",";
        } else {
            val += "\"field" + std::to_string(fieldIndex) + "\":" +
                "\"SchemaPerfTest" + std::to_string(fieldIndex) + "\",";
        }
    }
    val.back() = '}';
    size_t found = val.rfind("PerfTest");
    if (found != string::npos) {
        string insert = "PerfTest";
        if (recordInfo.valueLength > val.length()) {
            insert += string(recordInfo.valueLength - val.length(), recordInfo.valueFilledChr);
            val.replace(found, strlen("PerfTest"), insert);
        } else {
            MST_LOG("[GenerateFixedLenSchemaPerfRecord] recordInfo.valueLength(%u) is too short, " \
                "it should be more than %zu.", recordInfo.valueLength, val.length());
            return entry;
        }
    }
    string scmVal = valueSkipString + val;
    DistributedDB::Value schemaVal(scmVal.begin(), scmVal.end());
    entry.value = schemaVal;
    return entry;
}