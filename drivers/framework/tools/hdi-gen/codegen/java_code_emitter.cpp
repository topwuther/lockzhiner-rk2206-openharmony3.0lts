/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/java_code_emitter.h"
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS {
namespace HDI {
const char* JavaCodeEmitter::TAB = "    ";

JavaCodeEmitter::JavaCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    :LightRefCountBase(), ast_(ast), directory_(targetDirectory)
{
    if (ast_->GetASTFileType() == ASTFileType::AST_IFACE || ast_->GetASTFileType() == ASTFileType::AST_ICALLBACK) {
        interface_ = ast_->GetInterfaceDef();
    }

    if (interface_ != nullptr) {
        interfaceName_ = interface_->GetName();
        interfaceFullName_ = interface_->GetNamespace()->ToString() + interfaceName_;
        infName_ = interfaceName_.StartsWith("I") ? interfaceName_.Substring(1) : interfaceName_;
        proxyName_ = infName_ + "Proxy";
        proxyFullName_ = interface_->GetNamespace()->ToString() + proxyName_;
    } else {
        infName_ = ast_->GetName();
    }
}

String JavaCodeEmitter::FileName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    return name.Replace('.', '/');
}

void JavaCodeEmitter::EmitLicense(StringBuilder& sb)
{
    if (ast_->GetLicense().IsEmpty()) {
        return;
    }
    sb.Append(ast_->GetLicense()).Append("\n\n");
}

void JavaCodeEmitter::EmitPackage(StringBuilder& sb)
{
    sb.AppendFormat("package %s;\n", ast_->GetPackageName().string());
}

void JavaCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix).AppendFormat(
            "private static final int COMMAND_%s = IRemoteObject.MIN_TRANSACTION_ID + %d;\n",
            ConstantName(method->GetName()).string(), i);
    }
}

String JavaCodeEmitter::MethodName(const String& name)
{
    if (name.IsEmpty() || islower(name[0])) {
        return name;
    }
    return String::Format("%c%s", tolower(name[0]), name.Substring(1).string());
}

String JavaCodeEmitter::ConstantName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            if (i > 1) {
                sb.Append('_');
            }
            sb.Append(c);
        } else {
            sb.Append(toupper(c));
        }
    }

    return sb.ToString();
}

String JavaCodeEmitter::SpecificationParam(StringBuilder& paramSb, const String& prefix)
{
    int maxLineLen = 120;
    int replaceLen = 2;
    String paramStr = paramSb.ToString();
    int preIndex = 0;
    int curIndex = 0;

    String insertStr = String::Format("\n%s", prefix.string());
    for (; curIndex < paramStr.GetLength(); curIndex++) {
        if (curIndex == maxLineLen && preIndex > 0) {
            paramStr.Replace(preIndex, replaceLen, ",");
            paramStr.insert(preIndex + 1, insertStr);
        } else {
            if (paramStr[curIndex] == ',') {
                preIndex = curIndex;
            }
        }
    }
    return paramStr;
}
} // namespace HDI
} // namespace OHOS