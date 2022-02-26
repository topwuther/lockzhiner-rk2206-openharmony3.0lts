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

#include "notification_normal_content.h"

namespace OHOS {
namespace Notification {
std::string NotificationNormalContent::Dump()
{
    return "NotificationNormalContent[ " + NotificationBasicContent::Dump() + " ]";
}

bool NotificationNormalContent::Marshalling(Parcel &parcel) const
{
    return NotificationBasicContent::Marshalling(parcel);
}

NotificationNormalContent *NotificationNormalContent::Unmarshalling(Parcel &parcel)
{
    auto pContent = new NotificationNormalContent();
    if ((nullptr != pContent) && !pContent->ReadFromParcel(parcel)) {
        delete pContent;
        pContent = nullptr;
    }

    return pContent;
}
}  // namespace Notification
}  // namespace OHOS
