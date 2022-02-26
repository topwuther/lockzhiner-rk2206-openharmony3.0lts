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
#include "event_handler.h"
#include "element_name.h"
#include "ohos/aafwk/base/base_types.h"
#include "pending_want.h"
#define private public
#define protected public
#include "trigger_info.h"
#undef private
#undef protected
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
class TriggerInfoTest : public testing::Test {
public:
    TriggerInfoTest()
    {}
    ~TriggerInfoTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TriggerInfoTest::SetUpTestCase(void)
{}

void TriggerInfoTest::TearDownTestCase(void)
{}

void TriggerInfoTest::SetUp(void)
{}

void TriggerInfoTest::TearDown(void)
{}

/*
 * @tc.number    : TriggerInfo_0100
 * @tc.name      : TriggerInfo Constructors
 * @tc.desc      : 1.def Constructors
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0100, Function | MediumTest | Level1)
{
    TriggerInfo triggerInfo;
    EXPECT_EQ(triggerInfo.permission_, "");
    EXPECT_EQ(triggerInfo.extraInfo_, nullptr);
    EXPECT_EQ(triggerInfo.want_, nullptr);
    EXPECT_EQ(triggerInfo.resultCode_, 0);
}

/*
 * @tc.number    : TriggerInfo_0200
 * @tc.name      : TriggerInfo Constructors
 * @tc.desc      : 1.def Constructors
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0200, Function | MediumTest | Level1)
{
    std::string permission = "nihao";
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    int resultCode = 10;
    TriggerInfo triggerInfo(permission, wParams, want, resultCode);
    EXPECT_EQ(triggerInfo.permission_, permission);
    EXPECT_NE(triggerInfo.extraInfo_, wParams);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(triggerInfo.extraInfo_->GetParam(key))), value);
    EXPECT_NE(triggerInfo.want_, want);
    EXPECT_EQ(triggerInfo.want_->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(triggerInfo.want_->GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(triggerInfo.resultCode_, resultCode);
}

/*
 * @tc.number    : TriggerInfo_0300
 * @tc.name      : TriggerInfo Constructors
 * @tc.desc      : 1.copy Constructors
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0300, Function | MediumTest | Level1)
{
    std::string permission = "nihao";
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    int resultCode = 10;
    TriggerInfo triggerInfo(permission, wParams, want, resultCode);
    EXPECT_EQ(triggerInfo.permission_, permission);
    EXPECT_NE(triggerInfo.extraInfo_, wParams);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(triggerInfo.extraInfo_->GetParam(key))), value);
    EXPECT_NE(triggerInfo.want_, want);
    EXPECT_EQ(triggerInfo.want_->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(triggerInfo.want_->GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(triggerInfo.resultCode_, resultCode);
    TriggerInfo triggerInfo1(triggerInfo);
    EXPECT_EQ(triggerInfo1.permission_, triggerInfo.permission_);
    EXPECT_NE(triggerInfo1.extraInfo_, triggerInfo.extraInfo_);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(triggerInfo1.extraInfo_->GetParam(key))), value);
    EXPECT_NE(triggerInfo1.want_, triggerInfo.want_);
    EXPECT_EQ(triggerInfo1.want_->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(triggerInfo1.want_->GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(triggerInfo1.resultCode_, triggerInfo.resultCode_);
}

/*
 * @tc.number    : TriggerInfo_0400
 * @tc.name      : TriggerInfo::Builder SetPermission
 * @tc.desc      : 1.SetPermission
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0400, Function | MediumTest | Level1)
{
    std::shared_ptr<TriggerInfo::Builder> builder = std::make_shared<TriggerInfo::Builder>();
    builder->SetPermission("nihao");
    EXPECT_EQ(builder->permission_, "nihao");
}

/*
 * @tc.number    : TriggerInfo_0500
 * @tc.name      : TriggerInfo::Builder SetWantParams
 * @tc.desc      : 1.SetWantParams
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0500, Function | MediumTest | Level1)
{
    std::shared_ptr<TriggerInfo::Builder> builder = std::make_shared<TriggerInfo::Builder>();
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    builder->SetWantParams(wParams);
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(builder->params_->GetParam(key))), value);
}

/*
 * @tc.number    : TriggerInfo_0600
 * @tc.name      : TriggerInfo::Builder SetWant
 * @tc.desc      : 1.SetWant
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0600, Function | MediumTest | Level1)
{
    std::shared_ptr<TriggerInfo::Builder> builder = std::make_shared<TriggerInfo::Builder>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    builder->SetWant(want);
    EXPECT_EQ(builder->want_->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(builder->want_->GetElement().GetAbilityName(), "abilityName");
}

/*
 * @tc.number    : TriggerInfo_0700
 * @tc.name      : TriggerInfo::Builder SetResultCode
 * @tc.desc      : 1.SetResultCode
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0700, Function | MediumTest | Level1)
{
    std::shared_ptr<TriggerInfo::Builder> builder = std::make_shared<TriggerInfo::Builder>();
    int resultCode = 100;
    builder->SetResultCode(resultCode);
    EXPECT_EQ(builder->resultCode_, resultCode);
}

/*
 * @tc.number    : TriggerInfo_0800
 * @tc.name      : TriggerInfo::Builder Build
 * @tc.desc      : 1.Build,Great TriggerInfo object
 */
HWTEST_F(TriggerInfoTest, TriggerInfo_0800, Function | MediumTest | Level1)
{
    std::shared_ptr<TriggerInfo::Builder> builder = std::make_shared<TriggerInfo::Builder>();
    builder->SetPermission("nihao");
    bool value = true;
    std::string key = "key";
    std::shared_ptr<WantParams> wParams = std::make_shared<WantParams>();
    wParams->SetParam(key, Boolean::Box(value));
    builder->SetWantParams(wParams);
    std::shared_ptr<Want> want = std::make_shared<Want>();
    ElementName element("device", "bundleName", "abilityName");
    want->SetElement(element);
    builder->SetWant(want);
    int resultCode = 100;
    builder->SetResultCode(resultCode);

    auto info = builder->Build();
    EXPECT_NE(info, nullptr);
    EXPECT_EQ(info->permission_, "nihao");
    EXPECT_EQ(Boolean::Unbox(IBoolean::Query(info->extraInfo_->GetParam(key))), value);
    EXPECT_EQ(info->want_->GetElement().GetBundleName(), "bundleName");
    EXPECT_EQ(info->want_->GetElement().GetAbilityName(), "abilityName");
    EXPECT_EQ(info->resultCode_, resultCode);
}
}  // namespace OHOS::Notification::WantAgent