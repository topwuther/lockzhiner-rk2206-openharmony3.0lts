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
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0100
    * @tc.name      : 001.进行播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0100, Function | MediumTest | Level0)
{
    std::shared_ptr<PlayerSignal> testObj = std::make_shared<PlayerSignal>();
    std::shared_ptr<TestPlayer> player = std::make_shared<TestPlayer>(testObj);
    ASSERT_NE(nullptr, player);
    ASSERT_EQ(true, player->CreatePlayer());
    std::string uri = TestParamsConfig::GetInstance().GetUri();
    ASSERT_EQ(RET_OK, player->SetSource(uri));
    std::shared_ptr<TestPlayerCallback> testCallback = std::make_shared<TestPlayerCallback>(testObj);
    EXPECT_EQ(RET_OK, player->SetPlayerCallback(testCallback));
    sptr<Surface> videoSurface = player->GetVideoSurface(g_sub_config);
    EXPECT_EQ(RET_OK, player->SetVideoSurface(videoSurface));
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, player->Release());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0200
    * @tc.name      : 002.进行暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0200, Function | MediumTest | Level0)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0300
    * @tc.name      : 003.进行恢复播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0300, Function | MediumTest | Level0)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0400
    * @tc.name      : 004.进行停止播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0400, Function | MediumTest | Level0)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0500
    * @tc.name      : 005.进行结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0500, Function | MediumTest | Level0)
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

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0600
    * @tc.name      : 006.暂停后恢复播放，再次暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0600, Function | MediumTest | Level1)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0700
    * @tc.name      : 007.暂停后结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0700, Function | MediumTest | Level1)
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
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0800
    * @tc.name      : 008.暂停后恢复播放，再结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0800, Function | MediumTest | Level1)
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
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_0900
    * @tc.name      : 009.停止播放后重新开始播放，暂停后恢复播放，再结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_0900, Function | MediumTest | Level2)
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
    EXPECT_FALSE(player->IsPlaying());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1000
    * @tc.name      : 010.停止播放后重新开始播放，暂停后结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1000, Function | MediumTest | Level2)
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
    EXPECT_FALSE(player->IsPlaying());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1100
    * @tc.name      : 011.停止播放后重新开始播放，再次结束播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1100, Function | MediumTest | Level2)
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
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}


