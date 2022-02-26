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
#ifndef DISTRIBUTED_DB_SCHEMA_TEST_TOOLS_H
#define DISTRIBUTED_DB_SCHEMA_TEST_TOOLS_H

#include <cfloat>
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "types.h"
#include "distributeddb_data_generator.h"
const static int RECORDNUM = 257;
const static int TWO_RECORD = 2;
const static int RECORDSIZE = 4;
const static int ARRAY_SIZE = 3;
const static int OVER_MAXSIZE = 4097;
const static int SCHEMA_INDEX = 5;
const static unsigned int INDEX_FIFY = 5;
const static unsigned int INDEX_SIX = 6;
const static unsigned int INDEX_NINE = 9;
const static unsigned int INDEX_TEN = 10;
// the valid std::string of schema field
const static std::string VALID_VERSION_1 = "1.0";
const std::vector<std::string> VALID_VERSION = {VALID_VERSION_1};
const static std::string VALID_MODE_1 = "STRICT";
const static std::string VALID_MODE_2 = "COMPATIBLE";
const std::vector<std::string> VALID_MODE = {VALID_MODE_1, VALID_MODE_2};
const static std::string VALID_DEFINE_1 = "{\"field1\":\"STRING,NOT NULL,DEFAULT 'fxy'\"," \
    "\"_field1\":{\"field1\":\"STRING,NOT NULL,DEFAULT 'fxy'\",\"field2\":\"STRING  ,DEFAULT null\"," \
    "\"field3\":{\"field4\":\"STRING,NOT NULL  \",\"field5\":{\"field6\":\"STRING\t\"," \
    "\"field7\":[]}}},\"field1\":\"BOOL,NOT NULL,DEFAULT   true\"," \
    "\"field9\":\"BOOL,DEFAULT false\",\"Field9\":\"\tBOOL\"," \
    "\"field10\":{\"field10\":\"BOOL,DEFAULT null\",\"field11\":\"INTEGER, NOT NULL,DEFAULT -10\"," \
    "\"field12\":\"INTEGER,DEFAULT null\",\"field13\":\"INTEGER  \",\"field14\":{}}," \
    "\"field15\":\"LONG,NOT NULL,DEFAULT +10\",\"field16\":\"LONG,DEFAULT null\",\"field17\":\" LONG\"," \
    "\"field18\":\"DOUBLE,NOT NULL,DEFAULT -0.0\",\"_19\":\"DOUBLE,DEFAULT null\",\"f\":\"DOUBLE\"}";
const std::vector<std::string> VALID_DEFINE = {VALID_DEFINE_1};
const static std::string VALID_INDEX_1 = "[]";
const static std::string VALID_INDEX_2 = "[\" $.field1\",\"$.field10.field10 \",\"$.field10.field11\r\",\"$.f\"]";
const static std::string VALID_INDEX_3 = "[\"\t$.field1\",\" $._field1.field1\",\"$._field1.field3.field5.field6 \"," \
    "\"$.field9\",\"$.Field9\"]";
const static std::string VALID_INDEX_4 = "[\"$.field9\",\"$.field10.field10\",\"$._field1.field3.field4\"," \
    "\"$._field1.field3.field5.field6\"]";
const std::vector<std::string> VALID_INDEX = {VALID_INDEX_1, VALID_INDEX_2, VALID_INDEX_3};
const static std::string INVALID_INDEX_1 = "[19]";
const static std::string INVALID_INDEX_2 = "[\"$.field1\",\"$._field1.field5.field6\",\"$.field10.field13\"," \
    "\"$field20\"]";
const static std::string INVALID_INDEX_3 = "[\"$.field1\",\"$._field1.field5.field6\",\"$.field11.field10\"]";
const static std::string INVALID_INDEX_4 = "[\".field1\",\"$.field10.field11\"]";
const static std::string INVALID_INDEX_5 = "[\"$$.field1\",\"$.field10.field11\"]";
const static std::string INVALID_INDEX_6 = "[\"$.field1\",\"$.field10 .field11\"]";
const static std::string INVALID_INDEX_7 = "[\"$.field1\",\"$ .field10.field11\"]";
const static std::string INVALID_INDEX_8 = "[\"$.field1\",\"$.field1\"]";
const static std::string INVALID_INDEX_9 = "[\"$.field10.field14\"]";
const std::vector<std::string> INALID_INDEX = {INVALID_INDEX_1, INVALID_INDEX_2, INVALID_INDEX_3, INVALID_INDEX_4,
    INVALID_INDEX_5, INVALID_INDEX_6, INVALID_INDEX_7, INVALID_INDEX_8, INVALID_INDEX_9};
