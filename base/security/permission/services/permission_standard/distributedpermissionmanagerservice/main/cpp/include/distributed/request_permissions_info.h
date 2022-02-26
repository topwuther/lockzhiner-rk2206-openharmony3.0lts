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
#ifndef REQUEST_PERMISSIONS_INFO_H
#define REQUEST_PERMISSIONS_INFO_H
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include "device_info_manager.h"
#include "distributed_data_validator.h"
#include "on_request_permissions_result.h"
#include "permission_log.h"
#include "object_device_permission_manager.h"

#include "refbase.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace Permission {
class RequestPermissionsInfo {
public:
    RequestPermissionsInfo(const std::vector<std::string> permissions, const sptr<OnRequestPermissionsResult> &callback,
        const std::string &nodeId, const std::string &bundleName, int32_t reasonResId);
    /**
     * Check if the process on service will stopped and return
     *
     * @return true: the process will stop. false: the process will continue
     */
    bool NeedStopProcess();
    /**
     * prepare to process the request, and format the input parameters
     *
     * @return parameters check is pass return true, otherwise false
     */
    bool CanRequestPermissions();
    bool InitObjectLocaleReason();
    /**
     * Use the permission input by request to calc the grant result by local data
     *
     * @return the grant result map to request permissions
     */
    std::vector<int32_t> GetGrantResult();

    std::string ToString();

public:
    /**
     * The Uniq Id of the request
     */
    std::string requestId_;

    /**
     * The input permissions from APP
     */
    std::vector<std::string> permissions_;

    /**
     * The input callback from APP
     */
    sptr<OnRequestPermissionsResult> callback_;

    /**
     * The input device id from APP
     */
    std::string nodeId_;

    /**
     * The udid convert from nodeId
     */
    std::string deviceId_;

    /**
     * The package name of the APP
     */
    std::string bundleName_;

    /**
     * The std::string id to get the reason std::string
     */
    int32_t reasonResId_;

    int32_t uid_;

    int32_t pid_;

    /**
     * The std::string reason get from the bundleName + reasonResId
     */
    std::string reasonString_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif