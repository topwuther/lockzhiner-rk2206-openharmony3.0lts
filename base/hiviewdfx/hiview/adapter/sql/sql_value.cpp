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
#include "sql_value.h"

#include <securec.h>

using namespace std;

namespace {
const int MAX_BUFFER_LEN = 128;
}

namespace sql {
SqlValue::SqlValue()
{
    SetValue("", TYPE_UNDEFINED);
}

SqlValue::SqlValue(const string& value, FieldType type)
{
    SetValue(value, type);
}

SqlValue::SqlValue(const SqlValue& value)
{
    this->value = value.value;
    this->isNull = value.isNull;
    this->type = value.type;
}

SqlValue& SqlValue::operator=(const SqlValue& value)
{
    if (&value != this) {
        this->value = value.value;
        this->isNull = value.isNull;
        this->type = value.type;
    }
    return *this;
}

void SqlValue::SetValue(const string& value, FieldType type)
{
    isNull = true;
    this->value.clear();
    this->type = type;

    if (!value.empty()) {
        isNull = false;
        this->value = value;
        this->type = type;
    }
}

string SqlValue::ToSql(FieldType type)
{
    if (IsNull()) {
        return "null";
    }

    if (type == TYPE_TEXT) {
        return "'" + QuoteStr(AsString()) + "'";
    }

    return AsString();
}

string SqlValue::AsString()
{
    return value;
}

Integer SqlValue::AsInteger()
{
    if (IsNull()) {
        return 0;
    }

    long long tempValue = 0;
    if (!sscanf_s(value.c_str(), "%lld", &tempValue)) {
        return 0;
    }
    return tempValue;
}

double SqlValue::AsDouble()
{
    if (IsNull()) {
        return 0.0;
    }

    double tempValue = 0;
    if (!sscanf_s(value.c_str(), "%lf", &tempValue)) {
        return 0;
    }
    return tempValue;
}

void SqlValue::SetNull()
{
    isNull = true;
    value.clear();
}

void SqlValue::SetString(string tempValue)
{
    isNull = false;
    value = tempValue;
}

void SqlValue::SetInteger(Integer tempValue)
{
    char buffer[MAX_BUFFER_LEN];
    (void)sprintf_s(buffer, sizeof(buffer), "%lld", tempValue);
    isNull = false;
    value = buffer;
}

void SqlValue::SetDouble(double tempValue)
{
    char buffer[MAX_BUFFER_LEN];
    (void)sprintf_s(buffer, sizeof(buffer), "%0.8f", tempValue);
    isNull = false;
    value = buffer;
}

bool SqlValue::IsNull()
{
    return isNull;
}
};  // namespace sql
