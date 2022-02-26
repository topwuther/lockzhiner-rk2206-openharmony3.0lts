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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_TEST_MODULETEST_ANS_MT_CONSTANT_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_TEST_MODULETEST_ANS_MT_CONSTANT_H

#include <string>

namespace OHOS {
namespace Notification {

namespace {
const std::string APP_NAME = "bundleName";
const std::string NOTIFICATION_LABEL_0 = "Label0";
const std::string NOTIFICATION_LABEL_1 = "Label1";
const std::string NOTIFICATION_LABEL_2 = "Label2";
const std::string AN_NOT_EXIST_KEY = "AN_NOT_EXIST_KEY";
const std::string KEY_SPLITER = "_";

constexpr int UID = 1;
constexpr int CANCEL_REASON_DELETE = 2;
constexpr int APP_CANCEL_REASON_DELETE = 8;
constexpr int APP_CANCEL_ALL_REASON_DELETE = 9;
}  // namespace

}  // namespace Notification
}  // namespace OHOS

#endif
