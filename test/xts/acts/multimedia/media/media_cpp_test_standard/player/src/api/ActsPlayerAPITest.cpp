/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ActsPlayerAPITest.h"
#include "player.h"
using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
using namespace TestPlayerBasic;

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_SetSource_0100
    * @tc.name      : 01.SetSource操作在new之后
    * @tc.desc      : test SetSource
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_SetSource_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Prepare_0100
    * @tc.name      : 03.Prepare操作在SetSource之后
    * @tc.desc      : test Prepare
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Prepare_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_PrepareAsync_0100
    * @tc.name      : 03.PrepareAsync操作在SetSource之后
    * @tc.desc      : test PrepareAsync
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_PrepareAsync_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Play_0100
    * @tc.name      : 02.Play操作在PrepareAsync之后
    * @tc.desc      : test Play
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Play_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}


/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Stop_0100
    * @tc.name      : 04.Stop操作在Play之后
    * @tc.desc      : test Stop
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Stop_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Pause_0100
    * @tc.name      : 05.Pause操作在Play之后
    * @tc.desc      : test Pause
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Pause_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Reset_0100
    * @tc.name      : 06.Reset操作在Seek之后
    * @tc.desc      : test Reset
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Reset_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    int32_t time;
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Seek_0100
    * @tc.name      : 04.Seek操作在Play之后
    * @tc.desc      : test Seek
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Seek_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    int32_t time;
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Seek_0200
    * @tc.name      : 06.Seek操作在Pause之后
    * @tc.desc      : test Seek
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Seek_0200, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    int32_t time;
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Seek_0300
    * @tc.name      : 10.Seek操作调用3次
    * @tc.desc      : test Seek
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_Seek_0300, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    int32_t time;
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_SetPlaybackSpeed_0100
    * @tc.name      : 02.SetPlaybackSpeed操作在play之后
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_SetPlaybackSpeed_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    PlaybackRateMode mode;
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    ASSERT_EQ(RET_OK, player->Prepare());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->SetPlaybackSpeed(SPEED_FORWARD_0_75_X));
    EXPECT_EQ(RET_OK, player->GetPlaybackSpeed(mode));
    EXPECT_EQ(SPEED_FORWARD_0_75_X, mode);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_APITest_Seek_1500
    * @tc.name      : 15.GET操作在每个可进行的操作后都调用一次
    * @tc.desc      : test Seek
*/
HWTEST_F(ActsPlayerAPITest, SUB_MEDIA_PLAYER_APITest_GetParameter_0100, Reliability | MediumTest | Level2)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    int32_t time;
    int32_t duration;
    int32_t defaultDuration;
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    // prepare
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_EQ(RET_OK, player->GetDuration(defaultDuration));
    EXPECT_FALSE(player->IsPlaying());
    // play
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(FILE_BEGIN, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->GetDuration(duration));
    EXPECT_EQ(defaultDuration, duration);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
}
