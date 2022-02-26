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

#ifndef REQUEST_REMOTE_PERMISSION_COMMAND_H
#define REQUEST_REMOTE_PERMISSION_COMMAND_H

namespace OHOS {
namespace Security {
namespace Permission {
/**
 * Command which used to request permission on other device.
 *
 */
class RequestRemotePermissionCommand : public BaseRemoteCommand {
public:
    void Prepare() override;

    void Execute() override;

    void Finish() override;

    std::string ToJsonPayload() override;
    RequestRemotePermissionCommand(const std::string &json);
    RequestRemotePermissionCommand(
        const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId);
    virtual ~RequestRemotePermissionCommand() = default;

    void SetRequestPermissionInfo(int32_t uid, const std::vector<std::string> &permissions,
        const std::string &bundleName, const std::string &reason);

private:
    void StartActivityForRequestPermission(std::vector<std::string> &permissions);
    std::vector<std::string> FormatRequestPermissions(
        std::vector<std::string> &permissions, const std::string &deviceId, int32_t ruid);

    /**
     * The command name. Should be equal to class name.
     */
    const std::string COMMAND_NAME = "RequestRemotePermissionCommand";

    std::string requestId_;
    std::vector<std::string> permissions_;
    std::string bundleName_;
    std::string reason_;
    int32_t uid_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif