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

#include "bluetooth_log.h"
#include "bluetooth_raw_address.h"

namespace OHOS {
namespace Bluetooth {
bool BluetoothRawAddress::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(address_)) {
        return false;
    }
    return true;
}

BluetoothRawAddress *BluetoothRawAddress::Unmarshalling(Parcel &parcel)
{
    BluetoothRawAddress *rawaddress = new BluetoothRawAddress();

    bool noError = true;
    std::string address = "";
    if (parcel.ReadString(address)) {
        rawaddress->address_ = address;
    } else {
        noError = false;
    }
    if (!noError) {
        delete rawaddress;
        rawaddress = nullptr;
    }
    return rawaddress;
}

bool BluetoothRawAddress::WriteToParcel(Parcel &parcel)
{
    return Marshalling(parcel);
}

bool BluetoothRawAddress::ReadFromParcel(Parcel &parcel)
{
    bool noError = true;
    std::string address = "";
    if (parcel.ReadString(address)) {
        address_ = address;
    } else {
        noError = false;
    }
    return noError;
}
}  // namespace Bluetooth
}  // namespace OHOS
