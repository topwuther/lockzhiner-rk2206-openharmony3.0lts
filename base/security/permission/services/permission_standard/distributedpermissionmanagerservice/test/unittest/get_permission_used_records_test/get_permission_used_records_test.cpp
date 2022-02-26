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

using namespace std;
using namespace testing::ext;
using namespace OHOS::Security::Permission;

namespace OHOS {
pid_t IPCSkeleton::pid_ = 1;
pid_t IPCSkeleton::uid_ = 1;
std::string IPCSkeleton::localDeviceId_ = "1004";
std::string IPCSkeleton::deviceId_ = "";
}  // namespace OHOS

class GetPermissionUsedRecordsTest : public testing::Test {
public:
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
        int opCode = 4;
        int rejectCount = 3;
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
        genericRecordFore.Put(FIELD_OP_CODE, opCode);
        genericRecordFore.Put(FIELD_IS_FOREGROUND, 1);
        genericRecordFore.Put(FIELD_ACCESS_COUNT, rejectCount);
        genericRecordFore.Put(FIELD_REJECT_COUNT, rejectCount);

        GenericValues genericRecordBack;
        genericRecordBack.Put(FIELD_TIMESTAMP, timestamp - Constant::PRECISION);
        genericRecordBack.Put(FIELD_VISITOR_ID, visitorId);
        genericRecordBack.Put(FIELD_OP_CODE, opCode);
        genericRecordBack.Put(FIELD_IS_FOREGROUND, 0);
        genericRecordBack.Put(FIELD_ACCESS_COUNT, 1);
        genericRecordBack.Put(FIELD_REJECT_COUNT, 0);

        GenericValues genericRecord;
        genericRecord.Put(FIELD_TIMESTAMP, timestamp);
        genericRecord.Put(FIELD_VISITOR_ID, visitorId);
        genericRecord.Put(FIELD_OP_CODE, 1);
        genericRecord.Put(FIELD_IS_FOREGROUND, 0);
        genericRecord.Put(FIELD_ACCESS_COUNT, 1);
        genericRecord.Put(FIELD_REJECT_COUNT, rejectCount);
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
        cout << "SetUp() is running" << endl;
    }
    void TearDown()
    {
        cout << "TearDown()" << endl;
    }
    DistributedPermissionManagerService managerService;
};

/**
 * @tc.number: GetPermissionUsedRecords_0100
 * @tc.name: test get permission used record
 * @tc.desc: 1. request is null
 *           2. get all data
 *           3. flag is 0
 */
HWTEST_F(GetPermissionUsedRecordsTest, GetPermissionUsedRecords_0100, Function | MediumTest | Level1)
{
    QueryPermissionUsedRequest request;
    request.flag = 0;
    QueryPermissionUsedResult result;
    nlohmann::json jsonObj = request.to_json(request);
    string queryJsonStr = jsonObj.dump();
    string resultStr;

    uLong zipLen = queryJsonStr.length();
    uLong len = compressBound(zipLen);
    unsigned char *buf = (unsigned char *)malloc(len);
    bool state = ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len);
    EXPECT_TRUE(state);
    string enStr;
    Base64Util::Encode(buf, len, enStr);
    free(buf);
    buf = nullptr;
    int flag = managerService.GetPermissionRecords(enStr, len, zipLen, resultStr);
    EXPECT_EQ(flag, 0);

    unsigned char *pOut = (unsigned char *)malloc(len);
    Base64Util::Decode(resultStr, pOut, len);
    std::string resultJsonStr;
    ZipUtil::ZipUnCompress(pOut, len, resultJsonStr, zipLen);

    free(pOut);
    pOut = nullptr;

    nlohmann::json jsonRes = nlohmann::json::parse(resultJsonStr, nullptr, false);
    result.from_json(jsonRes, result);
    EXPECT_EQ(result.code, 200);
}

/**
 * @tc.number: GetPermissionUsedRecords_0200
 * @tc.name: test get permission used record
 * @tc.desc: 1. request is not null
 *           2. request flag is 1
 */
HWTEST_F(GetPermissionUsedRecordsTest, GetPermissionUsedRecords_0200, Function | MediumTest | Level1)
{
    QueryPermissionUsedRequest request;
    QueryPermissionUsedResult result;
    vector<string> perNames;
    perNames.emplace_back("ohos.permission.LOCATION");
    perNames.emplace_back("ohos.permission.READ_CONTACTS");
    request.deviceLabel = "device_name_0";
    request.bundleName = "bundleName_0";
    request.beginTimeMillis = 0L;
    request.endTimeMillis = 1625010710L;
    request.flag = 1;
    request.permissionNames = perNames;
    nlohmann::json jsonObj = request.to_json(request);
    string queryJsonStr = jsonObj.dump();
    string resultStr;

    uLong zipLen = queryJsonStr.length();
    uLong len = compressBound(zipLen);
    unsigned char *buf = (unsigned char *)malloc(len);
    bool state = ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len);
    EXPECT_TRUE(state);
    string enStr;
    Base64Util::Encode(buf, len, enStr);
    int flag = managerService.GetPermissionRecords(enStr, len, zipLen, resultStr);
    EXPECT_EQ(flag, 0);

    unsigned char *pOut = (unsigned char *)malloc(len);
    Base64Util::Decode(resultStr, pOut, len);
    std::string resultJsonStr;
    ZipUtil::ZipUnCompress(pOut, len, resultJsonStr, zipLen);

    nlohmann::json jsonRes = nlohmann::json::parse(resultJsonStr, nullptr, false);
    result.from_json(jsonRes, result);
    EXPECT_EQ(result.code, 200);
}

/**
 * @tc.number: GetPermissionUsedRecords_0300
 * @tc.name: test get permission used record
 * @tc.desc: 1. request is not null
 *           2. request flag is 1
 */
HWTEST_F(GetPermissionUsedRecordsTest, GetPermissionUsedRecords_0300, Function | MediumTest | Level1)
{
    QueryPermissionUsedRequest request;
    QueryPermissionUsedResult result;
    vector<string> perNames;
    perNames.emplace_back("ohos.permission.LOCATION");
    perNames.emplace_back("ohos.permission.READ_CONTACTS");
    request.deviceLabel = "device_name_01";
    request.bundleName = "bundleName_0";
    request.beginTimeMillis = 55555L;
    request.endTimeMillis = 0L;
    request.flag = 1;
    request.permissionNames = perNames;
    nlohmann::json jsonObj = request.to_json(request);
    string queryJsonStr = jsonObj.dump();
    string resultStr;

    uLong zipLen = queryJsonStr.length();
    uLong len = compressBound(zipLen);
    unsigned char *buf = (unsigned char *)malloc(len);
    bool state = ZipUtil::ZipCompress(queryJsonStr, zipLen, buf, len);
    EXPECT_TRUE(state);
    string enStr;
    Base64Util::Encode(buf, len, enStr);
    if (buf) {
        free(buf);
        buf = nullptr;
    }

    int flag = managerService.GetPermissionRecords(enStr, len, zipLen, resultStr);
    EXPECT_EQ(flag, 0);

    unsigned char *pOut = (unsigned char *)malloc(len);
    Base64Util::Decode(resultStr, pOut, len);
    std::string resultJsonStr;
    ZipUtil::ZipUnCompress(pOut, len, resultJsonStr, zipLen);
    free(pOut);
    pOut = nullptr;

    nlohmann::json jsonRes = nlohmann::json::parse(resultJsonStr, nullptr, false);
    result.from_json(jsonRes, result);
    EXPECT_EQ(result.code, 200);
}