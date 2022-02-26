/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/cpp_custom_types_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
void CppCustomTypesCodeEmitter::EmitCode()
{
    EmitCustomTypesHeaderFile();
    EmitCustomTypesSourceFile();
}

void CppCustomTypesCodeEmitter::EmitCustomTypesHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_).string());
    File file(filePath, File::WRITE);
    String marcoName = String::Format("%s.%s", ast_->GetPackageName().string(), infName_.string());

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, marcoName);
    sb.Append("\n");
    EmitHeaderFileInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitUsingNamespace(sb);
    sb.Append("\n");
    EmitCustomTypeDecls(sb);
    sb.Append("\n");
    EmitCustomTypeFuncDecl(sb);
    sb.Append("\n");
    EmitEndNamespace(sb);
    sb.Append("\n");
    EmitTailMacro(sb, marcoName);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCustomTypesCodeEmitter::EmitHeaderFileInclusions(StringBuilder& sb)
{
    EmitCustomTypesStdlibInclusions(sb);
    sb.Append("#include <message_parcel.h>\n");
    EmitImportInclusions(sb);
}

void CppCustomTypesCodeEmitter::EmitCustomTypesStdlibInclusions(StringBuilder& sb)
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

void CppCustomTypesCodeEmitter::EmitImportInclusions(StringBuilder& sb)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> importAst = importPair.second;
        String fileName = FileName(importAst->GetFullName());
        sb.Append("#include ").AppendFormat("\"%s.h\"\n", fileName.string());
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeDecls(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        EmitCustomTypeDecl(sb, type);
        if (i + 1 < ast_->GetTypeDefinitionNumber()) {
            sb.Append("\n");
        }
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeDecl(StringBuilder& sb, const AutoPtr<ASTType>& type)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_ENUM: {
            AutoPtr<ASTEnumType> enumType = dynamic_cast<ASTEnumType*>(type.Get());
            sb.Append(enumType->EmitCppTypeDecl()).Append("\n");
            break;
        }
        case TypeKind::TYPE_STRUCT: {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            sb.Append(structType->EmitCppTypeDecl()).Append("\n");
            break;
        }
        case TypeKind::TYPE_UNION: {
            AutoPtr<ASTUnionType> unionType = dynamic_cast<ASTUnionType*>(type.Get());
            sb.Append(unionType->EmitCppTypeDecl()).Append("\n");
            break;
        }
        default:
            break;
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeFuncDecl(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            EmitCustomTypeMarshallingDecl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeUnmarshallingDecl(sb, structType);
            if (i + 1 < ast_->GetTypeDefinitionNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeMarshallingDecl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("bool %sBlockMarshalling(OHOS::MessageParcel &data, const %s& %s);\n",
        type->GetName().string(), type->EmitCppType().string(), objName.string());
}

void CppCustomTypesCodeEmitter::EmitCustomTypeUnmarshallingDecl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("bool %sBlockUnmarshalling(OHOS::MessageParcel &data, %s& %s);\n",
        type->GetName().string(), type->EmitCppType().string(), objName.string());
}

void CppCustomTypesCodeEmitter::EmitCustomTypesSourceFile()
{
    String filePath = String::Format("%s%s.cpp", directory_.string(), FileName(infName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitSourceFileInclusions(sb);
    sb.Append("\n");
    EmitBeginNamespace(sb);
    sb.Append("\n");
    EmitCustomTypeDataProcess(sb);
    sb.Append("\n");
    EmitEndNamespace(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CppCustomTypesCodeEmitter::EmitSourceFileInclusions(StringBuilder& sb)
{
    sb.AppendFormat("#include \"%s.h\"\n", FileName(infName_).string());
    EmitSourceStdlibInclusions(sb);
}

void CppCustomTypesCodeEmitter::EmitSourceStdlibInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_log.h>\n");
    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT || type->GetTypeKind() == TypeKind::TYPE_UNION) {
            sb.Append("#include <securec.h>\n");
            break;
        }
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeDataProcess(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            EmitCustomTypeMarshallingImpl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeUnmarshallingImpl(sb, structType);
            if (i + 1 < ast_->GetTypeDefinitionNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CppCustomTypesCodeEmitter::EmitCustomTypeMarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");

    sb.AppendFormat("bool %sBlockMarshalling(OHOS::MessageParcel& data, const %s& %s)\n",
        type->EmitCppType().string(), type->EmitCppType().string(), objName.string());
    sb.Append("{\n");
    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String memberName = type->GetMemberName(i);

        String name = String::Format("%s.%s", objName.string(), memberName.string());
        memberType->EmitCppMarshalling("data", name, sb, TAB);
        if (i + 1 < type->GetMemberNumber()) {
            sb.Append("\n");
        }
    }

    sb.Append(TAB).Append("return true;\n");
    sb.Append("}\n");
}

void CppCustomTypesCodeEmitter::EmitCustomTypeUnmarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");

    sb.AppendFormat("bool %sBlockUnmarshalling(OHOS::MessageParcel& data, %s& %s)\n",
        type->GetName().string(), type->EmitCppType().string(), objName.string());
    sb.Append("{\n");

    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String memberName = type->GetMemberName(i);
        String name = String::Format("%s.%s", objName.string(), memberName.string());

        if (i > 0 &&
            (memberType->GetTypeKind() == TypeKind::TYPE_STRUCT
            || memberType->GetTypeKind() == TypeKind::TYPE_UNION
            || memberType->GetTypeKind() == TypeKind::TYPE_ARRAY
            || memberType->GetTypeKind() == TypeKind::TYPE_LIST)) {
            sb.Append("\n");
        }

        if (memberType->GetTypeKind() == TypeKind::TYPE_UNION) {
            String cpName = String::Format("%sCp", memberName.string());
            memberType->EmitCppUnMarshalling("data", cpName, sb, TAB, false);
            sb.Append(TAB).AppendFormat("(void)memcpy_s(&%s, sizeof(%s), %s, sizeof(%s));\n", name.string(),
                memberType->EmitCppType().string(), cpName.string(), memberType->EmitCppType().string());
        } else {
            memberType->EmitCppUnMarshalling("data", name, sb, TAB, false);
        }
    }

    sb.Append(TAB).AppendFormat("return true;\n", objName.string());
    sb.Append("}\n");
}

void CppCustomTypesCodeEmitter::EmitBeginNamespace(StringBuilder& sb)
{
    String nspace = ast_->GetPackageName();
    int index = nspace.IndexOf('.');
    while (index != -1) {
        sb.AppendFormat("namespace %s {\n", nspace.Substring(0, index).string());
        nspace = nspace.Substring(index + 1);
        index = nspace.IndexOf('.');
    }

    if (!nspace.IsEmpty()) {
        sb.AppendFormat("namespace %s {\n", nspace.string());
    }
}

void CppCustomTypesCodeEmitter::EmitEndNamespace(StringBuilder& sb)
{
    String nspace = ast_->GetPackageName();
    nspace = nspace.Substring(0, nspace.GetLength() - 1);
    while (!nspace.IsEmpty()) {
        int index = nspace.LastIndexOf('.');
        sb.AppendFormat("} // %s\n",
            (index != -1) ? nspace.Substring(index + 1, nspace.GetLength()).string() : nspace.string());
        nspace = nspace.Substring(0, index);
    }
}
} // namespace HDI
} // namespace OHOS