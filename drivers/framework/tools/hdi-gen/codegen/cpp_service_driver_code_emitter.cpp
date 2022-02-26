/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_service_driver_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
CppServiceDriverCodeEmitter::CppServiceDriverCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
    :CppCodeEmitter(ast, targetDirectory)
{
    String infFullName = String::Format("%sserver.%s",
        interface_->GetNamespace()->ToString().string(), infName_.string());
    sourceFileName_ = String::Format("%s_driver.cpp", FileName(infFullName).string());
}

void CppServiceDriverCodeEmitter::EmitCode()
{
    // the callback interface have no driver file.
    if (!isCallbackInterface()) {
        EmitDriverSourceFile();
    }
}

void CppServiceDriverCodeEmitter::EmitDriverSourceFile()
{
    String filePath = String::Format("%sserver/%s.cpp", directory_.string(), FileName(infName_ + "Driver").string());
    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppServiceDriverCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitDriverIncluions(sb);
    sb.Append("\n");
    EmitDriverUsings(sb);
    sb.Append("\n");
    EmitDriverServiceDecl(sb);
    sb.Append("\n");
    EmitDriverDispatch(sb);
    sb.Append("\n");
    EmitDriverInit(sb);
    sb.Append("\n");
    EmitDriverBind(sb);
    sb.Append("\n");
    EmitDriverRelease(sb);
    sb.Append("\n");
    EmitDriverEntryDefinition(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppServiceDriverCodeEmitter::EmitDriverIncluions(StringBuilder& sb)
{
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <hdf_base.h>\n");
    sb.Append("#include <osal_mem.h>\n");
    sb.Append("#include <hdf_device_desc.h>\n");
    sb.AppendFormat("#include \"%s.h\"\n", FileName(stubName_).string());
}

void CppServiceDriverCodeEmitter::EmitDriverUsings(StringBuilder& sb)
{
    String nspace(interface_->GetNamespace()->ToString());
    int index = nspace.LastIndexOf('.');
    if (index > 0) {
        nspace = nspace.Substring(0, index);
    }

    String fullName = CppFullName(nspace);
    sb.AppendFormat("using namespace %s;\n", fullName.string());
}

void CppServiceDriverCodeEmitter::EmitDriverServiceDecl(StringBuilder& sb)
{
    sb.AppendFormat("struct Hdf%s%s {\n", infName_.string(), "Service");
    sb.Append(TAB).Append("struct IDeviceIoService ioservice;\n");
    sb.Append(TAB).Append("void *instance;\n");
    sb.Append("};\n");
}

void CppServiceDriverCodeEmitter::EmitDriverDispatch(StringBuilder& sb)
{
    sb.AppendFormat("static int32_t %sServiceDispatch(struct HdfDeviceIoClient *client, int cmdId,\n",
        infName_.string());
    sb.Append(TAB).Append("struct HdfSBuf *data, struct HdfSBuf *reply)\n");
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("struct Hdf%sService *hdf%sService = CONTAINER_OF(\n",
        infName_.string(), infName_.string());
    sb.Append(TAB).Append(TAB).AppendFormat("client->device->service, struct Hdf%sService, ioservice);\n",
        infName_.string());
    sb.Append(TAB).AppendFormat("return %sServiceOnRemoteRequest(hdf%sService->instance, cmdId, data, reply);\n",
        infName_.string(), infName_.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverInit(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverInit(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverInit enter, new hdi impl.\");\n", infName_.string());
    sb.Append(TAB).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverBind(StringBuilder& sb)
{
    sb.AppendFormat("int Hdf%sDriverBind(struct HdfDeviceObject *deviceObject)\n", infName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverBind enter.\");\n", infName_.string());
    sb.Append("\n");
    sb.Append(TAB).AppendFormat("struct Hdf%sService *hdf%sService = (struct Hdf%sService *)OsalMemAlloc(\n",
        infName_.string(), infName_.string(), infName_.string());
    sb.Append(TAB).Append(TAB).AppendFormat("sizeof(struct Hdf%sService));\n", infName_.string());
    sb.Append(TAB).AppendFormat("if (hdf%sService == nullptr) {\n", infName_.string());
    sb.Append(TAB).Append(TAB).AppendFormat("HDF_LOGE(\"Hdf%sDriverBind OsalMemAlloc Hdf%sService failed!\");\n",
        infName_.string(), infName_.string());
    sb.Append(TAB).Append(TAB).Append("return HDF_FAILURE;\n");
    sb.Append(TAB).Append("}\n");
    sb.Append("\n");
    sb.Append(TAB).AppendFormat("hdf%sService->ioservice.Dispatch = %sServiceDispatch;\n",
        infName_.string(), infName_.string());
    sb.Append(TAB).AppendFormat("hdf%sService->ioservice.Open = NULL;\n", infName_.string());
    sb.Append(TAB).AppendFormat("hdf%sService->ioservice.Release = NULL;\n", infName_.string());
    sb.Append(TAB).AppendFormat("hdf%sService->instance = %sStubInstance();\n", infName_.string(), infName_.string());
    sb.Append("\n");
    sb.Append(TAB).AppendFormat("deviceObject->service = &hdf%sService->ioservice;\n", infName_.string());
    sb.Append(TAB).Append("return HDF_SUCCESS;\n");
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverRelease(StringBuilder& sb)
{
    sb.AppendFormat("void Hdf%sDriverRelease(struct HdfDeviceObject *deviceObject)", infName_.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("HDF_LOGI(\"Hdf%sDriverRelease enter.\");\n", interfaceName_.string());
    sb.Append("}\n");
}

void CppServiceDriverCodeEmitter::EmitDriverEntryDefinition(StringBuilder& sb)
{
    sb.AppendFormat("struct HdfDriverEntry g_%sDriverEntry = {\n", infName_.ToLowerCase().string());
    sb.Append(TAB).Append(".moduleVersion = 1,\n");
    sb.Append(TAB).AppendFormat(".moduleName = \"%s\",\n", infName_.ToLowerCase().string());
    sb.Append(TAB).AppendFormat(".Bind = Hdf%sDriverBind,\n", infName_.string());
    sb.Append(TAB).AppendFormat(".Init = Hdf%sDriverInit,\n", infName_.string());
    sb.Append(TAB).AppendFormat(".Release = Hdf%sDriverRelease,\n", infName_.string());
    sb.Append("};\n");
    sb.Append("\n");
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("extern \"C\" {\n");
    sb.Append("#endif\n");
    sb.AppendFormat("HDF_INIT(g_%sDriverEntry);\n", infName_.ToLowerCase().string());
    sb.Append("#ifndef __cplusplus\n");
    sb.Append("}\n");
    sb.Append("#endif\n");
}
} // namespace HDI
} // namespace OHOS