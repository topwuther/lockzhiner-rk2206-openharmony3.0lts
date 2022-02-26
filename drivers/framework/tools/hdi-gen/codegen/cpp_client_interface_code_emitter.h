/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_CLIENT_INTERFACE_CODE_EMITTER_H
#define OHOS_HDI_CPP_CLIENT_INTERFACE_CODE_EMITTER_H

#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppClientInterfaceCodeEmitter : public CppCodeEmitter {
public:
    CppClientInterfaceCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
        : CppCodeEmitter(ast, targetDirectory) {}

    virtual ~CppClientInterfaceCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitInterfaceHeaderFile();

    void EmitClientInfHeadMacro(StringBuilder& sb);

    void EmitClientInfTailMacro(StringBuilder& sb);

    void EmitInterfaceInclusions(StringBuilder& sb);

    void EmitInterfaceStdlibInclusions(StringBuilder& sb);

    void EmitInterfaceDBinderInclusions(StringBuilder& sb);

    void EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb);

    void EmitInterfaceDecl(StringBuilder& sb);

    void EmitInterfaceBody(StringBuilder& sb, const String& prefix);

    void EmitInterfaceDestruction(StringBuilder& sb, const String& prefix);

    void EmitInterfaceGetMethodDecl(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodsDecl(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_CLIENT_INTERFACE_CODE_EMITTER_H