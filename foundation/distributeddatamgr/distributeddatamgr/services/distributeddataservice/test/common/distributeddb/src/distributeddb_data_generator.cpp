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
#include <random>
#include <openssl/rand.h>
#include "distributeddb_data_generator.h"

namespace DistributedDBDataGenerator {
void GenerateRecord(unsigned int keyNo, DistributedDB::Entry &entry,
    std::vector<uint8_t> keyPrefix)
{
    std::string cntStr = std::to_string(keyNo);
    entry.key = keyPrefix;
    entry.value = { 'v' };
    for (auto cntStrIt = cntStr.begin(); cntStrIt != cntStr.end(); ++cntStrIt) {
        entry.key.push_back(*cntStrIt);
        entry.value.push_back(*cntStrIt);
    }
}

void GenerateCharSet(std::vector<uint8_t> &charSet)
{
    for (uint8_t ch = '0'; ch <= '9'; ch++) {
        charSet.push_back(ch);
    }
    for (uint8_t ch = 'A'; ch <= 'Z'; ch++) {
        charSet.push_back(ch);
    }
    for (uint8_t ch = 'a'; ch <= 'z'; ch++) {
        charSet.push_back(ch);
    }
}

void GenerateAlphaNumUnderlineCharSet(std::vector<uint8_t> &charSet)
{
    GenerateCharSet(charSet);
    charSet.push_back('_');
}

void GenerateSpecialCharSet(std::vector<uint8_t> &charSet)
{
    charSet.push_back('\\');
    charSet.push_back('/');
    charSet.push_back('&');
    charSet.push_back('^');
    charSet.push_back('%');
    charSet.push_back('#');
    charSet.push_back('-');
}

void GenerateFixedLenRandString(unsigned int neededLen, RandType randType, std::string &genString)
{
    genString.clear();
    std::vector<uint8_t> charSet;
    if (randType == RandType::ALPHA_NUM) {
        GenerateCharSet(charSet);
    } else if (randType == RandType::ALPHA_NUM_UNDERLINE) {
        GenerateAlphaNumUnderlineCharSet(charSet);
    } else if (randType == RandType::SPECIAL) {
        GenerateSpecialCharSet(charSet);
    }

    for (unsigned int index = 0; index < neededLen; ++index) {
        // the randIdx range is from 0 to (charSet.length() - 1) which is the elements quantity of charSet
        int randIdx = GetRandInt(0, charSet.size() - 1);
        genString.push_back(charSet[randIdx]);
    }
}

void GenerateRandRecord(DistributedDB::Entry &entry, EntrySize &entrySize, unsigned int keyNo)
{
    std::string cntStr = std::to_string(keyNo);
    unsigned int len = cntStr.length();
    if ((entrySize.keySize < len) || (entrySize.valSize < len)) {
        MST_LOG("ERROR:The size of key or value given is too small!");
        return;
    }
    std::vector<uint8_t> charSet;
    GenerateCharSet(charSet);
    for (unsigned int i = 0; i < entrySize.keySize - len; i++) {
        int seed = GetRandInt(0, 61); // the seed range is from 0 to 61 which is the elements quantity of charSet
        entry.key.push_back(charSet[seed]);
    }
    if (entrySize.valSize < ONE_K_LONG_STRING) {
        for (unsigned int i = 0; i < entrySize.valSize - len; i++) {
            int seed = GetRandInt(0, 61); // the seed range is from 0 to 61 which is the elements quantity of charSet
            entry.value.assign(entrySize.valSize - len, charSet[seed]);
        }
    } else {
        int seed = GetRandInt(0, 61); // the seed range is from 0 to 61 which is the elements quantity of charSet
        entry.value.assign(entrySize.valSize - len, charSet[seed]);
    }
    for (auto ch = cntStr.begin(); ch != cntStr.end(); ch++) {
        entry.key.push_back(*ch);
        entry.value.push_back(*ch);
    }
}

void GenerateLongRecord(unsigned int keyNo, DistributedDB::Entry &entry,
    const std::vector<uint8_t> &keyPrefix)
{
    std::string cntStr = std::to_string(keyNo);
    entry.key = keyPrefix;
    entry.value.assign(ONE_K_LONG_STRING, 'v');
    for (auto cntStrIt = cntStr.begin(); cntStrIt != cntStr.end(); ++cntStrIt) {
        entry.key.push_back(*cntStrIt);
        entry.value.push_back(*cntStrIt);
    }
}

void GenerateRecords(unsigned int recordNum, unsigned int start, std::vector<DistributedDB::Key> &allKeys,
    std::vector<DistributedDB::Entry> &entriesBatch, const std::vector<uint8_t> keyPrifix)
{
    DistributedDB::Entry entryCurrent;
    for (unsigned int cnt = start; cnt < start + recordNum; ++cnt) {
        GenerateRecord(cnt, entryCurrent, keyPrifix);
        allKeys.push_back(entryCurrent.key);
        entriesBatch.push_back(entryCurrent);
    }
}

void GenerateMaxBigRecord(unsigned int keyNo, DistributedDB::Entry &entry,
    const std::vector<uint8_t> &keyPrefix, unsigned int num)
{
    std::string cntStr = std::to_string(keyNo);
    entry.key = keyPrefix;
    entry.value.assign(FOUR_M_LONG_STRING, ('v' - num));
    for (auto cntStrIt = cntStr.begin(); cntStrIt != cntStr.end(); ++cntStrIt) {
        entry.key.push_back(*cntStrIt);
    }
    std::vector<uint8_t> keyTail;
    keyTail.assign(ONE_K_LONG_STRING - entry.key.size(), ('k' + num));
    for (auto iter = keyTail.begin(); iter != keyTail.end(); ++iter) {
        entry.key.push_back(*iter);
    }
}

bool GenerateMaxBigRecords(unsigned int recordNum, unsigned int start,
    std::vector<DistributedDB::Key> &allKeys, std::vector<DistributedDB::Entry> &entriesBatch)
{
    if (recordNum > (ACSIIEND - 'k')) {
        MST_LOG("Record generate failed, character is over ASCII, please use other method !");
        return false;
    } else {
        DistributedDB::Entry entryCurrent;
        for (unsigned int cnt = start; cnt < start + recordNum; ++cnt) {
            GenerateMaxBigRecord(cnt, entryCurrent, K_SEARCH_3, (cnt - start));
            allKeys.push_back(entryCurrent.key);
            entriesBatch.push_back(entryCurrent);
        }
    }
    return true;
}

void GenerateTenThousandRecords(unsigned int recordNum, unsigned int start,
    std::vector<DistributedDB::Key> &allKeys, std::vector<DistributedDB::Entry> &entriesBatch)
{
    DistributedDB::Entry entryCurrent;
    for (unsigned int cnt = start; cnt < start + recordNum; ++cnt) {
        GenerateLongRecord(cnt, entryCurrent, K_SEARCH_5);
        allKeys.push_back(entryCurrent.key);
        entriesBatch.push_back(entryCurrent);
    }
}

void GenerateNormalAsciiRecords(DistributedDB::Entry &entry)
{
    entry.key.clear();
    entry.value.clear();
    for (uint8_t lowc = 'a'; lowc <= 'z'; ++lowc) {
        entry.key.push_back(lowc);
        entry.value.push_back(lowc);
    }
    for (uint8_t bigc = 'A'; bigc <= 'Z'; ++bigc) {
        entry.key.push_back(bigc);
        entry.value.push_back(bigc);
    }
    for (uint8_t numc = '0'; numc <= '9'; ++numc) {
        entry.key.push_back(numc);
        entry.value.push_back(numc);
    }
}

void GenerateFullAsciiRecords(DistributedDB::Entry &entry)
{
    entry.key.clear();
    entry.value.clear();
    for (uint8_t lowc = ACSIIEND; lowc > 0; --lowc) {
        entry.key.push_back(lowc);
        entry.value.push_back(lowc);
    }
}

void GenerateBiggistKeyRecords(DistributedDB::Entry &entry)
{
    entry.key.clear();
    entry.value.clear();
    for (auto lowc = ONE_K_LONG_STRING; lowc > 0; --lowc) {
        entry.key.push_back(lowc);
    }
    entry.value.push_back('v');
}

DistributedDB::Entry GenerateFixedLenKVRecord(unsigned int serialNo,
    unsigned int keyLen, uint8_t keyFilledChr,
    unsigned int valueLen, uint8_t valueFilledChr)
{
    DistributedDB::Entry entry;
    std::string serialNoStr = std::to_string(serialNo);
    entry.key.assign(keyLen - serialNoStr.length(), keyFilledChr);
    entry.value.assign(valueLen - serialNoStr.length(), valueFilledChr);
    for (unsigned int index = 0; index < serialNoStr.size(); ++index) {
        entry.key.push_back(serialNoStr[index]);
        entry.value.push_back(serialNoStr[index]);
    }
    return entry;
}

void GenerateFixedRecords(std::vector<DistributedDB::Entry> &entries,
    std::vector<DistributedDB::Key> &allKeys,
    int recordNum, unsigned int keySize, unsigned int valSize)
{
    DistributedDB::Entry entry;
    for (int cnt = DEFAULT_START; cnt <= recordNum; ++cnt) {
        std::string cntStr = std::to_string(cnt);
        int len = cntStr.length();
        entry.key.assign((keySize - len), 'k');
        entry.value.assign((valSize - len), 'v');
        for (auto cntIt = cntStr.begin(); cntIt != cntStr.end(); ++cntIt) {
            entry.key.push_back(*cntIt);
            entry.value.push_back(*cntIt);
        }
        allKeys.push_back(entry.key);
        entries.push_back(entry);
        entry.key.clear();
        entry.value.clear();
    }
}

void GenerateOneRecordForImage(int entryNo, const EntrySize &entrySize,
    const std::vector<uint8_t> &keyPrefix, const std::vector<uint8_t> &val, DistributedDB::Entry &entry)
{
    std::vector<uint8_t> charSet;
    GenerateCharSet(charSet);
    std::string ind = std::to_string(entryNo);
    unsigned int len = ind.length();
    for (auto ch = IMAGE_VALUE_PRE.begin(); ch != IMAGE_VALUE_PRE.end(); ch++) {
        entry.value.push_back(*ch);
    }
    if ((entrySize.keySize < len) || (entrySize.valSize < (len + IMAGE_VALUE_PRE.size()))) {
        MST_LOG("ERROR:The size of key or value given is too small!");
        return;
    }
    entry.key = keyPrefix;
    for (unsigned int cnt = 0; cnt < (entrySize.keySize - len - keyPrefix.size()); cnt++) {
        entry.key.push_back(charSet[GetRandInt(0, 61)]); // randrom in 61 of 0-9,A-Z,a-z.
    }
    for (unsigned int it = 0; it < (entrySize.valSize - len - IMAGE_VALUE_PRE.size()); it++) {
        entry.value.push_back(val[0]);
    }
    for (auto ch = ind.begin(); ch != ind.end(); ch++) {
        entry.key.push_back(*ch);
        entry.value.push_back(*ch);
    }
}

void GenerateRecordsForImage(std::vector<DistributedDB::Entry> &entries, EntrySize &entrySize,
    int num, std::vector<uint8_t> keyPrefix, std::vector<uint8_t> val)
{
    for (int index = 1; index <= num; index++) {
        DistributedDB::Entry entry;
        GenerateOneRecordForImage(index, entrySize, keyPrefix, val, entry);
        entries.push_back(entry);
    }
}

void GenerateAppointPrefixAndSizeRecord(int recordNo, const EntrySize &entrySize,
    const std::vector<uint8_t> &keyPrefix, const std::vector<uint8_t> &valPrefix, DistributedDB::Entry &entry)
{
    std::string recNo = std::to_string(recordNo);
    unsigned int len = recNo.length();
    if ((entrySize.keySize < keyPrefix.size() + len) || (entrySize.valSize < valPrefix.size() + len)) {
        MST_LOG("ERROR:The size of key or value given is too small!");
        return;
    }
    entry.key = keyPrefix;
    entry.value = valPrefix;
    entry.key.insert(entry.key.end(), entrySize.keySize - keyPrefix.size() - len, '0');
    entry.value.insert(entry.value.end(), entrySize.valSize - valPrefix.size() - len, '0');
    for (auto ch = recNo.begin(); ch != recNo.end(); ch++) {
        entry.key.push_back(*ch);
        entry.value.push_back(*ch);
    }
}

void GenerateAppointPrefixAndSizeRecords(std::vector<DistributedDB::Entry> &entries, const EntrySize &entrySize,
    int num, const std::vector<uint8_t> &keyPrefix, const std::vector<uint8_t> &valPrefix)
{
    entries.clear();
    DistributedDB::Entry entry;
    for (int index = 1; index <= num; index++) {
        GenerateAppointPrefixAndSizeRecord(index, entrySize, keyPrefix, valPrefix, entry);
        entries.push_back(entry);
    }
}

int GetRandInt(const int randMin, const int randMax)
{
    std::random_device randDev;
    std::mt19937 genRand(randDev());
    std::uniform_int_distribution<int> disRand(randMin, randMax);
    return disRand(genRand);
}

void GenerateFixedLenRandRecords(std::vector<DistributedDB::Entry> &entries,
    std::vector<DistributedDB::Key> &allKeys,
    int recordNum, unsigned int keySize, unsigned int valSize)
{
    entries.clear();
    allKeys.clear();
    int idx = 0;
    DistributedDB::Entry entry;
    std::vector<uint8_t> charSet;
    GenerateCharSet(charSet);
    for (int cnt = DEFAULT_START; cnt <= recordNum; ++cnt) {
        std::string cntStr = std::to_string(cnt);
        int len = cntStr.length();
        entry.key.push_back('k');
        entry.value.push_back('v');
        for (unsigned int operCnt = 0; operCnt < keySize - len - 1; ++operCnt) {
            idx = GetRandInt(0, 61); // the seed range is from 0 to 61 which is the elements quantity of charSet
            entry.key.push_back(charSet[idx]);
        }
        for (unsigned int operCnt = 0; operCnt < valSize - len - 1; ++operCnt) {
            idx = GetRandInt(0, 61); // the seed range is from 0 to 61 which is the elements quantity of charSet
            entry.value.push_back(charSet[idx]);
        }
        for (auto cntIt = cntStr.begin(); cntIt != cntStr.end(); ++cntIt) {
            entry.key.push_back(*cntIt);
            entry.value.push_back(*cntIt);
        }
        allKeys.push_back(entry.key);
        entries.push_back(entry);
        entry.key.clear();
        entry.value.clear();
    }
}

const std::string GetDbType(const int type)
{
    switch(type) {
        case UNENCRYPTED_DISK_DB:
            return std::string("UnencrpytedDiskDB");
        case ENCRYPTED_DISK_DB:
            return std::string("EncrpytedDiskDB");
        case MEMORY_DB:
            return std::string("MemoryDB");
        default:
            return std::string("ErrorType");
    }
}

void GenerateRandomRecords(std::vector<DistributedDB::Entry> &entries, EntrySize &entrySize, int num)
{
    for (int index = 0; index < num; index++) {
        DistributedDB::Entry entry;
        entry.key.resize(entrySize.keySize);
        RAND_bytes(entry.key.data(), entrySize.keySize);
        entry.value.resize(entrySize.valSize);
        RAND_bytes(entry.value.data(), entrySize.valSize);
        entries.push_back(entry);
    }
}
// Get long schema define with long default x or fields' num
void GetLongSchemaDefine(LongDefine &param, std::string &longDefine)
{
    std::vector<std::string> defaultStr;
    std::string longString;
    for (int index = 1; index <= param.recordNum; index++) {
        std::string ind = std::to_string(index);
        int len = ind.length();
        if (param.recordSize < (len + 1)) {
            MST_LOG("ERROR:The size of key or value given is too small!");
            return;
        }
        longString.assign(param.recordSize - len, param.prefix);
        longString.append(ind);
        defaultStr.push_back(longString);
        longString.clear();
    }
    longDefine.append("{");
    for (int index = 0; index < param.recordNum; index++) {
        longDefine = longDefine + "\"field" + std::to_string(index) + "\":" + "\"STRING,NOT NULL,DEFAULT " + \
            "'" + defaultStr[index] + "'\",";
    }
    longDefine.erase(longDefine.size() - 1, 1);
    longDefine.append("}");
    MST_LOG("longDefine.size() is %zu", longDefine.size());
}
// splice different string to schema
const std::string SpliceToSchema(const std::string &version, const std::string &mode,
    const std::string &define, const std::string &index, const std::string &skipSize)
{
    std::string schema;
    std::string middleString;
    if (index.empty() && skipSize.empty()) {
        middleString = "";
    } else if (!index.empty() && skipSize.empty()) {
        middleString = middleString + ",\"SCHEMA_INDEXES\":" + index;
    } else if (index.empty() && !skipSize.empty()) {
        middleString = middleString + ",\"SCHEMA_SKIPSIZE\":" + skipSize;
    } else {
        middleString = middleString + ",\"SCHEMA_INDEXES\":" + index + "," + "\"SCHEMA_SKIPSIZE\":" + skipSize;
    }
    schema = schema + "{" + "\"SCHEMA_VERSION\"" + ":" + "\"" + version + "\"" + "," +
        "\"SCHEMA_MODE\":" + "\"" + mode + "\"" + "," +
        "\"SCHEMA_DEFINE\"" + ":"  + define + middleString + "}";
    return schema;
}
// the size of field is 64B, of DEFAULT x is 4K, all of them is valid
void GenerateLongValidSchema(Schema &validSchema, std::vector<std::string> &schema)
{
    std::string validLongSchema;
    LongDefine param;
    param.recordNum = ONE_RECORD;
    param.recordSize = FOUR_K_LONG_STRING;
    param.prefix = 'k';
    GetLongSchemaDefine(param, validLongSchema);
    validLongSchema.replace(2, 6, KEY_SIXTYFOUR_BYTE, 'a'); // the 6 str starting at 2 is being replaced.
    std::string splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0),
        validLongSchema, validSchema.index.at(0));
    schema.push_back(splicSchema);
}
// the num of field is 257 with repeat field name, the num of index is 32, but it is valid
void GenerateLargeValidSchema(Schema &validSchema, std::vector<std::string> &schema)
{
    std::string validLargeSchema;
    LongDefine param;
    param.recordNum = TWO_FIVE_SIX_RECORDS;
    param.recordSize = KEY_SIX_BYTE;
    param.prefix = 'k';
    GetLongSchemaDefine(param, validLargeSchema);
    validLargeSchema.erase(validLargeSchema.size() - 1, 1);
    validLargeSchema.append(",\"field0\":\"STRING,NOT NULL,DEFAULT 'kkkkk1'\"}");
    std::string splicSchema, largeIndexRes, largeIndex;
    for (int index = 0; index < KEY_THIRTYTWO_BYTE; index++) {
        largeIndexRes = largeIndexRes + "\"$.field" + std::to_string(index) + "\",";
    }
    largeIndexRes.erase(largeIndexRes.size() - 1, 1);
    largeIndex = largeIndex + "[" + largeIndexRes + "]";
    splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0),
        validLargeSchema, largeIndex);
    schema.push_back(splicSchema);
}