const static std::string INVALID_VERSION_1 = "";
const static std::string INVALID_VERSION_2 = "1";
const static std::string INVALID_VERSION_3 = "1.1";
const std::vector<std::string> INVALID_VERSION = {INVALID_VERSION_1, INVALID_VERSION_2, INVALID_VERSION_3};
const static std::string INVALID_MODE_1 = "strict";
const static std::string INVALID_MODE_2 = "compatible";
const static std::string INVALID_MODE_3 = "LOOSE";
const static std::string INVALID_MODE_4 = "";
const std::vector<std::string> INVALID_MODE = {INVALID_MODE_1, INVALID_MODE_2, INVALID_MODE_3, INVALID_MODE_4};
const static std::string INVALID_DEFINE_1 = "{}";
const static std::string INVALID_DEFINE_2 = "{\"field1\":{\"field2\":{\"field3\":{\"field4\":{\"field5\":{}}}}}}";
static std::vector<std::string> VALID_DEFINE_FIELD = {"\"field\""};
static std::vector<std::string> INVALID_DEFINE_FIELD = {"\"field!\"", "\"_ \"", "\"1_field\"", "field", "\"\""};
static std::vector<std::string> VALID_TYPE = {"STRING", "BOOL", "INTEGER", "LONG", "DOUBLE"};
static std::vector<std::string> INVALID_TYPE = {"", "string", "FLOAT"};
static std::vector<std::string> VALID_NOTNULL = {"NOT NULL"};
static std::vector<std::string> INVALID_NOTNULL = {"", "NULL", "null", "\"null\"", "EMPTY", "NOTNULL",
    "NOT  NULL", "NOT_NULL"};
static std::vector<std::string> VALID_DEFAULT = {"DEFAULT 'fxy'", "DEFAULT null", ""};
static std::vector<std::string> INVALID_DEFAULT = {"DAFAULT", "default", "DEFAULTtrue", "DEFAULT null",
    "DEFAULT \"fxy\"", "DEFAULT 'true'", "DEFAULT 'false'", "DAFAULT TRUE", "DEFAULT FALSE", "DEFAULT '10'",
    "DEFAULT '10.5'", "DEFAULT 10e4", "DEFAULT 1.05E-5", "DEFAULT 0X1A", ""};
// the order of three attributes is invalid
const std::string ATTRIBUTES_PRE1 = "\"INTEGER,NOT NULL,DEFAULT ";
const std::string ATTRIBUTES_PRE2 = "\"LONG,NOT NULL,DEFAULT ";
const std::string ATTRIBUTES_PRE3 = "\"DOUBLE,NOT NULL,DEFAULT ";
static std::vector<std::string> INVALID_ATTRIBUTES = {
    "\"STRING,DEFAULT 'fxy',NOT NULL\"",
    "\"DEFAULT 'fxy',NOT NULL,STRING\"",
    "\"DEFAULT 'fxy',STRING,NOT NULL\"",
    "\"NOT NULL,DEFAULT 'fxy',STRING\"",
    "\"NOT NULL,STRING,DEFAULT 'fxy'\"",
    "\"STRING,\"",
    "\"STRING,DEFAULT\"",
    "\"NOT NULL,DAFAULT 'fxy'\"",
    "\"DEFAULT 'fxy'\"",
    "\"NOT NULL\"",
    "\"STRING,DEFAULT NULL\"",
    "\"INTEGER,DEFAULT 'null'\"",
    "\"STRING,NOT NULL,DEFAULT 'fxy',\"",
    ATTRIBUTES_PRE1 + std::to_string(INT32_MAX) + "1\"",
    ATTRIBUTES_PRE1 + std::to_string(INT32_MIN) + "1\"",
    ATTRIBUTES_PRE2 + std::to_string(INT64_MAX) + "1\"",
    ATTRIBUTES_PRE2 + std::to_string(INT64_MIN) + "1\"",
    ATTRIBUTES_PRE3 + "1" + std::to_string(DBL_MAX) + "\"",
    ATTRIBUTES_PRE3 + "-1" + std::to_string(DBL_MAX) + "\""
};
struct SchemaDefine {
    std::vector<std::string> field;
    std::vector<std::string> type;
    std::vector<std::string> notnull;
    std::vector<std::string> defaultValue;
};

static std::string VALUE_MATCH_1 = "\"_field1\":{\"field1\":\"abc\",\"field2\":null,\"field3\":{\"field4\":\"def\"," \
    "\"field5\":{\"field6\":\"fxy\",\"field7\":[]}}}";
