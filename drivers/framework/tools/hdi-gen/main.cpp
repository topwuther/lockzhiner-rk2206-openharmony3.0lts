/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/code_generator.h"
#include "codegen/generator_factory.h"
#include "parser/module_parser.h"
#include "util/file.h"
#include "util/logger.h"
#include "util/options.h"

using namespace OHOS::HDI;

int main(int argc, char** argv)
{
    Options& options = Options::GetInstance().Parse(argc, argv);

    if (options.HasErrors()) {
        options.ShowErrors();
        return 0;
    }

    if (options.DoShowUsage()) {
        options.ShowUsage();
        return 0;
    }

    if (options.DoShowVersion()) {
        options.ShowVersion();
        return 0;
    }

    if (!options.DoCompile()) {
        return 0;
    }

    if (options.DoGetHashKey()) {
        std::unique_ptr<File> idlFile = std::make_unique<File>(options.GetSourceFile(), int(File::READ));
        if (!idlFile->IsValid()) {
            Logger::E("hdi-gen", "open idl file failed!");
            return -1;
        }

        printf("%s:%lu\n", idlFile->GetPath().string(), idlFile->GetHashKey());
        return 0;
    }

    ModuleParser moduleParser(options);
    if (!moduleParser.ParserDependencies()) {
        Logger::E("hdi-gen", "Parsing dependencies failed.");
        return -1;
    }

    if (!moduleParser.CompileFiles()) {
        Logger::E("hdi-gen", "Parsing .idl failed.");
        return -1;
    }

    AutoPtr<ASTModule> astModule = moduleParser.GetAStModule();

    if (!options.DoGenerateCode()) {
        return 0;
    }

    for (auto& astPair : astModule->GetAllAsts()) {
        AutoPtr<AST> ast = astPair.second;
        GeneratorFactory factory;
        AutoPtr<CodeGenerator> codeGen = factory.GetCodeGenerator(options.GetTargetLanguage());
        if (codeGen == nullptr) {
            Logger::E("hdi-gen", "new Generate failed.");
            return -1;
        }

        if (!codeGen->Initializate(ast, options.GetGenerationDirectory())) {
            Logger::E("hdi-gen", "Generate initializate failed.");
            return -1;
        }

        if (!codeGen->Generate()) {
            Logger::E("hdi-gen", "Generate \"%s\" codes failed.", options.GetTargetLanguage().string());
            return -1;
        }
    }

    return 0;
}