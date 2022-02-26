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

#define private public
#include "common_event_command.h"
#undef private
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "mock_common_event_stub.h"
#include "singleton.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::EventFwk;

namespace {
const std::string STRING_EVENT = "com.ces.event";
}  // namespace

class CemCommandPublishModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects();
    void SetMockObjects(const CommonEventManagerShellCommand &cmd) const;

    std::string cmd_ = "publish";
    sptr<ICommonEvent> proxyPtr_;
};

void CemCommandPublishModuleTest::SetUpTestCase()
{}

void CemCommandPublishModuleTest::TearDownTestCase()
{}

void CemCommandPublishModuleTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void CemCommandPublishModuleTest::TearDown()
{}

void CemCommandPublishModuleTest::MakeMockObjects()
{
    // mock a stub
    auto stubPtr = sptr<IRemoteObject>(new MockCommonEventStub());

    // mock a proxy
    proxyPtr_ = iface_cast<ICommonEvent>(stubPtr);

    // set the mock proxy
    auto commonEventPtr = DelayedSingleton<CommonEvent>::GetInstance();
    commonEventPtr->commonEventProxy_ = proxyPtr_;
}

void CemCommandPublishModuleTest::SetMockObjects(const CommonEventManagerShellCommand &cmd) const
{}

class CommonEventSubscriberTest : public CommonEventSubscriber {
public:
    explicit CommonEventSubscriberTest(const CommonEventSubscribeInfo &subscribeInfo)
        : CommonEventSubscriber(subscribeInfo)
    {}

    ~CommonEventSubscriberTest()
    {}

    void OnReceiveEvent(const CommonEventData &data)
    {}
};

/**
 * @tc.number: Cem_Command_Publish_ModuleTest_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name>" command.
 */
HWTEST_F(CemCommandPublishModuleTest, Cem_Command_Publish_ModuleTest_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_PUBLISH_COMMON_EVENT_OK + "\n");
}
