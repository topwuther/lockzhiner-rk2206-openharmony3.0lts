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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_H

#include <string>
#include <memory>
#include "parcel.h"
#include "pending_want.h"
#include "want.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
class WantAgent final : public std::enable_shared_from_this<WantAgent>, public Parcelable {
private:
    std::shared_ptr<PendingWant> pendingWant_;

    /**
     * Constructor.
     *
     * @param obj The proxy object.
     */
public:
    WantAgent(){};
    virtual ~WantAgent() = default;
    explicit WantAgent(const std::shared_ptr<PendingWant> &pendingWant);

    /**
     * Gets proxy obj.
     *
     * @return Return obj.
     */
    std::shared_ptr<PendingWant> GetPendingWant();

    /**
     * Sets proxy obj.
     *
     * @return Return obj.
     */
    void SetPendingWant(const std::shared_ptr<PendingWant> &pendingWant);

    /**
     * @description: Marshals a Want into a Parcel.
     * Fields in the Want are marshalled separately. If any field fails to be marshalled, false is returned.
     * @param parcel Indicates the Parcel object for marshalling.
     * @return Returns true if the marshalling is successful; returns false otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const;

    /**
     * @description: Unmarshals a Want from a Parcel.
     * Fields in the Want are unmarshalled separately. If any field fails to be unmarshalled, false is returned.
     * @param parcel Indicates the Parcel object for unmarshalling.
     * @return Returns true if the unmarshalling is successful; returns false otherwise.
     */
    static WantAgent *Unmarshalling(Parcel &parcel);
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_H