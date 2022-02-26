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
const std::string STRING_CODE = "1024";
const std::string STRING_DATA = "data";
}  // namespace

class CemCommandPublishTest : public testing::Test {
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

void CemCommandPublishTest::SetUpTestCase()
{}

void CemCommandPublishTest::TearDownTestCase()
{}

void CemCommandPublishTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void CemCommandPublishTest::TearDown()
{}

void CemCommandPublishTest::MakeMockObjects()
{
    // mock a stub
    auto stubPtr = sptr<IRemoteObject>(new MockCommonEventStub());

    // mock a proxy
    proxyPtr_ = iface_cast<ICommonEvent>(stubPtr);

    // set the mock proxy
    auto commonEventPtr = DelayedSingleton<CommonEvent>::GetInstance();
    commonEventPtr->commonEventProxy_ = proxyPtr_;
}

void CemCommandPublishTest::SetMockObjects(const CommonEventManagerShellCommand &cmd) const
{}

/**
 * @tc.number: Cem_Command_Publish_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish xxx" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -x" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -xxx" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish --x" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--x",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish --xxx" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--xxx",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: unknown option.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -h" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-h",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish --help" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"--help",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_0900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1000, Function | MediumTest | Level1)
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

/**
 * @tc.number: Cem_Command_Publish_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -c" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1100, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-c",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -c" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1200, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-c",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -c <code>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1300, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-c",
        (char *)STRING_CODE.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_EVENT_OPTION + "\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -c <code>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1400, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-c",
        (char *)STRING_CODE.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_PUBLISH_COMMON_EVENT_OK + "\n");
}

/**
 * @tc.number: Cem_Command_Publish_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -d" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1500, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-d",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -d" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1600, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-d",
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -d <data>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1700, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-d",
        (char *)STRING_DATA.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_EVENT_OPTION + "\n" + HELP_MSG_PUBLISH);
}

/**
 * @tc.number: Cem_Command_Publish_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -d <data>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1800, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-d",
        (char *)STRING_DATA.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_PUBLISH_COMMON_EVENT_OK + "\n");
}

/**
 * @tc.number: Cem_Command_Publish_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "cem publish -e <name> -c <code> -d <data>" command.
 */
HWTEST_F(CemCommandPublishTest, Cem_Command_Publish_1900, Function | MediumTest | Level1)
{
    char *argv[] = {
        (char *)TOOL_NAME.c_str(),
        (char *)cmd_.c_str(),
        (char *)"-e",
        (char *)STRING_EVENT.c_str(),
        (char *)"-c",
        (char *)STRING_CODE.c_str(),
        (char *)"-d",
        (char *)STRING_DATA.c_str(),
        (char *)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    CommonEventManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_PUBLISH_COMMON_EVENT_OK + "\n");
}
