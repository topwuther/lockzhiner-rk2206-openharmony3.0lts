/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "parser/parser.h"
#include "ast/ast_array_type.h"
#include "ast/ast_enum_type.h"
#include "ast/ast_list_type.h"
#include "ast/ast_map_type.h"
#include "ast/ast_parameter.h"
#include "ast/ast_sequenceable_type.h"
#include "ast/ast_struct_type.h"
#include "ast/ast_union_type.h"
#include "util/logger.h"
#include "util/string_builder.h"

namespace OHOS {
namespace HDI {
const char* Parser::TAG = "Parser";

Parser::Parser(const Options& options, const AutoPtr<ASTModule>& module)
    :options_(options), lexer_(nullptr), errors_(nullptr), ast_(nullptr), astModule_(module)
{}

bool Parser::Parse(const String& sourceFile)
{
    lexer_ = std::make_shared<Lexer>();
    bool ret = lexer_->OpenSourceFile(sourceFile);
    if (!ret) {
        Logger::E(TAG, "Fail to open file '%s'.", sourceFile.string());
        return false;
    }

    ret = ParseFile();
    ret = CheckIntegrity() && ret;
    ret = AddAst() && ret;
    if (!ret || errors_ != nullptr) {
        ShowError();
        return false;
    }

    if (options_.DoDumpAST()) {
        String astStr = ast_->Dump("");
        printf("%s\n", astStr.string());
    }

    return ret;
}

bool Parser::Parse(const String& sourceFile, std::shared_ptr<FileDetail>& fileDetailPtr)
{
    lexer_ = std::make_shared<Lexer>();
    bool ret = lexer_->OpenSourceFile(sourceFile);
    if (!ret) {
        Logger::E(TAG, "Fail to open file '%s'.", sourceFile.string());
        return false;
    }

    ret = ParseIdlDetail(fileDetailPtr);
    if (!ret) {
        ShowError();
        return false;
    }

    return ret;
}

bool Parser::ParseIdlDetail(std::shared_ptr<FileDetail>& fileDetailPtr)
{
    bool ret = true;
    fileDetailPtr = std::make_shared<FileDetail>();
    fileDetailPtr->SetFilePath(lexer_->GetFilePath());

    Token token;
    while ((token = lexer_->PeekToken()) != Token::END_OF_FILE) {
        switch (token) {
            case Token::PACKAGE:
                ret = ParseIdlPackage(fileDetailPtr) && ret;
                break;
            case Token::IMPORT:
                ret = ParseIdlImport(fileDetailPtr) && ret;
                break;
            default:
                lexer_->GetToken();
                break;
        }
    }
    lexer_->GetToken();
    return ret;
}

bool Parser::ParseIdlPackage(std::shared_ptr<FileDetail>& fileDetailPtr)
{
    lexer_->GetToken();
    String packageName;

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        if (token == Token::SEMICOLON) {
            LogError(String::Format("expected identifier before ';'."));
            lexer_->GetToken();
        } else {
            LogError(String::Format("'%s' is an invalid package name.", lexer_->DumpToken().string()));
        }
        lexer_->SkipCurrentLine();
        return false;
    } else {
        lexer_->GetToken();
        packageName = lexer_->GetIdentifier();
        token = lexer_->PeekToken();
    }

    if (token != Token::SEMICOLON) {
        LogError(String::Format("expected ';' before '%s'", lexer_->DumpToken().string()));
        return false;
    }

    // read ';'
    lexer_->GetToken();

    if (packageName.IsEmpty()) {
        LogError(String("Package name is not expected."));
        return false;
    } else if (!IsValidTypeName(packageName)) {
        LogError(String::Format("Package name '%s' is illegal.", packageName.string()));
        return false;
    } else if (!CheckPackageName(lexer_->GetFilePath(), packageName)) {
        LogError(String::Format("Package name '%s' does not match file apth '%s'.",
            packageName.string(), lexer_->GetFilePath().string()));
        return false;
    }

    fileDetailPtr->SetPackageName(packageName);

    return true;
}

bool Parser::ParseIdlImport(std::shared_ptr<FileDetail>& fileDetailPtr)
{
    lexer_->GetToken();

    String importPkgName;
    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        if (token == Token::SEMICOLON) {
            LogError(String::Format("expected identifier before ';'."));
            lexer_->GetToken();
        } else {
            LogError(String::Format("'%s' is an invalid import package name.", lexer_->DumpToken().string()));
        }
        lexer_->SkipCurrentLine();
        return false;
    }
    lexer_->GetToken();
    importPkgName = lexer_->GetIdentifier();

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("expected ';' before '%s'.", lexer_->DumpToken().string()));
        return false;
    }

    // read ';'
    lexer_->GetToken();
    if (importPkgName.IsEmpty()) {
        LogError(String("import package name is not expected."));
        return false;
    } else if (!IsValidTypeName(importPkgName)) {
        LogError(String::Format("import package name '%s' is illegal.", importPkgName.string()));
        return false;
    }

    if (!fileDetailPtr->AddImport(importPkgName)) {
        LogError(String::Format("the package of '%s' has been imported.", importPkgName.string()));
        return false;
    }

    return true;
}

