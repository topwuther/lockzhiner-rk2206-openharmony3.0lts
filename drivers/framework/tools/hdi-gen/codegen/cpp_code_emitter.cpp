/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_code_emitter.h"
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>

namespace OHOS {
namespace HDI {
const char* CppCodeEmitter::TAB = "    ";

CppCodeEmitter::CppCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    : LightRefCountBase(), ast_(ast), directory_(targetDirectory)
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

String CppCodeEmitter::FileName(const String& name)
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

void CppCodeEmitter::EmitInterfaceMethodCommands(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("enum {\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(TAB).AppendFormat("CMD_%s,\n", ConstantName(method->GetName()).string());
    }
    sb.Append(prefix).Append("};\n");
}

void CppCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat(param->EmitCppParameter());
}

void CppCodeEmitter::EmitLicense(StringBuilder& sb)
{
    if (ast_->GetLicense().IsEmpty()) {
        return;
    }
    sb.Append(ast_->GetLicense()).Append("\n\n");
}

void CppCodeEmitter::EmitHeadMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#ifndef ").Append(macroName).Append("\n");
    sb.Append("#define ").Append(macroName).Append("\n");
}

void CppCodeEmitter::EmitTailMacro(StringBuilder& sb, const String& fullName)
{
    String macroName = MacroName(fullName);
    sb.Append("#endif // ").Append(macroName).Append("\n\n");
}

void CppCodeEmitter::EmitBeginNamespace(StringBuilder& sb)
{
    String nspace = interface_->GetNamespace()->ToString();
    int index = nspace.IndexOf('.');
    while (index != -1) {
        sb.AppendFormat("namespace %s {\n", nspace.Substring(0, index).string());
        nspace = nspace.Substring(index + 1);
        index = nspace.IndexOf('.');
    }
}

void CppCodeEmitter::EmitEndNamespace(StringBuilder& sb)
{
    String nspace = interface_->GetNamespace()->ToString();
    nspace = nspace.Substring(0, nspace.GetLength() - 1);
    while (!nspace.IsEmpty()) {
        int index = nspace.LastIndexOf('.');
        sb.AppendFormat("} // %s\n",
            (index != -1) ? nspace.Substring(index + 1, nspace.GetLength()).string() : nspace.string());
        nspace = nspace.Substring(0, index);
    }
}

void CppCodeEmitter::EmitUsingNamespace(StringBuilder& sb)
{
    sb.Append("using namespace OHOS;\n");
    EmitImportUsingNamespace(sb);
}

void CppCodeEmitter::EmitImportUsingNamespace(StringBuilder& sb)
{
    using StringSet = std::unordered_set<String, StringHashFunc, StringEqualFunc>;
    StringSet namespaceSet;
    String selfNameSpace = CppNameSpace(ast_->GetFullName());

    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> import = importPair.second;
        String nameSpace = CppNameSpace(import->GetFullName());
        if (nameSpace.Equals(selfNameSpace)) {
            continue;
        }
        namespaceSet.emplace(nameSpace);
    }

    for (const auto& nspace : namespaceSet) {
        sb.Append("using namespace ").AppendFormat("%s;\n", nspace.string());
    }
}

void CppCodeEmitter::EmitWriteMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    type->EmitCppWriteVar(parcelName, param->GetName(), sb, prefix);
}

void CppCodeEmitter::EmitReadMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    bool initVariable, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    type->EmitCppReadVar(parcelName, param->GetName(), sb, prefix, initVariable);
}

void CppCodeEmitter::EmitLocalVariable(const AutoPtr<ASTParameter>& param, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppLocalVar()).Append("\n");
}

String CppCodeEmitter::MacroName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String macro = name.Replace('.', '_').ToUpperCase() + "_H";
    return macro;
}

String CppCodeEmitter::CppFullName(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    return name.Replace(".", "::");
}

String CppCodeEmitter::ConstantName(const String& name)
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

String CppCodeEmitter::CppNameSpace(const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    String space;
    int lastIndex = name.LastIndexOf('.');
    if (lastIndex != -1) {
        space = name.Substring(0, lastIndex);
    } else {
        space = name;
    }

    return CppFullName(space);
}

String CppCodeEmitter::SpecificationParam(StringBuilder& paramSb, const String& prefix)
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