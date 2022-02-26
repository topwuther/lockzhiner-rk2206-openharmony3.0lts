/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ast/ast_parameter.h"
#include "ast/ast_array_type.h"
#include "ast/ast_list_type.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
String ASTParameter::Dump(const String& prefix)
{
    StringBuilder sb;

    sb.Append(prefix);
    sb.Append('[');

    if (attribute_ == ParamAttr::PARAM_IN) {
        sb.Append("in");
    } else {
        sb.Append("out");
    }

    sb.Append("] ");
    sb.Append(type_->ToString()).Append(' ');
    sb.Append(name_);

    return sb.ToString();
}

String ASTParameter::EmitCParameter()
{
    StringBuilder sb;

    switch (type_->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
        case TypeKind::TYPE_BYTE:
        case TypeKind::TYPE_SHORT:
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_LONG:
        case TypeKind::TYPE_UCHAR:
        case TypeKind::TYPE_USHORT:
        case TypeKind::TYPE_UINT:
        case TypeKind::TYPE_ULONG:
        case TypeKind::TYPE_FLOAT:
        case TypeKind::TYPE_DOUBLE:
        case TypeKind::TYPE_ENUM:
        case TypeKind::TYPE_FILEDESCRIPTOR:
        case TypeKind::TYPE_STRING:
        case TypeKind::TYPE_INTERFACE:
        case TypeKind::TYPE_STRUCT:
        case TypeKind::TYPE_UNION:
        case TypeKind::TYPE_VOID: {
            if (attribute_ == ParamAttr::PARAM_IN) {
                return String::Format("%s %s", type_->EmitCType(TypeMode::PARAM_IN).string(), name_.string());
            } else {
                return String::Format("%s %s", type_->EmitCType(TypeMode::PARAM_OUT).string(), name_.string());
            }
        }
        case TypeKind::TYPE_ARRAY:
        case TypeKind::TYPE_LIST: {
            StringBuilder paramStr;
            if (attribute_ == ParamAttr::PARAM_IN) {
                paramStr.AppendFormat("%s %s", type_->EmitCType(TypeMode::PARAM_IN).string(), name_.string());
            } else {
                paramStr.AppendFormat("%s %s", type_->EmitCType(TypeMode::PARAM_OUT).string(), name_.string());
            }

            if (attribute_ == ParamAttr::PARAM_IN) {
                paramStr.AppendFormat(", uint32_t %sLen", name_.string());
            } else {
                paramStr.AppendFormat(", uint32_t* %sLen", name_.string());
            }

            return paramStr.ToString();
        }
        default:
            return String::Format("unknow type %s", name_.string());
    }

    return sb.ToString();
}

String ASTParameter::EmitCppParameter()
{
    if (attribute_ == ParamAttr::PARAM_IN) {
        return String::Format("%s %s", type_->EmitCppType(TypeMode::PARAM_IN).string(), name_.string());
    } else {
        return String::Format("%s %s", type_->EmitCppType(TypeMode::PARAM_OUT).string(), name_.string());
    }
}

String ASTParameter::EmitJavaParameter()
{
    StringBuilder sb;
    switch (type_->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
        case TypeKind::TYPE_BYTE:
        case TypeKind::TYPE_SHORT:
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_LONG:
        case TypeKind::TYPE_UCHAR:
        case TypeKind::TYPE_USHORT:
        case TypeKind::TYPE_UINT:
        case TypeKind::TYPE_ULONG:
        case TypeKind::TYPE_FLOAT:
        case TypeKind::TYPE_DOUBLE:
        case TypeKind::TYPE_ENUM:
        case TypeKind::TYPE_FILEDESCRIPTOR:
        case TypeKind::TYPE_STRING:
        case TypeKind::TYPE_SEQUENCEABLE:
        case TypeKind::TYPE_INTERFACE:
        case TypeKind::TYPE_STRUCT:
        case TypeKind::TYPE_UNION:
        case TypeKind::TYPE_VOID:
        case TypeKind::TYPE_ARRAY:
        case TypeKind::TYPE_LIST:
        case TypeKind::TYPE_MAP: {
            return String::Format("%s %s", type_->EmitJavaType(TypeMode::NO_MODE, false).string(), name_.string());
        }
        default:
            return String::Format("unknow type %s", name_.string());
    }

    return sb.ToString();
}

String ASTParameter::EmitCLocalVar()
{
    StringBuilder sb;
    sb.AppendFormat("%s %s", type_->EmitCType(TypeMode::LOCAL_VAR).string(), name_.string());
    switch (type_->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(" = false");
            break;
        case TypeKind::TYPE_BYTE:
        case TypeKind::TYPE_SHORT:
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_LONG:
        case TypeKind::TYPE_UCHAR:
        case TypeKind::TYPE_USHORT:
        case TypeKind::TYPE_UINT:
        case TypeKind::TYPE_ULONG:
        case TypeKind::TYPE_FLOAT:
        case TypeKind::TYPE_DOUBLE:
            sb.Append(" = 0");
            break;
        case TypeKind::TYPE_STRING:
        case TypeKind::TYPE_ARRAY:
        case TypeKind::TYPE_LIST:
        case TypeKind::TYPE_STRUCT:
        case TypeKind::TYPE_UNION:
        case TypeKind::TYPE_INTERFACE:
            sb.Append(" = NULL");
            break;
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(" = -1");
            break;
        default:
            break;
    }
    sb.Append(";");
    return sb.ToString();
}

String ASTParameter::EmitCppLocalVar()
{
    StringBuilder sb;
    sb.AppendFormat("%s %s", type_->EmitCppType(TypeMode::LOCAL_VAR).string(), name_.string());
    switch (type_->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(" = false");
            break;
        case TypeKind::TYPE_BYTE:
        case TypeKind::TYPE_SHORT:
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_LONG:
        case TypeKind::TYPE_UCHAR:
        case TypeKind::TYPE_USHORT:
        case TypeKind::TYPE_UINT:
        case TypeKind::TYPE_ULONG:
        case TypeKind::TYPE_FLOAT:
        case TypeKind::TYPE_DOUBLE:
            sb.Append(" = 0");
            break;
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(" = -1");
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            sb.Append(" = nullptr");
            break;
        default:
            break;
    }
    sb.Append(";");
    return sb.ToString();
}

String ASTParameter::EmitJavaLocalVar()
{
    return "";
}
} // namespace HDI
} // namespace OHOS