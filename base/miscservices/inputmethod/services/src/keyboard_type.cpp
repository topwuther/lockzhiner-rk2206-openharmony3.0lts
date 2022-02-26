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

#include "keyboard_type.h"

namespace OHOS {
namespace MiscServices {
    using namespace std;
    /*! Constructor
    */
    KeyboardType::KeyboardType()
    {
    }

    /*! Constructor from another instance
      \param type the source instance
    */
    KeyboardType::KeyboardType(const KeyboardType& type)
    {
        mId = type.mId;
        mHashCode = type.mHashCode;
        mLabelId = type.mLabelId;
        mIconId = type.mIconId;
        mIsAsciiCapable = type.mIsAsciiCapable;
        mLanguage = type.mLanguage;
        mInputSource = type.mInputSource;
        mCustomizedValue = type.mCustomizedValue;
    }

    /*! Destructor
    */
    KeyboardType::~KeyboardType()
    {
    }

    /*! Get value from another instance
      \param type source instance
      \return return this
    */
    KeyboardType& KeyboardType::operator =(const KeyboardType& type)
    {
        if (this == &type) {
            return *this;
        }

        mId = type.mId;
        mHashCode = type.mHashCode;
        mLabelId = type.mLabelId;
        mIconId = type.mIconId;
        mIsAsciiCapable = type.mIsAsciiCapable;
        mLanguage = type.mLanguage;
        mInputSource = type.mInputSource;
        mCustomizedValue = type.mCustomizedValue;

        return *this;
    }

    /*! Write the details of object to parcel
    */
    bool KeyboardType::Marshalling(Parcel &parcel) const
    {
        if (!(parcel.WriteInt32(mId)
            && parcel.WriteInt32(mHashCode)
            && parcel.WriteInt32(mLabelId)
            && parcel.WriteInt32(mIconId)
            && parcel.WriteBool(mIsAsciiCapable)
            && parcel.WriteString16(mLanguage)
            && parcel.WriteString16(mInputSource)
            && parcel.WriteString16(mCustomizedValue)))
            return false;
        return true;
    }

    /*! Read the details of object from parcel
      \param parcel read the details of object from this parcel
      \return ErrorCode::NO_ERROR
      \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    KeyboardType *KeyboardType::Unmarshalling(Parcel &parcel)
    {
        auto info = new KeyboardType();
        info->mId = parcel.ReadInt32();
        info->mHashCode = parcel.ReadInt32();
        info->mLabelId = parcel.ReadInt32();
        info->mIconId = parcel.ReadInt32();
        info->mIsAsciiCapable = parcel.ReadBool();
        info->mLanguage = parcel.ReadString16();
        info->mInputSource = parcel.ReadString16();
        info->mCustomizedValue = parcel.ReadString16();
        return info;
    }

    void KeyboardType::setId(int32_t typeId)
    {
        mId = typeId;
        if (typeId != ID_NONE) {
          mHashCode = typeId;
        } else {
          mHashCode = ID_NONE;
        }

    }

    void KeyboardType::setLabelId(int32_t labelId)
    {
        mLabelId = labelId;
    }

    void KeyboardType::setIconId(int32_t iconId)
    {
        mIconId = iconId;
    }

    void KeyboardType::setAsciiCapability(bool isAsciiCapable)
    {
        mIsAsciiCapable = isAsciiCapable;
    }

    void KeyboardType::setLanguage(u16string language)
    {
        mLanguage = language;
    }

    void KeyboardType::setInputSource(u16string inputSource)
    {
        mInputSource = inputSource;
    }

    void KeyboardType::setCustomizedValue(u16string keyBoardTypeCustomizedValue)
    {
        mCustomizedValue = keyBoardTypeCustomizedValue;
    }

    int32_t KeyboardType::getId() const
    {
        return mId;
    }

    int32_t KeyboardType::getLabelId() const
    {
        return mLabelId;
    }

    int32_t KeyboardType::getIconId() const
    {
        return mIconId;
    }

    /*! Get hash code of the object
      \return return hashCode value
    */
    int KeyboardType::getHashCode() const
    {
        return mHashCode;
    }

    /*! Get language of the object
      \return return the language of this object
    */
    u16string KeyboardType::getLanguage() const
    {
        return mLanguage;
    }

    u16string KeyboardType::getInputSource() const
    {
        return mInputSource;
    }

    u16string KeyboardType::getCustomizedValue() const
    {
        return mCustomizedValue;
    }
}
}