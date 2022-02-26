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

#ifndef APP_KVSTORE_OBSERVER_H
#define APP_KVSTORE_OBSERVER_H

#include "app_change_notification.h"

namespace OHOS {
namespace AppDistributedKv {
//  This is a abstract classes. Client needs to implement this class by self.
class AppKvStoreObserver {
public:
    KVSTORE_API AppKvStoreObserver() = default;

    KVSTORE_API virtual ~AppKvStoreObserver()
    {}
    // This virtual function will be called on store change.
    // Client needs to override this function to receive change notification.
    // Parameters:
    //     ChangeNotification: all changes from other devices.
    KVSTORE_API virtual void OnChange(const AppChangeNotification &appChangeNotification) = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS

#endif  // APP_KV_STORE_OBSERVER_H
