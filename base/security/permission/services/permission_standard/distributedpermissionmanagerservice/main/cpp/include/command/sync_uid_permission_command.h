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

#ifndef SYNC_UID_PERMISSION_COMMAND_H
#define SYNC_UID_PERMISSION_COMMAND_H

#include "permission_fetcher.h"

namespace OHOS {
namespace Security {
namespace Permission {
/**
 * SyncUidPermissionCommand
 */
class SyncUidPermissionCommand : public BaseRemoteCommand {
public:
    void Prepare() override;

    void Execute() override;

    void Finish() override;

    std::string ToJsonPayload() override;

    SyncUidPermissionCommand(const std::string &json);
    SyncUidPermissionCommand(const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);
    virtual ~SyncUidPermissionCommand() = default;

private:
    /**
     * The command name. Should be equal to class name.
     */
    const std::string COMMAND_NAME = "SyncUidPermissionCommand";
    int32_t uid_;
    UidBundleBo uidPermission_;
    std::shared_ptr<PermissionFetcher> permissionFetcher_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif