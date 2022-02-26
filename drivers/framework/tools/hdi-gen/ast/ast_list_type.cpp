/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_list_type.h"

namespace OHOS {
namespace HDI {
bool ASTListType::IsListType()
{
    return true;
}

String ASTListType::ToString()
{
    return String::Format("List<%s>", elementType_->ToString().string());
}

TypeKind ASTListType::GetTypeKind()
{
    return TypeKind::TYPE_LIST;
}

String ASTListType::EmitCType(TypeMode mode) const
{
    switch (mode) {
        case TypeMode::NO_MODE:
            return String::Format("%s*", elementType_->EmitCType(TypeMode::NO_MODE).string());
        case TypeMode::PARAM_IN: {
            if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
                return String::Format("%s*", elementType_->EmitCType(TypeMode::NO_MODE).string());
            } else if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT
                || elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
                return String::Format("%s", elementType_->EmitCType(TypeMode::PARAM_IN).string());
            } else {
                return String::Format("%s*", elementType_->EmitCType(TypeMode::PARAM_IN).string());
            }
        }
        case TypeMode::PARAM_OUT: {
            if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT
                || elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
                return elementType_->EmitCType(TypeMode::PARAM_OUT);
            } else {
                return String::Format("%s*", elementType_->EmitCType(TypeMode::PARAM_OUT).string());
            }
        }
        case TypeMode::LOCAL_VAR: {
            if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT
                || elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
                return String::Format("%s", elementType_->EmitCType(TypeMode::LOCAL_VAR).string());
            } else {
                return String::Format("%s*", elementType_->EmitCType(TypeMode::LOCAL_VAR).string());
            }
        }
        default:
            return "unknow type";
    }
}

String ASTListType::EmitCppType(TypeMode mode) const
{
    switch (mode) {
        case TypeMode::NO_MODE:
            return String::Format("std::vector<%s>", elementType_->EmitCppType(TypeMode::NO_MODE).string());
        case TypeMode::PARAM_IN:
            return String::Format("const std::vector<%s>&", elementType_->EmitCppType(TypeMode::NO_MODE).string());
        case TypeMode::PARAM_OUT:
            return String::Format("std::vector<%s>&", elementType_->EmitCppType(TypeMode::NO_MODE).string());
        case TypeMode::LOCAL_VAR:
            return String::Format("std::vector<%s>", elementType_->EmitCppType(TypeMode::NO_MODE).string());
        default:
            return "unknow type";
    }
}

String ASTListType::EmitJavaType(TypeMode mode, bool isInnerType) const
{
    return String::Format("List<%s>", elementType_->EmitJavaType(mode, true).string());
}

