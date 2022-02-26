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

#include "errors.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "sam_log.h"
#include "system_ability_manager.h"

using namespace OHOS;

int main(int argc, char *argv[])
{
    HILOGI("%{public}s called, enter System Ability Manager ", __func__);

    OHOS::sptr<OHOS::SystemAbilityManager> manager = OHOS::SystemAbilityManager::GetInstance();
    manager->Init();
    // Tell IPCThreadState we're the service manager
    OHOS::sptr<OHOS::IRemoteObject> serv = manager->AsObject();
    IPCSkeleton::SetContextObject(serv);

    // Create IPCThreadPool and join in.
    HILOGI("start System Ability Manager Loop");
    OHOS::IPCSkeleton::JoinWorkThread();
    return -1;
}