/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_SERVICE_DRIVER_CODE_EMITTER_H
#define OHOS_HDI_CPP_SERVICE_DRIVER_CODE_EMITTER_H

#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppServiceDriverCodeEmitter : public CppCodeEmitter {
public:
    CppServiceDriverCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory);

    virtual ~CppServiceDriverCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitDriverSourceFile();

    void EmitDriverIncluions(StringBuilder& sb);

    void EmitDriverUsings(StringBuilder& sb);

    void EmitDriverServiceDecl(StringBuilder& sb);

    void EmitDriverDispatch(StringBuilder& sb);

    void EmitDriverInit(StringBuilder& sb);

    void EmitDriverBind(StringBuilder& sb);

    void EmitDriverRelease(StringBuilder& sb);

    void EmitDriverEntryDefinition(StringBuilder& sb);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_SERVICE_DRIVER_CODE_EMITTER_H