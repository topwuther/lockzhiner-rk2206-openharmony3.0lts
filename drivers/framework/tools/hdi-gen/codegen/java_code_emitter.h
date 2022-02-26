/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_JAVA_CODE_EMITTER_H
#define OHOS_HDI_JAVA_CODE_EMITTER_H

#include "ast/ast.h"
#include "util/autoptr.h"
#include "util/light_refcount_base.h"
#include "util/string.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
class JavaCodeEmitter : public LightRefCountBase {
public:
    JavaCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory);

    virtual ~JavaCodeEmitter() = default;

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

    bool CreateDirectory();

    void EmitLicense(StringBuilder& sb);

    void EmitPackage(StringBuilder& sb);

    void EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix);

    String MethodName(const String& name);

    String ConstantName(const String& name);

    String SpecificationParam(StringBuilder& paramSb, const String& prefix);

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
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_JAVA_CODE_EMITTER_H