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

#ifndef GENERIC_KV_DB_CONNECTION_H
#define GENERIC_KV_DB_CONNECTION_H

#include <atomic>
#include <list>
#include <mutex>

#include "ikvdb_connection.h"
#include "notification_chain.h"

namespace DistributedDB {
class GenericKvDB;

class GenericKvDBConnection : public IKvDBConnection {
public:
    explicit GenericKvDBConnection(GenericKvDB *kvDB);
    ~GenericKvDBConnection() override;

    DISABLE_COPY_ASSIGN_MOVE(GenericKvDBConnection);

    // Register observer.
    KvDBObserverHandle *RegisterObserver(unsigned mode, const Key &key,
        const KvDBObserverAction &action, int &errCode) override;

    // Unregister observer.
    int UnRegisterObserver(const KvDBObserverHandle *observerHandle) override;

    // Register a conflict notifier.
    int SetConflictNotifier(int conflictType, const KvDBConflictAction &action) override;

    // Close and release the connection.
    int Close() final;

    std::string GetIdentifier() const override;

    // Pragma interface.
    int Pragma(int cmd, void *parameter) override;

    // Parse event types(from observer mode).
    virtual int TranslateObserverModeToEventTypes(unsigned mode, std::list<int> &eventTypes) const = 0;

    // Set it to 'safe' state to delete the connection
    void SetSafeDeleted();

    int GetEntries(const IOption &option, const Key &keyPrefix, std::vector<Entry> &entries) const override;

    int GetEntries(const IOption &option, const Query &query, std::vector<Entry> &entries) const override;

    int GetResultSet(const IOption &option, const Key &keyPrefix, IKvDBResultSet *&resultSet) const override;

    int GetResultSet(const IOption &option, const Query &query, IKvDBResultSet *&resultSet) const override;

    int GetCount(const IOption &option, const Query &query, int &count) const override;

    void ReleaseResultSet(IKvDBResultSet *&resultSet) override;

    int RegisterLifeCycleCallback(const DatabaseLifeCycleNotifier &notifier) override;

    int GetSecurityOption(int &securityLabel, int &securityFlag) const override;
protected:
    // Get the stashed 'KvDB_ pointer' without ref.
    template<typename DerivedDBType>
    DerivedDBType *GetDB() const
    {
        return static_cast<DerivedDBType *>(kvDB_);
    }

    // Register an event listener with observer action data.
    NotificationChain::Listener *RegisterSpecialListener(int type, const Key &key,
        const KvDBObserverAction &action, bool conflict, int &errCode);

    virtual int PreCheckExclusiveStatus();

    void ResetExclusiveStatus();

    // Called in Close(), overriding of Close() is forbidden.
    virtual int PreClose();

    GenericKvDB *kvDB_;
    std::atomic<bool> isExclusive_;

private:
    int GetEventType(unsigned mode, std::list<int> &eventTypes) const;

    int RegisterObserverForOneType(int type, const Key &key, const KvDBObserverAction &action,
        NotificationChain::Listener *&listener);

    // Soft limit of a connection observer count.
    static constexpr int MAX_OBSERVER_COUNT = 8;

    bool isSafeDeleted_;
    std::mutex observerListLock_;
    std::list<KvDBObserverHandle *> observerList_;
};
}

#endif // GENERIC_KV_DB_CONNECTION_H
