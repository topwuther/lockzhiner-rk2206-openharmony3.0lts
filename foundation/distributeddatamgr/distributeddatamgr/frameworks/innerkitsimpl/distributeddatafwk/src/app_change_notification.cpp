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

#include "app_change_notification.h"

namespace OHOS {
namespace AppDistributedKv {
AppChangeNotification::AppChangeNotification()
{}

AppChangeNotification::AppChangeNotification(const std::list<Entry> &insertEntries,
                                             const std::list<Entry> &updateEntries,
                                             const std::list<Entry> &deleteEntries,
                                             const std::string &deviceId,
                                             const bool isClear)
    : insertEntries_(insertEntries), updateEntries_(updateEntries), deleteEntries_(deleteEntries),
      deviceId_(deviceId), isClear_(isClear)
{}

AppChangeNotification::~AppChangeNotification()
{}

const std::list<Entry> &AppChangeNotification::GetInsertEntries() const
{
    return this->insertEntries_;
}

const std::list<Entry> &AppChangeNotification::GetUpdateEntries() const
{
    return this->updateEntries_;
}

const std::list<Entry> &AppChangeNotification::GetDeleteEntries() const
{
    return this->deleteEntries_;
}

const std::string &AppChangeNotification::GetDeviceId() const
{
    return this->deviceId_;
}

bool AppChangeNotification::IsClear() const
{
    return this->isClear_;
}
}  // namespace AppDistributedKv
}  // namespace OHOS
