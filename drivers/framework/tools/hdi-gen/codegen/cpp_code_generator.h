/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_CODE_GENERATOR_H
#define OHOS_HDI_CPP_CODE_GENERATOR_H

#include "codegen/code_generator.h"
#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppCodeGenerator : public CodeGenerator {
public:
    CppCodeGenerator() : CodeGenerator(),
        emitters_() {}

    ~CppCodeGenerator() override {};

    bool Initializate(const AutoPtr<AST>& ast, const String& targetDirectory) override;
    bool Generate() const override;
private:
    bool ResolveDirectory() override;

    static const char* TAG;
    std::vector<AutoPtr<CppCodeEmitter>> emitters_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_CODE_GENERATOR_H