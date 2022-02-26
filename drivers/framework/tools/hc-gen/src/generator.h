/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HC_GEN_GENERATOR_H
#define HC_GEN_GENERATOR_H

#include <memory>
#include <string>
#include "ast.h"

namespace OHOS {
namespace Hardware {
class Generator {
public:
    Generator(std::shared_ptr<Ast> ast) : ast_(ast) {};

    virtual ~Generator() = default;

    virtual bool Output() = 0;

protected:
    std::shared_ptr<Ast> ast_;
};
} // Hardware
} // OHOS

#endif // HC_GEN_GENERATOR_H
