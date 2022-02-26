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

#include "permission.h"
#include "field_const.h"

namespace OHOS {
namespace Security {
namespace Permission {
int DataTranslator::TranslationIntoPermissionDef(const GenericValues& inGenericValues, PermissionDef& outPermissionDef)
{
    outPermissionDef.permissionName = inGenericValues.GetString(FIELD_PERMISSION_NAME);
    outPermissionDef.bundleName = inGenericValues.GetString(FIELD_BUNDLE_NAME);
    outPermissionDef.grantMode = inGenericValues.GetInt(FIELD_GRANT_MODE);
    outPermissionDef.availableScope = inGenericValues.GetInt(FIELD_AVAILABLE_SCOPE);
    outPermissionDef.label = inGenericValues.GetString(FIELD_LABEL);
    outPermissionDef.labelId = inGenericValues.GetInt(FIELD_LABEL_ID);
    outPermissionDef.description = inGenericValues.GetString(FIELD_DESCRIPTION);
    outPermissionDef.descriptionId = inGenericValues.GetInt(FIELD_DESCRIPTION_ID);
    return RET_SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(const PermissionDef& inPermissionDef, GenericValues& outGenericValues)
{
    outGenericValues.Put(FIELD_PERMISSION_NAME, inPermissionDef.permissionName);
    outGenericValues.Put(FIELD_BUNDLE_NAME, inPermissionDef.bundleName);
    outGenericValues.Put(FIELD_GRANT_MODE, inPermissionDef.grantMode);
    outGenericValues.Put(FIELD_AVAILABLE_SCOPE, inPermissionDef.availableScope);
    outGenericValues.Put(FIELD_LABEL, inPermissionDef.label);
    outGenericValues.Put(FIELD_LABEL_ID, inPermissionDef.labelId);
    outGenericValues.Put(FIELD_DESCRIPTION, inPermissionDef.description);
    outGenericValues.Put(FIELD_DESCRIPTION_ID, inPermissionDef.descriptionId);
    return RET_SUCCESS;
}

int DataTranslator::TranslationIntoPermissionReq(const GenericValues& inGenericValues, PermissionReq& outPermissionReq)
{
    outPermissionReq.reqPermissionName = inGenericValues.GetString(FIELD_PERMISSION_NAME);
    outPermissionReq.isGranted = inGenericValues.GetInt(FIELD_GRANTED) != 0;
    outPermissionReq.flags = inGenericValues.GetInt(FIELD_FLAGS);
    return RET_SUCCESS;
}

int DataTranslator::TranslationIntoGenericValues(const PermissionReq& inPermissionReq, GenericValues& outGenericValues)
{
    outGenericValues.Put(FIELD_PERMISSION_NAME, inPermissionReq.reqPermissionName);
    outGenericValues.Put(FIELD_GRANTED, inPermissionReq.isGranted ? 1 : 0);
    outGenericValues.Put(FIELD_FLAGS, inPermissionReq.flags);
    return RET_SUCCESS;
}

std::string DataTranslator::ToString(const PermissionDef& inPermissionDef)
{
    std::string infos;
    infos.append(R"({"permissionName": ")" + inPermissionDef.permissionName + R"(")");
    infos.append(R"(, "bundleName": ")" + inPermissionDef.bundleName + R"(")");
    infos.append(R"(, "grantMode": )" + std::to_string(inPermissionDef.grantMode));
    infos.append(R"(, "availableScope": )" + std::to_string(inPermissionDef.availableScope));
    infos.append(R"(, "label": ")" + inPermissionDef.label + R"(")");
    infos.append(R"(, "labelId": )" + std::to_string(inPermissionDef.labelId));
    infos.append(R"(, "description": ")" + inPermissionDef.description + R"(")");
    infos.append(R"(, "descriptionId": )" + std::to_string(inPermissionDef.descriptionId));
    infos.append("}");
    return infos;
}
} // namespace Permission
} // namespace Security
} // namespace OHOS