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

#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>

#include "variant_value.h"

#include "sqlite3sym.h"

namespace OHOS {
namespace Security {
namespace Permission {
class Statement final {
public:
    enum State { BUSY, ROW, DONE, MISUSE, UNKNOWN };

    Statement(sqlite3* db, const std::string& sql);
    virtual ~Statement();

    void Bind(const int index, const std::string& text);
    void Bind(const int index, int value);
    void Bind(const std::string& tableColumnName, const VariantValue& value);

    State Step();
    int Reset();

    std::string GetColumnString(const int column) const;
    int GetColumnInt(const int column) const;
    std::string GetColumnName(const int column) const;
    int GetParameterIndex(const std::string& name) const;
    int GetColumnCount() const;
    VariantValue GetValue(const int column) const;

private:
    sqlite3* db_;
    sqlite3_stmt* statement_;
    const std::string sql_;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // STATEMENT_H
