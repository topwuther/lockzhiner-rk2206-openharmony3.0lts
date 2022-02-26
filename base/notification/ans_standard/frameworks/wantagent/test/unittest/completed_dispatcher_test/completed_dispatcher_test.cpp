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
#include "completed_callback.h"
#define private public
#define protected public
#include "completed_dispatcher.h"
#undef private
#undef protected
#include "element_name.h"
#include "event_handler.h"
#include "pending_want.h"
#include "ohos/aafwk/base/base_types.h"
#include "want.h"
#include "want_params.h"
#include "want_receiver_stub.h"
#include "ohos/aafwk/base/bool_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AppExecFwk;
using vector_str = std::vector<std::string>;

namespace OHOS::Notification::WantAgent {
class CompletedDispatcherTest : public testing::Test {
public:
    CompletedDispatcherTest()
    {}
    ~CompletedDispatcherTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    Want MakeWant(std::string deviceId, std::string abilityName, std::string bundleName);

    class CompletedCallbackSon : public CompletedCallback {
        /**
         * Called when a Send operation as completed.
         *
         * @param want The original Want that was sent.
         * @param resultCode The final result code determined by the Send.
         * @param resultData The final data collected by a broadcast.
         * @param resultExtras The final extras collected by a broadcast.
         */
    public:
        void OnSendFinished(const AAFwk::Want &want, int resultCode, const std::string &resultData,
            const AAFwk::WantParams &resultExtras) override;
        static int code;
    };
};

int CompletedDispatcherTest::CompletedCallbackSon::code = 0;

void CompletedDispatcherTest::CompletedCallbackSon::OnSendFinished(
    const AAFwk::Want &want, int resultCode, const std::string &resultData, const AAFwk::WantParams &resultExtras)
{
    code = 100;
}

Want CompletedDispatcherTest::MakeWant(std::string deviceId, std::string abilityName, std::string bundleName)
{
    ElementName element(deviceId, bundleName, abilityName);
    Want want;
    want.SetElement(element);
    return want;
}

void CompletedDispatcherTest::SetUpTestCase(void)
{}

void CompletedDispatcherTest::TearDownTestCase(void)
{}

void CompletedDispatcherTest::SetUp(void)
{}

void CompletedDispatcherTest::TearDown(void)
{}

/*
 * @tc.number    : CompletedDispatcher_0100
 * @tc.name      : CompletedDispatcher Constructors
 * @tc.desc      : 1.The parameter is nullptr
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0100, Function | MediumTest | Level1)
{
    CompletedDispatcher completedDispatcher(nullptr, nullptr, nullptr);
    EXPECT_EQ(completedDispatcher.pendingWant_, nullptr);
    EXPECT_EQ(completedDispatcher.callback_, nullptr);
    EXPECT_EQ(completedDispatcher.handler_, nullptr);
}

/*
 * @tc.number    : CompletedDispatcher_0200
 * @tc.name      : CompletedDispatcher Constructors
 * @tc.desc      : 1.The parameter is not nullptr
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CompletedDispatcherTest::CompletedCallbackSon> callBack =
        std::make_shared<CompletedDispatcherTest::CompletedCallbackSon>();
    CompletedDispatcher completedDispatcher(nullptr, callBack, nullptr);
    EXPECT_EQ(completedDispatcher.pendingWant_, nullptr);
    EXPECT_EQ(completedDispatcher.callback_, callBack);
    EXPECT_EQ(completedDispatcher.handler_, nullptr);
}

/*
 * @tc.number    : CompletedDispatcher_0300
 * @tc.name      : CompletedDispatcher PerformReceive
 * @tc.desc      : 1.The parameter is not nullptr
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<CompletedDispatcherTest::CompletedCallbackSon> callBack =
        std::make_shared<CompletedDispatcherTest::CompletedCallbackSon>();
    CompletedDispatcher completedDispatcher(nullptr, callBack, nullptr);

    Want want = MakeWant("device", "ability", "bundleName");
    std::string key = "key";
    bool value = true;
    WantParams wParams;
    wParams.SetParam(key, Boolean::Box(value));
    completedDispatcher.PerformReceive(want, 10, "test", wParams, 0, 0, 1);
    EXPECT_EQ(completedDispatcher.want_.GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(completedDispatcher.want_.GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(completedDispatcher.resultCode_, 10);
    EXPECT_EQ(completedDispatcher.resultData_, "test");
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(completedDispatcher.resultExtras_.GetParam(key))), value);
    EXPECT_EQ(CompletedCallbackSon::code, 100);
    CompletedCallbackSon::code = 0;
}

/*
 * @tc.number    : CompletedDispatcher_0400
 * @tc.name      : CompletedDispatcher PerformReceive
 * @tc.desc      : 1.The parameter is not nullptr
 *                 2.called callBack
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<CompletedDispatcherTest::CompletedCallbackSon> callBack =
        std::make_shared<CompletedDispatcherTest::CompletedCallbackSon>();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>();
    CompletedDispatcher completedDispatcher(nullptr, callBack, handler);

    Want want = MakeWant("device", "ability", "bundleName");
    std::string key = "key";
    bool value = false;
    WantParams wParams;
    wParams.SetParam(key, Boolean::Box(value));
    completedDispatcher.PerformReceive(want, 10, "test", wParams, 0, 0, 1);
    EXPECT_EQ(completedDispatcher.want_.GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(completedDispatcher.want_.GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(completedDispatcher.resultCode_, 10);
    EXPECT_EQ(completedDispatcher.resultData_, "test");
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(completedDispatcher.resultExtras_.GetParam(key))), value);
    EXPECT_EQ(CompletedCallbackSon::code, 0);
    CompletedCallbackSon::code = 0;
}

/*
 * @tc.number    : CompletedDispatcher_0500
 * @tc.name      : CompletedDispatcher Run
 * @tc.desc      : 1.The parameter is not nullptr
 *                 2.called callBack
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0500, Function | MediumTest | Level1)
{
    std::shared_ptr<CompletedDispatcherTest::CompletedCallbackSon> callBack =
        std::make_shared<CompletedDispatcherTest::CompletedCallbackSon>();
    CompletedDispatcher completedDispatcher(nullptr, callBack, nullptr);
    EXPECT_EQ(completedDispatcher.callback_, callBack);
    completedDispatcher.Run();
    EXPECT_EQ(CompletedCallbackSon::code, 100);
    CompletedCallbackSon::code = 0;
}

/*
 * @tc.number    : CompletedDispatcher_0600
 * @tc.name      : CompletedDispatcher Run
 * @tc.desc      : 1.The parameter is nullptr
 *                 2.no called callBack
 */
HWTEST_F(CompletedDispatcherTest, CompletedDispatcher_0600, Function | MediumTest | Level1)
{
    CompletedDispatcher completedDispatcher(nullptr, nullptr, nullptr);
    EXPECT_EQ(completedDispatcher.callback_, nullptr);
    completedDispatcher.Run();
    EXPECT_EQ(CompletedCallbackSon::code, 0);
    CompletedCallbackSon::code = 0;
}
}  // namespace OHOS::Notification::WantAgent