void ASTListType::EmitCProxyWriteVar(const String& parcelName, const String& name, const String& gotoLabel,
    StringBuilder& sb, const String& prefix) const
{
    String lenName = String::Format("%sLen", name.string());
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteUint32(%s, %s)) {\n",
        parcelName.string(), lenName.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
    sb.Append(prefix).Append("}\n");
    sb.Append(prefix).AppendFormat("for (uint32_t i = 0; i < %s; i++) {\n", lenName.string());

    String elementName;
    String elementReadName;

    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT
        || elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        elementName = String::Format("%s[i]", name.string());
        elementReadName = "&" + elementName;
    } else {
        elementName = String::Format("%s[i]", name.string());
        elementReadName = elementName;
    }

    elementType_->EmitCProxyWriteVar(parcelName, elementReadName, gotoLabel, sb, prefix + TAB);
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCStubWriteVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix) const
{
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteUint32(%s, %sLen)) {\n",
        parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string(), name.string());

    String element;
    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT
        || elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        element = String::Format("&%s[i]", name.string());
    } else {
        element = String::Format("%s[i]", name.string());
    }
    elementType_->EmitCStubWriteVar(parcelName, element, sb, prefix + TAB);
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCProxyReadVar(const String& parcelName, const String& name, bool isInnerType,
    const String& gotoLabel, StringBuilder& sb, const String& prefix) const
{
String lenName = String::Format("%sLen", name.string());
    sb.Append(prefix).AppendFormat("if (!HdfSbufReadUint32(%s, %s)) {\n",
        parcelName.string(), lenName.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s size failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
    sb.Append(prefix).Append("}\n");

    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
        sb.Append(prefix).AppendFormat("*%s = (%s*)OsalMemAlloc(sizeof(%s) * (*%s));\n",
            name.string(), elementType_->EmitCType().string(), elementType_->EmitCType().string(),
            lenName.string());
        sb.Append(prefix).AppendFormat("if (*%s == NULL) {\n", name.string());
        sb.Append(prefix + TAB).AppendFormat("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
        sb.Append(prefix).AppendFormat("}\n");
    } else {
        sb.Append(prefix).AppendFormat("*%s = (%s*)OsalMemCalloc(sizeof(%s) * (*%s));\n",
            name.string(), elementType_->EmitCType().string(), elementType_->EmitCType().string(),
            lenName.string());
        sb.Append(prefix).AppendFormat("if (*%s == NULL) {\n", name.string());
        sb.Append(prefix + TAB).AppendFormat("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
        sb.Append(prefix).AppendFormat("}\n");
    }
    sb.Append(prefix).AppendFormat("for (uint32_t i = 0; i < *%s; i++) {\n", lenName.string());
    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
        String cpName = String::Format("%sCp", name.string());
        elementType_->EmitCProxyReadVar(parcelName, cpName, true, gotoLabel, sb, prefix + TAB);
        sb.Append(prefix).Append(TAB).AppendFormat("(*%s)[i] = strdup(%sCp);\n",
            name.string(), name.string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        String element = String::Format("&(*%s)[i]", name.string());
        elementType_->EmitCProxyReadVar(parcelName, element, true, gotoLabel, sb, prefix + TAB);
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        String element = String::Format("&(*%s)[i]", name.string());
        String elementCp = String::Format("%sElementCp", name.string());
        elementType_->EmitCProxyReadVar(parcelName, elementCp, true, gotoLabel, sb, prefix + TAB);
        sb.Append(prefix + TAB).AppendFormat("(void)memcpy_s(%s, sizeof(%s), %s, sizeof(%s));\n",
            element.string(), elementType_->EmitCType().string(), elementCp.string(),
            elementType_->EmitCType().string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_FILEDESCRIPTOR) {
        String element = String::Format("(*%s)[i]", name.string());
        elementType_->EmitCProxyReadVar(parcelName, element, true, gotoLabel, sb, prefix + TAB);
    } else {
        String element = String::Format("&(*%s)[i]", name.string());
        elementType_->EmitCProxyReadVar(parcelName, element, true, gotoLabel, sb, prefix + TAB);
    }
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCStubReadVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix) const
{
    String lenName = String::Format("%sLen", name.string());
    sb.Append(prefix).AppendFormat("if (!HdfSbufReadUint32(%s, &%s)) {\n",
        parcelName.string(), lenName.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s size failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");

    sb.Append(prefix).AppendFormat("if (%s > 0) {\n", lenName.string());
    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
        sb.Append(prefix + TAB).AppendFormat("%s = (%s*)OsalMemAlloc(sizeof(%s) * (%s));\n", name.string(),
            elementType_->EmitCType().string(), elementType_->EmitCType().string(), lenName.string());
        sb.Append(prefix + TAB).AppendFormat("if (%s == NULL) {\n", name.string());
        sb.Append(prefix + TAB + TAB).AppendFormat("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("goto errors;\n");
        sb.Append(prefix + TAB).AppendFormat("}\n");
    } else {
        sb.Append(prefix + TAB).AppendFormat("%s = (%s*)OsalMemCalloc(sizeof(%s) * (%s));\n",
            name.string(), elementType_->EmitCType().string(), elementType_->EmitCType().string(),
            lenName.string());
        sb.Append(prefix + TAB).AppendFormat("if (%s == NULL) {\n", name.string());
        sb.Append(prefix + TAB + TAB).AppendFormat("ec = HDF_ERR_MALLOC_FAIL;\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("goto errors;\n");
        sb.Append(prefix + TAB).AppendFormat("}\n");
    }

    sb.Append(prefix + TAB).AppendFormat("for (uint32_t i = 0; i < %s; i++) {\n", lenName.string());

    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
        String element = String::Format("%sCp", name.string());
        elementType_->EmitCStubReadVar(parcelName, element, sb, prefix + TAB + TAB);
        sb.Append(prefix + TAB + TAB).AppendFormat("%s[i] = strdup(%sCp);\n", name.string(), name.string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        String element = String::Format("&%s[i]", name.string());
        elementType_->EmitCStubReadVar(parcelName, element, sb, prefix + TAB + TAB);
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        String element = String::Format("%s[i]", name.string());
        String elementCp = String::Format("%sElementCp", name.string());
        elementType_->EmitCStubReadVar(parcelName, elementCp, sb, prefix + TAB + TAB);
        sb.Append(prefix + TAB + TAB).AppendFormat("(void)memcpy_s(&%s, sizeof(%s), %s, sizeof(%s));\n",
            element.string(), elementType_->EmitCType().string(), elementCp.string(),
            elementType_->EmitCType().string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_FILEDESCRIPTOR) {
        String element = String::Format("%s[i]", name.string());
        elementType_->EmitCStubReadVar(parcelName, element, sb, prefix + TAB + TAB);
    } else {
        String element = String::Format("&%s[i]", name.string());
        elementType_->EmitCStubReadVar(parcelName, element, sb, prefix + TAB + TAB);
    }
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCppWriteVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, unsigned int innerLevel) const
{
    sb.Append(prefix).AppendFormat("if (!%s.WriteUint32(%s.size())) {\n", parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s.size() failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix).Append("}\n");
    String elementName = String::Format("it%d", innerLevel++);
    sb.Append(prefix).AppendFormat("for (auto %s : %s) {\n", elementName.string(), name.string());

    elementType_->EmitCppWriteVar(parcelName, elementName, sb, prefix + TAB, innerLevel);
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCppReadVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, bool initVariable, unsigned int innerLevel) const
{
    if (initVariable) {
        sb.Append(prefix).AppendFormat("%s %s;\n", EmitCppType().string(), name.string());
    }
    sb.Append(prefix).AppendFormat("uint32_t %sSize = %s.ReadUint32();\n", name.string(), parcelName.string());
    sb.Append(prefix).AppendFormat("for (uint32_t i%d = 0; i%d < %sSize; ++i%d) {\n",
        innerLevel, innerLevel, name.string(), innerLevel);

    String valueName = String::Format("value%d", innerLevel++);
    elementType_->EmitCppReadVar(parcelName, valueName, sb, prefix + TAB, true, innerLevel);
    sb.Append(prefix + TAB).AppendFormat("%s.push_back(%s);\n", name.string(), valueName.string());
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCMarshalling(const String& name, StringBuilder& sb, const String& prefix) const
{
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteUint32(data, %sLen)) {\n", name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %sLen failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return false;\n");
    sb.Append(prefix).Append("}\n");
    sb.Append(prefix).AppendFormat("for (uint32_t i = 0; i < %sLen; i++) {\n", name.string());

    String elementName = String::Format("(%s)[i]", name.string());
    elementType_->EmitCMarshalling(elementName, sb, prefix + TAB);
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCUnMarshalling(const String& name, StringBuilder& sb, const String& prefix,
    std::vector<String>& freeObjStatements) const
{
    String lenName = String::Format("%sLen", name.string());
    sb.Append(prefix).AppendFormat("if (!HdfSbufReadUint32(data, &%s)) {\n", lenName.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n", lenName.string());
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");

    sb.Append(prefix).AppendFormat("if (%s > 0) {\n", lenName.string());
    String newPrefix = prefix + TAB;

    sb.Append(newPrefix).AppendFormat("%s = (%s*)OsalMemCalloc(sizeof(%s) * %s);\n",
        name.string(), elementType_->EmitCType().string(), elementType_->EmitCType().string(), lenName.string());
    sb.Append(newPrefix).AppendFormat("if (%s == NULL) {\n", name.string());
    sb.Append(newPrefix + TAB).AppendFormat("goto errors;\n");
    sb.Append(newPrefix).Append("}\n");

    freeObjStatements.push_back(String::Format("OsalMemFree(%s);\n", name.string()));
    sb.Append(newPrefix).AppendFormat("for (uint32_t i = 0; i < %s; i++) {\n", lenName.string());
    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRING) {
        String element = String::Format("%sElement", name.string());
        elementType_->EmitCUnMarshalling(element, sb, newPrefix + TAB, freeObjStatements);
        sb.Append(newPrefix).Append(TAB).AppendFormat("%s[i] = strdup(%s);\n",
            name.string(), element.string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        String element = String::Format("&%s[i]", name.string());
        elementType_->EmitCUnMarshalling(element, sb, newPrefix + TAB, freeObjStatements);
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        String element = String::Format("%s[i]", name.string());
        String elementCp = String::Format("%sElementCp", name.string());
        elementType_->EmitCUnMarshalling(elementCp, sb, newPrefix + TAB, freeObjStatements);
        sb.Append(newPrefix + TAB).AppendFormat("(void)memcpy_s(&%s, sizeof(%s), %s, sizeof(%s));\n",
            element.string(), elementType_->EmitCType().string(), elementCp.string(),
            elementType_->EmitCType().string());
    } else {
        String element = String::Format("%s[i]", name.string());
        elementType_->EmitCUnMarshalling(element, sb, newPrefix + TAB, freeObjStatements);
    }
    sb.Append(newPrefix).Append("}\n");
    sb.Append(prefix).Append("}\n");
    freeObjStatements.pop_back();
}

void ASTListType::EmitCppMarshalling(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, unsigned int innerLevel) const
{
    sb.Append(prefix).AppendFormat("if (!%s.WriteUint32(%s.size())) {\n", parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s.size failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return false;\n");
    sb.Append(prefix).Append("}\n");
    String elementName = String::Format("it%d", innerLevel++);
    sb.Append(prefix).AppendFormat("for (auto %s : %s) {\n", elementName.string(), name.string());

    elementType_->EmitCppMarshalling(parcelName, elementName, sb, prefix + TAB, innerLevel);
    sb.Append(prefix).Append("}\n");
}

void ASTListType::EmitCppUnMarshalling(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, bool emitType, unsigned int innerLevel) const
{
    int index = name.IndexOf('.', 0);
    String memberName = name.Substring(index + 1);
    if (emitType) {
        sb.Append(prefix).AppendFormat("%s %s;\n", EmitCppType().string(), memberName.string());
    }
    sb.Append(prefix).AppendFormat("uint32_t %sSize = %s.ReadUint32();\n",
        memberName.string(), parcelName.string());
    sb.Append(prefix).AppendFormat("for (uint32_t i%d = 0; i%d < %sSize; ++i%d) {\n",
        innerLevel, innerLevel, memberName.string(), innerLevel);

    String valueName = String::Format("value%d", innerLevel++);
    if (elementType_->GetTypeKind() == TypeKind::TYPE_STRUCT) {
        sb.Append(prefix + TAB).AppendFormat("%s %s;\n",
            elementType_->EmitCppType().string(), valueName.string());
        elementType_->EmitCppUnMarshalling(parcelName, valueName, sb, prefix + TAB, true, innerLevel);
        sb.Append(prefix + TAB).AppendFormat("%s.push_back(%s);\n", name.string(), valueName.string());
    } else if (elementType_->GetTypeKind() == TypeKind::TYPE_UNION) {
        sb.Append(prefix + TAB).AppendFormat("%s %s;\n",
            elementType_->EmitCppType().string(), valueName.string());
        String cpName = String::Format("%sCp", valueName.string());
        elementType_->EmitCppUnMarshalling(parcelName, cpName, sb, prefix + TAB, true, innerLevel);
        sb.Append(prefix + TAB).AppendFormat("(void)memcpy_s(&%s, sizeof(%s), %s, sizeof(%s));\n",
            valueName.string(), elementType_->EmitCppType().string(), cpName.string(),
            elementType_->EmitCppType().string());
        sb.Append(prefix + TAB).AppendFormat("%s.push_back(%s);\n", name.string(), valueName.string());
    } else {
        elementType_->EmitCppUnMarshalling(parcelName, valueName, sb, prefix + TAB, true, innerLevel);
        sb.Append(prefix + TAB).AppendFormat("%s.push_back(%s);\n", name.string(), valueName.string());
    }
    sb.Append(prefix).Append("}\n");
}
} // namespace HDI
} // namespace OHOS