/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1300
    * @tc.name      : 013.停止播放后暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1300, Function | MediumTest | Level1)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_NE(RET_OK, player->Pause());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1400
    * @tc.name      : 014.开始播放，进行Seek，再暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1400, Function | MediumTest | Level1)
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

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1500
    * @tc.name      : 015.开始播放，暂停后进行Seek，再恢复播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1500, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1600
    * @tc.name      : 016.开始播放，暂停后恢复播放，进行Seek，再暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1600, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    for (int32_t i = 1; i < 4; i++) {
        EXPECT_EQ(RET_OK, player->Seek(i * TIME_SEC2MS, SEEK_MODE));
        EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
        EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(i * TIME_SEC2MS, time, SEEK_MODE));
    }
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1700
    * @tc.name      : 017.开始播放，进行Seek
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1700, Function | MediumTest | Level0)
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

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1800
    * @tc.name      : 018.开始播放，进行Seek，停止播放，
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1800, Function | MediumTest | Level1)
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

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_1900
    * @tc.name      : 019.开始播放，停止播放，进行Seek
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_1900, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2000
    * @tc.name      : 020.开始播放，暂停后进行Seek
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2000, Function | MediumTest | Level1)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    for (int32_t i = 0; i < durationTime; i = i + (durationTime / 3) + 1) {
        EXPECT_EQ(RET_OK, player->Seek(i, SEEK_MODE));
        EXPECT_FALSE(player->IsPlaying());
        EXPECT_EQ(RET_OK, player->Play());
        EXPECT_TRUE(player->IsPlaying());
        EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
        EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(i, time, SEEK_MODE));
        EXPECT_EQ(RET_OK, player->Pause());
    }
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2100
    * @tc.name      : 021.开始播放，暂停后进行Seek，停止播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2100, Function | MediumTest | Level2)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    for (int32_t i = 0; i < durationTime; i = i + (durationTime / 3) + 1) {
        EXPECT_EQ(RET_OK, player->Seek(i, SEEK_MODE));
        EXPECT_FALSE(player->IsPlaying());
        EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
        EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(i, time, SEEK_MODE));
    }
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2200
    * @tc.name      : 022.开始播放，暂停后恢复播放，进行Seek
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2200, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_TRUE(player->IsPlaying());
    (void)sleep(PLAYING_TIME);
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_NEAR(PLAYING_TIME * TIME_SEC2MS  + SEEK_TIME_5_SEC, time, DELTA_TIME);
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2300
    * @tc.name      : 023.开始播放，暂停后恢复播放，进行Seek，停止播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2300, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(SEEK_TIME_5_SEC, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2400
    * @tc.name      : 024.开始播放，停止播放，进行Seek，重新播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2400, Function | MediumTest | Level2)
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

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_NE(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2500
    * @tc.name      : 025.进行Seek，Seek到文件开始的位置
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2500, Function | MediumTest | Level1)
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
    EXPECT_EQ(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Seek(FILE_BEGIN, SEEK_MODE));
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(FILE_BEGIN, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2600
    * @tc.name      : 026.开始播放，停止播放，进行Seek,再暂停
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2600, Function | MediumTest | Level3)
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

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_NE(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    EXPECT_NE(RET_OK, player->Pause());

    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2700
    * @tc.name      : 027.开始播放，停止播放，进行Seek，再进行恢复播放操作
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2700, Function | MediumTest | Level3)
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

    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_NE(RET_OK, player->Seek(SEEK_TIME_5_SEC, SEEK_MODE));

    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_EQ(FILE_BEGIN, time);
    ASSERT_EQ(RET_OK, player->PrepareAsync());
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());

    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2800
    * @tc.name      : 028.进行Seek，Seek到文件结尾的位置
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2800, Function | MediumTest | Level1)
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
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_2900
    * @tc.name      : 029.进行Seek，Seek到超过文件结尾的位置
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_2900, Function | MediumTest | Level3)
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
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_3000
    * @tc.name      : 030.进行Seek，Seek到文件随机的位置
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_3000, Function | MediumTest | Level1)
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
    EXPECT_EQ(RET_OK, player->Seek(durationTime / 3, SEEK_MODE));
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime / 3, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_3100
    * @tc.name      : 031.进行Seek，seek后开始进行基本播控操作
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_3100, Function | MediumTest | Level2)
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
    for (int32_t i = 0; i < durationTime; i = i + (durationTime / 3) + 1) {
        EXPECT_EQ(RET_OK, player->Seek(i, SEEK_MODE));
        EXPECT_TRUE(player->IsPlaying());
        EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
        EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(i, time, SEEK_MODE));
    }
    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Stop());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_3200
    * @tc.name      : 032.暂停时Seek到文件开始，恢复播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_3200, Function | MediumTest | Level1)
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

    EXPECT_EQ(RET_OK, player->Pause());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Seek(FILE_BEGIN, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(FILE_BEGIN, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    (void)sleep(PLAYING_TIME);
    EXPECT_TRUE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_3300
    * @tc.name      : 033.暂停时Seek到文件结尾，恢复播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_3300, Function | MediumTest | Level2)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime, SEEK_MODE));
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    (void)sleep(PLAYING_TIME);

    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}

/**
    * @tc.number    : SUB_MEDIA_PLAYER_Local_Async_Function_03_3400
    * @tc.name      : 034.暂停时Seek到超过文件结尾的位置，恢复播放
    * @tc.desc      : 播控
*/
HWTEST_F(ActsPlayerFuncTest, SUB_MEDIA_PLAYER_Local_Async_Function_03_3400, Function | MediumTest | Level3)
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
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetDuration(durationTime));
    EXPECT_EQ(RET_OK, player->Seek(durationTime + 1, SEEK_MODE));

    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, player->GetCurrentTime(time));
    EXPECT_TRUE(TestParamsConfig::GetInstance().CompareTime(durationTime, time, SEEK_MODE));
    EXPECT_EQ(RET_OK, player->Play());
    EXPECT_EQ(RET_OK, testCallback->WaitForState(PLAYER_PLAYBACK_COMPLETE));
    EXPECT_EQ(RET_OK, player->Reset());
    EXPECT_FALSE(player->IsPlaying());
    EXPECT_EQ(RET_OK, testCallback->errorNum_);
}