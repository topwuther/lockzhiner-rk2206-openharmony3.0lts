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

#include "input_channel.h"
#include "input_attribute.h"
#include "global.h"

namespace OHOS {
namespace MiscServices {
    /*! Constructor
    */
    InputChannel::InputChannel()
    {
    }

    /*! Destructor
    */
    InputChannel::~InputChannel()
    {
    }

    /*! Write InputChannel to parcel
      \param[out] parcel the data of InputChannel is written to this parcel returned to caller
      \return ErrorCode::NO_ERROR
      \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    bool InputChannel::Marshalling(Parcel &parcel) const
    {
        parcel.ParseFrom(inputChannelParcel.GetData(), inputChannelParcel.GetDataSize());
        return NO_ERROR;
    }

    /*! Get InputChannel from parcel
      \param parcel get the data of InputChannel from this parcel
      \return ErrorCode::NO_ERROR
      \return ErrorCode::ERROR_NULL_POINTER parcel is null
    */
    InputChannel *InputChannel::Unmarshalling(Parcel &parcel)
    {
        auto inputChannel = new InputChannel();
        inputChannel->inputChannelParcel.RewindRead(0);
        inputChannel->inputChannelParcel.ParseFrom(parcel.GetData(), parcel.GetDataSize());
        inputChannel->inputChannelParcel.RewindRead(0);

        inputChannel->name = inputChannel->inputChannelParcel.ReadString16();
        inputChannel->inputChannelParcel.RewindRead(0);
        return inputChannel;
    }
}
}
