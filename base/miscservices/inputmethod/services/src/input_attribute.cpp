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

#include "input_attribute.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    */
    InputAttribute::InputAttribute() : enterKeyType(0), inputOption(0)
    {
    }

    /*! Constructor
      \param attribute the source attribute copied to this instance
    */
    InputAttribute::InputAttribute(const InputAttribute& attribute)
    {
        inputPattern = attribute.inputPattern;
        enterKeyType = attribute.enterKeyType;
        inputOption = attribute.inputOption;
    }

    /*! Destructor
    */
    InputAttribute::~InputAttribute()
    {
    }

    /*! operator=
      \param attribute the source attribute copied to this instance
      \return return this
    */
    InputAttribute& InputAttribute::operator =(const InputAttribute& attribute)
    {
        if (this == &attribute) {
            return *this;
        }
        inputPattern = attribute.inputPattern;
        enterKeyType = attribute.enterKeyType;
        inputOption = attribute.inputOption;
        return *this;
    }

    /*! Write InputAttribute to parcel
      \param[out] parcel write the data of InputAttribute to this parcel returned to caller
      \return ErrorCode::NO_ERROR
      \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    bool InputAttribute::Marshalling(OHOS::Parcel &parcel) const
    {
        if (!(parcel.WriteInt32(inputPattern)
            && parcel.WriteInt32(enterKeyType)
            && parcel.WriteInt32(inputOption)))
            return false;
        return true;
    }

    /*! Read InputAttribute from parcel
      \param parcel read the data of InputAttribute from this parcel
      \return ErrorCode::NO_ERROR
      \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    InputAttribute *InputAttribute::Unmarshalling(OHOS::Parcel &parcel)
    {
        auto info = new InputAttribute();
        info->inputPattern = parcel.ReadInt32();
        info->enterKeyType = parcel.ReadInt32();
        info->inputOption = parcel.ReadInt32();
        return info;
    }

    /*! Get the security flag
      \return true - It's password EditView. The input method management service should start system
      security input method engine for this view.
      \return false - It's an normal view.
    */
    bool InputAttribute::GetSecurityFlag()
    {
        if (inputPattern == PATTERN_PASSWORD) {
            return true;
        }
        return false;
    }

    /*! Set input pattern.
        \n It's added for Unit test of PerUserSession
        \param pattern PATTERN_PASSWORD or 0.
    */
    void InputAttribute::SetInputPattern(int32_t pattern)
    {
        inputPattern = pattern;
    }
}
}
