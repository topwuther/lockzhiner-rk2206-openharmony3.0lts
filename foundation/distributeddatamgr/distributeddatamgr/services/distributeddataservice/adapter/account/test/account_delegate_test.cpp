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

#define LOG_TAG "AccountDelegateTest"
#include <gtest/gtest.h>
#include <memory.h>
#include "account_delegate.h"
#include "log_print.h"

using namespace OHOS::DistributedKv;
using namespace testing::ext;
class AccountDelegateTest : public testing::Test {
};

class AccountObserver : public AccountDelegate::Observer {
public:
    void OnAccountChanged(const AccountEventInfo &info)
    {
    }
    // must specify unique name for observer
    std::string Name()
    {
        return "accountTestObserver";
    }
};
/**
* @tc.name: Test001
* @tc.desc: Test account login, logout, unbind;
* @tc.type: FUNC
* @tc.require: AR000D08K2 AR000CQDUM
* @tc.author: hongbo
*/
HWTEST_F(AccountDelegateTest, Test001, TestSize.Level0)
{
    ZLOGD("start:");
    auto account = AccountDelegate::GetInstance();
    auto observer = std::make_shared<AccountObserver>();
    ZLOGD("observer:");
    account->Subscribe(observer);
    ZLOGD("observer subscribed");

    account->Unsubscribe(observer);
    ASSERT_TRUE(true);
}

/**
* @tc.name: Test002
* @tc.desc: Test getaccount
* @tc.type: FUNC
* @tc.require: AR000D487D
* @tc.author: hongbo
*/
HWTEST_F(AccountDelegateTest, Test002, TestSize.Level0)
{
    auto account = AccountDelegate::GetInstance();
    auto id = account->MAIN_DEVICE_ACCOUNT_ID;
    ZLOGD("observer subscribed %s", id.c_str());
    ASSERT_TRUE(!id.empty());
}
