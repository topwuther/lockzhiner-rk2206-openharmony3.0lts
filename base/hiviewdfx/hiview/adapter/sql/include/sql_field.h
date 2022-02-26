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
#ifndef SQL_FIELD_H
#define SQL_FIELD_H

#include "sql_common.h"

namespace sql {
using std::string;
class SqlField {
public:
    SqlField(string name, FieldType type, FieldFlag isPrimaryKey, FieldFlag isNull);
    SqlField(string name, FieldType type);
    SqlField(const SqlField& value);
    SqlField(FieldUse isEndingField);
    virtual ~SqlField(void){};

public:
    string GetName() const;
    size_t GetIndex();
    void setIndex(size_t index);
    FieldType GetType();
    string GetTypeStr();
    bool IsPrimaryKey();
    bool IsNull();
    string GetDefinition();
    bool IsEndingField();

private:
    string name;
    FieldType type;
    bool isPrimaryKey;
    bool isNull;
    size_t index;
    bool isEndingField;
};
}  // namespace sql

#endif
