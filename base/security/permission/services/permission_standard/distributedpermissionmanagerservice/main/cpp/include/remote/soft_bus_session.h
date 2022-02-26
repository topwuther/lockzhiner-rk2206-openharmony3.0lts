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

#ifndef SOFT_BUS_SESSION_H
#define SOFT_BUS_SESSION_H

#include <string>
#include "device_info.h"

namespace OHOS {
namespace Security {
namespace Permission {
struct SoftBusSession {
    bool inited = false;
    int sessionId = -1;
    DeviceId deviceId;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // SOFT_BUS_SESSION_LISTENER_H
