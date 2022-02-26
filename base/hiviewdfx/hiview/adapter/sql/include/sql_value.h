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
#ifndef SQL_VALUE_H
#define SQL_VALUE_H

#include "sql_common.h"

namespace sql {
using std::string;
class SqlValue {
public:
    SqlValue();
    SqlValue(const string& value, FieldType type);
    SqlValue(const SqlValue& value);
    SqlValue& operator=(const SqlValue& value);
    string ToSql(FieldType type);
    virtual ~SqlValue(){};

public:
    string AsString();
    Integer AsInteger();
    double AsDouble();

public:
    void SetNull();
    void SetString(string value);
    void SetInteger(Integer value);
    void SetDouble(double value);

public:
    bool IsNull();
    void SetValue(const string& value, FieldType type);
    static SqlValue sql;
private:
    string value;
    bool isNull;
    FieldType type;
};
};  // namespace sql

#endif
