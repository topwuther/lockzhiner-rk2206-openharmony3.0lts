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
#include "sql_common.h"

#include <securec.h>

#include <sstream>
#include <string>

#include "sql_wrapper.h"

using namespace std;

namespace {
const int INT_TO_STR_BUFF = 32;
const int INTEGER_TO_STR_BUFF = 64;
}  // namespace

namespace sql {
string IntToStr(int value)
{
    char buffer[INT_TO_STR_BUFF];
    (void)sprintf_s(buffer, sizeof(buffer), "%d", value);
    return buffer;
}

string IntegerToStr(Integer value)
{
    char buffer[INTEGER_TO_STR_BUFF];
    (void)sprintf_s(buffer, sizeof(buffer), "%lld", value);
    return buffer;
}

string QuoteStr(string value)
{
    std::ostringstream str;
    for (string::iterator iter = value.begin(); iter != value.end(); ++iter) {
        if (*iter == '\'') {
            str << *iter;
        }
        str << *iter;
    }
    return str.str();
}

string& TrimLeft(string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    return str;
}

string& TrimRight(string& str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(),
        str.end());
    return str;
}

string& Trim(string& str)
{
    TrimLeft(str);
    TrimRight(str);
    return str;
}

string Trim(const string& str)
{
    string trimStr = str;
    return Trim(trimStr);
}

char* Sqlite3Snprintf(int n, char *buf, const char *format, ...)
{
    char *str = nullptr;
    va_list ap;
    va_start(ap, format);
    str = sqlite3_vsnprintf(n, buf, format, ap);
    va_end(ap);
    return str;
}
};  // namespace sql