bool Parser::ParseFile()
{
    bool ret = true;

    ast_ = new AST();
    ast_->SetIdlFile(lexer_->GetFilePath());

    ParseLicense();

    Token token;
    while ((token = lexer_->PeekToken()) != Token::END_OF_FILE) {
        switch (token) {
            case Token::PACKAGE:
                ret = ParsePackageName() && ret;
                continue;
            case Token::IMPORT:
                ret = ParseImport() && ret;
                continue;
            case Token::SEQUENCEABLE:
                ret = ParseSequenceable() && ret;
                continue;
            case Token::BRACKETS_LEFT:
                ret = ParseAttribute() && ret;
                continue;
            case Token::INTERFACE:
                ret = ParseInterface() && ret;
                continue;
            case Token::COMMENT_LINE:
                lexer_->GetToken();
                continue;
            case Token::ENUM:
                ret = ParseEnumDefine() && ret;
                continue;
            case Token::STRUCT:
                ret = ParseStructDefine() && ret;
                continue;
            case Token::UNION:
                ret = ParseUnionDefine() && ret;
                continue;
            default:
                LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
                lexer_->GetToken();
                ret = false;
                continue;
        }
    }
    lexer_->GetToken();

    // here, ast_ cannot be a sequenceable idl
    if (ast_->GetInterfaceDef() != nullptr) {
        if (ast_->GetInterfaceDef()->IsCallback()) {
            ast_->SetAStFileType(ASTFileType::AST_ICALLBACK);
        } else {
            ast_->SetAStFileType(ASTFileType::AST_IFACE);
        }
    } else {
        ast_->SetAStFileType(ASTFileType::AST_TYPES);
    }

    return ret;
}

bool Parser::ParseLicense()
{
    Token token = lexer_->PeekToken(false);
    if (token != Token::COMMENT_BLOCK) {
        return false;
    }

    lexer_->GetToken(false);

    ast_->SetLicense(lexer_->GetComment());

    return true;
}

bool Parser::ParsePackageName()
{
    lexer_->GetToken();

    String packageFullName;

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        if (token == Token::SEMICOLON) {
            LogError(String::Format("expected identifier before ';'."));
        } else {
            LogError(String::Format("'%s' is an invalid package name.", lexer_->DumpToken().string()));
        }
        lexer_->SkipCurrentLine();
        lexer_->GetToken();
        return false;
    } else {
        lexer_->GetToken();
        packageFullName = lexer_->GetIdentifier();
        token = lexer_->PeekToken();
    }

    if (token != Token::SEMICOLON) {
        LogError(String::Format("expected ';' before '%s'", lexer_->DumpToken().string()));
        return false;
    }

    // read ';'
    lexer_->GetToken();

    if (packageFullName.IsEmpty()) {
        LogError(String("Package name is not expected."));
        return false;
    } else if (!IsValidTypeName(packageFullName)) {
        LogError(String::Format("Package name '%s' is illegal.", packageFullName.string()));
        return false;
    } else if (!CheckPackageName(lexer_->GetFilePath(), packageFullName)) {
        LogError(String::Format("Package name '%s' does not match file apth '%s'.",
            packageFullName.string(), lexer_->GetFilePath().string()));
        return false;
    }

    ast_->SetPackageName(packageFullName);

    return true;
}

bool Parser::ParseImport()
{
    lexer_->GetToken();

    String packageFullName;

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        if (token == Token::SEMICOLON) {
            LogError(String::Format("expected identifier before ';'."));
        } else {
            LogError(String::Format("%s is an invalid import package name.", lexer_->DumpToken().string()));
        }
        lexer_->SkipCurrentLine();
        lexer_->GetToken();
        return false;
    }
    lexer_->GetToken();
    packageFullName = lexer_->GetIdentifier();

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("expected ';' before '%s'.", lexer_->DumpToken().string()));
        return false;
    }

    // read ';'
    lexer_->GetToken();
    if (packageFullName.IsEmpty()) {
        LogError(String("import package name is not expected."));
        return false;
    } else if (!IsValidTypeName(packageFullName)) {
        LogError(String::Format("import package name '%s' is illegal.", packageFullName.string()));
        return false;
    }

    AutoPtr<AST> importAst = astModule_->GetAst(packageFullName);
    if (importAst == nullptr) {
        LogError(String("not find import ast."));
        return false;
    }

    if (!ast_->AddImport(importAst)) {
        LogError(String::Format("the package '%s' has been import", packageFullName.string()));
        return false;
    }

    return true;
}

