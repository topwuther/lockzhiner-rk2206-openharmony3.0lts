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

#ifndef SERVICE_REGISTRY_INCLUDE_H
#define SERVICE_REGISTRY_INCLUDE_H

#include "ipc_types.h"
#include <iremote_broker.h>
#include "ipc_object_stub.h"
#include "iremote_proxy.h"
#include "if_system_ability_manager.h"

namespace OHOS {
class IServiceRegistry : public IRemoteBroker {
public:
    /**
     * Retrieve an existing service, blocking for a few seconds
     * if it doesn't yet exist.
     */
    virtual sptr<IRemoteObject> GetService(const std::u16string& name) = 0;

    /**
     * Retrieve an existing service, non-blocking.
     */
    virtual sptr<IRemoteObject> CheckService(const std::u16string& name) = 0;

    /**
     * Register a service.
     */
    virtual int AddService(const std::u16string& name, const sptr<IRemoteObject> &service,
        bool allowIsolated = false, int dumpsysFlags = 0) = 0;

    enum {
        GET_SERVICE_TRANSACTION = FIRST_CALL_TRANSACTION,
        CHECK_SERVICE_TRANSACTION,
        ADD_SERVICE_TRANSACTION,
    };
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"");
};

class ServiceRegistry {
public:
    /**
     * get Service registry instance.
     */
    static sptr<IServiceRegistry> GetInstance();

private:
    static std::mutex serviceRegistryLock_;
};

class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient& GetInstance();

    /**
     * get system ability manager.
     */
    sptr<ISystemAbilityManager> GetSystemAbilityManager();

    /**
     * get remote object of samgr.
     */
    sptr<IRemoteObject> GetRegistryRemoteObject();

    /**
     * destroy remote object of samgr.
     */
    void DestroySystemAbilityManagerObject();

private:
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;

    sptr<ISystemAbilityManager> systemAbilityManager_;
    std::mutex systemAbilityManagerLock_;
};
} // namespace OHOS

#endif // ZIPC_SERVICE_REGISTRY_INCLUDE_H