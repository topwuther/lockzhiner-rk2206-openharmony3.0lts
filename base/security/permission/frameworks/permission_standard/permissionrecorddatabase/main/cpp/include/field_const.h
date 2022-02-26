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

#ifndef FIELD_CONST_H
#define FIELD_CONST_H

#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
const std::string FIELD_ID = "id";
const std::string FIELD_DEVICE_ID = "device_id";
const std::string FIELD_DEVICE_NAME = "device_name";
const std::string FIELD_BUNDLE_USER_ID = "bundle_user_id";
const std::string FIELD_BUNDLE_NAME = "bundle_name";
const std::string FIELD_BUNDLE_LABEL = "bundle_label";
const std::string FIELD_TIMESTAMP = "timestamp";
const std::string FIELD_TIMESTAMP_BEGIN = "timestamp_begin";
const std::string FIELD_TIMESTAMP_END = "timestamp_end";
const std::string FIELD_VISITOR_ID = "visitor_id";
const std::string FIELD_OP_CODE = "op_code";
const std::string FIELD_IS_FOREGROUND = "is_foreground";
const std::string FIELD_ACCESS_COUNT = "access_count";
const std::string FIELD_REJECT_COUNT = "reject_count";
const std::string FIELD_PERMISSION_NAMES = "permissionNames";
const std::string FIELD_BEGIN_TIME_MILLIS = "beginTimeMillis";
const std::string FIELD_END_TIME_MILLIS = "endTimeMillis";
const std::string FIELD_FLAG = "flag";
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // FIELD_CONST_H
