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

#ifndef UPDATER_CALLBACK_H
#define UPDATER_CALLBACK_H

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "iupdate_callback.h"
#include "update_callback_stub.h"
#include "update_helper.h"
#include "system_ability.h"

namespace OHOS {
namespace update_engine {
class UpdateCallback : public UpdateCallbackStub {
public:
    explicit UpdateCallback() = default;

    ~UpdateCallback() = default;

    void OnCheckVersionDone(const VersionInfo &info) override;

    void OnDownloadProgress(const Progress &progress) override;

    void OnUpgradeProgress(const Progress &progress) override;
};
}
} // namespace OHOS
#endif // UPDATER_CALLBACK_H