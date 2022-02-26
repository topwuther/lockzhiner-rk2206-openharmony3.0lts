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

#ifndef UPDATE_SERVICE_STUB_H_
#define UPDATE_SERVICE_STUB_H_

#include <functional>
#include <iostream>
#include <map>
#include "iremote_stub.h"
#include "iupdate_service.h"
#include "message_parcel.h"
#include "parcel.h"

namespace OHOS {
namespace update_engine {
class UpdateServiceStub : public IRemoteStub<IUpdateService> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption &option) override;

    using UpdateServiceStubPtr = UpdateServiceStub *;
    using RequestFuncType = std::function<int(UpdateServiceStubPtr service,
        MessageParcel &data, MessageParcel &reply, MessageOption &option)>;
};
}
}
#endif // UPDATE_SERVICE_STUB_H_