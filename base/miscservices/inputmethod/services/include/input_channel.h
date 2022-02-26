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

#ifndef FM_IMMS_PROJECT__INPUTCHANNEL_H
#define FM_IMMS_PROJECT__INPUTCHANNEL_H
#include <string>
#include "parcel.h"
#include "global.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
    class InputChannel : public Parcelable {
    public:
        InputChannel();
        ~InputChannel();
        virtual bool Marshalling(Parcel &parcel) const override;
        static InputChannel *Unmarshalling(Parcel &parcel);
    private:
        std::u16string name;
        MessageParcel inputChannelParcel;
        InputChannel(const InputChannel& channel);
        InputChannel& operator =(const InputChannel& channel);
    };
}
}
#endif // INPUTMETHODSYSTEMSERVICE_INPUTCHANNEL_H
