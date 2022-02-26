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
#ifndef KVSTORE_META_MANAGER_H
#define KVSTORE_META_MANAGER_H

#include <mutex>
#include <nlohmann/json.hpp>

#include "app_device_status_change_listener.h"
#include "hks_api.h"
#include "hks_param.h"
#include "types.h"
#include "system_ability.h"
#include "kv_store_delegate.h"
#include "kv_store_delegate_manager.h"
#include "kvstore_impl.h"
#include "single_kvstore_impl.h"

namespace OHOS {
namespace DistributedKv {
enum FLAG {
    UPDATE,
    DELETE,
    CHECK_EXIST,
    UPDATE_LOCAL,
    DELETE_LOCAL,
    CHECK_EXIST_LOCAL,
};

struct Serializable {
    using json = nlohmann::json;
    template<typename T>
    static T GetVal(const json &j, const std::string &name, json::value_t type, const T &def);
    static bool CheckJsonValue(const json &j, const std::string &name, json::value_t type);
    static json ToJson(const std::string &jsonStr);
};

struct StrategyMeta {
    std::string devId;
    std::string devAccId;
    std::string grpId;
    std::string bundleName;
    std::string storeId;
};

struct SecretKeyMetaData {
    static constexpr const char *SKEY = "skey";
    std::vector<uint8_t> timeValue {};
    std::vector<uint8_t> secretKey {};
    KvStoreType kvStoreType = KvStoreType::INVALID_TYPE;
    SecretKeyMetaData() {}
    explicit SecretKeyMetaData(const nlohmann::json &jObject)
    {
        Unmarshal(jObject);
    }

    std::vector<uint8_t> Marshal() const;
    void Unmarshal(const nlohmann::json &jObject);
    operator std::vector<uint8_t>() const
    {
        return Marshal();
    }
private:
    static constexpr const char *TIME = "time";
    static constexpr const char *KVSTORE_TYPE = "kvStoreType";
};

struct KvStoreMetaData {
    static constexpr const char *APP_ID = "appId";
    static constexpr const char *BUNDLE_NAME = "bundleName";
    using json = nlohmann::json;
    std::string appId = "";
    std::string appType = "";
    std::string bundleName = "";
    std::string dataDir = "";
    std::string deviceAccountId = "";
    std::string deviceId = "";
    bool isAutoSync = false;
    bool isBackup = false;
    bool isEncrypt = false;
    KvStoreType kvStoreType = KvStoreType::DEVICE_COLLABORATION;
    std::string schema = "";
    std::string storeId = "";
    std::string userId = "";
    std::int32_t uid = -1;
    std::uint32_t version = 0;
    int securityLevel = 0;
    bool isDirty = false;
    std::string Marshal() const;
    void Unmarshal(const json &jObject);

    static bool CheckChiefValues(const json &jObject);

    static inline std::string GetAppId(const json &jObject)
    {
        return Serializable::GetVal<std::string>(jObject, APP_ID, json::value_t::string, "");
    }

    static inline std::string GetBundleName(const json &jObject)
    {
        return Serializable::GetVal<std::string>(jObject, BUNDLE_NAME, json::value_t::string, "");
    }
    static inline std::string GetStoreId(const json &jObject)
    {
        return Serializable::GetVal<std::string>(jObject, STORE_ID, json::value_t::string, "");
    }
private:
    static constexpr const char *KVSTORE_TYPE = "kvStoreType";
    static constexpr const char *DEVICE_ID = "deviceId";
    static constexpr const char *USER_ID = "userId";
    static constexpr const char *STORE_ID = "storeId";
    static constexpr const char *ENCRYPT = "isEncrypt";
    static constexpr const char *BACKUP = "isBackup";
    static constexpr const char *AUTO_SYNC = "isAutoSync";
    static constexpr const char *SCHEMA = "schema";
    static constexpr const char *DATA_DIR = "dataDir";
    static constexpr const char *APP_TYPE = "appType";
    static constexpr const char *DEVICE_ACCOUNT_ID = "deviceAccountID";
    static constexpr const char *UID = "UID";
    static constexpr const char *VERSION = "version";
    static constexpr const char *SECURITY_LEVEL = "securityLevel";
    static constexpr const char *DIRTY_KEY = "isDirty";
};


struct MetaData {
    std::int32_t kvStoreType;
    KvStoreMetaData kvStoreMetaData;
    SecretKeyMetaData secretKeyMetaData;

