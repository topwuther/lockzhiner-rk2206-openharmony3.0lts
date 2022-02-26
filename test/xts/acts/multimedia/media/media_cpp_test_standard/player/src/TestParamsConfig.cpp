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

#include "TestParamsConfig.h"
#include "parameter.h"
#include "media_errors.h"

using namespace OHOS;
using namespace OHOS::Media;
using namespace TestPlayerBasic;

namespace TestPlayerBasic {
TestParamsConfig::TestParamsConfig()
{
}
TestParamsConfig::~TestParamsConfig()
{
}
std::string TestParamsConfig::GetUri()
{
    char path[PARA_MAX_LEN] = "/data/media/1h264_320x240_60.3gp";
    GetParameter("sys.media.test.stream.path", "/data/media/1h264_320x240_60.3gp", &path[0], PARA_MAX_LEN);
    MEDIA_INFO_LOG("PATH : %s", path);
    return path;
}
void TestParamsConfig::InitPlayProtocol()
{
    MEDIA_INFO_LOG("%s", __FUNCTION__);
    char protocol[PARA_MAX_LEN] = "local";
    GetParameter("sys.media.test.play.protocol", "local", &protocol[0], PARA_MAX_LEN);
    if (strcmp(protocol, "local") == 0) {
        TestPlayerBasic::TestParamsConfig::GetInstance().SetMountPath("file://data/media/");

        MEDIA_INFO_LOG("MOUNT_PATH %s", TestPlayerBasic::TestParamsConfig::GetInstance().GetMountPath().c_str());
    }
}

bool TestParamsConfig::CompareTime(int32_t expectTime, int32_t realTime, OHOS::Media::PlayerSeekMode seekMode)
{
    MEDIA_INFO_LOG("CompareTime: expectTime %d, realTime %d", expectTime, realTime);
    if (seekMode  == OHOS::Media::PlayerSeekMode::SEEK_CLOSEST) {
        if (std::abs(expectTime - realTime) < CLOSEST_DELTA_TIME) {
            return true;
        } else {
            return false;
        }
    }
    if (std::abs(expectTime - realTime) < DELTA_TIME) {
        return true;
    }
    return false;
}
}

