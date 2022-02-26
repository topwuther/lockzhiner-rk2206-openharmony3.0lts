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

#ifndef DISTRIBUTEDDATAMGR_FAULT_REPORTER_H
#define DISTRIBUTEDDATAMGR_FAULT_REPORTER_H

#include "dfx_types.h"

namespace OHOS {
namespace DistributedKv {
class FaultReporter {
public:
    KVSTORE_API virtual ReportStatus Report(const FaultMsg &msg) = 0;
    KVSTORE_API virtual ~FaultReporter() {}
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_FAULT_REPORTER_H