std::vector<std::string> GetValidSchema(Schema &validSchema, bool hasIndex)
{
    std::vector<std::string> schema;
    for (auto iter1 = validSchema.version.begin(); iter1 != validSchema.version.end(); iter1++) {
        for (auto iter2 = validSchema.mode.begin(); iter2 != validSchema.mode.end(); iter2++) {
            std::string splicSchema;
            if (hasIndex) {
                for (auto iter3 = validSchema.index.begin(); iter3 != validSchema.index.end(); iter3++) {
                    splicSchema = SpliceToSchema(*iter1, *iter2, validSchema.define.at(0), *iter3);
                    schema.push_back(splicSchema);
                }
            } else {
                for (auto iter3 = validSchema.define.begin(); iter3 != validSchema.define.end(); iter3++) {
                    splicSchema = SpliceToSchema(*iter1, *iter2, *iter3, validSchema.index.at(0));
                    schema.push_back(splicSchema);
                }
            }
        }
    }
    GenerateLongValidSchema(validSchema, schema);
    GenerateLargeValidSchema(validSchema, schema);
    std::string schemaWithoutIndex;
    schemaWithoutIndex = schemaWithoutIndex + "{" + "\"SCHEMA_VERSION\"" + ":" + "\"" + validSchema.version.at(0) +
        "\"" + "," + "\"SCHEMA_MODE\"" + ":" + "\"" + validSchema.mode.at(0) + "\"" + "," +
        "\"SCHEMA_DEFINE\"" + ":" + validSchema.define.at(0) + "}";
    schema.push_back(schemaWithoutIndex);
    MST_LOG("The number of valid schema is %zd", schema.size());
    return schema;
}

