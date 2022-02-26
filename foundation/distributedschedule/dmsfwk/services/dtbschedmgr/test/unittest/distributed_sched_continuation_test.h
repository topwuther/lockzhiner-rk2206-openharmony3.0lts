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

#ifndef DISTRIBUTED_SCHED_CONTINUATION_TEST_H
#define DISTRIBUTED_SCHED_CONTINUATION_TEST_H

#include "distributed_sched_continuation.h"
#include "distributed_sched_proxy.h"
#include "distributed_sched_service.h"
#include "dtbschedmgr_log.h"
#include "gtest/gtest.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ohos/aafwk/content/want.h"
#include "system_ability_definition.h"
#include "test_log.h"

namespace OHOS {
namespace DistributedSchedule {
class DSchedContinuationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
protected:
    sptr<IRemoteObject> GetDSchedService() const;
    int32_t PushAbilityToken();
    std::shared_ptr<AAFwk::Want> MockWant(const std::string& bundleName, const std::string& ability,
        int32_t flags);
    std::shared_ptr<AppExecFwk::AbilityInfo> MockAbilityInfo(const std::string& bundleName,
        const std::string& ability, const std::string& devId);
    int32_t StartContinuation(const sptr<IRemoteObject>& abilityToken, int32_t flags);

    std::shared_ptr<DSchedContinuation> dschedContinuation_;
    bool timeoutFlag_ = false;
};
} // namespace DistributedSchedule
} // namespace OHOS

#endif // DISTRIBUTED_SCHED_CONTINUATION_TEST_H