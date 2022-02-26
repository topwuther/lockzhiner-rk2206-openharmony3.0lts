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
#include <cstdint>
#include <thread>
#include <vector>
#include "kvstore_client_death_observer.h"
#include "kvstore_data_service.h"

using namespace testing::ext;
using namespace OHOS::DistributedKv;
using namespace OHOS;

class KvStoreDataServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void KvStoreDataServiceTest::SetUpTestCase(void)
{}

void KvStoreDataServiceTest::TearDownTestCase(void)
{}

void KvStoreDataServiceTest::SetUp(void)
{}

void KvStoreDataServiceTest::TearDown(void)
{}

/**
* @tc.name: RegisterClientDeathObserver001
* @tc.desc: register client death observer
* @tc.type: FUNC
* @tc.require: AR000CQDU2
* @tc.author: liuyuhui
*/
HWTEST_F(KvStoreDataServiceTest, RegisterClientDeathObserver001, TestSize.Level1)
{
    AppId appId;
    appId.appId = "app0";

    KvStoreDataService kvDataService;
    Status status = kvDataService.RegisterClientDeathObserver(appId, new KvStoreClientDeathObserver());

    EXPECT_EQ(status, Status::SUCCESS) << "RegisterClientDeathObserver failed";
}
