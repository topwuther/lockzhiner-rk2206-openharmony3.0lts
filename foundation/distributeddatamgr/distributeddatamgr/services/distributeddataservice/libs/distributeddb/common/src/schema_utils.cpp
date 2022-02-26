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

#include "schema_utils.h"
#include <map>
#include <cmath>
#include <cfloat>
#include <cctype>
#include "db_errno.h"
#include "log_print.h"

namespace DistributedDB {
namespace
{
    // Currently supported types
    const std::map<std::string, FieldType> FIELD_TYPE_DIC = {
        {KEYWORD_TYPE_BOOL, FieldType::LEAF_FIELD_BOOL},
        {KEYWORD_TYPE_INTEGER, FieldType::LEAF_FIELD_INTEGER},
        {KEYWORD_TYPE_LONG, FieldType::LEAF_FIELD_LONG},
        {KEYWORD_TYPE_DOUBLE, FieldType::LEAF_FIELD_DOUBLE},
        {KEYWORD_TYPE_STRING, FieldType::LEAF_FIELD_STRING},
    };

    bool IsLegalFieldCharacter(char character)
    {
        return (std::isalnum(character) || character == '_');
    }
    void TrimFiled(std::string &inString)
    {
        inString.erase(0, inString.find_first_not_of("\r\t "));
        size_t temp = inString.find_last_not_of("\r\t ");
        if (temp < inString.size()) {
            inString.erase(temp + 1);
        }
    }

