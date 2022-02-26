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

#include "broadcast_sender.h"
#include <memory>
#include "broadcast_sender_impl.h"

namespace OHOS::DistributedKv {
std::shared_ptr<BroadcastSender> BroadcastSender::instance_;
std::mutex BroadcastSender::mutex_;
std::shared_ptr<BroadcastSender> BroadcastSender::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<BroadcastSenderImpl>();
        }
    }
    return instance_;
}
}
