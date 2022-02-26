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

#ifndef I_KVSTORE_SYNC_CALLBACK_H
#define I_KVSTORE_SYNC_CALLBACK_H

#include <map>
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "types.h"

namespace OHOS {
namespace DistributedKv {

class IKvStoreSyncCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DistributedKv.IKvStoreSyncCallback");
    virtual void SyncCompleted(const std::map<std::string, Status> &results) = 0;
};

class KvStoreSyncCallbackStub : public IRemoteStub<IKvStoreSyncCallback> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
};

class KvStoreSyncCallbackProxy : public IRemoteProxy<IKvStoreSyncCallback> {
public:
    explicit KvStoreSyncCallbackProxy(const sptr<IRemoteObject> &impl);
    ~KvStoreSyncCallbackProxy() = default;
    void SyncCompleted(const std::map<std::string, Status> &results) override;
private:
    static inline BrokerDelegator<KvStoreSyncCallbackProxy> delegator_;
};

}  // namespace DistributedKv
}  // namespace OHOS

#endif  // I_KVSTORE_SYNC_CALLBACK_H
