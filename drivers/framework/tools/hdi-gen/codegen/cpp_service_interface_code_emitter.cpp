/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_interface_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
void CppServiceInterfaceCodeEmitter::EmitCode()
{
    if (!isCallbackInterface()) {
        EmitInterfaceHeaderFile();
    }
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceHeaderFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sserver/%s.h", directory_.string(), FileName(interfaceName_).string());
    } else {
        filePath = String::Format("%s%s.h", directory_.string(), FileName(interfaceName_).string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppServiceInterfaceCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitServiceInfHeadMacro(sb);
    sb.Append("\n");
    EmitInterfaceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitUsingNamespace(sb);
    sb.Append("\n");
    EmitInterfaceDecl(sb);
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitServiceInfTailMacro(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceInterfaceCodeEmitter::EmitServiceInfHeadMacro(StringBuilder& sb)
{
    String serviceMacroName(interfaceFullName_);
    if (!isCallbackInterface()) {
        serviceMacroName += ".service";
    }
    EmitHeadMacro(sb, serviceMacroName);
}

void CppServiceInterfaceCodeEmitter::EmitServiceInfTailMacro(StringBuilder& sb)
{
    String serviceMacroName(interfaceFullName_);
    if (!isCallbackInterface()) {
        serviceMacroName += ".service";
    }
    EmitTailMacro(sb, serviceMacroName);
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceInclusions(StringBuilder& sb)
{
    EmitInterfaceStdlibInclusions(sb);
    EmitInterfaceDBinderInclusions(sb);
    EmitInterfaceSelfDefinedTypeInclusions(sb);
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceStdlibInclusions(StringBuilder& sb)
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

void CppServiceInterfaceCodeEmitter::EmitInterfaceDBinderInclusions(StringBuilder& sb)
{
    sb.Append("#include <stdint.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <iservmgr_hdi.h>\n");
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceSelfDefinedTypeInclusions(StringBuilder& sb)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;

        String fileName;
        if (importAst->GetASTFileType() == ASTFileType::AST_ICALLBACK && importAst->GetInterfaceDef() != nullptr) {
            String ifaceName = importAst->GetInterfaceDef()->GetName();
            String name = ifaceName.StartsWith("I") ? ifaceName.Substring(1) : ifaceName;
            String stubName = name + "Proxy";
            fileName = FileName(importAst->GetInterfaceDef()->GetNamespace()->ToString() + stubName);
        } else {
            fileName = FileName(importAst->GetFullName());
        }
        sb.Append("#include ").AppendFormat("\"%s.h\"\n", fileName.string());
    }
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceDecl(StringBuilder& sb)
{
    EmitInterfaceMethodCommands(sb, "");
    sb.Append("\n");

    sb.AppendFormat("class %s {\n", interface_->GetName().string());
    sb.Append("public:\n");
    EmitInterfaceBody(sb, TAB);
    sb.Append("};\n");
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceBody(StringBuilder& sb, const String& prefix)
{
    EmitInterfaceDestruction(sb, TAB);
    sb.Append("\n");
    EmitInterfaceMethods(sb, TAB);
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() {}\n", interface_->GetName().string());
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceMethods(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethod(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceInterfaceCodeEmitter::EmitInterfaceMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
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
} // namespace HDI
} // namespace OHOS