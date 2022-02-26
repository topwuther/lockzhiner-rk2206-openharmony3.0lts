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

#ifndef FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_DATA_H
#define FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_DATA_H

#include <string>
#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace EventFwk {
using Want = OHOS::AAFwk::Want;

class CommonEventData : public Parcelable {
public:
    /**
     * Default constructor used to create an empty CommonEventData instance.
     */
    CommonEventData();

    /**
     * Create a CommonEventData instance based on the want parameter to set
     * the content of a common want.
     *
     * @param want the want of a common event
     */
    explicit CommonEventData(const Want &want);

    /**
     * Create a CommonEventData instance based on the parameters want, code,
     * and data to set the content of the common event.
     *
     * @param want the want of a common event
     * @param code the code of a common event
     * @param data the data of a common event
     */
    CommonEventData(const Want &want, const int &code, const std::string &data);

    /**
     * Default deconstructor used to deconstruct.
     *
     */
    ~CommonEventData();

    /**
     * Set the want attribute of a common event.
     *
     * @param want the want of a common event
     */
    void SetWant(const Want &want);

    /**
     * Obtain the Want attribute of a common event.
     *
     * @return the want of a common event
     */
    const Want &GetWant() const;

    /**
     * Set the result code of the common event.
     *
     * @param code the code of a common event
     */
    void SetCode(const int &code);

    /**
     * Obtain the result code of a common event.
     *
     * @return the code of a common event
     */
    int GetCode() const;

    /**
     * Set the result data of a common event.
     *
     * @param data the data of a common event
     */
    void SetData(const std::string &data);

    /**
     * Obtain the result data of a common event, which is set by
     * setData(java.lang.String).
     *
     * @return the data of a common event
     */
    std::string GetData() const;

    /**
     * Marshal a common event data object into a Parcel.
     *
     * @param parcel the common event data into the parcel
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * Unmarshal this common event data object from a Parcel.
     * @return the common event data
     */
    static CommonEventData *Unmarshalling(Parcel &parcel);

private:
    /**
     * Read a CommonEventData object from a Parcel.
     *
     * @param parcel the parcel
     */
    bool ReadFromParcel(Parcel &parcel);

private:
    Want want_;
    int code_;
    std::string data_;
};
}  // namespace EventFwk
}  // namespace OHOS

#endif  // FOUNDATION_EVENT_CESFWK_KITS_NATIVE_INCLUDE_COMMON_EVENT_DATA_H