    static inline KvStoreType GetKvStoreType(const nlohmann::json &jObject)
    {
        return Serializable::GetVal<KvStoreType>(jObject, KVSTORE_TYPE, nlohmann::json::value_t::number_unsigned,
                                                 KvStoreType::INVALID_TYPE);
    }
private:
    static constexpr const char *KVSTORE_TYPE = "kvStoreType";
};

struct DelegateGuard {
    using Fn = std::function<void()>;
    Fn action_;
    DelegateGuard(Fn action) : action_(std::forward<Fn>(action)) {}

    ~DelegateGuard()
    {
        if (action_) {
            action_();
        }
    }
    DelegateGuard() = delete;
    DelegateGuard(const DelegateGuard &) = delete;
    DelegateGuard &operator=(const DelegateGuard &) = delete;
};

class KvStoreMetaManager {
public:
    using NbDelegate = std::unique_ptr<DistributedDB::KvStoreNbDelegate,
        std::function<void(DistributedDB::KvStoreNbDelegate *)>>;

    class MetaDeviceChangeListenerImpl : public AppDistributedKv::AppDeviceStatusChangeListener {
        void OnDeviceChanged(const AppDistributedKv::DeviceInfo &info,
                             const AppDistributedKv::DeviceChangeType &type) const override;

        AppDistributedKv::ChangeLevelType GetChangeLevelType() const override;
    };

    ~KvStoreMetaManager();

    static KvStoreMetaManager &GetInstance();

    void InitMetaParameter();

    void InitMetaListener(std::function<void(const KvStoreMetaData &metaData)> observer);

    const NbDelegate &GetMetaKvStore();

    Status CheckUpdateServiceMeta(const std::vector<uint8_t> &metaKey, FLAG flag, const std::vector<uint8_t> &val = {});

    Status GenerateRootKey();

    Status CheckRootKeyExist();

    static std::vector<uint8_t> GetMetaKey(
        const std::string &deviceAccountId, const std::string &groupId, const std::string &bundleName,
        const std::string &storeId, const std::string &key = "");

    static std::string GetSecretKeyFile(const std::string &deviceAccountId, const std::string &appId,
                                        const std::string &storeId);

    static std::string GetSecretSingleKeyFile(const std::string &deviceAccountId, const std::string &appId,
                                              const std::string &storeId);

    Status GetSecretKeyFromMeta(const std::vector<uint8_t> &metaSecretKey,
                                std::vector<uint8_t> &key, bool &outdated);

    std::vector<uint8_t> EncryptWorkKey(const std::vector<uint8_t> &key);

    bool DecryptWorkKey(const std::vector<uint8_t> &encryptedKey, std::vector<uint8_t> &key);

    Status WriteSecretKeyToMeta(const std::vector<uint8_t> &metaKey, const std::vector<uint8_t> &key);

    Status WriteSecretKeyToFile(const std::string &secretKeyFile, const std::vector<uint8_t> &key);

    Status
    RemoveSecretKey(const std::string &deviceAccountId, const std::string &bundleName, const std::string &storeId);

    Status
    RecoverSecretKeyFromFile(const std::string &secretKeyFile, const std::vector<uint8_t> &metaSecretKey,
                             std::vector<uint8_t> &key, bool &outdated);

    void ReKey(const std::string &deviceAccountId, const std::string &bundleName, const std::string &storeId,
               sptr<IKvStoreImpl> store);

