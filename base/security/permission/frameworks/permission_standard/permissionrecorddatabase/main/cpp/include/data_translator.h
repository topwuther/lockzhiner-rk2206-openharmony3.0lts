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

#ifndef DATA_TRANSLATOR_H
#define DATA_TRANSLATOR_H

#include <string>

#include "bundle_permission_used_record.h"
#include "constant.h"
#include "generic_values.h"
#include "permission_record.h"
#include "permission_visitor.h"
#include "query_permission_used_request.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DataTranslator final {
public:
    static int TranslationIntoPermissionRecord(
        const GenericValues &inGenericValues, PermissionRecord &outPermissionRecord);
    static int TranslationIntoGenericValues(
        const PermissionRecord &inPermissionRecord, GenericValues &outGenericValues);
    static int TranslationIntoPermissionVisitor(
        const GenericValues &inGenericValues, PermissionVisitor &outPermissionVisitor);
    static int TranslationIntoGenericValues(
        const PermissionVisitor &inPermissionVisitor, GenericValues &outGenericValues);
    static int TranslationIntoGenericValues(const QueryPermissionUsedRequest &inQueryPermissionUsedRequest,
        GenericValues &outVisitorGenericValues, GenericValues &outRecordAndGenericValues,
        GenericValues &outRecordOrGenericValues);
    static int TranslationIntoBundlePermissionUsedRecord(
        const GenericValues &inGenericValues, BundlePermissionUsedRecord &outBundlePermissionUsedRecord);
    static int TranslationIntoPermissionUsedRecord(
        const GenericValues &inGenericValues, PermissionUsedRecord &outPermissionUsedRecord);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // DATA_TRANSLATOR_H
