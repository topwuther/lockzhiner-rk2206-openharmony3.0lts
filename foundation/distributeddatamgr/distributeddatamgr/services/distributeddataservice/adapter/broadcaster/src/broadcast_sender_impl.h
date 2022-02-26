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

#ifndef BROADCAST_SENDER_IMPL_H
#define BROADCAST_SENDER_IMPL_H

#include "broadcast_sender.h"

namespace OHOS::DistributedKv {
class BroadcastSenderImpl : public BroadcastSender {
public:
    void SendEvent(const EventParams &params) override;
private:
    static const inline std::string ACTION_NAME = "DistributedDataMgrStarter";
    static const inline std::string PKG_NAME = "pkgName";
};
}
#endif // BROADCAST_SENDER_IMPL_H
