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

#ifndef SERVICES_INCLUDE_TIME_COMMON_H
#define SERVICES_INCLUDE_TIME_COMMON_H

#include "errors.h"
#include "time_hilog_wreapper.h"

namespace OHOS {
namespace MiscServices {
#define TIME_SERVICE_NAME "TimeService"

enum TimeModule {
    TIME_MODULE_SERVICE_ID = 0x04,
};
// time error offset, used only in this file.
constexpr ErrCode TIME_ERR_OFFSET = ErrCodeOffset(SUBSYS_SMALLSERVICES, TIME_MODULE_SERVICE_ID);

enum TimeError {
    E_TIME_OK = TIME_ERR_OFFSET,
    E_TIME_SA_DIED,
    E_TIME_READ_PARCEL_ERROR,
    E_TIME_WRITE_PARCEL_ERROR,
    E_TIME_PUBLISH_FAIL,
    E_TIME_TRANSACT_ERROR,
    E_TIME_DEAL_FAILED,
    E_TIME_PARAMETERS_INVALID,
    E_TIME_SET_RTC_FAILED,
    E_TIME_NOT_FOUND,
    E_TIME_NO_PERMISSION,
};
} // namespace MiscServices
} // namespace OHOS
#endif // SERVICES_INCLUDE_TIME_COMMON_H