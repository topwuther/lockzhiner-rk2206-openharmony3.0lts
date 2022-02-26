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

#ifndef APP_DISTRIBUTEDDATA_INCLUDE_DATA_CHANGE_LISTENER_H
#define APP_DISTRIBUTEDDATA_INCLUDE_DATA_CHANGE_LISTENER_H

#include "app_types.h"
#include "visibility.h"
namespace OHOS {
namespace AppDistributedKv {
class AppDataChangeListener {
public:
    KVSTORE_API AppDataChangeListener() = default;
    KVSTORE_API virtual ~AppDataChangeListener() {};

    KVSTORE_API virtual void OnMessage(const DeviceInfo &info, const uint8_t *ptr, const int size,
                                       const PipeInfo &pipeInfo) const = 0;
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#endif  // APP_DISTRIBUTEDDATA_INCLUDE_DATA_CHANGE_LISTENER_H
