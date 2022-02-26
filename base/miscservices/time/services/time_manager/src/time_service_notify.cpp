/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "time_service_notify.h"

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include <unistd.h>

using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace OHOS {
namespace MiscServices {
void TimeServiceNotify::RegisterPublishEvents()
{
    if (publishInfo_ != nullptr) {
        return;
    }
    publishInfo_ = new (std::nothrow)CommonEventPublishInfo();
    publishInfo_->SetOrdered(false);
    timeChangeWant_ = new (std::nothrow)IntentWant();
    timeChangeWant_->SetAction(CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    timeZoneChangeWant_ = new (std::nothrow)IntentWant();
    timeZoneChangeWant_->SetAction(CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED);
}

void TimeServiceNotify::PublishEvents(int64_t eventTime, sptr<IntentWant> want)
{
    if ((want == nullptr) || (publishInfo_ == nullptr)) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "Invalid parameter");
        return;
    }

    TIME_HILOGI(TIME_MODULE_SERVICE, "Start to publish event %{public}s at %{public}lld",
        want->GetAction().c_str(), static_cast<long long>(eventTime));
    CommonEventData event(*want);
    CommonEventManager::PublishCommonEvent(event, *publishInfo_, nullptr);
    TIME_HILOGI(TIME_MODULE_SERVICE, "Publish event %{public}s done", want->GetAction().c_str());
}

void TimeServiceNotify::PublishTimeChanageEvents(int64_t eventTime)
{
    PublishEvents(eventTime, timeChangeWant_);
}

void TimeServiceNotify::PublishTimeZoneChangeEvents(int64_t eventTime)
{
    PublishEvents(eventTime, timeZoneChangeWant_);
}
} // MiscService 
} // OHOS