void GetLongIndex(Schema &validSchema, std::vector<std::string> &schema)
{
    std::string validLargeSchema, largeIndexRes, largeIndex, splicSchema;
    LongDefine param;
    param.recordNum = FIFTY_RECORDS;
    param.recordSize = KEY_SIX_BYTE;
    param.prefix = 'k';
    GetLongSchemaDefine(param, validLargeSchema);
    for (int index = 0; index <= KEY_THIRTYTWO_BYTE; index++) {
        largeIndexRes = largeIndexRes + "\"$.field" + std::to_string(index) + "\",";
    }
    largeIndexRes.erase(largeIndexRes.size() - 1, 1);
    largeIndex = largeIndex + "[" + largeIndexRes + "]";
    splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0),
        validLargeSchema, largeIndex);
    schema.push_back(splicSchema);
}
void GenarateOtherInvalidSchema(Schema &validSchema, std::map<int, std::vector<std::string>> &result)
{
    // exist no Metafield or lack of Metafield
    std::string invalidSchema;
    std::vector<std::string> schema;
    invalidSchema = invalidSchema + "{" + "\"SCHEMA_VERSION\"" + ":" + "\"" + validSchema.version.at(0) +
        "\"" + "," + "\"SCHEMA_MODE\"" + ":" + "\"" + validSchema.mode.at(0) + "\"" + "," +
        "\"SCHEMA_DEFINE\"" + ":"  + validSchema.define.at(0) + "," +
        "\"SCHEMA_NOTE\"" + ":" + "[]" + "}";
    schema.push_back(invalidSchema);
    invalidSchema.clear();
    invalidSchema = invalidSchema + "{" + "\"SCHEMA_VERSION\"" + ":" + "\"" + validSchema.version.at(0) +
        "\"" + "," + "\"SCHEMA_MODE\"" + ":" + "\"" + validSchema.mode.at(0) + "\"" + "}";
    schema.push_back(invalidSchema);

    // the schema is invalid that is a Json object
    invalidSchema.clear();
    invalidSchema = invalidSchema + "[" + "\"SCHEMA_VERSION\"" + ":" + "\"" + validSchema.version.at(0) +
        "\"" + "," + "\"SCHEMA_MODE\"" + ":" + "\"" + validSchema.mode.at(0) + "\"" + "]";
    schema.push_back(invalidSchema);

    // if the schema is \",nullptr,space,tab or enter or other not match Json, it is invalid
    schema.push_back("\"");
    schema.push_back(" ");
    schema.push_back("\t");
    schema.push_back("\r");
    invalidSchema.clear();
    invalidSchema = invalidSchema + "{" + "\"SCHEMA_VERSION\"" + ":" + "\"" + validSchema.version.at(0) +
        "\"" + "," + "\"SCHEMA_MODE\"" + ":" + "\"" + validSchema.mode.at(0) + "\"" + "," +
        "\"SCHEMA_DEFINE\"" + ":"  + validSchema.define.at(0) + "," + "}";
    schema.push_back(invalidSchema);

    // if the schema's size is over 512K, it is invalid
    invalidSchema.clear();
    LongDefine param;
    param.recordNum = TWO_FIVE_SIX_RECORDS;
    param.recordSize = TWO_K_LONG_STRING;
    param.prefix = 'k';
    GetLongSchemaDefine(param, invalidSchema);
    std::string splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0),
        invalidSchema, validSchema.index.at(0));
    schema.push_back(splicSchema);

    // if the num of index is over 32, it is invalid
    GetLongIndex(validSchema, schema);
    result[4] = schema; // this invalid scenes' index is 4.
}

