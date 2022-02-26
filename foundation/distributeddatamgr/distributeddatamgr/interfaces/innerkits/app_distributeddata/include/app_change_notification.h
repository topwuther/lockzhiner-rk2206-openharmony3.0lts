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

#ifndef APP_CHANGE_NOTIFICATION_H
#define APP_CHANGE_NOTIFICATION_H

#include <time.h>
#include <list>
#include "app_types.h"

namespace OHOS {
namespace AppDistributedKv {
class AppChangeNotification final {
public:
    AppChangeNotification();

    // constructor using changing data.
    AppChangeNotification(const std::list<Entry> &insertEntries, const std::list<Entry> &updateEntries,
                          const std::list<Entry> &deleteEntries, const std::string &deviceId, const bool isClear);

    KVSTORE_API ~AppChangeNotification();

    // Function to get all inserted entries for this changing.
    KVSTORE_API const std::list<Entry> &GetInsertEntries() const;

    // Function to get all updated entries for this changing.
    KVSTORE_API const std::list<Entry> &GetUpdateEntries() const;

    // Function to get all deleted entries for this changing.
    KVSTORE_API const std::list<Entry> &GetDeleteEntries() const;

    // Function to get from device id.
    KVSTORE_API const std::string &GetDeviceId() const;

    // Function to check if this change is made by calling clear function.
    KVSTORE_API bool IsClear() const;

private:
    std::list<Entry> insertEntries_;

    std::list<Entry> updateEntries_;

    std::list<Entry> deleteEntries_;

    std::string deviceId_;

    bool isClear_ = false;
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_CHANGE_NOTIFICATION_H
