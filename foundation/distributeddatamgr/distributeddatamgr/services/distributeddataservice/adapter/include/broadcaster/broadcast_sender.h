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

#ifndef BROADCAST_SENDER_H
#define BROADCAST_SENDER_H

#include <string>
#include <mutex>
#include "visibility.h"

namespace OHOS::DistributedKv {
struct EventParams {
    std::string userId;
    std::string appId;
    std::string storeId;
};

class BroadcastSender {
public:
    KVSTORE_API virtual void SendEvent(const EventParams &params) = 0;
    KVSTORE_API virtual ~BroadcastSender() {};
    KVSTORE_API static std::shared_ptr<BroadcastSender> GetInstance();
private:
    static std::mutex mutex_;
    static std::shared_ptr<BroadcastSender> instance_;
};
}
#endif // BROADCAST_SENDER_H
