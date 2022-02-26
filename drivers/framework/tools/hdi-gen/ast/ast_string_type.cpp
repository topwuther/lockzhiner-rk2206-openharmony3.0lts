/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_string_type.h"

namespace OHOS {
namespace HDI {
bool ASTStringType::IsStringType()
{
    return true;
}

String ASTStringType::ToString()
{
    return "String";
}

TypeKind ASTStringType::GetTypeKind()
{
    return TypeKind::TYPE_STRING;
}

String ASTStringType::EmitCType(TypeMode mode) const
{
    switch (mode) {
        case TypeMode::NO_MODE:
            return "char*";
        case TypeMode::PARAM_IN:
            return "const char*";
        case TypeMode::PARAM_OUT:
            return "char**";
        case TypeMode::LOCAL_VAR:
            return "char*";
        default:
            return "unknow type";
    }
}

String ASTStringType::EmitCppType(TypeMode mode) const
{
    switch (mode) {
        case TypeMode::NO_MODE:
            return "std::string";
        case TypeMode::PARAM_IN:
            return "const std::string&";
        case TypeMode::PARAM_OUT:
            return "std::string&";
        case TypeMode::LOCAL_VAR:
            return "std::string";
        default:
            return "unknow type";
    }
}

String ASTStringType::EmitJavaType(TypeMode mode, bool isInnerType) const
{
    return "String";
}

void ASTStringType::EmitCProxyWriteVar(const String& parcelName, const String& name, const String& gotoLabel,
    StringBuilder& sb, const String& prefix) const
{
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteString(%s, %s)) {\n",
        parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCStubWriteVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix) const
{
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteString(%s, %s)) {\n",
        parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCProxyReadVar(const String& parcelName, const String& name, bool isInnerType,
    const String& gotoLabel, StringBuilder& sb, const String& prefix) const
{
    sb.Append(prefix).AppendFormat("const char *%s = HdfSbufReadString(%s);\n",
        name.string(), parcelName.string());
    sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).AppendFormat("goto %s;\n", gotoLabel.string());
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCStubReadVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix) const
{
    sb.Append(prefix).AppendFormat("const char *%s = HdfSbufReadString(%s);\n",
        name.string(), parcelName.string());
    sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("ec = HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCppWriteVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, unsigned int innerLevel) const
{
    sb.Append(prefix).AppendFormat("if (!%s.WriteString(%s)) {\n", parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return HDF_ERR_INVALID_PARAM;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCppReadVar(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, bool initVariable, unsigned int innerLevel) const
{
    if (initVariable) {
        sb.Append(prefix).AppendFormat("%s %s = %s.ReadString();\n",
            EmitCppType().string(), name.string(), parcelName.string());
    } else {
        sb.Append(prefix).AppendFormat("%s = %s.ReadString();\n", name.string(), parcelName.string());
    }
}

void ASTStringType::EmitCMarshalling(const String& name, StringBuilder& sb, const String& prefix) const
{
    sb.Append(prefix).AppendFormat("if (!HdfSbufWriteString(data, %s)) {\n", name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return false;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCUnMarshalling(const String& name, StringBuilder& sb, const String& prefix,
    std::vector<String>& freeObjStatements) const
{
    sb.Append(prefix).AppendFormat("const char *%s = HdfSbufReadString(data);\n", name.string());
    sb.Append(prefix).AppendFormat("if (%s == NULL) {\n", name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: read %s failed!\", __func__);\n", name.string());
    EmitFreeStatements(freeObjStatements, sb, prefix + TAB);
    sb.Append(prefix + TAB).Append("goto errors;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCppMarshalling(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, unsigned int innerLevel) const
{
    sb.Append(prefix).AppendFormat("if (!%s.WriteString(%s)) {\n", parcelName.string(), name.string());
    sb.Append(prefix + TAB).AppendFormat(
        "HDF_LOGE(\"%%{public}s: write %s failed!\", __func__);\n", name.string());
    sb.Append(prefix + TAB).Append("return false;\n");
    sb.Append(prefix).Append("}\n");
}

void ASTStringType::EmitCppUnMarshalling(const String& parcelName, const String& name, StringBuilder& sb,
    const String& prefix, bool emitType, unsigned int innerLevel) const
{
    if (emitType) {
        sb.Append(prefix).AppendFormat("%s %s = %s.ReadString();\n",
            EmitCppType().string(), name.string(), parcelName.string());
    } else {
        sb.Append(prefix).AppendFormat("%s = %s.ReadString();\n",
            name.string(), parcelName.string());
    }
}
} // namespace HDI
} // namespace OHOS