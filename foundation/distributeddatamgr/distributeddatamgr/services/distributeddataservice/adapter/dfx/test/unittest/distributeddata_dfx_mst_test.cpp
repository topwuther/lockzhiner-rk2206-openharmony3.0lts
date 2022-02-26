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

#include <gtest/gtest.h>
#include <openssl/sha.h>
#include "reporter.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;

class DistributedataDfxMSTTest : public testing::Test {
public:
    static void SetUpTestCase(void);

    static void TearDownTestCase(void);

    void SetUp();

    void TearDown();
};

void DistributedataDfxMSTTest::SetUpTestCase() {}

void DistributedataDfxMSTTest::TearDownTestCase() {}

void DistributedataDfxMSTTest::SetUp() {}

void DistributedataDfxMSTTest::TearDown() {}

/**
  * @tc.name: Dfx001
  * @tc.desc: send data to 1 device, then check reporter message.
  * @tc.type: send data
  * @tc.require: AR000CQE1L
  * @tc.author: hongbo
  */
HWTEST_F(DistributedataDfxMSTTest, Dfx001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. getcommunicationFault instance
     * @tc.expected: step1. Expect instance is not null.
     */
    auto comFault = Reporter::GetInstance()->CommunicationFault();
    EXPECT_NE(nullptr, comFault);
    struct FaultMsg msg{.faultType= FaultType::SERVICE_FAULT, .moduleName = "comm", .interfaceName = "sendData",
            .errorType = Fault::CF_CREATE_SESSION};
    auto repStatus = comFault->Report(msg);
    EXPECT_TRUE(repStatus == ReportStatus::SUCCESS);

    std::string myuid = "203230afadj020003";
    std::vector<uint8_t> value(myuid.begin(), myuid.end());
    std::vector<uint8_t> result;

    SHA256_CTX *context = new (std::nothrow) SHA256_CTX;
    if (context == nullptr || !SHA256_Init(context)) {
        delete context;
        return;
    }

    if (!SHA256_Update(context, value.data(), value.size())) {
        delete context;
        return;
    }
    result.resize(SHA256_DIGEST_LENGTH);
    SHA256_Final(result.data(), context);
    delete context;
}
