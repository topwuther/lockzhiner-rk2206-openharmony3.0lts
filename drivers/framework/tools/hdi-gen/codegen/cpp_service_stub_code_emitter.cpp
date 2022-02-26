/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_stub_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
CppServiceStubCodeEmitter::CppServiceStubCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    :CppCodeEmitter(ast, targetDirectory)
{
    String stubFullName = String::Format("%sserver.%s",
        interface_->GetNamespace()->ToString().string(), stubName_.string());
    sourceFileName_ = String::Format("%s.cpp", FileName(stubFullName).string());
}

void CppServiceStubCodeEmitter::EmitCode()
{
    EmitStubHeaderFile();
    EmitStubSourceFile();
}

void CppServiceStubCodeEmitter::EmitStubHeaderFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sserver/%s.h", directory_.string(), FileName(infName_ + "Stub").string());
    } else {
        filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_ + "Stub").string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppServiceStubCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, stubFullName_);
    sb.Append("\n");
    EmitStubHeaderInclusions(sb);
    sb.Append("\n");

    if (!isCallbackInterface()) {
        EmitStubDecl(sb);
    } else {
        EmitCbStubDecl(sb);
    }
    sb.Append("\n");
    EmitTailMacro(sb, stubFullName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceStubCodeEmitter::EmitStubHeaderInclusions(StringBuilder& sb)
{
    sb.Append("#include <message_parcel.h>\n");
    sb.Append("#include <message_option.h>\n");
    sb.Append("#include <refbase.h>\n");
    if (!isCallbackInterface()) {
        sb.AppendFormat("#include \"%s_service.h\"\n", FileName(infName_).string());
    } else {
        sb.Append("#include <iremote_stub.h>\n");
        sb.AppendFormat("#include \"%s.h\"\n", FileName(interfaceName_).string());
    }
}

void CppServiceStubCodeEmitter::EmitStubDecl(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitStubUsingNamespace(sb);
    sb.Append("\n");
    sb.AppendFormat("class %s {\n", stubName_.string());
    sb.Append("public:\n");
    EmitStubBody(sb, TAB);
    sb.Append("};\n");

    sb.Append("\n");
    EmitEndNamespace(sb);

    sb.Append("\n");
    EmitStubExternalsMethodsDel(sb);
}

void CppServiceStubCodeEmitter::EmitCbStubDecl(StringBuilder& sb)
{
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitStubUsingNamespace(sb);
    sb.Append("\n");
    sb.AppendFormat("class %s : public IRemoteStub<%s> {\n", stubName_.string(), interfaceName_.string());
    EmitCbStubBody(sb, TAB);
    sb.Append("};\n");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
}

void CppServiceStubCodeEmitter::EmitStubUsingNamespace(StringBuilder& sb)
{
    sb.Append("using namespace OHOS;\n");
}

void CppServiceStubCodeEmitter::EmitStubBody(StringBuilder& sb, const String& prefix)
{
    sb.Append("public:\n");
    EmitStubDestruction(sb, prefix);
    sb.Append("\n");
    EmitStubMethodDecls(sb, prefix);
    sb.Append("\n");
    EmitStubOnRequestMethodDecl(sb, prefix);
    sb.Append("\n");
    EmitStubMembers(sb, prefix);
}

void CppServiceStubCodeEmitter::EmitCbStubBody(StringBuilder& sb, const String& prefix)
{
    sb.Append("public:\n");
    EmitStubDestruction(sb, prefix);
    sb.Append("\n");
    EmitCbStubOnRequestDecl(sb, prefix);
    EmitStubMethodDecls(sb, prefix);
}

void CppServiceStubCodeEmitter::EmitStubDestruction(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("virtual ~%s() {}\n", stubName_.string());
}

void CppServiceStubCodeEmitter::EmitCbStubOnRequestDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).Append("int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,\n");
    sb.Append(prefix + TAB).Append("MessageOption &option) override;\n");
}

void CppServiceStubCodeEmitter::EmitStubMethodDecls(StringBuilder& sb, const String& prefix)
{
    if (interface_->GetMethodNumber() > 0) {
        if (isCallbackInterface()) {
            sb.Append("private:\n");
        }

        for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
            AutoPtr<ASTMethod> method = interface_->GetMethod(i);
            EmitStubMethodDecl(method, sb, prefix);
            if (i + 1 < interface_->GetMethodNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CppServiceStubCodeEmitter::EmitStubMethodDecl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %s%s(MessageParcel& data, MessageParcel& reply, MessageOption& option);\n",
        stubName_.string(), method->GetName().string());
}

void CppServiceStubCodeEmitter::EmitStubOnRequestMethodDecl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %sOnRemoteRequest(int cmdId, MessageParcel& data, MessageParcel& reply,\n",
        stubName_.string());
    sb.Append(prefix).Append(TAB).Append("MessageOption& option);\n");
}