bool Parser::ParseSequenceable()
{
    lexer_->GetToken();
    String classFullName;

    Token token =  lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        if (token == Token::SEMICOLON) {
            LogError(String::Format("expected identifier before ';'."));
        } else {
            LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        }
        lexer_->SkipCurrentLine();
        lexer_->GetToken();
        return false;
    }
    lexer_->GetToken();
    classFullName = lexer_->GetIdentifier();

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("expected ';' before '%s'.", lexer_->DumpToken().string()));
        return false;
    }
    // read ';'
    lexer_->GetToken();

    if (classFullName.IsEmpty()) {
        LogError(String("class name is not expected."));
        return false;
    } else if (!IsValidTypeName(classFullName)) {
        LogError(String::Format("class name '%s' is illegal.", classFullName.string()));
        return false;
    }

    AutoPtr<ASTSequenceableType> sequenceable = new ASTSequenceableType();
    int index = classFullName.LastIndexOf('.');
    if (index != -1) {
        sequenceable->SetName(classFullName.Substring(index + 1));
        sequenceable->SetNamespace(ast_->ParseNamespace(classFullName.Substring(0, index + 1)));
    } else {
        sequenceable->SetName(classFullName);
    }

    AutoPtr<AST> sequenceableAst = new AST();
    sequenceableAst->SetFullName(classFullName);
    sequenceableAst->AddSequenceableDef(sequenceable);
    sequenceableAst->SetAStFileType(ASTFileType::AST_SEQUENCEABLE);
    ast_->AddImport(sequenceableAst);

    return true;
}

bool Parser::ParseAttribute()
{
    bool ret = true;
    AutoPtr<Attribute> attributes = nullptr;

    // read '['
    Token token = lexer_->GetToken();
    if (token == Token::BRACKETS_LEFT) {
        attributes = new Attribute();
        token = lexer_->PeekToken();
        while (token != Token::BRACKETS_RIGHT) {
            switch (token) {
                case Token::ONEWAY:
                    attributes->isOneWay = true;
                    break;
                case Token::CALLBACK:
                    attributes->isCallback = true;
                    break;
                case Token::FULL:
                    attributes->isFull = true;
                    break;
                case Token::LITE:
                    attributes->isLite = true;
                    break;
                default: {
                    LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
                    lexer_->SkipCurrentLine(Lexer::TokenToChar(Token::BRACKETS_RIGHT));
                    lexer_->GetToken();
                    return false;
                }
            }
            lexer_->GetToken();
            token = lexer_->PeekToken();
            if (token == Token::COMMA) {
                lexer_->GetToken();
                token = lexer_->PeekToken();
            } else if (token == Token::BRACKETS_RIGHT) {
                break;
            } else {
                LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
                return false;
            }
        }
        lexer_->GetToken();
    }

    token = lexer_->PeekToken();
    switch (token) {
        case Token::ENUM:
            ret = ParseEnumDefine(attributes) && ret;
            break;
        case Token::STRUCT:
            ret = ParseStructDefine(attributes) && ret;
            break;
        case Token::UNION:
            ret = ParseUnionDefine(attributes) && ret;
            break;
        case Token::INTERFACE:
            ret = ParseInterface(attributes) && ret;
            break;
        default: {
            LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
            return false;
        }
    }
    return ret;
}

bool Parser::ParseInterface(const AutoPtr<Attribute>& attributes)
{
    lexer_->GetToken();
    bool ret = true;
    bool isOneWay = false;
    bool isCallback = false;
    bool isFull = false;
    bool isLite = false;
    String interfaceName;

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        lexer_->Skip('}');
        lexer_->GetToken();
        return false;
    }
    lexer_->GetToken();

    interfaceName = lexer_->GetIdentifier();

    token = lexer_->PeekToken();
    if (token != Token::BRACES_LEFT) {
        LogError(String::Format("expected '{' after the '%s'.", lexer_->DumpToken().string()));
        lexer_->Skip('}');
        lexer_->GetToken();
        return false;
    }

    // read '{'
    lexer_->GetToken();

    if (interfaceName.IsEmpty()) {
        LogError(String("interface name is expected."));
        return false;
    }

    AutoPtr<ASTInterfaceType> interface = new ASTInterfaceType();
    interface->SetName(interfaceName);
    interface->SetNamespace(ast_->ParseNamespace(ast_->GetFullName()));

    if (!interface->GetName().Equals(ast_->GetName())) {
        LogError(String::Format("Module name '%s' is not equal to interface name  '%s'.",
            ast_->GetName().string(), interface->GetName().string()));
    }

    interface->SetLicense(ast_->GetLicense());
    if (attributes != nullptr) {
        interface->SetOneWay(attributes->isOneWay);
        interface->SetCallback(attributes->isCallback);
        interface->SetFull(attributes->isFull);
        interface->SetLite(attributes->isLite);
    }

    while (token != Token::BRACES_RIGHT && token != Token::END_OF_FILE) {
        ret = ParseMethod(interface) && ret;
        token = lexer_->PeekToken();
    }

    if (token != Token::BRACES_RIGHT) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        lexer_->SkipEof();
        return false;
    }

    lexer_->GetToken();

    if (ast_->GetInterfaceDef() != nullptr) {
        LogError(String::Format("an interface class already exists int idl file."));
        lexer_->SkipEof();
        return false;
    }

    ast_->AddInterfaceDef(interface);

    return ret;
}

