/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_service_interface_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
void CServiceInterfaceCodeEmitter::EmitCode()
{
    if (!isCallbackInterface()) {
        EmitInterfaceHeadrFile();
    }
}

void CServiceInterfaceCodeEmitter::EmitInterfaceHeadrFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sserver/%s.h", directory_.string(), FileName(interfaceName_).string());
    } else {
        filePath = String::Format("%s%s.h", directory_.string(), FileName(interfaceName_).string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CServiceInterfaceCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, interfaceFullName_);
    sb.Append("\n");
    EmitImportInclusions(sb);
    sb.Append("\n");
    EmitInterfaceDataDecls(sb);
    sb.Append("\n");
    EmitInterfaceMethodCommands(sb);
    sb.Append("\n");
    EmitInterfaceDefinition(sb);
    sb.Append("\n");
    EmitTailMacro(sb, interfaceFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceInterfaceCodeEmitter::EmitImportInclusions(StringBuilder& sb)
{
    sb.Append("#include <stdint.h>\n");
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

void CServiceInterfaceCodeEmitter::EmitInterfaceDataDecls(StringBuilder& sb)
{
    sb.Append("struct HdfSBuf;\n");
    sb.Append("struct HdfDeviceObject;\n");
    sb.Append("struct HdfDeviceIoClient;\n");
}

void CServiceInterfaceCodeEmitter::EmitInterfaceDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct %s {\n", interfaceName_.string());
    EmitInterfaceMethods(sb, TAB);
    sb.Append("};\n\n");
    EmitInterfaceInstanceMethodDecl(sb);
    sb.Append("\n");
    EmitInterfaceReleaseMethodDecl(sb);
    sb.Append("\n");
    EmitInterfaceRequestMethodDecl(sb);
}

void CServiceInterfaceCodeEmitter::EmitInterfaceMethods(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitInterfaceMethod(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CServiceInterfaceCodeEmitter::EmitInterfaceMethod(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t (*%s)(struct %s *self);\n",
            method->GetName().string(), interfaceName_.string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t (*%s)(struct %s *self, ",
            method->GetName().string(), interfaceName_.string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(");");
        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
}

void CServiceInterfaceCodeEmitter::EmitInterfaceInstanceMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct %s *Hdi%sInstance();\n", interfaceName_.string(), infName_.string());
}

void CServiceInterfaceCodeEmitter::EmitInterfaceReleaseMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("void Hdi%sRelease(struct %s *instance);\n",
        infName_.string(), interfaceName_.string());
}

void CServiceInterfaceCodeEmitter::EmitInterfaceRequestMethodDecl(StringBuilder& sb)
{
    sb.AppendFormat("int32_t %sServiceOnRemoteRequest(void *service, int cmdId,\n", infName_.string());
    sb.Append(TAB).Append("struct HdfSBuf *data, struct HdfSBuf *reply);\n");
}
} // namespace HDI
} // namespace OHOS