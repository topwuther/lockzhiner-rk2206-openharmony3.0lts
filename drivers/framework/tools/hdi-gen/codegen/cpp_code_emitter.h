/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_CPP_CODE_EMITTER_H
#define OHOS_HDI_CPP_CODE_EMITTER_H

#include "ast/ast.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/string.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
class CppCodeEmitter : public LightRefCountBase {
public:
    CppCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory);

    virtual ~CppCodeEmitter() = default;

    virtual void EmitCode() = 0;

    inline String GetSourceFile()
    {
        return sourceFileName_;
    }

    inline bool isInvaildDir()
    {
        return directory_.Equals("");
    }

    static String FileName(const String& name);
protected:

    void EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix);

    void EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix);

    void EmitLicense(StringBuilder& sb);

    void EmitHeadMacro(StringBuilder& sb, const String& fullName);

    void EmitTailMacro(StringBuilder& sb, const String& fullName);

    virtual void EmitBeginNamespace(StringBuilder& sb);

    virtual void EmitEndNamespace(StringBuilder& sb);

    void EmitUsingNamespace(StringBuilder& sb);

    void EmitImportUsingNamespace(StringBuilder& sb);

    void EmitWriteMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName, StringBuilder& sb,
        const String& prefix);

    void EmitReadMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName, bool initVariable,
        StringBuilder& sb, const String& prefix);

    void EmitLocalVariable(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix);

    String MacroName(const String& name);

    String CppFullName(const String& name);

    String ConstantName(const String& name);

    String CppNameSpace(const String& name);

    bool isCallbackInterface()
    {
        if (interface_ == nullptr) {
            return false;
        }

        return interface_->IsCallback();
    }

    String SpecificationParam(StringBuilder& sb, const String& prefix);

    static const char* TAB;

    AutoPtr<AST> ast_;
    AutoPtr<ASTInterfaceType> interface_;

    String directory_;
    String sourceFileName_;

    String interfaceName_;
    String interfaceFullName_;
    String infName_;
    String proxyName_;
    String proxyFullName_;
    String stubName_;
    String stubFullName_;
    String ImplName_;
    String ImplFullName_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_CPP_CODE_EMITTER_H