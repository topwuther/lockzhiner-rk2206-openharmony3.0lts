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

#ifndef _ans_st_slotgroup_
#define _ans_st_slotgroup_

#include <string>
#include "ability_loader.h"
#include "notification_helper.h"
#ifdef PRINT_LOG
#undef PRINT_LOG
#endif
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

class AnsSlotGroupSystemTest : public Ability {
protected:
    virtual void OnStart(const Want &want) override;
    virtual void OnStop() override;
    virtual void OnActive() override;
    virtual void OnInactive() override;
    virtual void OnBackground() override;
    virtual void OnForeground(const Want &want) override;
    virtual void OnNewWant(const Want &want) override;

private:
    void Clear();
    void GetWantInfo(const Want &want);

    std::string shouldReturn;
    std::string targetBundle;
    std::string targetAbility;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif