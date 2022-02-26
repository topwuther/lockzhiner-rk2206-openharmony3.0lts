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

#include "ActsPlayerFuncTest.h"
#include "securec.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Media;
using namespace testing::ext;
using namespace TestPlayerBasic;

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0100
    * @tc.name      : 001.prepare-reset-release
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0100, Function | MediumTest | Level0)
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
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0200
    * @tc.name      : 002.prepare-reset
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0200, Function | MediumTest | Level0)
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
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0300
    * @tc.name      : 003.prepare-stop-reset-release
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0300, Function | MediumTest | Level0)
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
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0400
    * @tc.name      : 004.prepare-stop-reset
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0400, Function | MediumTest | Level0)
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
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0600
    * @tc.name      : 006.prepare-play-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0600, Function | MediumTest | Level0)
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
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    (void)sleep(PLAYING_TIME);
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(time, PLAYING_TIME * TIME_SEC2MS, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0700
    * @tc.name      : 007.prepare-seek-play-reset-release
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0700, Function | MediumTest | Level0)
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
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(time, PLAYING_TIME * TIME_SEC2MS, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0800
    * @tc.name      : 008.prepare-seek-play-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0800, Function | MediumTest | Level0)
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
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(time, SEEK_TIME_5_SEC + PLAYING_TIME * TIME_SEC2MS, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_0900
    * @tc.name      : 009.prepare-play-seek-reset-release
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_0900, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1000
    * @tc.name      : 010.prepare-play-seek-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1000, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1100
    * @tc.name      : 011.prepare-play-seek-pause-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1100, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1200
    * @tc.name      : 012.prepare-play-seek-pause-reset-release
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1200, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1300
    * @tc.name      : 013.prepare-play-seek-pause-stop-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1300, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1400
    * @tc.name      : 014.prepare-play-seek-pause-stop-reset-release
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1400, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1500
    * @tc.name      : 015.prepare-play-stop-reset
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1500, Function | MediumTest | Level0)
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
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1600
    * @tc.name      : 016.prepare-play-stop-reset-release
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1600, Function | MediumTest | Level0)
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
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1700
    * @tc.name      : 017.prepare-stop
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1700, Function | MediumTest | Level0)
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

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1800
    * @tc.name      : 018.pause-GetCurrentTime-play-GetCurrentTime
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1800, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS + PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_1900
    * @tc.name      : 019.pause-GetCurrentTime-seek-play-GetCurrentTime
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_1900, Function | MediumTest | Level0)
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

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_2_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_2_SEC + PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2000
    * @tc.name      : 020.play-seek EOS-GetCurrentTime-进入PlaybackCompleted,loop=0
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2000, Function | MediumTest | Level1)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->SetLooping(false));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_FALSE(player->IsLooping());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2100
    * @tc.name      : 021.seek EOS-GetCurrentTime-进入PlaybackCompleted,loop=0
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2100, Function | MediumTest | Level1)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->SetLooping(false));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_FALSE(player->IsLooping());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2200
    * @tc.name      : 022.play-seek EOS-GetCurrentTime,loop=1
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2200, Function | MediumTest | Level1)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time_before;
    int32_t time_after;
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->SetLooping(true));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsLooping());
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time_before));
    (void)sleep(PLAYING_TIME);
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time_after));
    EXPECT_NEAR(time_after - time_before, PLAYING_TIME * TIME_SEC2MS, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2300
    * @tc.name      : 023.prepare-play-seek EOS-GetCurrentTime-进入PlaybackCompleted-stop,loop=0
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2300, Function | MediumTest | Level1)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->SetLooping(false));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));

    EXPECT_FALSE(player->IsLooping());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2400
    * @tc.name      : 024.seek，seek随机+EOS附近
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2400, Function | MediumTest | Level1)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime - 1, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(durationTime, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2500
    * @tc.name      : 025.pause-seek-resume，不等seek完成
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2500, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    testObj->mutexFlag_ = true;
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(SEEK_TIME_5_SEC + PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2600
    * @tc.name      : 026.seek，不等seek完成，seek
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2600, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    testObj->mutexFlag_ = true;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}
/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2625
    * @tc.name      : 026.seek，不等seek完成，pause
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2625, Function | MediumTest | Level3)
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
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    testObj->mutexFlag_ = true;
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}
/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2650
    * @tc.name      : 026.seek，不等seek完成，GetDuration
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2650, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}
/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2651
    * @tc.name      : 026.seek，不等seek完成，GetCurrentTime
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2651, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}
/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2675
    * @tc.name      : 026.seek，不等seek完成，play
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2675, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    testObj->mutexFlag_ = false;
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    testObj->mutexFlag_ = true;
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}
/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2700
    * @tc.name      : 027.prepare-seek未完成-seek eos-start-seek eos-start-stop
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2700, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2800
    * @tc.name      : 028.prepare-start-pause-seek eos-stop-prepare-start-stop
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2800, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Pause());

    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Stop());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_2900
    * @tc.name      : 029.prepare-seek eos-start-stop-prepare-seek eos-start-stop,loop=1
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_2900, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->SetLooping(true));
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_EQ(RET_OK, player->Stop());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_04_3000
    * @tc.name      : 030.prepare-start-pause-seek eos-start-stop-prepare-start-pause-seek-start-stop,loop=1
    * @tc.desc      :
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_04_3000, Function | MediumTest | Level3)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    int32_t time;
    int32_t time_before;
    int32_t time_after;
    int32_t durationTime;
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));

    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));

    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->SetLooping(true));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    (void)sleep(PLAYING_TIME);
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time_before));
    (void)sleep(PLAYING_TIME);
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time_after));
    EXPECT_NEAR(time_after - time_before, PLAYING_TIME * TIME_SEC2MS, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Stop());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_2_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_2_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

