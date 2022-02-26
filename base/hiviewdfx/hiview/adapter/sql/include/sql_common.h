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
#ifndef SQL_COMMON_H
#define SQL_COMMON_H

#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <ctime>

namespace sql {
using std::string;
enum FieldType {
    TYPE_UNDEFINED,
    TYPE_INT,
    TYPE_TEXT,
    TYPE_FLOAT,
};

enum FieldFlag {
    FIELD_NONE = 0,
    FIELD_NOT_NULL = 1,
    FIELD_NULL = 2,
    FIELD_PRIMARY_KEY = 3,
    FIELD_NOT_PRIMARY_KEY = 4,
};

enum FieldUse {
    FIELD_DEFAULT = 0,
    FIELD_END = 1,
};

const int INVALID_INDEX = -1;
using Integer = long long int;

string IntToStr(int value);
string IntegerToStr(Integer value);
string QuoteStr(string value);
string& TrimLeft(string& str);
string& TrimRight(string& str);
string& Trim(string& str);
string Trim(const string& str);

struct Sqlite3Db;
char* Sqlite3Snprintf(int n, char *buf, const char *format, ...);
};  // namespace sql

#endif
