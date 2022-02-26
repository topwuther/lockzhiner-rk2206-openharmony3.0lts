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

#ifndef FM_IMMS_PROJECT_INPUTATTRIBUTE_H
#define FM_IMMS_PROJECT_INPUTATTRIBUTE_H
#include "parcel.h"

namespace OHOS {
namespace MiscServices {
    class InputAttribute : public Parcelable {
    public:
        InputAttribute();
        InputAttribute(const InputAttribute& attribute);
        InputAttribute& operator =(const InputAttribute& attribute);
        ~InputAttribute();
        virtual bool Marshalling(Parcel &parcel) const override;
        static InputAttribute *Unmarshalling(Parcel &parcel);
        void SetInputPattern(int32_t pattern);
        bool GetSecurityFlag();
        static const int32_t PATTERN_TEXT = 0x00000001;
        static const int32_t PATTERN_PASSWORD = 0x00000007;
    private:
        int32_t inputPattern;
        int32_t enterKeyType;
        int32_t inputOption;
    };
}
}

#endif // FM_IMMS_PROJECT_INPUTATTRIBUTE_H
