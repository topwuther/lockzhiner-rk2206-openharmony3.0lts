/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_common_test.h"
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"

using namespace testing::ext;

class AudioSapmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AudioSapmTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void AudioSapmTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void AudioSapmTest::SetUp()
{
}

void AudioSapmTest::TearDown()
{
}

HWTEST_F(AudioSapmTest, AudioSapmTest001, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTNEWCOMPONENT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioSapmTest, AudioSapmTest002, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTADDROUTES, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioSapmTest, AudioSapmTest003, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTNEWCONTROLS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioSapmTest, AudioSapmTest004, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTPOWERCOMPONET, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioSapmTest, AudioSapmTest005, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREFRESHTIME, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