std::map<int, std::vector<std::string>> GetInvalidSchema(Schema &invalidSchema, Schema &validSchema, bool hasIndex)
{
    std::map<int, std::vector<std::string>> result;
    std::vector<std::string> schema;
    std::string splicSchema;
    for (auto iter = invalidSchema.version.begin(); iter != invalidSchema.version.end(); iter++) {
        splicSchema = SpliceToSchema(*iter, validSchema.mode.at(0), validSchema.define.at(0), validSchema.index.at(0));
        schema.push_back(splicSchema);
    }
    result[0] = schema;
    schema.clear();
    for (auto iter = invalidSchema.mode.begin(); iter != invalidSchema.mode.end(); iter++) {
        splicSchema = SpliceToSchema(validSchema.version.at(0), *iter, validSchema.define.at(0),
            validSchema.index.at(0));
        schema.push_back(splicSchema);
    }
    result[1] = schema;
    schema.clear();
    if (hasIndex) {
        for (auto iter = invalidSchema.index.begin(); iter != invalidSchema.index.end(); iter++) {
            splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0),
                validSchema.define.at(0), *iter);
            schema.push_back(splicSchema);
        }
        result[3] = schema; // 3 is the invalid SCHEMA_INDEX.
    } else {
        for (auto iter = invalidSchema.define.begin(); iter != invalidSchema.define.end(); iter++) {
            splicSchema = SpliceToSchema(validSchema.version.at(0), validSchema.mode.at(0), *iter, "[]");
            schema.push_back(splicSchema);
        }
        result[2] = schema; // 2 is the invalid SCHEMA_DEFINE.
    }
    GenarateOtherInvalidSchema(validSchema, result);
    MST_LOG("The number of invalid schema is %zd", result[0].size() + result[1].size() +
        result[2].size() + result[3].size() + result[4].size()); // 1, 2, 3, 4 are the index of invalid field.
    return result;
}
} // end of namespace DistributedDBDataGenerator