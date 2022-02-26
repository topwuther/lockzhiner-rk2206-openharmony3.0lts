/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_CONST_H
#define TEST_CONST_H

#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
static const std::string TEST_BUNDLE_NAME = "ohos";
static const std::string TEST_PERMISSION_NAME_ALPHA = "ohos.permission.ALPHA";
static const std::string TEST_PERMISSION_NAME_BETA = "ohos.permission.BETA";
static const std::string TEST_PERMISSION_NAME_GAMMA = "ohos.permission.GAMMA";
static const std::string TEST_LABEL = "test label";
static const std::string TEST_DESCRIPTION = "test description";

static const int TEST_LABEL_ID = 9527;
static const int TEST_DESCRIPTION_ID = 9528;
static const int TEST_INVALID_USER_ID = -1;
static const int TEST_USER_ID = 0;
static const int TEST_SUB_USER_ID = 10;
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // TEST_CONST_H
