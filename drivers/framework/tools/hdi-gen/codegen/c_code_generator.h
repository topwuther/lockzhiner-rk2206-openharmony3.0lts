/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_CODE_GENERATOR_H
#define OHOS_HDI_C_CODE_GENERATOR_H

#include "codegen/c_code_emitter.h"
#include "codegen/code_generator.h"

namespace OHOS {
namespace HDI {
class CCodeGenerator : public CodeGenerator {
public:
    CCodeGenerator() : CodeGenerator(),
        emitters_() {}

    virtual ~CCodeGenerator() = default;

    bool Initializate(const AutoPtr<AST>& ast, const String& targetDirectory) override;
    bool Generate() const override;
private:
    bool ResolveDirectory() override;

    static const char* TAG;
    std::vector<AutoPtr<CCodeEmitter>> emitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_CODE_GENERATOR_H