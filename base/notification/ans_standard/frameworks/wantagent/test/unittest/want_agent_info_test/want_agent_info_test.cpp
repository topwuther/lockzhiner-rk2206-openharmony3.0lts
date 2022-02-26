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
#include "want_agent_helper.h"
#define private public
#define protected public
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
class WantAgentInfoTest : public testing::Test {
public:
    WantAgentInfoTest()
    {}
    ~WantAgentInfoTest()
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

void WantAgentInfoTest::WantSender::Send(SenderInfo &senderInfo)
{}

void WantAgentInfoTest::SetUpTestCase(void)
{}

void WantAgentInfoTest::TearDownTestCase(void)
{}

void WantAgentInfoTest::SetUp(void)
{}

void WantAgentInfoTest::TearDown(void)
{}

/*
 * @tc.number    : WantAgentInfo_0100
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0100, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flag, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetRequestCode(), requestCode);
    EXPECT_EQ(wantAgentInfo->GetOperationType(), operationType);
    EXPECT_EQ(wantAgentInfo->GetFlags().at(0), flag);
    EXPECT_NE(wantAgentInfo->GetWants().at(0), want);
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetBundleName(), want->GetElement().GetBundleName());
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetAbilityName(), want->GetElement().GetAbilityName());
    EXPECT_NE(wantAgentInfo->GetExtraInfo(), extraInfo);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(wantAgentInfo->GetExtraInfo()->GetParam(key))),
        Boolean::Unbox(IBoolean::Query(extraInfo->GetParam(key))));
}

/*
 * @tc.number    : WantAgentInfo_0200
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Constructors
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0200, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetRequestCode(), requestCode);
    EXPECT_EQ(wantAgentInfo->GetOperationType(), operationType);
    EXPECT_EQ(wantAgentInfo->GetFlags().at(0), flag);
    EXPECT_NE(wantAgentInfo->GetWants().at(0), want);
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetBundleName(), want->GetElement().GetBundleName());
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetAbilityName(), want->GetElement().GetAbilityName());
    EXPECT_NE(wantAgentInfo->GetExtraInfo(), extraInfo);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(wantAgentInfo->GetExtraInfo()->GetParam(key))),
        Boolean::Unbox(IBoolean::Query(extraInfo->GetParam(key))));
}

/*
 * @tc.number    : WantAgentInfo_0300
 * @tc.name      : WantAgentInfo Constructors
 * @tc.desc      : 1.Copy Constructors
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0300, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetRequestCode(), requestCode);
    EXPECT_EQ(wantAgentInfo->GetOperationType(), operationType);
    EXPECT_EQ(wantAgentInfo->GetFlags().at(0), flag);
    EXPECT_NE(wantAgentInfo->GetWants().at(0), want);
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetBundleName(), want->GetElement().GetBundleName());
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetAbilityName(), want->GetElement().GetAbilityName());
    EXPECT_NE(wantAgentInfo->GetExtraInfo(), extraInfo);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(wantAgentInfo->GetExtraInfo()->GetParam(key))),
        Boolean::Unbox(IBoolean::Query(extraInfo->GetParam(key))));

    WantAgentInfo wantAgentInfo1(wantAgentInfo);
    EXPECT_EQ(wantAgentInfo->GetRequestCode(), wantAgentInfo1.GetRequestCode());
    EXPECT_EQ(wantAgentInfo->GetOperationType(), wantAgentInfo1.GetOperationType());
    EXPECT_EQ(wantAgentInfo->GetFlags().at(0), wantAgentInfo1.GetFlags().at(0));
    EXPECT_NE(wantAgentInfo->GetWants().at(0), wantAgentInfo1.GetWants().at(0));
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetBundleName(),
        wantAgentInfo1.GetWants().at(0)->GetElement().GetBundleName());
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetAbilityName(),
        wantAgentInfo1.GetWants().at(0)->GetElement().GetAbilityName());
    EXPECT_NE(wantAgentInfo->GetExtraInfo(), wantAgentInfo1.GetExtraInfo());
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(wantAgentInfo->GetExtraInfo()->GetParam(key))),
        Boolean::Unbox(IBoolean::Query(wantAgentInfo1.GetExtraInfo()->GetParam(key))));
}

/*
 * @tc.number    : WantAgentInfo_0400
 * @tc.name      : WantAgentInfo GetRequestCode
 * @tc.desc      : 1.GetRequestCode
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0400, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetRequestCode(), requestCode);
}

/*
 * @tc.number    : WantAgentInfo_0500
 * @tc.name      : WantAgentInfo GetOperationType
 * @tc.desc      : 1.GetOperationType
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0500, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetOperationType(), operationType);
}

/*
 * @tc.number    : WantAgentInfo_0600
 * @tc.name      : WantAgentInfo GetFlags
 * @tc.desc      : 1.GetFlags
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0600, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetFlags().at(0), flag);
}

/*
 * @tc.number    : WantAgentInfo_0700
 * @tc.name      : WantAgentInfo GetWants
 * @tc.desc      : 1.GetWants
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0700, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetBundleName(), want->GetElement().GetBundleName());
    EXPECT_EQ(wantAgentInfo->GetWants().at(0)->GetElement().GetAbilityName(), want->GetElement().GetAbilityName());
}

/*
 * @tc.number    : WantAgentInfo_0800
 * @tc.name      : WantAgentInfo GetWants
 * @tc.desc      : 1.GetWants
 */
HWTEST_F(WantAgentInfoTest, WantAgentInfo_0800, Function | MediumTest | Level1)
{
    int requestCode = 10;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    std::vector<WantAgentConstant::Flags> flags;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::CONSTANT_FLAG;
    flags.emplace_back(flag);
    std::vector<std::shared_ptr<Want>> wants;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    wants.emplace_back(want);
    std::shared_ptr<WantParams> extraInfo = std::make_shared<WantParams>();
    bool value = true;
    std::string key = "key";
    extraInfo->SetParam(key, Boolean::Box(value));
    std::shared_ptr<WantAgentInfo> wantAgentInfo =
        std::make_shared<WantAgentInfo>(requestCode, operationType, flags, wants, extraInfo);
    EXPECT_NE(wantAgentInfo->GetExtraInfo(), extraInfo);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(wantAgentInfo->GetExtraInfo()->GetParam(key))),
        Boolean::Unbox(IBoolean::Query(extraInfo->GetParam(key))));
}
}  // namespace OHOS::Notification::WantAgent