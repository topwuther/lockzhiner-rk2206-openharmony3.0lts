/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/c_custom_types_code_emitter.h"
#include "util/file.h"

namespace OHOS {
namespace HDI {
void CCustomTypesCodeEmitter::EmitCode()
{
    EmitCustomTypesHeaderFile();
    EmitCustomTypesSourceFile();
}

void CCustomTypesCodeEmitter::EmitCustomTypesHeaderFile()
{
    String filePath = String::Format("%s%s.h", directory_.string(), FileName(infName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitHeadMacro(sb, infName_);
    sb.Append("\n");
    EmitHeaderInclusions(sb);
    sb.Append("\n");
    EmitHeadExternC(sb);
    sb.Append("\n");
    EmitCustomTypeDecls(sb);
    sb.Append("\n");
    EmitCustomTypeFuncDecl(sb);
    sb.Append("\n");
    EmitTailExternC(sb);
    sb.Append("\n");
    EmitTailMacro(sb, infName_);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CCustomTypesCodeEmitter::EmitHeaderInclusions(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            sb.Append("#include <hdf_sbuf.h>\n");
            break;
        }
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeDecls(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        EmitCustomTypeDecl(sb, type);
        if (i + 1 < ast_->GetTypeDefinitionNumber()) {
            sb.Append("\n");
        }
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeDecl(StringBuilder& sb, const AutoPtr<ASTType>& type)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_ENUM: {
            AutoPtr<ASTEnumType> enumType = dynamic_cast<ASTEnumType*>(type.Get());
            sb.Append(enumType->EmitCTypeDecl()).Append("\n");
            break;
        }
        case TypeKind::TYPE_STRUCT: {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            sb.Append(structType->EmitCTypeDecl()).Append("\n");
            break;
        }
        case TypeKind::TYPE_UNION: {
            AutoPtr<ASTUnionType> unionType = dynamic_cast<ASTUnionType*>(type.Get());
            sb.Append(unionType->EmitCTypeDecl()).Append("\n");
            break;
        }
        default:
            break;
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeFuncDecl(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            EmitCustomTypeMarshallingDecl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeUnmarshallingDecl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeFreeDecl(sb, structType);
            if (i + 1 < ast_->GetTypeDefinitionNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeMarshallingDecl(StringBuilder& sb,
    const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("bool %sBlockMarshalling(struct HdfSBuf *data, const %s *%s);\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
}

void CCustomTypesCodeEmitter::EmitCustomTypeUnmarshallingDecl(StringBuilder& sb,
    const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("bool %sBlockUnmarshalling(struct HdfSBuf *data, %s *%s);\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
}

void CCustomTypesCodeEmitter::EmitCustomTypeFreeDecl(StringBuilder& sb,
    const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("void %sFree(%s *%s, bool freeSelf);\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
}

void CCustomTypesCodeEmitter::EmitCustomTypesSourceFile()
{
    String filePath = String::Format("%s%s.c", directory_.string(), FileName(infName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitSoucreIncludsions(sb);
    sb.Append("\n");
    EmitCustomTypeDataProcess(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void CCustomTypesCodeEmitter::EmitSoucreIncludsions(StringBuilder& sb)
{
    sb.AppendFormat("#include \"%s.h\"\n", FileName(infName_).string());
    EmitSourceStdlibInclusions(sb);
}

void CCustomTypesCodeEmitter::EmitSourceStdlibInclusions(StringBuilder& sb)
{
    sb.Append("#include <hdf_log.h>\n");
    sb.Append("#include <osal_mem.h>\n");

    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT
            || type->GetTypeKind() == TypeKind::TYPE_UNION) {
            sb.Append("#include <securec.h>\n");
            break;
        }
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeDataProcess(StringBuilder& sb)
{
    for (size_t i = 0; i < ast_->GetTypeDefinitionNumber(); i++) {
        AutoPtr<ASTType> type = ast_->GetTypeDefintion(i);
        if (type->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            AutoPtr<ASTStructType> structType = dynamic_cast<ASTStructType*>(type.Get());
            EmitCustomTypeMarshallingImpl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeUnmarshallingImpl(sb, structType);
            sb.Append("\n");
            EmitCustomTypeFreeImpl(sb, structType);
            if (i + 1 < ast_->GetTypeDefinitionNumber()) {
                sb.Append("\n");
            }
        }
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeMarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("bool %sBlockMarshalling(struct HdfSBuf *data, const %s *%s)\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
    sb.Append("{\n");
    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        String memberName = type->GetMemberName(i);
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String name = String::Format("%s->%s", objName.string(), memberName.string());
        memberType->EmitCMarshalling(name, sb, TAB);
        sb.Append("\n");
    }

    sb.Append(TAB).Append("return true;\n");
    sb.Append("}\n");
}

void CCustomTypesCodeEmitter::EmitCustomTypeUnmarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    freeObjStatements_.clear();

    sb.AppendFormat("bool %sBlockUnmarshalling(struct HdfSBuf *data, %s *%s)\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
    sb.Append("{\n");

    sb.Append(TAB).AppendFormat("if (%s == NULL) {\n", objName.string());
    sb.Append(TAB).Append(TAB).Append("return false;\n");
    sb.Append(TAB).Append("}\n");

    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String memberName = type->GetMemberName(i);
        String name = String::Format("%s->%s", objName.string(), memberName.string());

        if (memberType->GetTypeKind() == TypeKind::TYPE_STRING) {
            String tmpName = String::Format("%sCp", memberName.string());
            memberType->EmitCUnMarshalling(tmpName, sb, TAB, freeObjStatements_);
            sb.Append(TAB).AppendFormat("%s = strdup(%s);\n", name.string(), tmpName.string());
            sb.Append(TAB).AppendFormat("if (%s == NULL) {\n", name.string());
            sb.Append(TAB).Append(TAB).Append("goto errors;\n");
            sb.Append(TAB).Append("}\n");
            sb.Append("\n");
        } else if (memberType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
            String paramName = String::Format("&%s", name.string());
            memberType->EmitCUnMarshalling(paramName, sb, TAB, freeObjStatements_);
            sb.Append("\n");
        } else if (memberType->GetTypeKind() == TypeKind::TYPE_UNION) {
            String tmpName = String::Format("%sCp", memberName.string());
            memberType->EmitCUnMarshalling(tmpName, sb, TAB, freeObjStatements_);
            sb.Append(TAB).AppendFormat("(void)memcpy_s(&%s, sizeof(%s), %s, sizeof(%s));\n",
                name.string(), memberType->EmitCType().string(),
                tmpName.string(), memberType->EmitCType().string());
            sb.Append("\n");
        } else if (memberType->GetTypeKind() == TypeKind::TYPE_ARRAY) {
            String tmpName = String::Format("%sCp", memberName.string());
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(memberType.Get());
            AutoPtr<ASTType> elementType = arrayType->GetElementType();
            sb.Append(TAB).AppendFormat("%s* %s = NULL;\n", elementType->EmitCType().string(), tmpName.string());
            sb.Append(TAB).AppendFormat("uint32_t %sLen = 0;\n", tmpName.string());
            memberType->EmitCUnMarshalling(tmpName, sb, TAB, freeObjStatements_);
            sb.Append(TAB).AppendFormat("%s = %s;\n", name.string(), tmpName.string());
            sb.Append(TAB).AppendFormat("%sLen = %sLen;\n", name.string(), tmpName.string());
            sb.Append("\n");
        } else if (memberType->GetTypeKind() == TypeKind::TYPE_LIST) {
            String tmpName = String::Format("%sCp", memberName.string());
            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(memberType.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();
            sb.Append(TAB).AppendFormat("%s* %s = NULL;\n", elementType->EmitCType().string(), tmpName.string());
            sb.Append(TAB).AppendFormat("uint32_t %sLen = 0;\n", tmpName.string());
            memberType->EmitCUnMarshalling(tmpName, sb, TAB, freeObjStatements_);
            sb.Append(TAB).AppendFormat("%s = %s;\n", name.string(), tmpName.string());
            sb.Append(TAB).AppendFormat("%sLen = %sLen;\n", name.string(), tmpName.string());
            sb.Append("\n");
        } else {
            memberType->EmitCUnMarshalling(name, sb, TAB, freeObjStatements_);
            sb.Append("\n");
        }
    }

    sb.Append(TAB).AppendFormat("return true;\n");
    sb.Append("errors:\n");
    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String memberName = type->GetMemberName(i);
        String name = String::Format("%s->%s", objName.string(), memberName.string());
        EmitError(name, memberType, sb, TAB);
    }

    sb.Append(TAB).Append("return false;\n");
    sb.Append("}\n");
}


void CCustomTypesCodeEmitter::EmitError(const String& name, const AutoPtr<ASTType>& type,
    StringBuilder& sb, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_STRING: {
            sb.Append(prefix).AppendFormat("if (%s != NULL) {\n", name.string());
            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n\n");
            break;
        }
        case TypeKind::TYPE_ARRAY: {
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(type.Get());
            AutoPtr<ASTType> elementType = arrayType->GetElementType();
            String lenName = String::Format("%sLen", name.string());

            sb.Append(prefix).AppendFormat("if (%s > 0 && %s != NULL) {\n", lenName.string(), name.string());
            if (elementType->GetTypeKind() == TypeKind::TYPE_STRING
                || elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %s; i++) {\n", lenName.string());
                String elementName = String::Format("(%s)[i]", name.string());

                if (elementType->GetTypeKind() == TypeKind::TYPE_STRING) {
                    sb.Append(prefix + TAB + TAB).AppendFormat("if (%s != NULL) {\n", elementName.string());
                    sb.Append(prefix + TAB + TAB + TAB).AppendFormat("OsalMemFree(%s);\n", elementName.string());
                    sb.Append(prefix + TAB + TAB).Append("}\n");
                } else if (elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                    sb.Append(prefix + TAB + TAB).AppendFormat("%sFree(&(%s), false);\n",
                        elementType->GetName().string(), elementName.string());
                }

                sb.Append(prefix + TAB).Append("}\n");
            }

            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n");
            sb.Append("\n");
            break;
        }
        case TypeKind::TYPE_LIST: {
            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(type.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();
            String lenName = String::Format("%sLen", name.string());

            sb.Append(prefix).AppendFormat("if (%s > 0 && %s != NULL) {\n", lenName.string(), name.string());
            if (elementType->GetTypeKind() == TypeKind::TYPE_STRING
                || elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %s; i++) {\n", lenName.string());
                String elementName = String::Format("(%s)[i]", name.string());

                if (elementType->GetTypeKind() == TypeKind::TYPE_STRING) {
                    sb.Append(prefix + TAB + TAB).AppendFormat("if (%s != NULL) {\n", elementName.string());
                    sb.Append(prefix + TAB + TAB + TAB).AppendFormat("OsalMemFree(%s);\n", elementName.string());
                    sb.Append(prefix + TAB + TAB).Append("}\n");
                } else if (elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                    sb.Append(prefix + TAB + TAB).AppendFormat("%sFree(&(%s), false);\n",
                        elementType->GetName().string(), elementName.string());
                }

                sb.Append(prefix + TAB).Append("}\n");
            }

            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n");
            sb.Append("\n");
            break;
        }
        case TypeKind::TYPE_STRUCT: {
            sb.Append(prefix).AppendFormat("%sFree(&%s, false);\n", type->GetName().string(), name.string());
            sb.Append(prefix).Append("\n");
            break;
        }
        default:
            break;
    }
}

void CCustomTypesCodeEmitter::EmitCustomTypeFreeImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type)
{
    String objName("dataBlock");
    sb.AppendFormat("void %sFree(%s *%s, bool freeSelf)\n",
        type->GetName().string(), type->EmitCType().string(), objName.string());
    sb.Append("{\n");
    sb.Append(TAB).AppendFormat("if (%s == NULL) {\n", objName.string());
    sb.Append(TAB).Append(TAB).Append("return;\n");
    sb.Append(TAB).Append("}\n");
    sb.Append("\n");

    for (size_t i = 0; i < type->GetMemberNumber(); i++) {
        AutoPtr<ASTType> memberType = type->GetMemberType(i);
        String memberName = type->GetMemberName(i);
        String name = String::Format("%s->%s", objName.string(), memberName.string());
        EmitCustomTypeMemberFree(sb, name, memberType, TAB);
    }

    sb.Append(TAB).Append("if (freeSelf) {\n");
    sb.Append(TAB).Append(TAB).Append("OsalMemFree(dataBlock);\n");
    sb.Append(TAB).Append("}\n");

    sb.Append("}\n");
}

void CCustomTypesCodeEmitter::EmitCustomTypeMemberFree(StringBuilder& sb, const String& name,
    const AutoPtr<ASTType>& type, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_STRING: {
            sb.Append(prefix).AppendFormat("if (%s != NULL) {\n", name.string());
            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n\n");
            break;
        }
        case TypeKind::TYPE_ARRAY: {
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(type.Get());
            AutoPtr<ASTType> elementType = arrayType->GetElementType();

            sb.Append(prefix).AppendFormat("if (%sLen > 0 && %s != NULL) {\n", name.string(), name.string());
            if (elementType->GetTypeKind() == TypeKind::TYPE_STRING) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string());
                sb.Append(prefix + TAB + TAB).AppendFormat("OsalMemFree(%s[i]);\n", name.string());
                sb.Append(prefix + TAB).Append("}\n");
            } else if (elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string());
                sb.Append(prefix + TAB + TAB).AppendFormat("%sFree(%s, false);\n",
                    elementType->GetName().string(), name.string());
                sb.Append(prefix + TAB).Append("}\n");
            }
            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n\n");
            break;
        }
        case TypeKind::TYPE_LIST: {
            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(type.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();

            sb.Append(prefix).AppendFormat("if (%sLen > 0 && %s != NULL) {\n", name.string(), name.string());
            if (elementType->GetTypeKind() == TypeKind::TYPE_STRING) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string());
                sb.Append(prefix + TAB + TAB).AppendFormat("OsalMemFree(%s[i]);\n", name.string());
                sb.Append(prefix + TAB).Append("}\n");
            } else if (elementType->GetTypeKind() == TypeKind::TYPE_STRUCT) {
                sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string());
                sb.Append(prefix + TAB + TAB).AppendFormat("%sFree(%s, false);\n",
                    elementType->GetName().string(), name.string());
                sb.Append(prefix + TAB).Append("}\n");
            }
            sb.Append(prefix + TAB).AppendFormat("OsalMemFree(%s);\n", name.string());
            sb.Append(prefix).Append("}\n\n");
            break;
        }
        case TypeKind::TYPE_STRUCT: {
            sb.Append(prefix).AppendFormat("%sFree(&%s, false);\n\n", type->GetName().string(), name.string());
            break;
        }
        default:
            break;
    }
}
} // namespace HDI
} // namespace OHOS