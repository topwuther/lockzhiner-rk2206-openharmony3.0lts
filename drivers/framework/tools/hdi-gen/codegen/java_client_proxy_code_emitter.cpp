/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/java_client_proxy_code_emitter.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
void JavaClientProxyCodeEmitter::EmitCode()
{
    EmitProxyFile();
}

void JavaClientProxyCodeEmitter::EmitProxyFile()
{
    String filePath = String::Format("%s/%s.java", directory_.string(), FileName(proxyName_).string());
    if (!File::CreateParentDir(filePath)) {
        Logger::E("CppClientInterfaceCodeEmitter", "Create '%s' failed!", filePath.string());
        return;
    }

    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitLicense(sb);
    EmitPackage(sb);
    sb.Append("\n");
    EmitProxyImports(sb);
    sb.Append("\n");
    EmitProxyImpl(sb);

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void JavaClientProxyCodeEmitter::EmitProxyImports(StringBuilder& sb)
{
    EmitProxyCorelibImports(sb);
    EmitProxySelfDefinedTypeImports(sb);
    EmitProxyDBinderImports(sb);
}

void JavaClientProxyCodeEmitter::EmitProxyCorelibImports(StringBuilder& sb)
{
    bool includeList = false;
    bool includeMap = false;
    const AST::TypeStringMap& types = ast_->GetTypes();
    for (const auto& pair : types) {
        AutoPtr<ASTType> type = pair.second;
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_LIST: {
                if (!includeList) {
                    sb.Append("import java.util.List;\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::TYPE_MAP: {
                if (!includeMap) {
                    sb.Append("import java.util.Map;\n");
                    sb.Append("import java.util.HashMap;\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void JavaClientProxyCodeEmitter::EmitProxySelfDefinedTypeImports(StringBuilder& sb)
{
    for (const auto& importPair : ast_->GetImports()) {
        AutoPtr<AST> import = importPair.second;
        sb.AppendFormat("import %s;\n", import->GetFullName().string());
    }
}

void JavaClientProxyCodeEmitter::EmitProxyDBinderImports(StringBuilder& sb)
{
    sb.Append("import ohos.hiviewdfx.HiLog;\n");
    sb.Append("import ohos.hiviewdfx.HiLogLabel;\n");
    sb.Append("import ohos.rpc.IRemoteObject;\n");
    sb.Append("import ohos.rpc.RemoteException;\n");
    sb.Append("import ohos.rpc.MessageParcel;\n");
    sb.Append("import ohos.rpc.MessageOption;\n");
}

void JavaClientProxyCodeEmitter::EmitProxyImpl(StringBuilder& sb)
{
    sb.AppendFormat("public class %s implements %s {\n", proxyName_.string(), interfaceName_.string());
    EmitProxyConstants(sb, TAB);
    sb.Append("\n");
    sb.Append(TAB).AppendFormat(
        "private static final HiLogLabel TAG = new HiLogLabel(HiLog.LOG_CORE, 0xD001510, \"%s\");\n",
        interfaceFullName_.string());
    sb.Append(TAB).Append("private final IRemoteObject remote;\n");
    sb.Append(TAB).Append("private static final int ERR_OK = 0;\n");
    sb.Append("\n");
    EmitProxyConstructor(sb, TAB);
    sb.Append("\n");
    EmitProxyMethodImpls(sb, TAB);
    sb.Append("};");
}

void JavaClientProxyCodeEmitter::EmitProxyConstants(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("private static final String DESCRIPTOR = \"%s\";\n\n",
        interfaceFullName_.string());
    EmitInterfaceMethodCommands(sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitProxyConstructor(StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("public %s(IRemoteObject remote) {\n", proxyName_.string());
    sb.Append(prefix + TAB).Append("this.remote = remote;\n");
    sb.Append(prefix).Append("}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("@Override\n");
    sb.Append(prefix).Append("public IRemoteObject asObject() {\n");
    sb.Append(prefix + TAB).Append("return remote;\n");
    sb.Append(prefix).Append("}\n");
}

void JavaClientProxyCodeEmitter::EmitProxyMethodImpls(StringBuilder& sb, const String& prefix)
{
    for (size_t i = 0; i < interface_->GetMethodNumber(); i++) {
        AutoPtr<ASTMethod> method = interface_->GetMethod(i);
        EmitProxyMethodImpl(method, sb, prefix);
        if (i + 1 < interface_->GetMethodNumber()) {
            sb.Append("\n");
        }
    }
}

void JavaClientProxyCodeEmitter::EmitProxyMethodImpl(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append("@Override\n");
    if (method->GetParameterNumber() == 0) {
        sb.Append(prefix).AppendFormat("public int %s() throws RemoteException ",
            MethodName(method->GetName()).string());
    } else {
        StringBuilder paramStr;
        paramStr.Append(prefix).AppendFormat("public int %s(", MethodName(method->GetName()).string());
        for (size_t i = 0; i < method->GetParameterNumber(); i++) {
            AutoPtr<ASTParameter> param = method->GetParameter(i);
            EmitInterfaceMethodParameter(param, paramStr, "");
            if (i + 1 < method->GetParameterNumber()) {
                paramStr.Append(", ");
            }
        }
        paramStr.Append(") throws RemoteException");

        sb.Append(SpecificationParam(paramStr, prefix + TAB));
        sb.Append("\n");
    }
    EmitProxyMethodBody(method, sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitInterfaceMethodParameter(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append(param->EmitJavaParameter());
}

void JavaClientProxyCodeEmitter::EmitProxyMethodBody(const AutoPtr<ASTMethod>& method, StringBuilder& sb,
    const String& prefix)
{
    sb.Append(prefix).Append("{\n");
    sb.Append(prefix + TAB).Append("MessageParcel data = MessageParcel.obtain();\n");
    sb.Append(prefix + TAB).Append("MessageParcel reply = MessageParcel.obtain();\n");
    sb.Append(prefix + TAB).AppendFormat("MessageOption option = new MessageOption(MessageOption.TF_SYNC);\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("    data.writeInterfaceToken(DESCRIPTOR);\n");

    bool needBlankLine = false;
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_IN) {
            EmitWriteMethodParameter(param, "data", sb, prefix + TAB);
            needBlankLine = true;
        } else {
            AutoPtr<ASTType> type = param->GetType();
            if (type->GetTypeKind() == TypeKind::TYPE_ARRAY) {
                EmitWriteOutArrayVariable("data", param->GetName(), type, sb, prefix + TAB);
            }
        }
    }
    if (needBlankLine) {
        sb.Append("\n");
    }

    sb.Append(prefix + TAB).Append("try {\n");
    sb.Append(prefix + TAB + TAB).AppendFormat("if (remote.sendRequest(COMMAND_%s, data, reply, option)) {\n",
        ConstantName(method->GetName()).string());
    sb.Append(prefix + TAB + TAB + TAB).Append("return 1;\n");
    sb.Append(prefix + TAB + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("    reply.readException();\n");
    for (size_t i = 0; i < method->GetParameterNumber(); i++) {
        AutoPtr<ASTParameter> param = method->GetParameter(i);
        if (param->GetAttribute() == ParamAttr::PARAM_OUT) {
            EmitReadMethodParameter(param, "reply", sb, prefix + TAB + TAB);
        }
    }

    sb.Append(prefix + TAB).Append("} finally {\n");
    sb.Append(prefix + TAB + TAB).Append("data.reclaim();\n");
    sb.Append(prefix + TAB + TAB).Append("reply.reclaim();\n");
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append(prefix + TAB).Append("return 0;\n");
    sb.Append(prefix).Append("}\n");
}

void JavaClientProxyCodeEmitter::EmitWriteMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    EmitWriteVariable(parcelName, param->GetName(), type, sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitReadMethodParameter(const AutoPtr<ASTParameter>& param, const String& parcelName,
    StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    EmitReadOutVariable(parcelName, param->GetName(), type, sb, prefix);
}

void JavaClientProxyCodeEmitter::EmitWriteVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTType>& type, StringBuilder& sb, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s.writeBoolean(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s.writeByte(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s.writeShort(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s.writeInt(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s.writeLong(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s.writeFloat(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s.writeDouble(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s.writeString(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            if (type->EmitJavaType(TypeMode::NO_MODE).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("%s.writeRemoteObject(%s);\n", parcelName.string(), name.string());
                break;
            }
            sb.Append(prefix).AppendFormat("%s.writeSequenceable(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_INTERFACE:
            sb.Append(prefix).AppendFormat("%s.writeRemoteObject(%s.asObject());\n", parcelName.string(),
                name.string());
            break;
        case TypeKind::TYPE_LIST: {
            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(type.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();

            sb.Append(prefix).AppendFormat("%s.writeInt(%s.size());\n", parcelName.string(), name.string());
            sb.Append(prefix).AppendFormat("for (%s element : %s) {\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string());
            EmitWriteVariable(parcelName, "element", elementType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_MAP: {
            AutoPtr<ASTMapType> mapType = dynamic_cast<ASTMapType*>(type.Get());
            AutoPtr<ASTType> keyType = mapType->GetKeyType();
            AutoPtr<ASTType> valueType = mapType->GetValueType();

            sb.Append(prefix).AppendFormat("%s.writeInt(%s.size());\n", parcelName.string(), name.string());
            sb.Append(prefix).AppendFormat("for (Map.Entry<%s, %s> entry : %s.entrySet()) {\n",
                keyType->EmitJavaType(TypeMode::NO_MODE, true).string(),
                valueType->EmitJavaType(TypeMode::NO_MODE, true).string(), name.string());
            EmitWriteVariable(parcelName, "entry.getKey()", keyType, sb, prefix + TAB);
            EmitWriteVariable(parcelName, "entry.getValue()", valueType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_ARRAY: {
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(type.Get());
            AutoPtr<ASTType> elementType = arrayType->GetElementType();

            sb.Append(prefix).AppendFormat("if (%s == null) {\n", name.string());
            sb.Append(prefix).AppendFormat("    %s.writeInt(-1);\n", parcelName.string());
            sb.Append(prefix).Append("} else { \n");
            EmitWriteArrayVariable(parcelName, name, elementType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitWriteArrayVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTType>& type, StringBuilder& sb, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s.writeBooleanArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s.writeByteArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s.writeShortArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s.writeIntArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s.writeLongArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s.writeFloatArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s.writeDoubleArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s.writeStringArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            sb.Append(prefix).AppendFormat("%s.writeSequenceableArray(%s);\n", parcelName.string(), name.string());
            break;
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitWriteOutArrayVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTType>& type, StringBuilder& sb, const String& prefix)
{
    sb.Append(prefix).AppendFormat("if (%s == null) {\n", name.string());
    sb.Append(prefix).AppendFormat("    %s.writeInt(-1);\n", parcelName.string());
    sb.Append(prefix).Append("} else {\n");
    sb.Append(prefix).AppendFormat("    %s.writeInt(%s.length);\n", parcelName.string(), name.string());
    sb.Append(prefix).Append("}\n");
}

void JavaClientProxyCodeEmitter::EmitReadVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTType>& type, ParamAttr attribute, StringBuilder& sb, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s %s = %s.readBoolean();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s %s = %s.readByte();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s %s = %s.readShort();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s %s = %s.readInt();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s %s = %s.readLong();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s %s = %s.readFloat();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s %s = %s.readDouble();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s %s = %s.readString();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            if (attribute == ParamAttr::PARAM_OUT && type->EmitJavaType(TypeMode::NO_MODE).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("IRemoteObject %s = %s.readRemoteObject();\n",
                    name.string(), parcelName.string());
                break;
            }
            if (attribute == ParamAttr::PARAM_OUT) {
                sb.Append(prefix).AppendFormat("%s %s = new %s();\n",
                    type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(),
                    type->EmitJavaType(TypeMode::NO_MODE).string());
            }
            sb.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.string());

            break;
        case TypeKind::TYPE_INTERFACE:
            sb.Append(prefix).AppendFormat("%s %s = %s.asInterface(%s.readRemoteObject());\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(),
                StubName(type->EmitJavaType(TypeMode::NO_MODE)).string(), parcelName.string());
            break;
        case TypeKind::TYPE_LIST: {
            sb.Append(prefix).AppendFormat("%s %s = new Array%s();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(),
                type->EmitJavaType(TypeMode::NO_MODE).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.string(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.string());
            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(type.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();
            EmitReadVariable(parcelName, "value", elementType, ParamAttr::PARAM_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.add(value);\n", name.string());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_MAP: {
            sb.Append(prefix).AppendFormat("%s %s = new Hash%s();\n",
                type->EmitJavaType(TypeMode::NO_MODE).string(), name.string(),
                type->EmitJavaType(TypeMode::NO_MODE).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.string(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.string());

            AutoPtr<ASTMapType> mapType = dynamic_cast<ASTMapType*>(type.Get());
            AutoPtr<ASTType> keyType = mapType->GetKeyType();
            AutoPtr<ASTType> valueType = mapType->GetValueType();

            EmitReadVariable(parcelName, "key", keyType, ParamAttr::PARAM_IN, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ParamAttr::PARAM_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.put(key, value);\n", name.string());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_ARRAY: {
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(type.Get());
            if (attribute == ParamAttr::PARAM_OUT) {
                EmitReadOutArrayVariable(parcelName, name, arrayType, sb, prefix);
            } else {
                EmitReadArrayVariable(parcelName, name, arrayType, attribute, sb, prefix);
            }
            break;
        }
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitReadArrayVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTArrayType>& arrayType, ParamAttr attribute, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> elementType = arrayType->GetElementType();
    switch (elementType->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readBooleanArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readByteArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readShortArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readIntArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readLongArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readFloatArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readDoubleArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s[] %s = %s.readStringArray();\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            sb.Append(prefix).AppendFormat("int size = %s.readInt();\n", parcelName.string());
            sb.Append(prefix).AppendFormat("%s %s = new %s[size];\n",
                elementType->EmitJavaType(TypeMode::NO_MODE).string(), name.string(),
                elementType->EmitJavaType(TypeMode::NO_MODE).string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < size; ++i) {\n");
            EmitReadVariable(parcelName, "value", elementType, ParamAttr::PARAM_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s[i] = value;\n", name.string());
            sb.Append(prefix).Append("}\n");
            break;
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitReadOutArrayVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTArrayType>& arrayType, StringBuilder& sb, const String& prefix)
{
    AutoPtr<ASTType> elementType = arrayType->GetElementType();
    switch (elementType->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s.readBooleanArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s.readByteArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s.readShortArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s.readIntArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s.readLongArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s.readFloatArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s.readDoubleArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s.readStringArray(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            sb.Append(prefix).AppendFormat("%s.readSequenceableArray(%s);\n", parcelName.string(), name.string());
            break;
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitReadOutVariable(const String& parcelName, const String& name,
    const AutoPtr<ASTType>& type, StringBuilder& sb, const String& prefix)
{
    switch (type->GetTypeKind()) {
        case TypeKind::TYPE_BOOLEAN:
            sb.Append(prefix).AppendFormat("%s = %s.readBoolean();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_BYTE:
            sb.Append(prefix).AppendFormat("%s = %s.readByte();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SHORT:
            sb.Append(prefix).AppendFormat("%s = %s.readShort();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_INT:
        case TypeKind::TYPE_FILEDESCRIPTOR:
            sb.Append(prefix).AppendFormat("%s = %s.readInt();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_LONG:
            sb.Append(prefix).AppendFormat("%s = %s.readLong();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_FLOAT:
            sb.Append(prefix).AppendFormat("%s = %s.readFloat();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_DOUBLE:
            sb.Append(prefix).AppendFormat("%s = %s.readDouble();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_STRING:
            sb.Append(prefix).AppendFormat("%s = %s.readString();\n",
                name.string(), parcelName.string());
            break;
        case TypeKind::TYPE_SEQUENCEABLE:
            if (type->EmitJavaType(TypeMode::NO_MODE).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("%s = %s.readRemoteObject();\n", name.string(), parcelName.string());
                break;
            }
            sb.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.string());
            break;
        case TypeKind::TYPE_INTERFACE:
            sb.Append(prefix).AppendFormat("%s = %s.asInterface(%s.readRemoteObject());\n", name.string(),
                StubName(type->EmitJavaType(TypeMode::NO_MODE)).string(), parcelName.string());
            break;
        case TypeKind::TYPE_LIST: {
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.string(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.string());

            AutoPtr<ASTListType> listType = dynamic_cast<ASTListType*>(type.Get());
            AutoPtr<ASTType> elementType = listType->GetElementType();

            EmitReadVariable(parcelName, "value", elementType, ParamAttr::PARAM_OUT, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.add(value);\n", name.string());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_MAP: {
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.string(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.string());

            AutoPtr<ASTMapType> mapType = dynamic_cast<ASTMapType*>(type.Get());
            AutoPtr<ASTType> keyType = mapType->GetKeyType();
            AutoPtr<ASTType> valueType = mapType->GetValueType();

            EmitReadVariable(parcelName, "key", keyType, ParamAttr::PARAM_OUT, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ParamAttr::PARAM_OUT, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.put(key, value);\n", name.string());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::TYPE_ARRAY: {
            AutoPtr<ASTArrayType> arrayType = dynamic_cast<ASTArrayType*>(type.Get());
            EmitReadOutArrayVariable(parcelName, name, arrayType, sb, prefix);
            break;
        }
        default:
            break;
    }
}

void JavaClientProxyCodeEmitter::EmitLocalVariable(const AutoPtr<ASTParameter>& param, StringBuilder& sb,
    const String& prefix)
{
    AutoPtr<ASTType> type = param->GetType();
    if (type->GetTypeKind() == TypeKind::TYPE_SEQUENCEABLE) {
        sb.Append(prefix).AppendFormat("%s %s = new %s();\n",
            type->EmitJavaType(TypeMode::NO_MODE).string(), param->GetName().string(),
            type->EmitJavaType(TypeMode::NO_MODE).string());
    } else if (type->GetTypeKind() == TypeKind::TYPE_LIST) {
        sb.Append(prefix).AppendFormat("%s %s = new Array%s();\n",
            type->EmitJavaType(TypeMode::NO_MODE).string(), param->GetName().string(),
            type->EmitJavaType(TypeMode::NO_MODE).string());
    } else if (type->GetTypeKind() == TypeKind::TYPE_MAP) {
        sb.Append(prefix).AppendFormat("%s %s = new Hash%s();\n",
            type->EmitJavaType(TypeMode::NO_MODE).string(), param->GetName().string(),
            type->EmitJavaType(TypeMode::NO_MODE).string());
    } else {
        sb.Append(prefix).AppendFormat("%s %s;\n", type->EmitJavaType(TypeMode::NO_MODE).string(),
            param->GetName().string());
    }
}

String JavaClientProxyCodeEmitter::StubName(const String& name)
{
    return name.StartsWith("I") ? (name.Substring(1) + "Stub") : (name + "Stub");
}
} // namespace HDI
} // namespace OHOS