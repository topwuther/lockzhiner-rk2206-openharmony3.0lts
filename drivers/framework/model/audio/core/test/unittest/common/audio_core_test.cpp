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
namespace {
class AudioCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AudioCoreTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void AudioCoreTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void AudioCoreTest::SetUp()
{
}

void AudioCoreTest::TearDown()
{
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterPlatform, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERPLATFORM, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterDai, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERDAI, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterAccessory, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERACCESSORY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterCodec, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERCODEC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_SocRegisterDsp, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERDSP, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_SocDeviceRegister, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTSOCDEVICEREGISTER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_BindDaiLink, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTBINDDAILINK, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_UpdateCodecRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTUPDATECODECREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_UpdateAccessoryRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTUPDATEACCESSORYREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_UpdateCodecAiaoRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTUPDATECODECAIAOREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_UpdateAccessoryAiaoRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTUPDATEACCESSORYAIAOREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_KcontrolGetCodec, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTKCONTROLGETCODEC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_KcontrolGetAccessory, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTKCONTROLGETACCESSORY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AddControl, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTADDCONTROL, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AddControls, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTADDCONTROLS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecAiaoReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECAIAOREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryAiaoReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYAIAOREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecWriteReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECWRITEREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryWriteReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYWRITEREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecAiaoWriteReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECAIAOWRITEREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryAiaoWriteReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYAIAOWRITEREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_InfoCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTINFOCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecGetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECGETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryGetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYGETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecAiaoGetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECAIAOGETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryAiaoGetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYAIAOGETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecSetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECSETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessorySetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYSETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_CodecAiaoSetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTCODECAIAOSETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AccessoryAiaoSetCtrlOps, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTACCESSORYAIAOSETCTRLOPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
}