bool Parser::ParseMethod(const AutoPtr<ASTInterfaceType>& interface)
{
    bool ret = true;
    AutoPtr<Attribute> attributes = new Attribute();
    ret = ParseMethodAttr(interface, attributes) && ret;
    if (!ret) {
        return false;
    }

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("Method name is expected."));
        if (token == Token::BRACES_RIGHT) {
            return false;
        }
        // jump over colon
        lexer_->GetToken();
        while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
            token = lexer_->PeekToken();
            if (token == Token::BRACES_RIGHT) {
                break;
            }
            lexer_->GetToken();
        }
        return false;
    }
    token = lexer_->GetToken();

    AutoPtr<ASTMethod> method = new ASTMethod();
    method->SetName(lexer_->GetIdentifier());

    if (attributes != nullptr) {
        method->SetOneWay(attributes->isOneWay);
        method->SetFull(attributes->isFull);
        method->SetLite(attributes->isLite);
    }

    token = lexer_->PeekToken();
    if (token != Token::PARENTHESES_LEFT) {
        LogError(String("'(' is expected."));
        if (token == Token::BRACES_RIGHT) {
            return false;
        }
        // jump over colon
        lexer_->GetToken();
        while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
            token = lexer_->PeekToken();
            if (token == Token::BRACES_RIGHT) {
                break;
            }
            lexer_->GetToken();
        }
        return false;
    }
    token = lexer_->GetToken();

    token = lexer_->PeekToken();
    while (token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
        ret = ParseParameter(method) && ret;
        token = lexer_->PeekToken();
        if (token == Token::COMMA) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        } else if (token != Token::PARENTHESES_RIGHT) {
            LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
            lexer_->SkipCurrentLine(';');
            lexer_->GetToken();
            return false;
        }
    }

    lexer_->GetToken();
    if (!ret) {
        lexer_->SkipCurrentLine();
        return false;
    }

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String("';' is expected."));
        return false;
    }
    lexer_->GetToken();
    interface->AddMethod(method);
    return ret;
}

bool Parser::ParseMethodAttr(const AutoPtr<ASTInterfaceType>& interface, const AutoPtr<Attribute>& attributes)
{
    if (interface == nullptr || attributes == nullptr) {
        return false;
    }

    Token token = lexer_->PeekToken();
    if (token == Token::BRACES_RIGHT) {
        LogError(String::Format("%s has no method.", interface->GetName().string()));
        lexer_->SkipCurrentLine(Lexer::TokenToChar(Token::BRACES_RIGHT));
        lexer_->GetToken();
        return false;
    }

    if (token == Token::BRACKETS_LEFT) {
        lexer_->GetToken();
        token = lexer_->PeekToken();
        while (token != Token::BRACKETS_RIGHT) {
            switch (token) {
                case Token::ONEWAY:
                    attributes->isOneWay = true;
                    break;
                case Token::FULL:
                    attributes->isFull = true;
                    break;
                case Token::LITE:
                    attributes->isLite = true;
                    break;
                default: {
                    LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
                    lexer_->SkipCurrentLine(Lexer::TokenToChar(Token::BRACKETS_RIGHT));
                    lexer_->GetToken();
                    return false;
                }
            }
            lexer_->GetToken();
            token = lexer_->PeekToken();
            if (token == Token::COMMA) {
                lexer_->GetToken();
                token = lexer_->PeekToken();
            } else if (token == Token::BRACKETS_RIGHT) {
            } else {
                LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
                lexer_->SkipCurrentLine(Lexer::TokenToChar(Token::BRACKETS_RIGHT));
                lexer_->GetToken();
                return false;
            }
        }
        lexer_->GetToken();
    }
    return true;
}

bool Parser::ParseParameter(const AutoPtr<ASTMethod>& method)
{
    Token token = lexer_->PeekToken();
    if (token != Token::BRACKETS_LEFT) {
        LogError(String("'[' is expected."));
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        }
        return false;
    }
    lexer_->GetToken();

    AutoPtr<ASTParameter> parameter = new ASTParameter();
    if (!ParseParamAttr(parameter)) {
        return false;
    }

    token = lexer_->PeekToken();
    if (token != Token::BRACKETS_RIGHT) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        }
        return false;
    }
    lexer_->GetToken();
    AutoPtr<ASTType> type = ParseType();
    if (type == nullptr) {
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        }
        return false;
    }

    token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("Parameter name is expected."));
        while (token != Token::COMMA && token != Token::PARENTHESES_RIGHT && token != Token::END_OF_FILE) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        }
        return false;
    }
    lexer_->GetToken();
    parameter->SetName(lexer_->GetIdentifier());
    parameter->SetType(type);
    method->AddParameter(parameter);
    return true;
}

