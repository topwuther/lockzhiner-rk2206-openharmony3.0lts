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

#ifndef OHOS_DISTRIBUTED_CALLER_INFO_H
#define OHOS_DISTRIBUTED_CALLER_INFO_H

namespace OHOS {
namespace DistributedSchedule {
enum {
    CALLER_TYPE_NONE = 0,
    CALLER_TYPE_HARMONY = 1,
};

struct CallerInfo {
    int32_t uid = -1;
    int32_t pid = -1;
    int32_t callerType = CALLER_TYPE_NONE;
    std::string sourceDeviceId;
    int32_t duid = -1;
    std::string callerAppId;
    std::vector<std::string> bundleNames;
};
} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CALLER_INFO_H