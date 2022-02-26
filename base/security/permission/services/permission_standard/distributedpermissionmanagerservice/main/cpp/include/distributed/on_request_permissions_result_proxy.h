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

#ifndef ON_REQUEST_PERMISSION_RESULT_PROXY_H
#define ON_REQUEST_PERMISSION_RESULT_PROXY_H

#include "iremote_proxy.h"
#include "on_request_permissions_result.h"

namespace OHOS {
namespace Security {
namespace Permission {
class OnRequestPermissionsResultProxy : public IRemoteProxy<OnRequestPermissionsResult> {
public:
    explicit OnRequestPermissionsResultProxy(const sptr<IRemoteObject> &object);

    virtual ~OnRequestPermissionsResultProxy() override;
    virtual void OnResult(
        const std::string nodeId, std::vector<std::string> permissions, std::vector<int32_t> grantResults) override;

    virtual void OnCancel(const std::string nodeId, std::vector<std::string> permissions) override;

    virtual void OnTimeOut(const std::string nodeId, std::vector<std::string> permissions) override;

private:
    static inline BrokerDelegator<OnRequestPermissionsResultProxy> delegator_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // ON_USING_PERMISSION_REMINDER_PROXY_H
