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
#include "want_agent.h"
#include "want_agent_constant.h"
#define private public
#define protected public
#include "want_agent_helper.h"
#include "want_agent_info.h"
#undef private
#undef protected
#include "want_params.h"
#include "want_receiver_stub.h"
#include "want_sender_stub.h"
#include "ohos/aafwk/base/bool_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using OHOS::AppExecFwk::ElementName;
using namespace OHOS::AppExecFwk;
using vector_str = std::vector<std::string>;

namespace OHOS::Notification::WantAgent {
class WantAgentHelperTest : public testing::Test {
public:
    WantAgentHelperTest()
    {}
    ~WantAgentHelperTest()
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

void WantAgentHelperTest::WantSender::Send(SenderInfo &senderInfo)
{}

void WantAgentHelperTest::SetUpTestCase(void)
{}

void WantAgentHelperTest::TearDownTestCase(void)
{}

void WantAgentHelperTest::SetUp(void)
{}

void WantAgentHelperTest::TearDown(void)
{}

/*
 * @tc.number    : WantAgentHelper_0100
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::ONE_TIME_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::ONE_TIME_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x40000000));
}

/*
 * @tc.number    : WantAgentHelper_0200
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::NO_BUILD_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::NO_BUILD_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x20000000));
}

/*
 * @tc.number    : WantAgentHelper_0300
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CANCEL_PRESENT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CANCEL_PRESENT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x10000000));
}

/*
 * @tc.number    : WantAgentHelper_0400
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::UPDATE_PRESENT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x8000000));
}

/*
 * @tc.number    : WantAgentHelper_0500
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CONSTANT_FLAG
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0x4000000));
}

/*
 * @tc.number    : WantAgentHelper_0600
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::REPLACE_BUNDLE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    EXPECT_EQ(
        wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(WantAgentConstant::Flags::ONE_TIME_FLAG));
}

/*
 * @tc.number    : WantAgentHelper_0700
 * @tc.name      : WantAgentHelper Constructors
 * @tc.desc      : 1.WantAgentConstant::Flags::CONSTANT_FLAG | UPDATE_PRESENT_FLAG | REPLACE_BUNDLE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::vector<WantAgentConstant::Flags> flags;
    flags.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    flags.emplace_back(WantAgentConstant::Flags::UPDATE_PRESENT_FLAG);
    flags.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    EXPECT_EQ(wantAgentHelper->FlagsTransformer(flags), static_cast<unsigned int>(0xc000000));
}

/*
 * @tc.number    : WantAgentHelper_0800
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    WantAgentInfo wantAgentInfo;
    auto wantAgent = wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_0900
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *          `      2.wantAgentInfo.wants_ empty
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_0900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.clear();
    auto wantAgent = wantAgentHelper->GetWantAgent(nullptr, wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1000
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() != wantAgentInfo.flags_.siez()
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.clear();
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1100
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return 0
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    WantAgentInfo wantAgentInfo;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::REPLACE_BUNDLE);
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1200
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_ABILITY
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITY;
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1300
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_ABILITIES
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_ABILITIES;
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1400
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_SERVICE;
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1500
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::START_FOREGROUND_SERVICE;
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1600
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is WantAgentConstant::OperationType::START_FOREGROUND_SERVICE
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = WantAgentConstant::OperationType::SEND_COMMON_EVENT;
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_NE(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1700
 * @tc.name      : WantAgentHelper GetWantAgent
 * @tc.desc      : 1.GetWantAgent context is not nullptr
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.FlagsTransformer return ok
 *                 4.Type is 100
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(context, wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1800
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_.siez() == 0
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.clear();
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_1900
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.siez() != wantAgentInfo.flags_.siez()
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_1900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.clear();
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2000
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.wantAgentInfo.wants_[0] is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(nullptr);
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2100
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    wantAgentInfo.extraInfo_ = wParams;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2200
 * @tc.name      : WantAgentHelper GetWantAgent(const WantAgentInfo &paramsInfo)
 * @tc.desc      : 1.wantAgentInfo.wants_ not empty
 *                 2.wantAgentInfo.wants_.siez() == wantAgentInfo.flags_.siez()
 *                 3.wantAgentInfo.wants_[0] is not nullptr
 *                 4.wantAgentInfo.extraInfo_ is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    WantAgentInfo wantAgentInfo;
    wantAgentInfo.wants_.emplace_back(want);
    wantAgentInfo.flags_.emplace_back(WantAgentConstant::Flags::CONSTANT_FLAG);
    wantAgentInfo.operationType_ = static_cast<WantAgentConstant::OperationType>(100);
    wantAgentInfo.requestCode_ = 10;
    wantAgentInfo.extraInfo_ = nullptr;
    auto wantAgent = wantAgentHelper->GetWantAgent(wantAgentInfo);
    EXPECT_EQ(wantAgent, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_2300
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<IWantSender> target(new (std::nothrow) WantSender());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2400
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2500
 * @tc.name      : WantAgentHelper GetType
 * @tc.desc      : 1.agent is not nullptr,PendingWant is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto type = wantAgentHelper->GetType(wantAgent);
    EXPECT_EQ(type, WantAgentConstant::OperationType::UNKNOWN_TYPE);
}

/*
 * @tc.number    : WantAgentHelper_2600
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    std::shared_ptr<WantAgent> wantAgent2(nullptr);
    auto isEqual = wantAgentHelper->JudgeEquality(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, true);
}

/*
 * @tc.number    : WantAgentHelper_2700
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<IWantSender> target(new (std::nothrow) WantSender());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2(nullptr);
    auto isEqual = wantAgentHelper->JudgeEquality(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, false);
}

/*
 * @tc.number    : WantAgentHelper_2800
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2800, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<IWantSender> target(new (std::nothrow) WantSender());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2 = std::make_shared<WantAgent>(pendingWant);
    auto isEqual = wantAgentHelper->JudgeEquality(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, true);
}

/*
 * @tc.number    : WantAgentHelper_2900
 * @tc.name      : WantAgentHelper JudgeEquality
 * @tc.desc      : 1.JudgeEquality,Judge whether it is equal or not
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_2900, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    sptr<IWantSender> target(new (std::nothrow) WantSender());
    std::shared_ptr<PendingWant> pendingWant = std::make_shared<PendingWant>(target);
    std::shared_ptr<PendingWant> pendingWant2(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    std::shared_ptr<WantAgent> wantAgent2 = std::make_shared<WantAgent>(pendingWant2);
    auto isEqual = wantAgentHelper->JudgeEquality(wantAgent, wantAgent2);
    EXPECT_EQ(isEqual, false);
}

/*
 * @tc.number    : WantAgentHelper_3000
 * @tc.name      : WantAgentHelper GetHashCode
 * @tc.desc      : 1.GetHashCode WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3000, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto hashCode = wantAgentHelper->GetHashCode(wantAgent);
    EXPECT_EQ(hashCode, 0);
}

/*
 * @tc.number    : WantAgentHelper_3100
 * @tc.name      : WantAgentHelper GetHashCode
 * @tc.desc      : 1.GetHashCode WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3100, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto hashCode = wantAgentHelper->GetHashCode(wantAgent);
    EXPECT_EQ(hashCode, 0);
}

/*
 * @tc.number    : WantAgentHelper_3200
 * @tc.name      : WantAgentHelper GetBundleName
 * @tc.desc      : 1.GetBundleName WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3200, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto bundleName = wantAgentHelper->GetBundleName(wantAgent);
    EXPECT_EQ(bundleName, "");
}

/*
 * @tc.number    : WantAgentHelper_3300
 * @tc.name      : WantAgentHelper GetBundleName
 * @tc.desc      : 1.GetBundleName WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3300, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto bundleName = wantAgentHelper->GetBundleName(wantAgent);
    EXPECT_EQ(bundleName, "");
}

/*
 * @tc.number    : WantAgentHelper_3400
 * @tc.name      : WantAgentHelper GetUid
 * @tc.desc      : 1.GetUid WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3400, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto uid = wantAgentHelper->GetUid(wantAgent);
    EXPECT_EQ(uid, -1);
}

/*
 * @tc.number    : WantAgentHelper_3500
 * @tc.name      : WantAgentHelper GetUid
 * @tc.desc      : 1.GetUid WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3500, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto uid = wantAgentHelper->GetUid(wantAgent);
    EXPECT_EQ(uid, -1);
}

/*
 * @tc.number    : WantAgentHelper_3600
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3600, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<WantAgent> wantAgent(nullptr);
    auto want = wantAgentHelper->GetWant(wantAgent);
    EXPECT_EQ(want, nullptr);
}

/*
 * @tc.number    : WantAgentHelper_3700
 * @tc.name      : WantAgentHelper GetWant
 * @tc.desc      : 1.GetWant WantAgent.PendingWant.target is nullptr
 */
HWTEST_F(WantAgentHelperTest, WantAgentHelper_3700, Function | MediumTest | Level1)
{
    std::shared_ptr<WantAgentHelper> wantAgentHelper = std::make_shared<WantAgentHelper>();
    std::shared_ptr<PendingWant> pendingWant(nullptr);
    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant);
    auto want = wantAgentHelper->GetWant(wantAgent);
    EXPECT_EQ(want, nullptr);
}
}  // namespace OHOS::Notification::WantAgent