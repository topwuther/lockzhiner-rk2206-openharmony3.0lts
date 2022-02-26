/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_client_interface_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
void CppClientInterfaceCodeEmitter::EmitCode()
{
    EmitInterfaceHeaderFile();
}

void CppClientInterfaceCodeEmitter::EmitInterfaceHeaderFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sclient/%s.h", directory_.string(), FileName(interfaceName_).string());
    } else {
        filePath = String::Format("%s%s.h", directory_.string(), FileName(interfaceName_).string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppClientInterfaceCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitClientInfHeadMacro(sb);
    sb.Append("\n");
    EmitInterfaceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitUsingNamespace(sb);
    sb.Append("\n");
    EmitInterfaceMethodCommands(sb, "");
    sb.Append("\n");
    EmitInterfaceDecl(sb);
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitClientInfTailMacro(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppClientInterfaceCodeEmitter::EmitClientInfHeadMacro(StringBuilder& sb)
{
    String clientMacroName(interfaceFullName_);
    if (!isCallbackInterface()) {
        clientMacroName += ".client";
    }
    EmitHeadMacro(sb, clientMacroName);
}

void CppClientInterfaceCodeEmitter::EmitClientInfTailMacro(StringBuilder& sb)
{
    String clientMacroName(interfaceFullName_);
    if (!isCallbackInterface()) {
        clientMacroName += ".client";
    }
    EmitTailMacro(sb, clientMacroName);
}

void CppClientInterfaceCodeEmitter::EmitInterfaceInclusions(StringBuilder& sb)
{
    EmitInterfaceStdlibInclusions(sb);
    EmitInterfaceDBinderInclusions(sb);
    EmitInterfaceSelfDefinedTypeInclusions(sb);
}

void CppClientInterfaceCodeEmitter::EmitInterfaceStdlibInclusions(StringBuilder& sb)
{
    bool includeString = false;
    bool includeList = false;
    bool includeMap = false;

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_STRING: {
                if (!includeString) {
                    sb.Append("#include <string>\n");
                    includeString = true;
                }
                break;
            }
            case TypeKind::TYPE_ARRAY:
            case TypeKind::TYPE_LIST: {
                if (!includeList) {
                    sb.Append("#include <vector>\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::TYPE_MAP: {
                if (!includeMap) {
                    sb.Append("#include <map>\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void CppClientInterfaceCodeEmitter::EmitInterfaceDBinderInclusions(StringBuilder& sb)
{
    sb.Append("#include <stdint.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <iservmgr_hdi.h>\n");
}

void CppClientInterfaceCodeEmitter::EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;

        String fileName;
        if (importAst->GetASTFileType() == ASTFileType::AST_ICALLBACK && importAst->GetInterfaceDef() != nullptr) {
            String ifaceName = importAst->GetInterfaceDef()->GetName();
            String name = ifaceName.StartsWith("I") ? ifaceName.Substring(1) : ifaceName;
            String stubName = name + "Service";
            fileName = FileName(importAst->GetInterfaceDef()->GetNamespace()->ToString() + stubName);
        } else {
            fileName = FileName(importAst->GetFullName());
        }
        sb.Append("#include ").AppendFormat("\"%s.h\"\n", fileName.string());
    }
}

void CppClientInterfaceCodeEmitter::EmitInterfaceDecl(StringBuilder& sb)
{
    sb.AppendFormat("class %s : public IRemoteBroker {\n", interface_->GetName().string());
    sb.Append("public:\n");
    EmitInterfaceBody(sb, TAB);
    sb.Append("};\n\n");
}

void CppClientInterfaceCodeEmitter::EmitInterfaceBody(StringBuilder& sb, const String& prefix)
{
    sb.Append(TAB).AppendFormat("DECLARE_INTERFACE_DESCRIPTOR(u\"%s\");\n", interfaceFullName_.string());
    sb.Append("\n");
    EmitInterfaceDestruction(sb, TAB);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        EmitInterfaceGetMethodDecl(sb, TAB);
        sb.Append("\n");
    }
    EmitInterfaceMethodsDecl(sb, TAB);
}

void CppClientInterfaceCodeEmitter::EmitInterfaceDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() = default;\n", interface_->GetName().string());
}

void CppClientInterfaceCodeEmitter::EmitInterfaceGetMethodDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("static sptr<%s> Get();\n", interface_->GetName().string());
}

void CppClientInterfaceCodeEmitter::EmitInterfaceMethodsDecl(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethodDecl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppClientInterfaceCodeEmitter::EmitInterfaceMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("virtual int32_t %s() = 0;\n", method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("virtual int32_t %s(", method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(") = 0;");
        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
}

void CppClientInterfaceCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppParameter());
}
} // namespace HDI
} // namespace OHOS