    void ReKey(const std::string &deviceAccountId, const std::string &bundleName, const std::string &storeId,
               sptr<ISingleKvStore> store);

    void GetStrategyMetaKey(const StrategyMeta &params, std::string &retVal);

    Status DeleteStrategyMeta(const std::string &bundleName, const std::string &storeId);

    Status SaveStrategyMetaEnable(const std::string &key, bool enable);

    Status SaveStrategyMetaLabels(const std::string &key,
                                  const std::vector<std::string> &localLabels,
                                  const std::vector<std::string> &remoteSupportLabels);

    Status CheckSyncPermission(const std::string &userId, const std::string &appId, const std::string &storeId,
                               uint8_t flag, const std::string &deviceId);

    Status QueryKvStoreMetaDataByDeviceIdAndAppId(const std::string &devId, const std::string &appId,
                                                  KvStoreMetaData &val);
    // json rule
    void ToJson(nlohmann::json &j, const KvStoreMetaData &k);

    void FromJson(const nlohmann::json &j, KvStoreMetaData &k);

    Status GetKvStoreMeta(const std::vector<uint8_t> &metaKey, KvStoreMetaData &kvStoreMetaData);

    bool GetKvStoreMetaDataByBundleName(const std::string &bundleName, KvStoreMetaData &metaData);

    bool GetKvStoreMetaDataByAppId(const std::string &appId, KvStoreMetaData &metaData);

    bool GetFullMetaData(std::map<std::string, MetaData> &entries);
private:
    NbDelegate CreateMetaKvStore();

    KvStoreMetaManager();

    void InitMetaData();

    void SubscribeMetaKvStore();

    void SyncMeta();

    void ConcatWithSharps(const std::vector<std::string> &params, std::string &retVal);

    Status GetStategyMeta(const std::string &key, std::map<std::string, std::vector<std::string>> &strategies);

    bool GetKvStoreMetaByType(const std::string &name, const std::string &val, KvStoreMetaData &metaData);

    class KvStoreMetaObserver : public DistributedDB::KvStoreObserver {
    public:
        virtual ~KvStoreMetaObserver();

        // Database change callback
        void OnChange(const DistributedDB::KvStoreChangedData &data) override;
        std::function<void(const KvStoreMetaData &)> notify_ = nullptr;
    };

    static const inline std::string META_DB_APP_ID = "distributeddata";
    static constexpr const char *ROOT_KEY_ALIAS = "distributed_db_root_key";
    static constexpr const char *STRATEGY_META_PREFIX = "StrategyMetaData";
    static constexpr const char *CAPABILITY_ENABLED = "capabilityEnabled";
    static constexpr const char *CAPABILITY_RANGE = "capabilityRange";
    static constexpr const char *LOCAL_LABEL = "localLabel";
    static constexpr const char *REMOTE_LABEL = "remoteLabel";
    static constexpr const char *HARMONY_APP = "harmony";
    static constexpr const char *HKS_BLOB_TYPE_NONCE = "Z5s0Bo571KoqwIi6";
    static constexpr const char *HKS_BLOB_TYPE_AAD = "distributeddata";
    static constexpr int KEY_SIZE = 32;
    static constexpr int HOURS_PER_YEAR = (24 * 365);

    NbDelegate metaDelegate_ {};
    std::string metaDBDirectory_;
    DistributedDB::KvStoreDelegateManager kvStoreDelegateManager_;
    std::vector<uint8_t> vecRootKeyAlias_ {};
    std::vector<uint8_t> vecNonce_ {};
    std::vector<uint8_t> vecAad_ {};
    static std::condition_variable cv_;
    static std::mutex cvMutex_;
    static MetaDeviceChangeListenerImpl listener_;
    KvStoreMetaObserver metaObserver_;
};
}  // namespace DistributedKv
}  // namespace OHOS
#endif // KVSTORE_META_MANAGER_H
