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

#ifndef PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_STUB_H
#define PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_STUB_H

#include "idistributed_permission.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace Permission {
class DistributedPermissionStub : public IRemoteStub<IDistributedPermission> {
public:
    DistributedPermissionStub();
    virtual ~DistributedPermissionStub() override;

    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(DistributedPermissionStub);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // PERMISSION_INNERKITS_INCLUDE_DISRIBUTED_PERMISSION_STUB_H