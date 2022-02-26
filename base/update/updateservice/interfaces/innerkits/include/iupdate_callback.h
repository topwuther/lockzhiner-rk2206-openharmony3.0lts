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

#ifndef IUPDATER_CALLBACK_H
#define IUPDATER_CALLBACK_H

#include <iostream>
#include "update_helper.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace update_engine {
class IUpdateCallback : public OHOS::IRemoteBroker {
public:
    virtual ~IUpdateCallback() = default;
    enum {
        CHECK_VERSION = 1,
        DOWNLOAD,
        UPGRADE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Update.IUpdateCallback");
public:
    virtual void OnCheckVersionDone(const VersionInfo &info) = 0;

    virtual void OnDownloadProgress(const Progress &progress) = 0;

    virtual void OnUpgradeProgress(const Progress &progress) = 0;
};
} // namespace update_engine
} // namespace OHOS
#endif // IUPDATER_CALLBACK_H