bool Parser::ParseParamAttr(const AutoPtr<ASTParameter>& parameter)
{
    if (parameter == nullptr) {
        return false;
    }
    Token token = lexer_->PeekToken();
    if (token == Token::IN) {
        lexer_->GetToken();
        parameter->SetAttribute(ParamAttr::PARAM_IN);
    } else if (token == Token::OUT) {
        lexer_->GetToken();
        parameter->SetAttribute(ParamAttr::PARAM_OUT);
    } else {
        LogError(String::Format("'%s' is not a parameter property, it must be 'in' or 'out'.",
            lexer_->DumpToken().string()));
        while (token != Token::SEMICOLON && token != Token::END_OF_FILE) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        }
        return false;
    }
    return true;
}

AutoPtr<ASTType> Parser::ParseType()
{
    int typeLineNo = lexer_->GetTokenLineNumber();
    int typeColumnNo = lexer_->GetTokenColumnNumber();
    AutoPtr<ASTType> type;

    Token token = lexer_->PeekToken();
    if (IsPrimitiveType(token)) {
        lexer_->GetToken();
        type = ast_->FindType(lexer_->DumpToken());
    } else if (token == Token::LIST) {
        type = ParseList();
    } else if (token == Token::MAP) {
        type = ParseMap();
    } else if (token == Token::ENUM || token == Token::STRUCT || token == Token::UNION) {
        type = ParseCustomType();
    } else if (token == Token::IDENTIFIER) {
        lexer_->GetToken();
        type = ast_->FindType(lexer_->GetIdentifier());
    } else if (token == Token::UNSIGNED) {
        String unsignedStr = lexer_->DumpToken();
        lexer_->GetToken();
        token = lexer_->PeekToken();
        switch (token) {
            case Token::CHAR:
            case Token::SHORT:
            case Token::INTEGER:
            case Token::LONG: {
                type = ast_->FindType(unsignedStr + " " + lexer_->DumpToken());
                lexer_->GetToken();
                break;
            }
            default: {
                LogError(typeLineNo, typeColumnNo,
                    String::Format("'unsigned %s' type was not declared in the idl file.",
                        lexer_->DumpToken().string()));
                return nullptr;
            }
        }
    } else {
        LogError(typeLineNo, typeColumnNo, String("Invalid type name."));
        return nullptr;
    }

    if (type == nullptr) {
        LogError(typeLineNo, typeColumnNo, String::Format("Type '%s' was not declared in any idl file.",
            lexer_->DumpToken().string()));
    }

    token = lexer_->PeekToken();
    if (token == Token::BRACKETS_LEFT) {
        lexer_->GetToken();
        token = lexer_->PeekToken();
        if (token != Token::BRACKETS_RIGHT) {
            LogError(typeLineNo, typeColumnNo, String("']' is expected."));
            return nullptr;
        }
        lexer_->GetToken();

        if (type != nullptr) {
            AutoPtr<ASTArrayType> arrayType = new ASTArrayType();
            arrayType->SetElementType(type);

            type = ast_->FindType(arrayType->ToString());
            if (type == nullptr) {
                ast_->AddType(arrayType.Get());
                type = static_cast<ASTType*>(arrayType.Get());
            }
        }
    }

    if (!CheckType(typeLineNo, typeColumnNo, type)) {
        return nullptr;
    }

    return type;
}

AutoPtr<ASTType> Parser::ParseList()
{
    lexer_->GetToken();

    Token token = lexer_->PeekToken();
    if (token != Token::ANGLE_BRACKETS_LEFT) {
        LogError(String("'<' is expected."));
        return nullptr;
    }
    lexer_->GetToken();

    AutoPtr<ASTType> type = ParseType();
    if (type == nullptr) {
        lexer_->SkipCurrentLine('>');
        lexer_->GetToken();
        return nullptr;
    }

    token = lexer_->PeekToken();
    if (token != Token::ANGLE_BRACKETS_RIGHT) {
        LogError(String("'>' is expected."));
        return nullptr;
    }
    lexer_->GetToken();

    AutoPtr<ASTListType> list = new ASTListType();
    list->SetElementType(type);

    AutoPtr<ASTType> ret = ast_->FindType(list->ToString());
    if (ret == nullptr) {
        ast_->AddType(list.Get());
        ret = list.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseMap()
{
    lexer_->GetToken();

    Token token = lexer_->PeekToken();
    if (token != Token::ANGLE_BRACKETS_LEFT) {
        LogError(String("'<' is expected."));
        return nullptr;
    }
    lexer_->GetToken();

    AutoPtr<ASTType> keyType = ParseType();
    if (keyType == nullptr) {
        lexer_->SkipCurrentLine('>');
        return nullptr;
    }

    token = lexer_->PeekToken();
    if (token != Token::COMMA) {
        LogError(String("',' is expected."));
        return nullptr;
    }
    lexer_->GetToken();

    AutoPtr<ASTType> valueType = ParseType();
    if (valueType == nullptr) {
        lexer_->SkipCurrentLine('>');
        return nullptr;
    }

    token = lexer_->PeekToken();
    if (token != Token::ANGLE_BRACKETS_RIGHT) {
        LogError(String("'>' is expected."));
        return nullptr;
    }
    lexer_->GetToken();

    AutoPtr<ASTMapType> map = new ASTMapType();
    map->SetKeyType(keyType);
    map->SetValueType(valueType);

    AutoPtr<ASTType> ret = ast_->FindType(map->ToString());
    if (ret == nullptr) {
        ast_->AddType(map.Get());
        ret = map.Get();
    }

    return ret;
}

AutoPtr<ASTType> Parser::ParseCustomType()
{
    Token token = lexer_->GetToken();
    String typePrefix;
    if (token == Token::ENUM) {
        typePrefix = "enum";
    } else if (token == Token::STRUCT) {
        typePrefix = "struct";
    } else if (token == Token::UNION) {
        typePrefix = "union";
    }

    token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("custom type name is expected."));
        return nullptr;
    }

    String typeName = typePrefix + " " + lexer_->GetIdentifier();
    lexer_->GetToken();
    AutoPtr<ASTType> type = ast_->FindType(typeName);
    if (type != nullptr) {
        ast_->AddType(type);
    }

    return type;
}

bool Parser::ParseEnumDefine(const AutoPtr<Attribute>& attributes)
{
    lexer_->GetToken();
    AutoPtr<ASTEnumType> type = new ASTEnumType();

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("custom enum type name is expected."));
        return false;
    }
    type->SetName(lexer_->GetIdentifier());
    lexer_->GetToken();

    if (!ParseEnumBaseType(type)) {
        return false;
    }

    token = lexer_->PeekToken();
    if (token != Token::BRACES_LEFT) {
        lexer_->SkipCurrentLine(';');
        return false;
    }
    lexer_->GetToken();

    if (!ParseEnumMember(type)) {
        return false;
    }

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        lexer_->SkipCurrentLine();
        return false;
    }
    lexer_->GetToken();

    if (attributes != nullptr) {
        type->SetFull(attributes->isFull);
        type->SetLite(attributes->isLite);
    }

    ast_->AddTypeDefinition(type.Get());
    return true;
}

bool Parser::ParseEnumBaseType(const AutoPtr<ASTEnumType>& type)
{
    Token token = lexer_->PeekToken();
    if (token == Token::COLON) {
        lexer_->GetToken();
        AutoPtr<ASTType> baseType = ParseType();
        if (baseType == nullptr) {
            LogError("no base type of enum type");
            lexer_->SkipCurrentLine(';');
            return false;
        }
        if (baseType->IsByteType() ||
            baseType->IsShortType() ||
            baseType->IsIntegerType() ||
            baseType->IsLongType() ||
            baseType->IsUcharType() ||
            baseType->IsUshortType() ||
            baseType->IsUintType() ||
            baseType->IsUlongType()) {
            ast_->AddType(baseType);
            type->SetBaseType(baseType);
            type->SetDisplayBase(true);
        } else {
            LogError("invaild base type of enum type");
            lexer_->SkipCurrentLine(';');
            return false;
        }
    }

    if (type->GetBaseType() == nullptr) {
        AutoPtr<ASTType> baseType = ast_->FindType("int");
        if (baseType == nullptr) {
            ast_->AddType(new ASTIntegerType());
            baseType = ast_->FindType("int");
        }
        type->SetBaseType(baseType);
    }
    return true;
}

bool Parser::ParseEnumMember(const AutoPtr<ASTEnumType>& type)
{
    Token token = lexer_->PeekToken();
    while (token != Token::BRACES_RIGHT) {
        if (token != Token::IDENTIFIER) {
            lexer_->SkipCurrentLine(';');
            return false;
        }
        AutoPtr<ASTEnumValue> enumValue = new ASTEnumValue(lexer_->GetIdentifier());
        lexer_->GetToken();

        token = lexer_->PeekToken();
        if (token == Token::ASSIGN) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
            if (token == Token::DIGIT) {
                enumValue->SetValue(lexer_->GetDigit());
                lexer_->GetToken();
                token = lexer_->PeekToken();
            } else {
                return false;
            }
        }

        if (token == Token::COMMA) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        } else if (token == Token::BRACES_RIGHT) {
        } else {
            lexer_->SkipCurrentLine(';');
            return false;
        }
        enumValue->SetType(type->GetBaseType());
        type->AddMember(enumValue);
    }
    lexer_->GetToken();
    return true;
}

bool Parser::ParseStructDefine(const AutoPtr<Attribute>& attributes)
{
    lexer_->GetToken();

    AutoPtr<ASTStructType> type = new ASTStructType();
    if (type == nullptr) {
        lexer_->SkipCurrentLine(';');
        return false;
    }

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("custom struct type name is expected."));
        return false;
    }
    type->SetName(lexer_->GetIdentifier());
    lexer_->GetToken();

    token = lexer_->PeekToken();
    if (token != Token::BRACES_LEFT) {
        lexer_->SkipCurrentLine(';');
        return false;
    }
    lexer_->GetToken();

    token = lexer_->PeekToken();
    while (token != Token::BRACES_RIGHT) {
        AutoPtr<ASTType> member = ParseType();
        if (member == nullptr) {
            lexer_->SkipCurrentLine(';');
            return false;
        }
        String typeName = member->ToString();

        token = lexer_->PeekToken();
        if (token != Token::IDENTIFIER) {
            LogError(String("member name is expected."));
            return false;
        }
        String memberName = lexer_->GetIdentifier();
        lexer_->GetToken();

        type->AddMember(member, memberName);

        token = lexer_->PeekToken();
        if (token == Token::SEMICOLON) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        } else if (token == Token::BRACES_RIGHT) {
            break;
        } else {
            lexer_->SkipCurrentLine(';');
            return false;
        }
    }
    lexer_->GetToken();

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        lexer_->SkipCurrentLine();
        return false;
    }
    lexer_->GetToken();

    if (attributes != nullptr) {
        type->SetFull(attributes->isFull);
        type->SetLite(attributes->isLite);
    }

    ast_->AddTypeDefinition(type.Get());
    return true;
}

bool Parser::ParseUnionDefine(const AutoPtr<Attribute>& attributes)
{
    lexer_->GetToken();

    AutoPtr<ASTUnionType> type = new ASTUnionType();
    if (type == nullptr) {
        lexer_->SkipCurrentLine(';');
        return false;
    }

    Token token = lexer_->PeekToken();
    if (token != Token::IDENTIFIER) {
        LogError(String("custom union type name is expected."));
        return false;
    }
    type->SetName(lexer_->GetIdentifier());
    lexer_->GetToken();

    token = lexer_->PeekToken();
    if (token != Token::BRACES_LEFT) {
        lexer_->SkipCurrentLine(';');
        return false;
    }
    lexer_->GetToken();

    token = lexer_->PeekToken();
    while (token != Token::BRACES_RIGHT) {
        AutoPtr<ASTType> member = ParseType();
        if (member == nullptr) {
            lexer_->SkipCurrentLine(';');
            return false;
        }
        String typeName = member->ToString();

        token = lexer_->PeekToken();
        if (token != Token::IDENTIFIER) {
            LogError(String("member name is expected."));
            return false;
        }
        String memberName = lexer_->GetIdentifier();
        lexer_->GetToken();

        type->AddMember(member, memberName);

        token = lexer_->PeekToken();
        if (token == Token::SEMICOLON) {
            lexer_->GetToken();
            token = lexer_->PeekToken();
        } else if (token == Token::BRACES_RIGHT) {
            break;
        } else {
            lexer_->SkipCurrentLine(';');
            return false;
        }
    }
    lexer_->GetToken();

    token = lexer_->PeekToken();
    if (token != Token::SEMICOLON) {
        LogError(String::Format("'%s' is not expected.", lexer_->DumpToken().string()));
        lexer_->SkipCurrentLine();
        return false;
    }
    lexer_->GetToken();

    if (attributes != nullptr) {
        type->SetFull(attributes->isFull);
        type->SetLite(attributes->isLite);
    }

    ast_->AddTypeDefinition(type.Get());
    return true;
}

bool Parser::CheckType(int lineNo, int columnNo, const AutoPtr<ASTType>& type)
{
    if (type == nullptr) {
        return false;
    }

    if (options_.GetTargetLanguage().Equals("c")) {
        if (type->IsSequenceableType()) {
            LogError(lineNo, columnNo, String::Format("The sequenceable type is not supported by c language."));
            return false;
        }
    } else if (options_.GetTargetLanguage().Equals("java")) {
        switch (type->GetTypeKind()) {
            case TypeKind::TYPE_UCHAR:
            case TypeKind::TYPE_USHORT:
            case TypeKind::TYPE_UINT:
            case TypeKind::TYPE_ULONG:
            case TypeKind::TYPE_ENUM:
            case TypeKind::TYPE_STRUCT:
            case TypeKind::TYPE_UNION:
            case TypeKind::TYPE_UNKNOWN:
                LogError(lineNo, columnNo, String::Format("The '%s' type is not supported by java language.",
                    lexer_->DumpToken().string()));
                return false;
            default:
                break;
        }
    }

    return true;
}

