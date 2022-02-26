/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "cpp_code_generator.h"
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include "codegen/cpp_client_interface_code_emitter.h"
#include "codegen/cpp_client_proxy_code_emitter.h"
#include "codegen/cpp_custom_types_code_emitter.h"
#include "codegen/cpp_service_driver_code_emitter.h"
#include "codegen/cpp_service_impl_code_emitter.h"
#include "codegen/cpp_service_interface_code_emitter.h"
#include "codegen/cpp_service_stub_code_emitter.h"
#include "util/file.h"
#include "util/logger.h"

namespace OHOS {
namespace HDI {
const char* CppCodeGenerator::TAG = "CppCodeGenerator";

bool CppCodeGenerator::Initializate(const AutoPtr<AST>& ast, const String& targetDirectory)
{
    ast_ = ast;
    targetDirectory_ = targetDirectory;

    if (!ResolveDirectory()) {
        return false;
    }

    if (ast_->GetASTFileType() == ASTFileType::AST_TYPES) {
        AutoPtr<CppCodeEmitter> customTypesCodeEmitter = new CppCustomTypesCodeEmitter(ast_, targetDirectory_);
        emitters_.push_back(customTypesCodeEmitter);
        return true;
    }

    AutoPtr<CppCodeEmitter> clientInterfaceCodeEmitter = new CppClientInterfaceCodeEmitter(ast_, targetDirectory_);
    AutoPtr<CppCodeEmitter> clientProxyCodeEmitter = new CppClientProxyCodeEmitter(ast_, targetDirectory_);

    AutoPtr<CppCodeEmitter> serviceInterfaceCodeEmitter = new CppServiceInterfaceCodeEmitter(ast_, targetDirectory_);
    AutoPtr<CppCodeEmitter> serviceDriverCodeEmitter = new CppServiceDriverCodeEmitter(ast_, targetDirectory_);
    AutoPtr<CppCodeEmitter> serviceStubCodeEmitter = new CppServiceStubCodeEmitter(ast_, targetDirectory_);
    AutoPtr<CppCodeEmitter> serviceImplCodeEmitter = new CppServiceImplCodeEmitter(ast_, targetDirectory_);

    emitters_.push_back(clientInterfaceCodeEmitter);
    emitters_.push_back(clientProxyCodeEmitter);
    emitters_.push_back(serviceInterfaceCodeEmitter);
    emitters_.push_back(serviceDriverCodeEmitter);
    emitters_.push_back(serviceStubCodeEmitter);
    emitters_.push_back(serviceImplCodeEmitter);

    return true;
}

bool CppCodeGenerator::Generate() const
{
    for (auto emitter : emitters_) {
        if (!emitter->isInvaildDir()) {
            emitter->EmitCode();
        }
    }

    return true;
}

bool CppCodeGenerator::ResolveDirectory()
{
    String packageFilePath = String::Format("%s/%s/",
        targetDirectory_.string(), CppCodeEmitter::FileName(ast_->GetPackageName()).string());
    targetDirectory_ = packageFilePath;

    if (!File::CreateParentDir(targetDirectory_)) {
        Logger::E(TAG, "create '%s' directory failed!", targetDirectory_);
        return false;
    }
    return true;
}
} // namespace HDI
} // namespace OHOS