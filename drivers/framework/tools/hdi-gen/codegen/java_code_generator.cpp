/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "java_code_generator.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include "codegen/java_client_interface_code_emitter.h"
#include "codegen/java_client_proxy_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
const char* JavaCodeGenerator::TAG = "JavaCodeGenerator";

bool JavaCodeGenerator::Initializate(const AutoPtr<AST>& ast, const String& targetDirectory)
{
    if (ast->GetASTFileType() == ASTFileType::AST_TYPES) {
        Logger::E(TAG, "java has no types idl.");
        return false;
    }

    ast_ = ast;
    targetDirectory_ = targetDirectory;

    if (!ResolveDirectory()) {
        return false;
    }

    AutoPtr<JavaCodeEmitter> clientInterfaceCodeEmitter = new JavaClientInterfaceCodeEmitter(ast_, targetDirectory_);
    AutoPtr<JavaCodeEmitter> clientProxyCodeEmitter = new JavaClientProxyCodeEmitter(ast_, targetDirectory_);

    emitters_.push_back(clientInterfaceCodeEmitter);
    emitters_.push_back(clientProxyCodeEmitter);
    return true;
}

bool JavaCodeGenerator::Generate() const
{
    for (auto emitter : emitters_) {
        if (!emitter->isInvaildDir()) {
            emitter->EmitCode();
        }
    }

    return true;
}

bool JavaCodeGenerator::ResolveDirectory()
{
    String packageFilePath = String::Format("%s/%s/",
        targetDirectory_.string(), JavaCodeEmitter::FileName(ast_->GetPackageName()).string());
    targetDirectory_ = packageFilePath;

    if (!File::CreateParentDir(targetDirectory_)) {
        Logger::E(TAG, "create '%s' directory failed!", targetDirectory_);
        return false;
    }
    return true;
}
} // namespace HDI
} // namespace OHOS