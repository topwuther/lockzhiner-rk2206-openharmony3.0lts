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

#include "distributed_permission_event_handler.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
namespace {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {
    LOG_CORE, SECURITY_DOMAIN_PERMISSION, "DistributedPermissionEventHandler"};
}
DistributedPermissionEventHandler::DistributedPermissionEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    PERMISSION_LOG_INFO(LABEL, "enter");
}
DistributedPermissionEventHandler::~DistributedPermissionEventHandler() = default;

bool DistributedPermissionEventHandler::ProxyPostTask(const Callback &callback, int64_t delayTime)
{
    PERMISSION_LOG_DEBUG(LABEL, "PostTask without name");
    return AppExecFwk::EventHandler::PostTask(callback, delayTime);
}

bool DistributedPermissionEventHandler::ProxyPostTask(
    const Callback &callback, const std::string &name, int64_t delayTime)
{
    PERMISSION_LOG_DEBUG(LABEL, "PostTask with name");
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}
void DistributedPermissionEventHandler::ProxyRemoveTask(const std::string &name)
{
    AppExecFwk::EventHandler::RemoveTask(name);
}

}  // namespace Permission
}  // namespace Security
}  // namespace OHOS