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

#ifndef DISTRIBUTEDDATAMGR_UNINSTALLER_IMPL_H
#define DISTRIBUTEDDATAMGR_UNINSTALLER_IMPL_H

#include "common_event_subscriber.h"
#include "uninstaller.h"

namespace OHOS::DistributedKv {
using namespace OHOS::EventFwk;
using UninstallEventCallback = std::function<void(const std::string &bundleName, int userId)>;

class UninstallEventSubscriber : public CommonEventSubscriber {
public:
    UninstallEventSubscriber(const CommonEventSubscribeInfo &info,
        UninstallEventCallback callback);

    ~UninstallEventSubscriber() {};
    void OnReceiveEvent(const CommonEventData &event) override;
private:
    static const std::string USER_ID;
    UninstallEventCallback callback_;
};
class UninstallerImpl : public Uninstaller {
public:
    ~UninstallerImpl();

    Status Init(KvStoreDataService *kvStoreDataService) override;
private:
    std::shared_ptr<UninstallEventSubscriber> subscriber_ {};
};
}
#endif // DISTRIBUTEDDATAMGR_UNINSTALLER_IMPL_H
