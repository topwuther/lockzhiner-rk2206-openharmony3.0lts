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

#ifndef INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_INFO_H_
#define INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_INFO_H_

#include <string>

#include "parcel.h"

namespace OHOS {
struct SystemAbilityInfo : public Parcelable {
    int32_t systemAbilityId = 0;    // id of this system ability
    std::string deviceId;         // deviceId of this system ability

    bool ReadFromParcel(Parcel& parcel);
    virtual bool Marshalling(Parcel& parcel) const override;
    static SystemAbilityInfo* Unmarshalling(Parcel& parcel);
};
} // namespace OHOS
#endif // INTERFACES_INNERKITS_SAMGR_INCLUDE_SYSTEM_ABILITY_INFO_H_
