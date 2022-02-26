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

#ifndef REMOTE_COMMAND_FACTORY_H
#define REMOTE_COMMAND_FACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "sync_uid_permission_command.h"
#include "get_uid_permission_command.h"
#include "delete_uid_permission_command.h"
#include "request_remote_permission_command.h"
#include "request_remote_permission_callback_command.h"
#include "sync_uid_state_command.h"

namespace OHOS {
namespace Security {
namespace Permission {
class RemoteCommandFactory {
public:
    static RemoteCommandFactory &GetInstance();

    std::shared_ptr<SyncUidPermissionCommand> NewSyncUidPermissionCommand(
        const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);

    std::shared_ptr<GetUidPermissionCommand> NewGetUidPermissionCommand(
        const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);

    std::shared_ptr<DeleteUidPermissionCommand> NewDeleteUidPermissionCommand(
        const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);

    std::shared_ptr<RequestRemotePermissionCommand> NewRequestRemotePermissionCommand(
        const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId);

    std::shared_ptr<RequestRemotePermissionCallbackCommand> NewRequestRemotePermissionCallbackCommand(
        const std::string &srcDeviceId, const std::string &dstDeviceId, const std::string &requestId, const int32_t uid,
        const std::string &bundleName);

    std::shared_ptr<SyncUidStateCommand> NewSyncUidStateCommand(
        const int32_t uid, const std::string &srcDeviceId, const std::string &dstDeviceId);

    std::shared_ptr<BaseRemoteCommand> NewRemoteCommandFromJson(
        const std::string &commandName, const std::string &commandJsonString);

private:
    const std::string TAG = "RemoteCommandFactory";
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // REMOTE_COMMAND_FACTORY_H
