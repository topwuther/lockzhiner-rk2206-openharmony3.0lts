/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_client_proxy_code_emitter.h"
#include "util/file.h"

#include "util/logger.h"

namespace OHOS {
namespace HDI {
CppClientProxyCodeEmitter::CppClientProxyCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    :CppCodeEmitter(ast, targetDirectory)
{
    String proxyName = String::Format("%sclient.%s",
        interface_->GetNamespace()->ToString().string(), proxyName_.string());
    sourceFileName_ = String::Format("%s.cpp", FileName(proxyName).string());
}

void CppClientProxyCodeEmitter::EmitCode()
{
    EmitProxyHeaderFile();
    EmitProxySourceFile();
}

void CppClientProxyCodeEmitter::EmitProxyHeaderFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sclient/%s.h", directory_.string(), FileName(infName_ + "Proxy").string());
    } else {
        filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_ + "Proxy").string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppClientProxyCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, proxyFullName_);
    sb.Append("\n");
    EmitProxyHeadrInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitProxyDecl(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitTailMacro(sb, proxyFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppClientProxyCodeEmitter::EmitProxyHeadrInclusions(StringBuilder& sb)
{
    sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    sb.Append("#include <iremote_proxy.h>\n");
}

void CppClientProxyCodeEmitter::EmitProxyDecl(StringBuilder& sb, const String& prefix)
{
    sb.AppendFormat("class %s : public IRemoteProxy<%s> {\n",
        proxyName_.string(), interfaceName_.string());
    sb.Append("public:\n");
    EmitProxyConstructor(sb, TAB);
    sb.Append("\n");
    EmitProxyMethodDecls(sb, TAB);
    sb.Append("\n");
    sb.Append("private:\n");
    EmitProxyConstants(sb, TAB);
    sb.Append("};\n");
}

void CppClientProxyCodeEmitter::EmitProxyConstructor(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("explicit %s(\n", proxyName_.string());
    sb.Append(prefix + TAB).Append("const sptr<IRemoteObject>& remote)\n");
    sb.Append(prefix + TAB).AppendFormat(": IRemoteProxy<%s>(remote)\n", interfaceName_.string());
    sb.Append(prefix).Append("{}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("virtual ~%s() {}\n", proxyName_.string());
}

void CppClientProxyCodeEmitter::EmitProxyMethodDecls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodDecl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppClientProxyCodeEmitter::EmitProxyMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s() override;\n", method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s(", method->GetName().string());

        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitProxyMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(") override;");

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
}

void CppClientProxyCodeEmitter::EmitProxyConstants(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("static inline BrokerDelegator<%s> delegator_;\n", proxyName_.string());
}

void CppClientProxyCodeEmitter::EmitProxyMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append(param->EmitCppParameter());
}

void CppClientProxyCodeEmitter::EmitProxySourceFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sclient/%s.cpp", directory_.string(), FileName(infName_ + "Proxy").string());
    } else {
        filePath = String::Format("%s%s.cpp", directory_.string(), FileName(infName_ + "Proxy").string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppClientProxyCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitProxySourceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        EmitGetMethodImpl(sb, "");
        sb.Append("\n");
    }
    EmitProxyMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppClientProxyCodeEmitter::EmitProxySourceInclusions(StringBuilder& sb)
{
    sb.AppendFormat("#include \"%s.h\"\n", FileName(proxyName_).string());
    EmitProxySourceStdlibInclusions(sb);
}

void CppClientProxyCodeEmitter::EmitProxySourceStdlibInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <message_option.h>\n");
    sb.Append("#include <message_parcel.h>\n");

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
            sb.Append("#include <securec.h>\n");
            break;
        }
    }
}

void CppClientProxyCodeEmitter::EmitGetMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("sptr<%s> %s::Get()\n",
        interface_->GetName().string(), interface_->GetName().string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("do {\n");
    sb.Append(prefix + TAB + TAB).Append("using namespace OHOS::HDI::ServiceManager::V1_0;\n");
    sb.Append(prefix + TAB + TAB).Append("auto servMgr = IServiceManager::Get();\n");
    sb.Append(prefix + TAB + TAB).Append("if (servMgr == nullptr) {\n");
    sb.Append(prefix + TAB + TAB + TAB).Append("HDF_LOGE(\"%{public}s:get IServiceManager failed!\", __func__);\n");
    sb.Append(prefix + TAB + TAB + TAB).Append("break;\n");
    sb.Append(prefix + TAB + TAB).Append("}\n\n");
    sb.Append(prefix + TAB + TAB).AppendFormat("sptr<IRemoteObject> remote = servMgr->GetService(\"%sService\");\n",
        infName_.string());
    sb.Append(prefix + TAB + TAB).Append("if (remote != nullptr) {\n");
    sb.Append(prefix + TAB + TAB + TAB).AppendFormat("return iface_cast<%s>(remote);\n",
        interface_->GetName().string());
    sb.Append(prefix + TAB + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("} while(false);\n");
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: get %sService failed!\", __func__);\n", infName_.string());
    sb.Append(prefix + TAB).Append("return nullptr;\n");
    sb.Append(prefix).Append("}\n");
}


void CppClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppClientProxyCodeEmitter::EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("int32_t %s::%s()\n", proxyName_.string(), method->GetName().string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("int32_t %s::%s(", proxyName_.string(), method->GetName().string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitProxyMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }

        paramStr.Append(")");

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
    EmitProxyMethodBody(method, sb, prefix);
}

void CppClientProxyCodeEmitter::EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("MessageParcel data;\n");
    sb.Append(prefix + TAB).Append("MessageParcel reply;\n");
    sb.Append(prefix + TAB).Append("MessageOption option(MessageOption::TF_SYNC);\n");
    sb.Append("\n");

    if (method->GetParameterNumber() > 0) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_IN) {
                EmitWriteMethodParameter(param, "data", sb, prefix + TAB);
            }
        }
        sb.Append("\n");
    }

    sb.Append(prefix + TAB).AppendFormat("int32_t ec = Remote()->SendRequest(CMD_%s, data, reply, option);\n",
        ConstantName(method->GetName()).string());
    sb.Append(prefix + TAB).Append("if (ec != HDF_SUCCESS) {\n");
    sb.Append(prefix + TAB + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s failed, error code is %%d\", ec);\n", method->GetName().string());
    sb.Append(prefix + TAB + TAB).Append("return ec;\n");
    sb.Append(prefix + TAB).Append("}\n");

    if (!method->IsOneWay()) {
        sb.Append("\n");
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
                EmitReadMethodParameter(param, "reply", false, sb, prefix + TAB);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix + TAB).Append("return HDF_SUCCESS;\n");
    sb.Append(prefix).Append("}\n");
}
} // namespace HDI
} // namespace OHOS