void CppServiceStubCodeEmitter::EmitStubMembers(StringBuilder& sb, const String& prefix)
{
    sb.Append("private:\n");
    sb.Append(prefix).AppendFormat("%sService service;\n", infName_.string());
}

void CppServiceStubCodeEmitter::EmitStubExternalsMethodsDel(StringBuilder& sb)
{
    sb.AppendFormat("void *%sInstance();\n", stubName_.string());
    sb.Append("\n");
    sb.AppendFormat("void %sRelease(void *obj);\n", stubName_.string());
    sb.Append("\n");
    sb.AppendFormat(
        "int32_t %sServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf* data, struct HdfSBuf* reply);\n",
        infName_.string());
}

void CppServiceStubCodeEmitter::EmitStubSourceFile()
{
    String filePath;
    if (!isCallbackInterface()) {
        filePath = String::Format("%sserver/%s.cpp", directory_.string(), FileName(infName_ + "Stub").string());
    } else {
        filePath = String::Format("%s%s.cpp", directory_.string(), FileName(infName_ + "Stub").string());
    }

    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppServiceStubCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitStubSourceInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    if (!isCallbackInterface()) {
        EmitStubOnRequestMethodImpl(sb, "");
    } else {
        EmitCbStubOnRequestMethodImpl(sb, "");
    }
    sb.Append("\n");
    EmitStubMethodImpls(sb, "");
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");

    if (!isCallbackInterface()) {
        EmitStubExternalsMethodsImpl(sb, "");
    }

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceStubCodeEmitter::EmitStubSourceInclusions(StringBuilder& sb)
{
    sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
    EmitStubSourceStdlibInclusions(sb);
}

void CppServiceStubCodeEmitter::EmitStubSourceStdlibInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <hdf_sbuf_ipc.h>\n");

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_UNION) {
            sb.Append("#include <securec.h>\n");
            break;
        }
    }
}

