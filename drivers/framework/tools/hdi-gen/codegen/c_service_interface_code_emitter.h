/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_SERVICE_INTERFACE_CODEE_MITTER_H
#define OHOS_HDI_C_SERVICE_INTERFACE_CODEE_MITTER_H

#include "codegen/c_code_emitter.h"

namespace OHOS {
namespace HDI {
class CServiceInterfaceCodeEmitter : public CCodeEmitter {
public:
    CServiceInterfaceCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
        : CCodeEmitter(ast, targetDirectory) {}

    virtual ~CServiceInterfaceCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitInterfaceHeadrFile();

    void EmitImportInclusions(StringBuilder& sb);

    void EmitInterfaceDataDecls(StringBuilder& sb);

    void EmitInterfaceDefinition(StringBuilder& sb);

    void EmitInterfaceMethods(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitInterfaceInstanceMethodDecl(StringBuilder& sb);

    void EmitInterfaceReleaseMethodDecl(StringBuilder& sb);

    void EmitInterfaceRequestMethodDecl(StringBuilder& sb);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_SERVICE_INTERFACE_CODEE_MITTER_H