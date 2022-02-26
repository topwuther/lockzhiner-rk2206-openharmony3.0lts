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

#ifndef ON_PERMISSION_USED_RECORD_PROXY_H
#define ON_PERMISSION_USED_RECORD_PROXY_H

#include "iremote_proxy.h"
#include "nocopyable.h"
#include "on_permission_used_record.h"

namespace OHOS {
namespace Security {
namespace Permission {
class OnPermissionUsedRecordProxy : public IRemoteProxy<OnPermissionUsedRecord> {
public:
    explicit OnPermissionUsedRecordProxy(const sptr<IRemoteObject> &object);

    ~OnPermissionUsedRecordProxy() override;

    virtual void OnQueried(ErrCode code, QueryPermissionUsedResult &result) override;

private:
    DISALLOW_COPY_AND_MOVE(OnPermissionUsedRecordProxy);

    static inline BrokerDelegator<OnPermissionUsedRecordProxy> delegator_;

    // 1MB
    static constexpr int MAX_CAPACITY = 1048576;

    static constexpr int MAX_RECORDS_THRESHOLD = 16;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // ON_PERMISSION_USED_RECORD_PROXY_H
