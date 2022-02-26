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

#ifndef TIME_SERVICE_NOTIFY_H
#define TIME_SERVICE_NOTIFY_H

#include <common_event_publish_info.h>
#include <ohos/aafwk/content/want.h>
#include "time_common.h"


namespace OHOS {
namespace MiscServices {
class TimeServiceNotify {
public:
    TimeServiceNotify() = default;
    ~TimeServiceNotify() = default;
    void RegisterPublishEvents();
    void PublishTimeChanageEvents(int64_t eventTime);
    void PublishTimeZoneChangeEvents(int64_t eventTime);

private:
    using IntentWant = OHOS::AAFwk::Want;

    void PublishEvents(int64_t eventTime, sptr<IntentWant> intent);

    sptr<IntentWant> timeChangeWant_;
    sptr<IntentWant> timeZoneChangeWant_;
    sptr<OHOS::EventFwk::CommonEventPublishInfo> publishInfo_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // TIME_SERVICE_NOTIFY_H