bool Parser::CheckIntegrity()
{
    if (ast_ == nullptr) {
        LogError(String("ast is nullptr."));
        return false;
    }

    if (ast_->GetName().IsEmpty()) {
        LogError(String("ast's name is empty."));
        return false;
    }

    if (ast_->GetPackageName().IsEmpty()) {
        LogError(String("ast's package name is empty."));
        return false;
    }

    switch (ast_->GetASTFileType()) {
        case ASTFileType::AST_IFACE: {
            AutoPtr<ASTInterfaceType> interface = ast_->GetInterfaceDef();
            if (interface == nullptr) {
                LogError(String("ast's interface is empty."));
                return false;
            }

            if (ast_->GetTypeDefinitionNumber() > 0) {
                LogError(String("interface ast cannot has custom types."));
                return false;
            }

            if (interface->GetMethodNumber() == 0) {
                LogError(String("interface ast has no method."));
                return false;
            }

            break;
        }
        case ASTFileType::AST_ICALLBACK: {
            AutoPtr<ASTInterfaceType> interface = ast_->GetInterfaceDef();
            if (interface == nullptr) {
                LogError(String("ast's interface is empty."));
                return false;
            }

            if (!interface->IsCallback()) {
                LogError(String("ast is callback, but ast's interface is not callback."));
                return false;
            }
            break;
        }
        case ASTFileType::AST_SEQUENCEABLE: {
            LogError(String("it's impossible that ast is sequenceable."));
            return false;
        }
        case ASTFileType::AST_TYPES: {
            if (ast_->GetInterfaceDef() != nullptr) {
                LogError(String("custom ast cannot has interface."));
                return false;
            }
            break;
        }
        default:
            break;
    }

    return true;
}

bool Parser::IsValidTypeName(const String& typeName)
{
    if (typeName[0] == '.') {
        return false;
    }

    if (typeName[typeName.GetLength() - 1] == '.') {
        return false;
    }

    return true;
}

/*
* For example
* filePath: ./test/cpp_test/data_test/v1_0/IDataTest.idl
* package test.cpp_test.data_test.v1_0;
*/
bool Parser::CheckPackageName(const String& filePath, const String& packageName)
{
    String pkgToPath = packageName.Replace('.', '/');
    int index = filePath.LastIndexOf('/');
    if (index == -1) {
        return false;
    }

    String parentDir = filePath.Substring(0, index);
    if (parentDir.IndexOf(pkgToPath) == -1) {
        return false;
    }

    return true;
}

bool Parser::AddAst()
{
    if (ast_ == nullptr) {
        LogError(String("ast is nullptr."));
        return false;
    }
    astModule_->AddAST(ast_->GetFullName(), ast_);
    return true;
}

void Parser::LogError(const String& message)
{
    AutoPtr<ErrorInfo> error = new ErrorInfo();

    String sourceFile = lexer_->GetFilePath();
#ifdef __MINGW32__
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('\\') + 1);
#else
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('/') + 1);
#endif
    error->lineNo_ = lexer_->GetTokenLineNumber();
    error->columnNo_ = lexer_->GetTokenColumnNumber();
    error->message_ = message;

    if (errors_ == nullptr) {
        errors_ = error;
    } else {
        AutoPtr<ErrorInfo> pos = errors_;
        while (pos->next_ != nullptr) {
            pos = pos->next_;
        }
        pos->next_ = error;
    }
}

void Parser::LogError(int lineNo, int columnNo, const String& message)
{
    AutoPtr<ErrorInfo> error = new ErrorInfo();

    String sourceFile = lexer_->GetFilePath();
#ifdef __MINGW32__
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('\\') + 1);
#else
    error->file_ = sourceFile.Substring(sourceFile.LastIndexOf('/') + 1);
#endif
    error->lineNo_ = lineNo;
    error->columnNo_ = columnNo;
    error->message_ = message;

    if (errors_ == nullptr) {
        errors_ = error;
    } else {
        AutoPtr<ErrorInfo> pos = errors_;
        while (pos->next_ != nullptr) {
            pos = pos->next_;
        }
        pos->next_ = error;
    }
}

void Parser::ShowError()
{
    AutoPtr<ErrorInfo> error = errors_;
    while (error != nullptr) {
        Logger::E(TAG, "%s[line %d, column %d] %s", error->file_.string(),
            error->lineNo_, error->columnNo_, error->message_.string());
        error = error->next_;
    }
}
} // namespace HDI
} // namespace OHOS