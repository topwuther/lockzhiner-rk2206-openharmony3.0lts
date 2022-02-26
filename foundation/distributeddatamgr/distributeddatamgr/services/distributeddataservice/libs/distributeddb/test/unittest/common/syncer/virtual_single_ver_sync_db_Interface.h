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

#ifndef KVDB_SYNCABLE_TEST_H
#define KVDB_SYNCABLE_TEST_H

#include <map>
#include <vector>

#include "single_ver_kvdb_sync_interface.h"
#include "types.h"

namespace DistributedDB {
struct VirtualDataItem {
    Key key;
    Value value;
    TimeStamp timeStamp = 0;
    TimeStamp writeTimeStamp = 0;
    uint64_t flag = 0;
    bool isLocal = true;
    static const uint64_t DELETE_FLAG = 0x01;
    static const uint64_t LOCAL_FLAG = 0x02;
};
class VirtualSingleVerSyncDBInterface : public SingleVerKvDBSyncInterface {
public:
    int GetInterfaceType() const override;

    void IncRefCount() override;

    void DecRefCount() override;

    std::vector<uint8_t> GetIdentifier() const override;

    int GetMetaData(const Key& key, Value& value) const override;

    int PutMetaData(const Key& key, const Value& value) override;

    int GetAllMetaKeys(std::vector<Key>& keys) const override;

    int GetSyncData(TimeStamp begin, TimeStamp end, std::vector<DataItem> &dataItems,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const override;

    int GetSyncDataNext(std::vector<DataItem> &dataItems, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const override;

    void ReleaseContinueToken(ContinueToken& continueStmtToken) const override;

    int PutSyncData(std::vector<DataItem>& dataItems, const std::string &deviceName) override;

    void ReleaseKvEntry(const SingleVerKvEntry *entry) override;

    void GetMaxTimeStamp(TimeStamp& stamp) const override;

    int RemoveDeviceData(const std::string &deviceName, bool isNeedNotify) override;

    int GetSyncData(const Key& key, VirtualDataItem& item);

    int PutSyncData(const DataItem& item);

    int PutData(const Key &key, const Value &value, const TimeStamp &time, int flag);

    int GetSyncData(TimeStamp begin, TimeStamp end, std::vector<SingleVerKvEntry *> &entries,
        ContinueToken &continueStmtToken, const DataSizeSpecInfo &dataSizeInfo) const override;

    int GetSyncDataNext(std::vector<SingleVerKvEntry *> &entries, ContinueToken &continueStmtToken,
        const DataSizeSpecInfo &dataSizeInfo) const override;

    int PutSyncData(const std::vector<SingleVerKvEntry *> &entries, const std::string &deviceName) override;

    SchemaObject GetSchemaInfo() const override;

    bool CheckCompatible(const std::string& schema) const override;

    void SetSchemaInfo(const std::string& schema);

    const KvDBProperties &GetDbProperties() const override;

    void SetSaveDataDelayTime(uint64_t milliDelayTime);

    int GetSecurityOption(SecurityOption &option) const override;

    bool IsReadable() const override;

    void SetSecurityOption(SecurityOption &option);

    void NotifyRemotePushFinished(const std::string &targetId) const override;

private:
    int GetSyncData(TimeStamp begin, TimeStamp end, uint32_t blockSize, std::vector<VirtualDataItem>& dataItems,
        ContinueToken& continueStmtToken) const;

    int GetSyncDataNext(std::vector<VirtualDataItem>& dataItems,
        uint32_t blockSize, ContinueToken& continueStmtToken) const;

    int PutSyncData(std::vector<VirtualDataItem>& dataItems, const std::string &deviceName);

    std::map<std::vector<uint8_t>, std::vector<uint8_t>> metadata_;
    std::vector<VirtualDataItem> dbData_;
    std::string schema_;
    SchemaObject schemaObj_;
    KvDBProperties properties_;
    uint64_t saveDataDelayTime_;
    SecurityOption secOption_;
};
}  // namespace DistributedDB

#endif // KVDB_SYNCABLE_TEST_H