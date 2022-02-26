/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef OHOS_HDI_GENERATORFACTORY_H
#define OHOS_HDI_GENERATORFACTORY_H

#include "codegen/code_generator.h"

namespace OHOS {
namespace HDI {
class GeneratorFactory {
public:
    AutoPtr<CodeGenerator> GetCodeGenerator(const String& targetLanuage);
};
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_GENERATORFACTORY_H