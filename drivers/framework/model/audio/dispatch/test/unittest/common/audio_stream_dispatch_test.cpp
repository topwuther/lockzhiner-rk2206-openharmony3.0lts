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

class AudioStreamDispatchTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AudioStreamDispatchTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void AudioStreamDispatchTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void AudioStreamDispatchTest::SetUp()
{
}

void AudioStreamDispatchTest::TearDown()
{
}

HWTEST_F(AudioStreamDispatchTest, AudioStreamDispatchTest001, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTSTREAMDISPATCH, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
