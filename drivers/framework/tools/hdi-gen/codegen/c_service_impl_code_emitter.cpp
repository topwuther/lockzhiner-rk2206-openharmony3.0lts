/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_service_impl_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
CServiceImplCodeEmitter::CServiceImplCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    :CCodeEmitter(ast, targetDirectory)
{
    String infFullName = String::Format("%sserver.%s",
        interface_->GetNamespace()->ToString().string(), infName_.string());
    sourceFileName_ = String::Format("%s_service.c", FileName(infFullName).string());
}

void CServiceImplCodeEmitter::EmitCode()
{
    if (isCallbackInterface()) {
        EmitServiceImplHeaderFile();
    }
    EmitServiceImplSourceFile();
}

void CServiceImplCodeEmitter::EmitServiceImplHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_ + "Service").string());
    if (!File::CreateParentDir(filePath)) {
        Logger::E("CServiceDriverCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, ImplFullName_);
    sb.Append("\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitServiceImplConstructDecl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, ImplFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceImplCodeEmitter::EmitServiceImplConstructDecl(StringBuilder& sb)
{
    sb.AppendFormat("void %sServiceConstruct(struct %s* service);\n", infName_.string(), interfaceName_.string());
}

void CServiceImplCodeEmitter::EmitServiceImplSourceFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sserver/%s.c", directory_.string(), FileName(infName_ + "Service").string());
    } else {
        filePath = String::Format("%s%s.c", directory_.string(), FileName(infName_ + "Service").string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CServiceDriverCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitServiceImplInclusions(sb);
    sb.Append("\n");
    EmitServiceImplMethodImpls(sb, "");
    sb.Append("\n");
    EmitServiceImplConstruct(sb);
    if (!isCallbackInterface()) {
        sb.Append("\n");
        EmitServiceImplInstance(sb);
        sb.Append("\n");
        EmitServiceImplRelease(sb);
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CServiceImplCodeEmitter::EmitServiceImplInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <osal_mem.h>\n");
    sb.Append("#include <securec.h>\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
}

void CServiceImplCodeEmitter::EmitServiceImplMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitServiceImplMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CServiceImplCodeEmitter::EmitServiceImplMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s%s(struct %s *self)\n",
            infName_.string(), method->GetName().string(), interfaceName_.string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s%s(struct %s *self, ",
            infName_.string(), method->GetName().string(), interfaceName_.string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(")");
        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }

    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}

void CServiceImplCodeEmitter::EmitServiceImplConstruct(StringBuilder& sb)
{
    String objName("instance");
    sb.AppendFormat("void %sServiceConstruct(struct %s *%s)\n",
        infName_.string(), interfaceName_.string(), objName.string());
    sb.Append("{\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(TAB).AppendFormat("%s->%s = %s%s;\n",
            objName.string(), method->GetName().string(), infName_.string(), method->GetName().string());
    }
    sb.Append("}\n");
}

void CServiceImplCodeEmitter::EmitServiceImplInstance(StringBuilder& sb)
{
    String objName("instance");
    sb.AppendFormat("struct %s *Hdi%sInstance()\n", interfaceName_.string(), infName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("struct %s *%s = (struct %s*)OsalMemAlloc(sizeof(struct %s));\n",
        interfaceName_.string(), objName.string(), interfaceName_.string(), interfaceName_.string());
    sb.Append(TAB).AppendFormat("if (%s == NULL) {\n", objName.string());
    sb.Append(TAB).Append(TAB).AppendFormat("HDF_LOGE(\"%%{public}s: OsalMemAlloc struct %s %s failed!\", __func__);\n",
        interfaceName_.string(), objName.string());
    sb.Append(TAB).Append(TAB).Append("return NULL;\n");
    sb.Append(TAB).Append("}\n");
    sb.Append(TAB).AppendFormat("%sServiceConstruct(%s);\n", infName_.string(), objName.string());
    sb.Append(TAB).AppendFormat("return %s;\n", objName.string());
    sb.Append("}\n");
}

void CServiceImplCodeEmitter::EmitServiceImplRelease(StringBuilder& sb)
{
    sb.AppendFormat("void Hdi%sRelease(struct %s *instance)\n", infName_.string(), interfaceName_.string());
    sb.Append("{\n");
    sb.Append(TAB).Append("if (instance == NULL) {\n");
    sb.Append(TAB).Append(TAB).Append("return;\n");
    sb.Append(TAB).Append("}\n");
    sb.Append(TAB).Append("OsalMemFree(instance);\n");
    sb.Append("}\n");
}
} // namespace HDI
} // namespace OHOS