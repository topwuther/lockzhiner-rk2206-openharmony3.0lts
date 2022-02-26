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
#ifndef REQUEST_REMOTE_PERMISSION_CALLBACK_COMMAND_H
#define REQUEST_REMOTE_PERMISSION_CALLBACK_COMMAND_H

namespace OHOS {
namespace Security {
namespace Permission {
/**
 * Command which used to answer request permission on other device.
 *
 */
class RequestRemotePermissionCallbackCommand : public BaseRemoteCommand {
public:
    void Prepare() override;

    void Execute() override;

    void Finish() override;

    std::string ToJsonPayload() override;

    RequestRemotePermissionCallbackCommand(const std::string &json);
    RequestRemotePermissionCallbackCommand(const std::string &srcDeviceId, const std::string &dstDeviceId,
        const std::string &requestId, const int32_t uid, const std::string &bundleName);
    virtual ~RequestRemotePermissionCallbackCommand() = default;

private:
    /**
     * The command name. Should be equal to class name.
     */
    const std::string COMMAND_NAME = "RequestRemotePermissionCallbackCommand";
    /**
     * Current all granted the sensitive permission store in the object, used to sync to subject device
     */
    std::set<std::string> objectGrantedResult_;
    std::string requestId_;
    std::string bundleName_;
    int32_t uid_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif