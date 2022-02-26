/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "token.h"
#include <iomanip>
#include <map>

using namespace OHOS::Hardware;

std::string OHOS::Hardware::TokenType2String(int32_t type)
{
    static std::map<int32_t, std::string> tokenTypeMap = {
            {NUMBER,    "NUMBER"},
            {TEMPLATE,  "TEMPLATE"},
            {LITERAL,   "LITERAL"},
            {ROOT,      "ROOT"},
            {INCLUDE,   "INCLUDE"},
            {DELETE,    "DELETE"},
            {STRING,    "STRING"},
            {REF_PATH,  "REF_PATH"},
            {FILE_PATH, "FILE_PATH"}
    };

    std::string str;
    if (type < '~') {
        str.push_back(static_cast<char>(type));
        return str;
    } else if (tokenTypeMap.find(type) != tokenTypeMap.end()) {
        str = tokenTypeMap[type];
    }

    return str;
}

std::ostream &OHOS::Hardware::operator<<(std::ostream &stream, const OHOS::Hardware::Token &t)
{
    stream << "Token: type: " << std::setw(8) << ::std::left << TokenType2String(t.type).data();
    stream << " value: " << std::setw(8) << ::std::left;
    t.type != NUMBER ? stream << std::setw(20) << t.strval.data()
                     : stream << std::setw(0) << "0x" << std::setw(18) << std::hex << t.numval;
    stream << " lineno:" << t.lineNo;
    return stream;
}

bool Token::operator==(int32_t t) const
{
    return t == type;
}

bool Token::operator!=(int32_t t) const
{
    return t != type;
}
bool Token::operator==(Token &t) const
{
    return t.type == type && t.numval == numval && t.strval == strval;
}

bool Token::operator!=(Token &t) const
{
    return t.type != type || t.numval != numval || t.strval != strval;
}


