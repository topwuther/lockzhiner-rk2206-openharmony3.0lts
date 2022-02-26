/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_C_CUSTOM_TYPES_CODE_EMITTER_H
#define OHOS_HDI_C_CUSTOM_TYPES_CODE_EMITTER_H
#include <vector>
#include "codegen/c_code_emitter.h"

namespace OHOS {
namespace HDI {
class CCustomTypesCodeEmitter : public CCodeEmitter {
public:
    CCustomTypesCodeEmitter(const AutoPtr<AST>& ast, const String& targetDirectory)
        : CCodeEmitter(ast, targetDirectory) {}

    virtual ~CCustomTypesCodeEmitter() = default;

    void EmitCode() override;
private:
    void EmitCustomTypesHeaderFile();

    void EmitHeaderInclusions(StringBuilder& sb);

    void EmitCustomTypeDecls(StringBuilder& sb);

    void EmitCustomTypeDecl(StringBuilder& sb, const AutoPtr<ASTType>& type);

    void EmitCustomTypeFuncDecl(StringBuilder& sb);

    void EmitCustomTypeMarshallingDecl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitCustomTypeUnmarshallingDecl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitCustomTypeFreeDecl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitCustomTypesSourceFile();

    void EmitSoucreIncludsions(StringBuilder& sb);

    void EmitSourceStdlibInclusions(StringBuilder& sb);

    void EmitCustomTypeDataProcess(StringBuilder& sb);

    void EmitCustomTypeMarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitCustomTypeUnmarshallingImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitError(const String& name, const AutoPtr<ASTType>& type, StringBuilder& sb, const String& prefix);

    void EmitCustomTypeFreeImpl(StringBuilder& sb, const AutoPtr<ASTStructType>& type);

    void EmitCustomTypeMemberFree(StringBuilder& sb, const String& name, const AutoPtr<ASTType>& type,
        const String& prefix);

    std::vector<String> freeObjStatements_;
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_C_CUSTOM_TYPES_CODE_EMITTER_H