    // TYPE, [NOT NULL,] [DEFAULT X]
    // DEFAULT at last
    // State transition matrix
    const int STATE_TRANSFER[8][6] = { // 5 type input and 7 type state
        // blank, NOT NULL, DEFAULT, OTHER AlNUM, COMMA
        {0, -1, -1, 1, -1},     // state 0: empty
        {1, -1, -1, 1, 2},      // state 1: only type
        {2, 3, 5, -1, -1},      // state 2: alnum ,
        {3, -1, -1, -1, 4},     // state 3: alnum , notnull
        {4, -1, 5, -1, -1},     // state 4: alnum , notnull ,
        {6, -1, -1, -1, -1},    // state 5: finish with DEFAULT
        {6, -1, -1, 7, -1},     // state 6: finish with DEFAULT and blank
        {7, 7, 7, 7, 7},        // state 7: finish with DEFAULT and blank and no matter what value
    };
    enum StateTransferColNum {
        COLUMN_ILLEGAL = -1,
        COLUMN_BLANK,
        COLUMN_NOT_NULL,
        COLUMN_DEFAULT,
        COLUMN_OTHER_ALNUM,
        COLUMN_COMMA,
    };
} // namespace

// compare function can make sure not to cross the border, pos < oriContent.size() - 1
// Get symbol type and Converts to the corresponding column of the state transition matrix
int SchemaUtils::MakeTrans(const std::string &oriContent, size_t &pos)
{
    if (isspace(oriContent[pos])) {
        return COLUMN_BLANK;
    } else if (oriContent.compare(pos, KEYWORD_ATTR_NOT_NULL.size(), KEYWORD_ATTR_NOT_NULL) == 0) {
        pos = pos + KEYWORD_ATTR_NOT_NULL.size() - 1;
        return COLUMN_NOT_NULL;
    } else if (oriContent.compare(pos, KEYWORD_ATTR_DEFAULT.size(), KEYWORD_ATTR_DEFAULT) == 0) {
        pos = pos + KEYWORD_ATTR_DEFAULT.size() - 1;
        return COLUMN_DEFAULT;
    } else if (std::isalnum(oriContent[pos]) || oriContent[pos] == '\'' ||
        oriContent[pos] == '+' || oriContent[pos] == '-') {
        return COLUMN_OTHER_ALNUM;
    } else if (oriContent[pos] == ',') {
        return COLUMN_COMMA;
    } else {
        return COLUMN_ILLEGAL;
    }
}

// Use DFA to check and Parsing
// You can get the corresponding state meaning in the state transition matrix STATE_TRANSFER
int SchemaUtils::SplitSchemaAttribute(const std::string &inAttrString, std::vector<std::string> &outAttrString)
{
    int state = 0;
    outAttrString.resize(3); // attribute have 3 type keywords
    for (size_t i = 0; i < inAttrString.size(); i++) {
        int id = MakeTrans(inAttrString, i);
        if (id < 0) {
            LOGD("Split Schema Attribute err, Contains unrecognized content [%c]", inAttrString[i]);
            return -E_SCHEMA_PARSE_FAIL;
        }
        state = STATE_TRANSFER[state][id];
        if (state == 1) { // state 1 :Indicates that only type information is currently available
            outAttrString[0].push_back(inAttrString[i]);
        } else if (state == 3) { // state 3 :Gets the NOT_NULL keyword
            outAttrString[1] = KEYWORD_ATTR_NOT_NULL;
        } else if (state == 7) { // state 7 :Contains complete information
            // Get default string. Now transfer matrix can ensure > 1, but you should pay attention when fix it
            outAttrString[2] = inAttrString.substr(i - 1);
            break;
        } else if (state < 0) {
            LOGD("Split Schema Attribute err, err state [%d]", state);
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    // Only these states are legal, The meaning of the state can be seen in the matrix STATE_TRANSFER explanation
    if (!(state == 1 || state == 3 || state == 7)) {
        LOGD("Split Schema Attribute err, err state [%d]", state);
        return -E_SCHEMA_PARSE_FAIL;
    }
    return E_OK;
}

int SchemaUtils::TransToBool(const std::string &defaultContent, SchemaAttribute &outAttr)
{
    // Have been trim
    if (defaultContent.compare(KEYWORD_ATTR_VALUE_TRUE) == 0) {
        outAttr.defaultValue.boolValue = true;
        return E_OK;
    } else if (defaultContent.compare(KEYWORD_ATTR_VALUE_FALSE) == 0) {
        outAttr.defaultValue.boolValue = false;
        return E_OK;
    }
    LOGE("Default value can not transform to bool!!");
    return -E_SCHEMA_PARSE_FAIL;
}

int SchemaUtils::TransToString(const std::string &defaultContent, SchemaAttribute &outAttr)
{
    // Have been trim, Strip leading and trailing '
    if (defaultContent.size() > 1 && defaultContent.front() == '\'' && defaultContent.back() == '\'') {
        outAttr.defaultValue.stringValue = defaultContent.substr(1, defaultContent.size() - 2);
        if (outAttr.defaultValue.stringValue.size() > SCHEMA_DEFAULT_STRING_SIZE_LIMIT) {
            return -E_SCHEMA_PARSE_FAIL;
        }
        return E_OK;
    }
    LOGE("Substandard format! Default value can not transform to string!!");
    return -E_SCHEMA_PARSE_FAIL;
}

int SchemaUtils::TransToInteger(const std::string &defaultContent, SchemaAttribute &outAttr)
{
    // defaultContent can not be null
    if (defaultContent.empty()) {
        return -E_SCHEMA_PARSE_FAIL;
    }
    int transRes = std::atoi(defaultContent.c_str());
    std::string resReview = std::to_string(transRes);
    if (defaultContent.compare(defaultContent.find_first_not_of("+- "), defaultContent.size(),
        resReview, resReview.find_first_not_of("+- "), resReview.size()) == 0) {
        // Check the sign of the number
        if ((defaultContent[0] == '-' && resReview[0] == '-') ||
            (defaultContent[0] != '-' && resReview[0] != '-') ||
            transRes == 0) {
            outAttr.defaultValue.integerValue = transRes;
            return E_OK;
        }
    }
    LOGE("Default value can not transform to Integer!!");
    return -E_SCHEMA_PARSE_FAIL;
}

int SchemaUtils::TransToLong(const std::string &defaultContent, SchemaAttribute &outAttr)
{
    // defaultContent can not be null
    if (defaultContent.empty()) {
        return -E_SCHEMA_PARSE_FAIL;
    }
    int64_t transRes = std::atoll(defaultContent.c_str());
    std::string resReview = std::to_string(transRes);
    if (defaultContent.compare(defaultContent.find_first_not_of("+- "), defaultContent.size(),
        resReview, resReview.find_first_not_of("+- "), resReview.size()) == 0) {
        // Check the sign of the number
        if ((defaultContent[0] == '-' && resReview[0] == '-') ||
            (defaultContent[0] != '-' && resReview[0] != '-') ||
            transRes == 0) {
            outAttr.defaultValue.longValue = transRes;
            return E_OK;
        }
    }

    LOGE("Default value[%s] can not transform to LONG!!", resReview.c_str());
    return -E_SCHEMA_PARSE_FAIL;
}

int SchemaUtils::TransToDouble(const std::string &defaultContent, SchemaAttribute &outAttr)
{
    // defaultContent can not be null
    if (defaultContent.empty()) {
        return -E_SCHEMA_PARSE_FAIL;
    }

    // Disable scientific notation
    int dotCount = 0;
    for (const auto &iter : defaultContent) {
        if (!(std::isdigit(iter) || iter == '.' || iter == '-' || iter == '+')) {
            LOGE("Default value to double, exist invalid symbol[%c]", iter);
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (iter == '.') {
            dotCount++;
        }
        if (dotCount > 1) {
            LOGE("Default value to double, exist invalid extra dot");
            return -E_SCHEMA_PARSE_FAIL;
        }
    }

    char *end = nullptr;
    double transRes = std::strtod(defaultContent.c_str(), &end);
    // Double exist problems with accuracy， overflow is subject to the legality of the c++ conversion.
    if (transRes > -HUGE_VAL && transRes < HUGE_VAL && std::isfinite(transRes)) {
        // Cleared blank
        if (end != &defaultContent.back() + 1) {
            LOGD("Termination of parsing due to exception symbol");
            return -E_SCHEMA_PARSE_FAIL;
        }
        outAttr.defaultValue.doubleValue = transRes;
        return E_OK;
    }
    LOGE("Default value can not transform to double, overflow double max!");
    return -E_SCHEMA_PARSE_FAIL;
}

int SchemaUtils::TransformDefaultValue(std::string &defaultContent, SchemaAttribute &outAttr)
{
    TrimFiled(defaultContent);
    if (defaultContent.compare(KEYWORD_ATTR_VALUE_NULL) == 0 && outAttr.hasNotNullConstraint) {
        LOGE("NOT NULL and DEFAULT null Simultaneously");
        return -E_SCHEMA_PARSE_FAIL;
    } else if (defaultContent.compare(KEYWORD_ATTR_VALUE_NULL) == 0) {
        outAttr.hasDefaultValue = false;
        return E_OK;
    }

    int errCode = E_OK;
    switch (outAttr.type) {
        case FieldType::LEAF_FIELD_BOOL:
            errCode = TransToBool(defaultContent, outAttr);
            break;
        case FieldType::LEAF_FIELD_INTEGER:
            errCode = TransToInteger(defaultContent, outAttr);
            break;
        case FieldType::LEAF_FIELD_LONG:
            errCode = TransToLong(defaultContent, outAttr);
            break;
        case FieldType::LEAF_FIELD_DOUBLE:
            errCode = TransToDouble(defaultContent, outAttr);
            break;
        case FieldType::LEAF_FIELD_STRING:
            errCode = TransToString(defaultContent, outAttr);
            break;
        default:
            LOGE("Unrecognized or unsupported type, please check!!");
            errCode = -E_SCHEMA_PARSE_FAIL;
            break;
    }

    LOGD("SchemaAttribute type is [%d], transfer result is [%d]", outAttr.type, errCode);
    return errCode;
}

int SchemaUtils::ParseAndCheckSchemaAttribute(const std::string &inAttrString, SchemaAttribute &outAttr)
{
    if (inAttrString.empty()) {
        return -E_SCHEMA_PARSE_FAIL;
    }
    std::string tempinAttrString = inAttrString;
    TrimFiled(tempinAttrString);

    std::vector<std::string> attrContext;
    int errCode = SplitSchemaAttribute(inAttrString, attrContext);
    if (errCode != E_OK) {
        LOGD("Syntax error, please check!");
        return errCode;
    }
    errCode = ParseSchemaAttribute(attrContext, outAttr);
    if (errCode != E_OK) {
        LOGD("Grammatical error, please check!");
        return errCode;
    }

    return E_OK;
}

int SchemaUtils::ParseSchemaAttribute(std::vector<std::string> &attrContext, SchemaAttribute &outAttr)
{
    // After split attribute? attrContext include 3 type field
    if (attrContext.size() < 3) {
        LOGE("No parsing preprocessing!!");
        return -E_SCHEMA_PARSE_FAIL;
    }
    TrimFiled(attrContext[0]);
    if (FIELD_TYPE_DIC.find(attrContext[0]) == FIELD_TYPE_DIC.end()) {
        LOGE("Errno schema field type [%s]!!", attrContext[0].c_str());
        return -E_SCHEMA_PARSE_FAIL;
    } else {
        outAttr.type = FIELD_TYPE_DIC.at(attrContext[0]);
    }

    outAttr.hasNotNullConstraint = !attrContext[1].empty();

    // if DEFAULT value context exist, fix hasDefaultValue flag, 2nd represents the default value
    if (attrContext[2].empty()) {
        outAttr.hasDefaultValue = false;
    } else {
        outAttr.hasDefaultValue = true;
        int errCode = TransformDefaultValue(attrContext[2], outAttr); // 2nd element is DEFAULT value
        if (errCode != E_OK) {
            LOGE("Default value is malformed!!");
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    return E_OK;
}

namespace {
// Check prefix and attempt to find any illegal, returns E_OK if nothing illegal and an hasPrefix indicator.
int CheckDollarDotPrefix(const std::string &inPathStr, bool &hasPrefix)
{
    if (inPathStr.empty()) {
        return -E_SCHEMA_PARSE_FAIL;
    }
    if (inPathStr.size() >= std::string("$.").size()) {
        // In this case, $. prefix may exist, but also may not exist.
        if (inPathStr[0] == '$' && inPathStr[1] == '.') { // 1 for second char
            // $. prefix may exist
            hasPrefix = true;
            return E_OK;
        }
        if (inPathStr[0] == '$' && inPathStr[1] != '.') { // 1 for second char
            return -E_SCHEMA_PARSE_FAIL;
        }
        if (inPathStr[1] == '$') { // 1 for second char
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    // here, inPathStr not empty, has at least one char, should not begin with '.'
    if (inPathStr[0] == '.') {
        return -E_SCHEMA_PARSE_FAIL;
    }
    hasPrefix = false;
    return E_OK;
}
}

int SchemaUtils::ParseAndCheckFieldPath(const std::string &inPathString, FieldPath &outPath)
{
    std::string tempInPathString = inPathString;
    TrimFiled(tempInPathString);
    bool hasPrefix = false;
    int errCode = CheckDollarDotPrefix(tempInPathString, hasPrefix);
    if (errCode != E_OK) {
        LOGE("CheckDollarDotPrefix Fail.");
        return errCode;
    }
    if (!hasPrefix) {
        tempInPathString = std::string("$.") + tempInPathString;
    }

    for (size_t curPos = 1; curPos < tempInPathString.size();) {
        if (curPos + 1 == tempInPathString.size()) {
            LOGE("Dot at end will generate empty illegal path!");
            return -E_SCHEMA_PARSE_FAIL;
        }
        size_t nextPointPos = tempInPathString.find_first_of(".", curPos + 1);
        outPath.push_back(tempInPathString.substr(curPos + 1, nextPointPos - curPos - 1));
        curPos = nextPointPos;
    }

    if (outPath.size() > SCHEMA_FEILD_PATH_DEPTH_MAX) {
        LOGE("Parse Schema Index  depth illegality!");
        return -E_SCHEMA_PARSE_FAIL;
    }

    for (const auto &iter : outPath) {
        if (CheckFieldName(iter) != E_OK) {
            LOGE("Parse Schema Index field illegality!");
            return -E_SCHEMA_PARSE_FAIL;
        }
    }
    return E_OK;
}

int SchemaUtils::CheckFieldName(const FieldName &inName)
{
    if (inName.empty() || inName.size() > SCHEMA_FEILD_NAME_LENGTH_MAX) {
        LOGE("Schema FieldName have invalid size!");
        return -E_SCHEMA_PARSE_FAIL;
    }

    // The first letter must be a number or an underscore
    if (!(std::isalpha(inName[0]) || inName[0] == '_')) {
        LOGE("Schema FieldName begin with un support symbol!");
        return -E_SCHEMA_PARSE_FAIL;
    }

    // Must consist of numeric underscore letters
    for (const auto &iter : inName) {
        if (!(IsLegalFieldCharacter(iter))) {
            LOGE("Schema FieldName exist un support symbol!");
            return -E_SCHEMA_PARSE_FAIL;
        }
    }

    return E_OK;
}

std::string SchemaUtils::Strip(const std::string &inString)
{
    std::string stripRes = inString;
    TrimFiled(stripRes);
    return stripRes;
}

std::string SchemaUtils::StripNameSpace(const std::string &inFullName)
{
    auto pos = inFullName.find_last_of('.');
    if (pos == std::string::npos) { // No '.', so no namespace
        return inFullName;
    }
    return inFullName.substr(pos + 1);
}

std::string SchemaUtils::FieldTypeString(FieldType inType)
{
    static std::map<FieldType, std::string> fieldTypeMapString = {
        {FieldType::LEAF_FIELD_NULL, "NULL"},
        {FieldType::LEAF_FIELD_BOOL, "BOOL"},
        {FieldType::LEAF_FIELD_INTEGER, "INTEGER"},
        {FieldType::LEAF_FIELD_LONG, "LONG"},
        {FieldType::LEAF_FIELD_DOUBLE, "DOUBLE"},
        {FieldType::LEAF_FIELD_STRING, "STRING"},
        {FieldType::LEAF_FIELD_ARRAY, "ARRAY"},
        {FieldType::LEAF_FIELD_OBJECT, "LEAF_OBJECT"},
        {FieldType::INTERNAL_FIELD_OBJECT, "INTERNAL_OBJECT"},
    };
    return fieldTypeMapString[inType];
}

std::string SchemaUtils::SchemaTypeString(SchemaType inType)
{
    static std::map<SchemaType, std::string> schemaTypeMapString {
        {SchemaType::NONE, "NONE"},
        {SchemaType::JSON, "JSON-SCHEMA"},
        {SchemaType::FLATBUFFER, "FLATBUFFER-SCHEMA"},
        {SchemaType::UNRECOGNIZED, "UNRECOGNIZED"},
    };
    return schemaTypeMapString[inType];
}

std::string SchemaUtils::FieldPathString(const FieldPath &inPath)
{
    std::string outString = "$";
    for (const auto &entry : inPath) {
        outString += ".";
        outString += entry;
    }
    return outString;
}
} // namespace DistributedDB
