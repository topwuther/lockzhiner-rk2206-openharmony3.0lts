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

#include "data_translator.h"

#include "field_const.h"

namespace OHOS {
namespace Security {
namespace Permission {
int DataTranslator::TranslationIntoPermissionRecord(
    const GenericValues &inGenericValues, PermissionRecord &outPermissionRecord)
{
    outPermissionRecord.timestamp = inGenericValues.GetInt(FIELD_TIMESTAMP);
    outPermissionRecord.visitorId = inGenericValues.GetInt(FIELD_VISITOR_ID);
    outPermissionRecord.opCode = inGenericValues.GetInt(FIELD_OP_CODE);
    outPermissionRecord.isForeground = inGenericValues.GetInt(FIELD_IS_FOREGROUND);
    outPermissionRecord.accessCount = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
    outPermissionRecord.rejectCount = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(
    const PermissionRecord &inPermissionRecord, GenericValues &outGenericValues)
{
    outGenericValues.Put(FIELD_TIMESTAMP, inPermissionRecord.timestamp);
    outGenericValues.Put(FIELD_VISITOR_ID, inPermissionRecord.visitorId);
    outGenericValues.Put(FIELD_OP_CODE, inPermissionRecord.opCode);
    outGenericValues.Put(FIELD_IS_FOREGROUND, inPermissionRecord.isForeground);
    outGenericValues.Put(FIELD_ACCESS_COUNT, inPermissionRecord.accessCount);
    outGenericValues.Put(FIELD_REJECT_COUNT, inPermissionRecord.rejectCount);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoPermissionVisitor(
    const GenericValues &inGenericValues, PermissionVisitor &outPermissionVisitor)
{
    outPermissionVisitor.id = inGenericValues.GetInt(FIELD_ID);
    outPermissionVisitor.deviceId = inGenericValues.GetString(FIELD_DEVICE_ID);
    outPermissionVisitor.deviceName = inGenericValues.GetString(FIELD_DEVICE_NAME);
    outPermissionVisitor.bundleUserId = inGenericValues.GetInt(FIELD_BUNDLE_USER_ID);
    outPermissionVisitor.bundleName = inGenericValues.GetString(FIELD_BUNDLE_NAME);
    outPermissionVisitor.bundleLabel = inGenericValues.GetString(FIELD_BUNDLE_LABEL);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(
    const PermissionVisitor &inPermissionVisitor, GenericValues &outGenericValues)
{
    outGenericValues.Put(FIELD_DEVICE_ID, inPermissionVisitor.deviceId);
    outGenericValues.Put(FIELD_DEVICE_NAME, inPermissionVisitor.deviceName);
    outGenericValues.Put(FIELD_BUNDLE_USER_ID, inPermissionVisitor.bundleUserId);
    outGenericValues.Put(FIELD_BUNDLE_NAME, inPermissionVisitor.bundleName);
    outGenericValues.Put(FIELD_BUNDLE_LABEL, inPermissionVisitor.bundleLabel);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(const QueryPermissionUsedRequest &inQueryPermissionUsedRequest,
    GenericValues &outVisitorGenericValues, GenericValues &outRecordAndGenericValues,
    GenericValues &outRecordOrGenericValues)
{
    if (!inQueryPermissionUsedRequest.deviceLabel.empty()) {
        outVisitorGenericValues.Put(FIELD_DEVICE_NAME, inQueryPermissionUsedRequest.deviceLabel);
    }
    if (!inQueryPermissionUsedRequest.bundleName.empty()) {
        outVisitorGenericValues.Put(FIELD_BUNDLE_NAME, inQueryPermissionUsedRequest.bundleName);
    }
    std::string permissionNames;
    for (unsigned int i = 0; i < inQueryPermissionUsedRequest.permissionNames.size(); i++) {
        int32_t code = 0;
        std::string perName = inQueryPermissionUsedRequest.permissionNames[i];
        if (Constant::PermissionNameToOrFromOpCode(perName, code)) {
            outRecordOrGenericValues.Put(FIELD_OP_CODE + ToString(i), code);
        }
    }
    int64_t begin = inQueryPermissionUsedRequest.beginTimeMillis;
    int64_t end = inQueryPermissionUsedRequest.endTimeMillis;
    if (begin < 0 || end < 0 || (begin > end && end > 0)) {
        return Constant::FAILURE;
    }
    if (begin != 0) {
        outRecordAndGenericValues.Put(FIELD_TIMESTAMP_BEGIN, begin);
    }
    if (end != 0) {
        outRecordAndGenericValues.Put(FIELD_TIMESTAMP_END, end);
    }
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoBundlePermissionUsedRecord(
    const GenericValues &inGenericValues, BundlePermissionUsedRecord &outBundlePermissionUsedRecord)
{
    outBundlePermissionUsedRecord.deviceId = inGenericValues.GetString(FIELD_DEVICE_ID);
    outBundlePermissionUsedRecord.deviceLabel = inGenericValues.GetString(FIELD_DEVICE_NAME);
    outBundlePermissionUsedRecord.bundleName = inGenericValues.GetString(FIELD_BUNDLE_NAME);
    outBundlePermissionUsedRecord.bundleLabel = inGenericValues.GetString(FIELD_BUNDLE_LABEL);
    return Constant::SUCCESS;
}

int DataTranslator::TranslationIntoPermissionUsedRecord(
    const GenericValues &inGenericValues, PermissionUsedRecord &outPermissionUsedRecord)
{
    std::string perName;
    int32_t code = inGenericValues.GetInt(FIELD_OP_CODE);
    if (!Constant::PermissionNameToOrFromOpCode(perName, code)) {
        return Constant::FAILURE;
    }
    outPermissionUsedRecord.permissionName = perName;
    if (inGenericValues.GetInt(FIELD_IS_FOREGROUND) == 1) {
        outPermissionUsedRecord.accessCountFg = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
        outPermissionUsedRecord.rejectCountFg = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    } else {
        outPermissionUsedRecord.accessCountBg = inGenericValues.GetInt(FIELD_ACCESS_COUNT);
        outPermissionUsedRecord.rejectCountBg = inGenericValues.GetInt(FIELD_REJECT_COUNT);
    }
    if (inGenericValues.GetInt(FIELD_ACCESS_COUNT) > inGenericValues.GetInt(FIELD_REJECT_COUNT)) {
        if (inGenericValues.GetInt(FIELD_REJECT_COUNT) != 0) {
            outPermissionUsedRecord.lastRejectTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
        }
        outPermissionUsedRecord.lastAccessTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
    } else {
        if (inGenericValues.GetInt(FIELD_ACCESS_COUNT) != 0) {
            outPermissionUsedRecord.lastAccessTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
        }
        outPermissionUsedRecord.lastRejectTime = inGenericValues.GetInt64(FIELD_TIMESTAMP);
    }
    if (inGenericValues.GetInt(FIELD_FLAG) == 1) {
        if (outPermissionUsedRecord.accessCountFg > 0 || outPermissionUsedRecord.rejectCountFg > 0) {
            outPermissionUsedRecord.lastAccessTime > 0
                ? outPermissionUsedRecord.accessRecordFg.emplace_back(outPermissionUsedRecord.lastAccessTime)
                : outPermissionUsedRecord.rejectRecordFg.emplace_back(outPermissionUsedRecord.lastRejectTime);
        } else {
            outPermissionUsedRecord.lastAccessTime > 0
                ? outPermissionUsedRecord.accessRecordBg.emplace_back(outPermissionUsedRecord.lastAccessTime)
                : outPermissionUsedRecord.rejectRecordBg.emplace_back(outPermissionUsedRecord.lastRejectTime);
        }
    }

    return Constant::SUCCESS;
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS