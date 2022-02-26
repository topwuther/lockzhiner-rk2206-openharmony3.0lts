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
#include <thread>
#include <functional>

#include "gtest/gtest.h"
#include "distributed_permission_manager_service.h"
#include "on_permission_used_record_stub.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Security::Permission;
using ErrCode = int;

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class GetPermissionUsedRecordsAsynchTest : public testing::Test {
public:
    std::shared_ptr<DistributedPermissionManagerService> service;
    static void InitVisitorData()
    {
        std::vector<GenericValues> visitor;
        GenericValues genericVisitor;
        genericVisitor.Put(FIELD_DEVICE_ID, "device_0");
        genericVisitor.Put(FIELD_DEVICE_NAME, "device_name_0");
        genericVisitor.Put(FIELD_BUNDLE_USER_ID, 0);
        genericVisitor.Put(FIELD_BUNDLE_NAME, "bundleName_0");
        genericVisitor.Put(FIELD_BUNDLE_LABEL, "bundleInfo.label_0");
        visitor.emplace_back(genericVisitor);

        GenericValues genericVisitor1;
        genericVisitor1.Put(FIELD_DEVICE_ID, "device_1");
        genericVisitor1.Put(FIELD_DEVICE_NAME, "device_name_1");
        genericVisitor1.Put(FIELD_BUNDLE_USER_ID, 0);
        genericVisitor1.Put(FIELD_BUNDLE_NAME, "bundleName_1");
        genericVisitor1.Put(FIELD_BUNDLE_LABEL, "bundleInfo.label_1");
        visitor.emplace_back(genericVisitor1);
        DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_VISITOR, visitor);
    }
    static void InitRecordData(int64_t timestamp)
    {
        std::vector<GenericValues> visitor;
        DataStorage::GetRealDataStorage().Find(DataStorage::PERMISSION_VISITOR, visitor);
        int visitorId = 0;
        if (visitor.size() > 0) {
            visitorId = visitor[0].GetInt(FIELD_ID);
        }

        std::vector<GenericValues> record;
        GenericValues genericRecordFore;
        genericRecordFore.Put(FIELD_TIMESTAMP, timestamp);
        genericRecordFore.Put(FIELD_VISITOR_ID, visitorId);
        genericRecordFore.Put(FIELD_OP_CODE, 4);
        genericRecordFore.Put(FIELD_IS_FOREGROUND, 1);
        genericRecordFore.Put(FIELD_ACCESS_COUNT, 3);
        genericRecordFore.Put(FIELD_REJECT_COUNT, 2);

        GenericValues genericRecordBack;
        genericRecordBack.Put(FIELD_TIMESTAMP, timestamp - Constant::PRECISION);
        genericRecordBack.Put(FIELD_VISITOR_ID, visitorId);
        genericRecordBack.Put(FIELD_OP_CODE, 4);
        genericRecordBack.Put(FIELD_IS_FOREGROUND, 0);
        genericRecordBack.Put(FIELD_ACCESS_COUNT, 1);
        genericRecordBack.Put(FIELD_REJECT_COUNT, 0);

        GenericValues genericRecord;
        genericRecord.Put(FIELD_TIMESTAMP, timestamp);
        genericRecord.Put(FIELD_VISITOR_ID, visitorId);
        genericRecord.Put(FIELD_OP_CODE, 1);
        genericRecord.Put(FIELD_IS_FOREGROUND, 0);
        genericRecord.Put(FIELD_ACCESS_COUNT, 1);
        genericRecord.Put(FIELD_REJECT_COUNT, 3);
        record.emplace_back(genericRecordFore);
        record.emplace_back(genericRecordBack);
        record.emplace_back(genericRecord);
        DataStorage::GetRealDataStorage().Add(DataStorage::PERMISSION_RECORD, record);
    }
    static void SetUpTestCase()
    {
        GenericValues null;
        DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_VISITOR, null);
        DataStorage::GetRealDataStorage().Remove(DataStorage::PERMISSION_RECORD, null);
        InitVisitorData();
        InitRecordData(TimeUtil::GetTimestamp());
        cout << "SetUpTestCase()" << endl;
    }

    static void TearDownTestCase()
    {
        cout << "TearDownTestCase()" << endl;
    }
    void SetUp()
    {
        service = OHOS::DelayedSingleton<DistributedPermissionManagerService>::GetInstance();
        service->OnStart();
        sleep(1);
        cout << "SetUp() is running" << endl;
    }
    void TearDown()
    {
        cout << "TearDown()" << endl;
    }
    DistributedPermissionManagerService managerService;
};

class TestCallback : public OnPermissionUsedRecordStub {
public:
    TestCallback() = default;
    virtual ~TestCallback() = default;

    void OnQueried(ErrCode code, QueryPermissionUsedResult &result)
    {
        EXPECT_EQ(code, 200);
    }
};

/**
 * @tc.number: GetPermissionUsedRecordsAsynch_0100
 * @tc.name: test get permission used record asynch
 * @tc.desc: 1. request is null
 *           2. get all data
 */
HWTEST_F(GetPermissionUsedRecordsAsynchTest, GetPermissionUsedRecordsAsynch_0100, Function | MediumTest | Level1)
{
    OHOS::sptr<TestCallback> callback(new TestCallback());
    QueryPermissionUsedRequest request;

    nlohmann::json jsonObj = request.to_json(request);
    std::string queryJsonStr = jsonObj.dump();

    unsigned long zipLen = queryJsonStr.length();
    unsigned long len = compressBound(zipLen);
    unsigned char *buf = (unsigned char *)malloc(len + 1);
    if (!ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len)) {
        EXPECT_EQ(1, 0);
    }
    std::string queryGzipStr;
    Base64Util::Encode(buf, len, queryGzipStr);

    if (buf) {
        free(buf);
        buf = nullptr;
    }
    int flag = managerService.GetPermissionRecords(queryGzipStr, len, zipLen, callback);
    sleep(3);
    EXPECT_EQ(flag, 0);
}
