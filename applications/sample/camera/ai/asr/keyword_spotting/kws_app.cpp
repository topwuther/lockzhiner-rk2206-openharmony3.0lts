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

#include <cstdio>
#include <iostream>
#include <memory>

#include "kws_manager.h"

using namespace std;
using namespace KWS;

static void SampleHelp()
{
    printf("****************************************\n");
    printf("Select the behavior of KWS app.\n");
    printf("s: AudioCap (Press q to quit)\n");
    printf("q: quit the sample.\n");
    printf("****************************************\n");
}

int main()
{
    printf("Keyword spotting started.\n");
    SampleHelp();

    shared_ptr<KwsManager> kwsMgr = make_shared<KwsManager>(AUDIO_SAMPLE_RATE, AUDIO_CODEC_BITRATE);
    if (kwsMgr == nullptr) {
        printf("Keyword spotting failed to allocate KWSManager.\n");
        return -1;
    }
    if (!kwsMgr->Prepare()) {
        printf("Keyword spotting failed to prepare KWSManager.\n");
        return -1;
    }
    char input = ' ';
    while (cin >> input) {
        switch (input) {
            case 's':
                kwsMgr->Start();
                break;
            case 'q':
                kwsMgr->Stop();
                printf("Keyword spotting Terminated.\n");
                return 0;
            default:
                SampleHelp();
                break;
        }
    }
    return 0;
}