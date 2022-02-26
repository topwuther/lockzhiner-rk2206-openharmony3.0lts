/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CODEGENERATOR_H
#define OHOS_HDI_CODEGENERATOR_H

#include "ast/ast.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/string.h"

namespace OHOS {
namespace HDI {
class CodeGenerator : public LightRefCountBase {
public:
    CodeGenerator() : LightRefCountBase(),
        targetDirectory_(),
        ast_(nullptr) {}

    virtual ~CodeGenerator() = default;

    virtual bool Initializate(const AutoPtr<AST>& ast, const String& targetDirectory) = 0;

    virtual bool Generate() const = 0;
protected:
    virtual bool ResolveDirectory() = 0;

    String targetDirectory_;
    AutoPtr<AST> ast_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CODEGENERATOR_H