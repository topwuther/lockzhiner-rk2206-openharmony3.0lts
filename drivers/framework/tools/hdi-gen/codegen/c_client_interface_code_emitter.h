/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_CLIENT_INTERFACE_CODE_EMITTER_H
#define OHOS_HDI_C_CLIENT_INTERFACE_CODE_EMITTER_H

#include "codegen/c_code_emitter.h"

namespace OHOS {
namespace HDI {
class CClientInterfaceCodeEmitter : public CCodeEmitter {
public:
    CClientInterfaceCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
        : CCodeEmitter(ast, targetDirectory) {}

    virtual ~CClientInterfaceCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitInterfaceHeaderFile();

    void EmitImportInclusions(StringBuilder& sb);

    void EmitForwardDecls(StringBuilder& sb);

    void EmitInterfaceDecl(StringBuilder& sb);

    void EmitInterfaceMethodsDecl(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitInterfaceInstanceMethod(StringBuilder& sb);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_CLIENT_INTERFACE_CODE_EMITTER_H
