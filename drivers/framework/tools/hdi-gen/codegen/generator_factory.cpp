/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "codegen/generator_factory.h"
#include "codegen/c_code_generator.h"
#include "codegen/cpp_code_generator.h"
#include "codegen/java_code_generator.h"

namespace OHOS {
namespace HDI {
AutoPtr<CodeGenerator> GeneratorFactory::GetCodeGenerator(const String& targetLanuage)
{
    if (targetLanuage.Equals("c")) {
        return new CCodeGenerator();
    } else if (targetLanuage.Equals("cpp")) {
        return new CppCodeGenerator();
    } else if (targetLanuage.Equals("java")) {
        return new JavaCodeGenerator();
    }

    return nullptr;
}
} // namespace HDI
} // namespace OHOS