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

#ifndef PERMISSION_MANAGER_STUB_H
#define PERMISSION_MANAGER_STUB_H

#include "i_permission_manager.h"

#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace Security {
namespace Permission {
class PermissionManagerStub : public IRemoteStub<IPermissionManager> {
public:
    PermissionManagerStub() = default;
    virtual ~PermissionManagerStub() = default;

    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& options) override;

private:
    void VerifyPermissionInner(MessageParcel& data, MessageParcel& reply);
    void CanRequestPermissionInner(MessageParcel& data, MessageParcel& reply);
    void GrantUserGrantedPermissionInner(MessageParcel& data, MessageParcel& reply);
    void GrantSystemGrantedPermissionInner(MessageParcel& data, MessageParcel& reply);
    void RevokeUserGrantedPermissionInner(MessageParcel& data, MessageParcel& reply);
    void RevokeSystemGrantedPermissionInner(MessageParcel& data, MessageParcel& reply);
    void AddUserGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void AddSystemGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void RemoveUserGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void RemoveSystemGrantedReqPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void AddDefPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void RemoveDefPermissionsInner(MessageParcel& data, MessageParcel& reply);
    void GetDefPermissionInner(MessageParcel& data, MessageParcel& reply);
    bool IsAuthorizedCalling() const;

    static const int SYSTEM_UID = 1000;
    static const int ROOT_UID = 0;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // PERMISSION_MANAGER_STUB_H
