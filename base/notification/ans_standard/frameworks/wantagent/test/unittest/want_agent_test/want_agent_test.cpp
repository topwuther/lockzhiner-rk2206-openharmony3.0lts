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
#include "ability_context.h"
#include "completed_callback.h"
#include "context.h"
#include "context_container.h"
#include "element_name.h"
#include "event_handler.h"
#include "ohos/aafwk/base/base_types.h"
#include "pending_want.h"
#include "want.h"
#define private public
#define protected public
#include "want_agent.h"
#undef private
#undef protected
#include "want_agent_constant.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"
#include "want_params.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AppExecFwk;
using namespace OHOS::Notification::WantAgent;
using vector_str = std::vector<std::string>;

namespace OHOS::Notification::WantAgent {
class WantAgentTest : public testing::Test {
public:
    WantAgentTest()
    {}
    ~WantAgentTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    class WantSender : public AAFwk::WantSenderStub {
    public:
        void Send(SenderInfo &senderInfo) override;
    };
};

void WantAgentTest::WantSender::Send(SenderInfo &senderInfo)
{}

void WantAgentTest::SetUpTestCase(void)
{}

void WantAgentTest::TearDownTestCase(void)
{}

void WantAgentTest::SetUp(void)
{}

void WantAgentTest::TearDown(void)
{}

/*
 * @tc.number    : WantAgent_0100
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and GetPendingWant
 */
HWTEST_F(WantAgentTest, WantAgent_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(nullptr);
    EXPECT_EQ(wantAgent->GetPendingWant(), nullptr);
}

/*
 * @tc.number    : WantAgent_0200
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors and GetPendingWant
 */
HWTEST_F(WantAgentTest, WantAgent_0200, Function | MediumTest | Level1)
{
    sptr<IWantSender> target(new (std::nothrow) WantSender());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    EXPECT_EQ(wantAgent->GetPendingWant(), pendingWant);
}
}  // namespace OHOS::Notification::WantAgent