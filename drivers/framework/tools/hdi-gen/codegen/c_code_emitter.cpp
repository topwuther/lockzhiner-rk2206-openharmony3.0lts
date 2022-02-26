/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_code_emitter.h"
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>

#include "util/logger.h"

namespace OHOS {
namespace HDI {
const char* CCodeEmitter::TAB = "    ";

CCodeEmitter::CCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
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

        stubName_ = infName_ + "Stub";
        stubFullName_ = interface_->GetNamespace()->ToString() + stubName_;

        ImplName_ = infName_ + "Service";
        ImplFullName_ = interface_->GetNamespace()->ToString() + ImplName_;
    } else {
        infName_ = ast_->GetName();
    }
}

String CCodeEmitter::FileName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            // 2->Index of the last char array.
            if (i > 1 && name[i - 1] != '.' && name[i - 2] != '.') {
                sb.Append('_');
            }
            sb.Append(tolower(c));
        } else {
            sb.Append(c);
        }
    }

    return sb.ToString().Replace('.', '/');
}

void CCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& sb)
{
    sb.Append("enum {\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(TAB).AppendFormat("CMD_%s,\n", ConstantName(method->GetName()).string());
    }
    sb.Append("};\n");
}

void CCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& parameter, StringBuilder& sb,
    const String& prefix)
{
    AutoPtr<ASTType> type = parameter->GetType();
    sb.Append(prefix).Append(parameter->EmitCParameter());
}

void CCodeEmitter::EmitLicense(StringBuilder& sb)
{
    if (ast_->GetLicense().IsEmpty()) {
        return;
    }
    sb.Append(ast_->GetLicense()).Append("\n\n");
}

void CCodeEmitter::EmitHeadMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#ifndef ").Append(macroName).Append("\n");
    sb.Append("#define ").Append(macroName).Append("\n");
}

void CCodeEmitter::EmitTailMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#endif // ").Append(macroName);
}

void CCodeEmitter::EmitHeadExternC(StringBuilder& sb)
{
    sb.Append("#ifdef __cplusplus\n");
    sb.Append("extern \"C\" {\n");
    sb.Append("#endif /* __cplusplus */\n");
}

void CCodeEmitter::EmitTailExternC(StringBuilder& sb)
{
    sb.Append("#ifdef __cplusplus\n");
    sb.Append("}\n");
    sb.Append("#endif /* __cplusplus */\n");
}

String CCodeEmitter::MacroName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String macro = name.Replace('.', '_').ToUpperCase() + "_H";
    return macro;
}

String CCodeEmitter::ConstantName(const String& name)
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

String CCodeEmitter::SpecificationParam(StringBuilder& paramSb, const String& prefix)
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