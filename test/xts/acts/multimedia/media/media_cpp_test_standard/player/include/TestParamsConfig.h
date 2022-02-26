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

#ifndef PLAYER_COMMON_TEST_H
#define PLAYER_COMMON_TEST_H

#include "gtest/gtest.h"
#include "mediatest_log.h"
#include "player.h"

namespace {
    static const int32_t PLAYING_TIME = 3;
    static const int32_t SEEK_TIME_5_SEC = 5000;
    static const int32_t SEEK_TIME_2_SEC = 2000;
    static const int32_t DELTA_TIME = 1000;
    static const int32_t CLOSEST_DELTA_TIME = 300;
    static const int32_t WAITSECOND = 2;
    static const int32_t RET_OK = 0;
    static const int32_t FILE_BEGIN = 0;
    static const int32_t PARA_MAX_LEN = 256;
    static const int32_t TIME_SEC2MS = 1000;
    static const int32_t WAIT_TIME = 1000;
    static const OHOS::Media::PlayerSeekMode SEEK_MODE = OHOS::Media::PlayerSeekMode::SEEK_CLOSEST;
}
namespace TestPlayerBasic {
class TestParamsConfig {
public:
    static TestParamsConfig &GetInstance()
    {
        static TestParamsConfig config;
        return config;
    }

    std::string GetMountPath() const
    {
        return mountPath_;
    }

    void SetMountPath(std::string mountPath)
    {
        mountPath_ = mountPath;
    }
    bool CompareTime(int32_t expectTime, int32_t realTme, OHOS::Media::PlayerSeekMode seekMode);
    std::string GetUri();
    void InitPlayProtocol();

private:
    TestParamsConfig();
    ~TestParamsConfig();
    std::string mountPath_ = "";
};
}

#endif