static std::string VALUE_MATCH_2 = "\"field1\":false,\"field9\":null,\"Field9\":true,\"field10\":{\"field10\":true," \
    "\"field11\":-1000000,\"field12\":null,\"field13\":150000,\"field14\":{}},\"field15\":666,\"field16\":null," \
    "\"field17\":-100,\"field18\":-1.05e-4,\"_19\":0.0,\"f\":null";

const std::string VALID_COMBINATION_DEFINE = "{\"field1\":\"STRING ,DEFAULT null\",\"field2\":" \
    "{\"field3\":\"BOOL ,DEFAULT null\",\"field4\":{\"field5\":\"INTEGER ,DEFAULT null\",\"field6\":" \
    "{\"field7\":\"LONG ,DEFAULT null\",\"field8\":\"DOUBLE ,DEFAULT null\"}}}}";

const static std::string PERF_SCHEMA_DEFINE = "{\"field1\":\"LONG\"," \
    "\"field2\":\"STRING\", \"field3\":\"STRING\", \"field4\":\"STRING\", \"field5\":\"STRING\"," \
    "\"field6\":\"DOUBLE\", \"field7\":\"STRING\", \"field8\":\"STRING\", \"field9\":\"STRING\"," \
    "\"field10\":\"STRING\", \"field11\":\"STRING\", \"field12\":\"STRING\", \"field13\":\"STRING\"," \
    "\"field14\":\"STRING\", \"field15\":\"STRING\", \"field16\":\"STRING\", \"field17\":\"STRING\"," \
    "\"field18\":\"STRING\", \"field19\":\"STRING\", \"field20\":\"STRING\", \"field21\":\"STRING\"," \
    "\"field22\":\"STRING\", \"field23\":\"STRING\", \"field24\":\"STRING\", \"field25\":\"STRING\"," \
    "\"field26\":\"STRING\", \"field27\":\"STRING\", \"field28\":\"STRING\", \"field29\":\"STRING\"," \
    "\"field30\":\"STRING\"}";
const static std::string PERF_SCHEMA_SIX_INDEXES = "[\"$.field1\",\"$.field2\",\"$.field3\",\"$.field4\", " \
    "\"$.field5\", \"$.field6\"]";
const static std::string SKIP_SIZE = "1";
const static unsigned int FIRST_FIELD = 1;
const static unsigned int SECOND_FIELD = 2;
const static unsigned int THIRD_FIELD = 3;
const static unsigned int FOURTH_FIELD = 4;
const static unsigned int FIFTH_FIELD = 5;
const static unsigned int SIXTH_FIELD = 6;
const static unsigned int THIRTIETH_FIELD = 30;
const static unsigned int TEST_START_CNT = 1;

const static std::string VALUE_SKIP_STRING = "a";
struct RecordInfo {
    uint8_t keyFilledChr;
    unsigned int keyLength;
    uint8_t valueFilledChr;
    unsigned int valueLength;
    RecordInfo(uint8_t keyFilledChr, unsigned int keyLength, uint8_t valueFilledChr, unsigned int valueLength)
        : keyFilledChr(keyFilledChr), keyLength(keyLength), valueFilledChr(valueFilledChr), valueLength(valueLength)
    {
    }
};
class DistributedDBSchemaTestTools final {
public:

    DistributedDBSchemaTestTools() {}
    ~DistributedDBSchemaTestTools() {}

    // Delete the copy and assign constructors
    DistributedDBSchemaTestTools(const DistributedDBSchemaTestTools &DistributedDBSchemaTestTools) = delete;
    DistributedDBSchemaTestTools& operator=(const DistributedDBSchemaTestTools &DistributedDBSchemaTestTools) = delete;
    DistributedDBSchemaTestTools(DistributedDBSchemaTestTools &&DistributedDBSchemaTestTools) = delete;
    DistributedDBSchemaTestTools& operator=(DistributedDBSchemaTestTools &&DistributedDBSchemaTestTools) = delete;
    static DistributedDB::Entry GenerateFixedLenSchemaRecord(const unsigned long serialNo,
        const EntrySize &entrySize, const uint8_t keyFilledChr, const uint8_t valueFilledChr);
    static std::vector<DistributedDB::Entry> GenerateFixedSchemaRecords(std::vector<DistributedDB::Key> &allKeys,
        const int recordNum, const EntrySize &entrySize, const uint8_t keyFilledChr, const uint8_t valueFilledChr);
    static DistributedDB::Entry GenerateFixedLenSchemaPerfRecord(const uint64_t presetRecordsCnt,
        const uint64_t serialNo, const RecordInfo &recordInfo,
        const std::string &valueSkipString = VALUE_SKIP_STRING);
};
#endif // DISTRIBUTED_DB_SCHEMA_TEST_TOOLS_H
