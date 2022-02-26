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

#ifndef FM_IMMS_PROJECT_INPUTMETHODPROPERTY_H
#define FM_IMMS_PROJECT_INPUTMETHODPROPERTY_H
#include <vector>
#include <string>
#include "parcel.h"
#include "keyboard_type.h"

namespace OHOS {
namespace MiscServices {
    class InputMethodProperty : public Parcelable {
    public:
        std::u16string mImeId;
        std::u16string mPackageName;
        std::u16string mAbilityName;
        std::u16string mConfigurationPage;
        bool isSystemIme;
        int32_t mDefaultImeId;
        std::vector<KeyboardType*> mTypes;

        InputMethodProperty();
        ~InputMethodProperty();
        InputMethodProperty(const InputMethodProperty& property);
        InputMethodProperty& operator =(const InputMethodProperty& property);
        virtual bool Marshalling(Parcel &parcel) const override;
        static InputMethodProperty *Unmarshalling(Parcel &parcel);
    };
}
}
#endif // FM_IMMS_PROJECT_INPUTMETHODPROPERTY_H