void CppServiceStubCodeEmitter::EmitStubMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitStubMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void CppServiceStubCodeEmitter::EmitStubMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).AppendFormat(
        "int32_t %s::%s%s(MessageParcel& data, MessageParcel& reply, MessageOption& option)\n",
        stubName_.string(), stubName_.string(), method->GetName().string());
    sb.Append(prefix).Append("{\n");

    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_IN) {
            EmitReadMethodParameter(param, "data", true, sb, prefix + TAB);
            sb.Append("\n");
        } else {
            EmitLocalVariable(param, sb, prefix + TAB);
            sb.Append("\n");
        }
    }

    if (method->GetParameterNumber() == 0) {
        if (!isCallbackInterface()) {
            sb.Append(prefix + TAB).AppendFormat("int32_t ec = service.%s();\n", method->GetName().string());
        } else {
            sb.Append(prefix + TAB).AppendFormat("int32_t ec = %s();\n", method->GetName().string());
        }
    } else {
        if (!isCallbackInterface()) {
            sb.Append(prefix + TAB).AppendFormat("int32_t ec = service.%s(", method->GetName().string());
        } else {
            sb.Append(prefix + TAB).AppendFormat("int32_t ec = %s(", method->GetName().string());
        }

        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            sb.Append(param->GetName());
            if (i + 1 < method->GetParameterNumber()) {
                sb.Append(", ");
            }
        }
        sb.Append(");\n");
    }

    sb.Append(prefix + TAB).Append("if (ec != HDF_SUCCESS) {\n");
    sb.Append(prefix + TAB + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s failed, error code is %%d\", ec);\n", method->GetName().string());
    sb.Append(prefix + TAB + TAB).Append("return ec;\n");
    sb.Append(prefix + TAB).Append("}\n\n");

    if (!method->IsOneWay()) {
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
                EmitWriteMethodParameter(param, "reply", sb, prefix + TAB);
                sb.Append("\n");
            }
        }
    }

    sb.Append(prefix + TAB).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %s::%sOnRemoteRequest(int cmdId,\n",
        stubName_.string(), stubName_.string());
    sb.Append(prefix + TAB).Append("MessageParcel& data, MessageParcel& reply, MessageOption& option)\n");
    sb.Append(prefix).Append("{\n");

    sb.Append(prefix + TAB).Append("switch (cmdId) {\n");
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + TAB + TAB).AppendFormat("case CMD_%s:\n", ConstantName(method->GetName()).string());
        sb.Append(prefix + TAB + TAB + TAB).AppendFormat("return %sStub%s(data, reply, option);\n",
            infName_.string(), method->GetName().string());
    }

    sb.Append(prefix + TAB + TAB).Append("default: {\n");
    sb.Append(prefix + TAB + TAB + TAB).Append(
        "HDF_LOGE(\"%{public}s: not support cmd %{public}d\", __func__, cmdId);\n");
    sb.Append(prefix + TAB + TAB + TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitCbStubOnRequestMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("int32_t %s::OnRemoteRequest(uint32_t code,\n", stubName_.string());
    sb.Append(prefix + TAB).Append("MessageParcel& data, MessageParcel& reply, MessageOption& option)\n");
    sb.Append(prefix).Append("{\n");

    sb.Append(prefix + TAB).Append("switch (code) {\n");

    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        sb.Append(prefix + TAB + TAB).AppendFormat("case CMD_%s:\n", ConstantName(method->GetName()).string());
        sb.Append(prefix + TAB + TAB + TAB).AppendFormat("return %sStub%s(data, reply, option);\n",
            infName_.string(), method->GetName().string());
    }

    sb.Append(prefix + TAB + TAB).Append("default: {\n");
    sb.Append(prefix + TAB + TAB + TAB).Append(
        "HDF_LOGE(\"%{public}s: not support cmd %{public}d\", __func__, code);\n");
    sb.Append(prefix + TAB + TAB + TAB).Append("return IPCObjectStub::OnRemoteRequest(code, data, reply, option);\n");
    sb.Append(prefix + TAB + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubExternalsMethodsImpl(StringBuilder& sb, const String& prefix)
{
    EmitStubInstanceMethodImpl(sb, prefix);
    sb.Append("\n");
    EmitStubReleaseMethodImpl(sb, prefix);
    sb.Append("\n");
    EmitServiceOnRemoteRequest(sb, prefix);
}

void CppServiceStubCodeEmitter::EmitStubInstanceMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("void *%sInstance()\n", stubName_.string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(prefix + TAB).AppendFormat("return reinterpret_cast<void *>(new %s());\n", stubName_.string());
    sb.Append(prefix).Append("}\n");
}

void CppServiceStubCodeEmitter::EmitStubReleaseMethodImpl(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("void %sRelease(void *obj)\n", stubName_.string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(prefix + TAB).AppendFormat("delete reinterpret_cast<%s *>(obj);\n", stubName_.string());
    sb.Append(prefix).Append("}\n");
}

void CppServiceStubCodeEmitter::EmitServiceOnRemoteRequest(StringBuilder& sb, const String& prefix)
{
    String stubObjName = String::Format("%sStub", infName_.ToLowerCase().string());
    sb.Append(prefix).AppendFormat(
        "int32_t %sServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)\n",
        infName_.string());
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("using namespace %s;\n",
        EmitStubServiceUsings(interface_->GetNamespace()->ToString()).string());
    sb.Append(prefix + TAB).AppendFormat("%s *%s = reinterpret_cast<%s *>(stub);\n",
        stubName_.string(), stubObjName.string(), stubName_.string());
    sb.Append(prefix + TAB).Append("OHOS::MessageParcel *dataParcel = nullptr;\n");
    sb.Append(prefix + TAB).Append("OHOS::MessageParcel *replyParcel = nullptr;\n");
    sb.Append("\n");

    sb.Append(prefix + TAB).Append("(void)SbufToParcel(reply, &replyParcel);\n");
    sb.Append(prefix + TAB).Append("if (SbufToParcel(data, &dataParcel) != HDF_SUCCESS) {\n");
    sb.Append(prefix + TAB + TAB).Append(
        "HDF_LOGE(\"%{public}s:invalid data sbuf object to dispatch\", __func__);\n");
    sb.Append(prefix + TAB + TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).Append("}\n\n");
    sb.Append(prefix + TAB).Append("OHOS::MessageOption option;\n");
    sb.Append(prefix + TAB).AppendFormat("return %s->%sOnRemoteRequest(cmdId, *dataParcel, *replyParcel, option);\n",
        stubObjName.string(), stubName_.string());
    sb.Append(prefix).Append("}\n");
}

String CppServiceStubCodeEmitter::EmitStubServiceUsings(String nameSpace)
{
    int index = nameSpace.LastIndexOf('.');
    if (index > 0) {
        nameSpace = nameSpace.Substring(0, index);
    }
    return CppFullName(nameSpace);
}
} // namespace HDI
} // namespace OHOS