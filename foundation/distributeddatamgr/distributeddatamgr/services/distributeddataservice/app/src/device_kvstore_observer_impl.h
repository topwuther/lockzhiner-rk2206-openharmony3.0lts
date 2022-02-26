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

#ifndef DEVICE_KVSTORE_OBSERVER_IMPL_H
#define DEVICE_KVSTORE_OBSERVER_IMPL_H

#include "kvstore_observer_impl.h"

namespace OHOS::DistributedKv {
class DeviceKvStoreObserverImpl : public KvStoreObserverImpl {
public:
    DeviceKvStoreObserverImpl(SubscribeType subscribeType, sptr<IKvStoreObserver> observerProxy, bool deviceSync);
    ~DeviceKvStoreObserverImpl() override;
    void OnChange(const DistributedDB::KvStoreChangedData &data) override;
private:
    void Transfer(const std::list<DistributedDB::Entry> &input, std::list<Entry> &output, std::string &deviceId);
    bool deviceSync_;
    std::string localDeviceId_;
    sptr<IKvStoreObserver> observerProxy_;
};
}
#endif // DEVICE_KVSTORE_OBSERVER_IMPL_H
