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
#ifndef ON_REQUEST_PERMISSION_RESULT_H
#define ON_REQUEST_PERMISSION_RESULT_H
#include <string>
#include <map>
#include <iremote_broker.h>
namespace OHOS {
namespace Security {
namespace Permission {
class OnRequestPermissionsResult : public IRemoteBroker {
public:
    /**
     * The result of requestPermissionsFromRemote when User click OK
     *
     * @param nodeId The nodeId set in requestPermissionsFromRemote function
     * @param permissions The permissions set in requestPermissionsFromRemote function
     * @param grantResults The permissions grant result from remote UI
     */
    virtual void OnResult(
        const std::string nodeId, std::vector<std::string> permissions, std::vector<int32_t> grantResults) = 0;

    /**
     * The result of requestPermissionsFromRemote when User click cancel
     *
     * @param nodeId The nodeId set in requestPermissionsFromRemote function
     * @param permissions The permissions set in requestPermissionsFromRemote function
     */
    virtual void OnCancel(const std::string nodeId, std::vector<std::string> permissions) = 0;

    /**
     * The result of requestPermissionsFromRemote when User not click the UI and the request operation is time out
     *
     * @param nodeId The nodeId set in requestPermissionsFromRemote function
     * @param permissions The permissions set in requestPermissionsFromRemote function
     */
    virtual void OnTimeOut(const std::string nodeId, std::vector<std::string> permissions) = 0;
    enum class Message { ON_RESULT, ON_CANCEL, ON_TIME_OUT };

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.security.permission.OnRequestPermissionsResult");
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif