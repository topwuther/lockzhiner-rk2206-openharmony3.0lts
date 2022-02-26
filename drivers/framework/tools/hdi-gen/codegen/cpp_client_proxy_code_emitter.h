/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_CLIENT_PROXY_CODE_EMITTER_H
#define OHOS_HDI_CPP_CLIENT_PROXY_CODE_EMITTER_H

#include "codegen/cpp_code_emitter.h"

namespace OHOS {
namespace HDI {
class CppClientProxyCodeEmitter : public CppCodeEmitter {
public:
    CppClientProxyCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory);

    virtual ~CppClientProxyCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitProxyHeaderFile();

    void EmitProxyHeadrInclusions(StringBuilder& sb);

    void EmitProxyDecl(StringBuilder& sb, const String& prefix);

    void EmitProxyConstructor(StringBuilder& sb, const String& prefix);

    void EmitProxyMethodDecls(StringBuilder& sb, const String& prefix);

    void EmitProxyMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitProxyConstants(StringBuilder& sb, const String& prefix);

    void EmitProxyMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix);

    void EmitProxySourceFile();

    void EmitProxySourceInclusions(StringBuilder& sb);

    void EmitProxySourceStdlibInclusions(StringBuilder& sb);

    void EmitGetMethodImpl(StringBuilder& sb, const String& prefix);

    void EmitProxyMethodImpls(StringBuilder& sb, const String& prefix);

    void EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);

    void EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb, const String& prefix);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_CLIENT_PROXY_CODE_EMITTER_H