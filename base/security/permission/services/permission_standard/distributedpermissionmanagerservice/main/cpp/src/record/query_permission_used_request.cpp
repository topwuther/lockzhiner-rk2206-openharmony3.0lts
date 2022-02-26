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

#include "query_permission_used_request.h"

namespace OHOS {
namespace Security {
namespace Permission {
using namespace std;
bool QueryPermissionUsedRequest::Marshalling(Parcel &out) const
{
    if (!out.WriteString(this->deviceLabel)) {
        return false;
    }
    if (!out.WriteString(this->bundleName)) {
        return false;
    }
    if (!out.WriteStringVector(this->permissionNames)) {
        return false;
    }
    if (!out.WriteInt64(this->beginTimeMillis)) {
        return false;
    }
    if (!out.WriteInt64(this->endTimeMillis)) {
        return false;
    }
    if (!out.WriteInt32(this->flag)) {
        return false;
    }
    return true;
}

QueryPermissionUsedRequest *QueryPermissionUsedRequest::Unmarshalling(Parcel &in)
{
    auto *request = new (nothrow) QueryPermissionUsedRequest();
    if (request == nullptr) {
        delete request;
        request = nullptr;
        return request;
    }
    request->deviceLabel = in.ReadString();
    request->bundleName = in.ReadString();
    if (!in.ReadStringVector(&request->permissionNames)) {
        delete request;
        request = nullptr;
        return request;
    }
    if (!in.ReadInt64(request->beginTimeMillis)) {
        delete request;
        request = nullptr;
        return request;
    }
    if (!in.ReadInt64(request->endTimeMillis)) {
        delete request;
        request = nullptr;
        return request;
    }
    if (!in.ReadInt32(request->flag)) {
        delete request;
        request = nullptr;
        return request;
    }
    return request;
}

nlohmann::json QueryPermissionUsedRequest::to_json(const QueryPermissionUsedRequest &request) const
{
    nlohmann::json jsonObj = nlohmann::json{{"deviceLabel", request.deviceLabel},
        {"bundleName", request.bundleName},
        {"permissionNames", request.permissionNames},
        {"beginTimeMillis", request.beginTimeMillis},
        {"endTimeMillis", request.endTimeMillis},
        {"flag", request.flag}};
    return jsonObj;
}

void QueryPermissionUsedRequest::from_json(const nlohmann::json &jsonObj, QueryPermissionUsedRequest &request)
{
    jsonObj.at("deviceLabel").get_to(request.deviceLabel);
    jsonObj.at("bundleName").get_to(request.bundleName);
    jsonObj.at("permissionNames").get_to<std::vector<std::string>>(request.permissionNames);
    jsonObj.at("beginTimeMillis").get_to(request.beginTimeMillis);
    jsonObj.at("endTimeMillis").get_to(request.endTimeMillis);
    jsonObj.at("flag").get_to(request.flag);
}
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS