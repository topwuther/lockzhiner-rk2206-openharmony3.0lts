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

#ifndef FM_IMMS_PROJECT_KEYBOARDTYPE_H
#define FM_IMMS_PROJECT_KEYBOARDTYPE_H

#include <vector>
#include <string>
#include "parcel.h"

namespace OHOS {
namespace MiscServices {
    class KeyboardType : public Parcelable {
    public:
        KeyboardType();
        KeyboardType(const KeyboardType& type);
        ~KeyboardType();
        KeyboardType& operator =(const KeyboardType& type);
        virtual bool Marshalling(Parcel &parcel) const override;
        static KeyboardType *Unmarshalling(Parcel &parcel);
        void setId(int32_t typeId);
        void setLabelId(int32_t labelId);
        void setIconId(int32_t iconId);
        void setAsciiCapability(bool isAsciiCapable);
        void setLanguage(std::u16string language);
        void setInputSource(std::u16string inputSource);
        void setCustomizedValue(std::u16string customizedValue);
        int32_t getId() const;
        int32_t getLabelId() const;
        int32_t getIconId() const;
        int32_t getHashCode() const;
        std::u16string getLanguage() const;
        std::u16string getInputSource() const;
        std::u16string getCustomizedValue() const;
        bool supportsAscii();

    private:
        int32_t mId;
        int32_t mHashCode;
        int32_t mLabelId;
        int32_t mIconId;
        bool mIsAsciiCapable;
        std::u16string mLanguage;
        std::u16string mInputSource;
        std::u16string mCustomizedValue;

        const int32_t ID_NONE = 0;
    };
}
}
#endif // FM_IMMS_PROJECT_KEYBOARDTYPE_H
