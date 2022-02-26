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

#include "input_method_property.h"

namespace OHOS {
namespace MiscServices {
    using namespace std;
    /*! Constructor
    */
    InputMethodProperty::InputMethodProperty()
    {
    }

    /*! Destructor
    */
    InputMethodProperty::~InputMethodProperty()
    {
        for (int32_t i = 0; i < (int32_t)mTypes.size(); i++) {
            delete mTypes[i];
        }
        mTypes.clear();
    }

    /*! Constructor
    \param property the source property will be copied to this instance.
    */
    InputMethodProperty::InputMethodProperty(const InputMethodProperty& property)
    {
        mImeId = property.mImeId;
        mPackageName = property.mPackageName;
        mAbilityName = property.mAbilityName;
        mConfigurationPage = property.mConfigurationPage;
        isSystemIme = property.isSystemIme;
        mDefaultImeId = property.mDefaultImeId;

        for (int i = 0; i < (int)mTypes.size(); i++) {
            KeyboardType *type = new KeyboardType(*property.mTypes[i]);
            mTypes.push_back(type);
        }
    }

    /*! operator=
    \param property the source property will be copied to this instance.
    \return return this
    */
    InputMethodProperty& InputMethodProperty::operator =(const InputMethodProperty& property)
    {
        if (this == &property) {
            return *this;
        }
        mImeId = property.mImeId;
        mPackageName = property.mPackageName;
        mAbilityName = property.mAbilityName;
        mConfigurationPage = property.mConfigurationPage;
        isSystemIme = property.isSystemIme;
        mDefaultImeId = property.mDefaultImeId;

        for (int i = 0; i < (int)mTypes.size(); i++) {
            KeyboardType *type = new KeyboardType(*property.mTypes[i]);
            mTypes.push_back(type);
        }
        return *this;
    }

    /*! Write InputMethodProperty to parcel
    \param[out] parcel the information is written to this parcel returned to caller
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    bool InputMethodProperty::Marshalling(Parcel &parcel) const
    {
        if (!(parcel.WriteString16(mImeId)
            && parcel.WriteString16(mPackageName)
            && parcel.WriteString16(mAbilityName)
            && parcel.WriteString16(mConfigurationPage)
            && parcel.WriteBool(isSystemIme)
            && parcel.WriteInt32(mDefaultImeId)))
            return false;
        int32_t size = (int32_t)mTypes.size();
        parcel.WriteInt32(size);
        if (size == 0) {
            return true;
        }
        for (int i = 0; i < size; i++) {
            parcel.WriteParcelable(mTypes[i]);
        }
        return true;
    }

    /*! Get InputMethodProperty from parcel
    \param parcel read InputMethodProperty from this parcel
    \return ErrorCode::NO_ERROR
    \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    InputMethodProperty *InputMethodProperty::Unmarshalling(Parcel &parcel)
    {
        auto info = new InputMethodProperty();
        info->mImeId = parcel.ReadString16();
        info->mPackageName = parcel.ReadString16();
        info->mAbilityName = parcel.ReadString16();
        info->mConfigurationPage = parcel.ReadString16();
        info->isSystemIme = parcel.ReadBool();
        info->mDefaultImeId = parcel.ReadInt32();

        int32_t size = parcel.ReadInt32();
        if (size == 0)
            return info;
        for (int i = 0; i < size; i++) {
            info->mTypes.push_back(parcel.ReadParcelable<KeyboardType>());
        }
        return info;
    }
}
}