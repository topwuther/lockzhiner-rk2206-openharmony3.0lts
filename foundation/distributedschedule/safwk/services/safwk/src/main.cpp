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

#include <sys/prctl.h>

#include "ipc_skeleton.h"
#include "local_ability_manager.h"
#include "safwk_log.h"
#include "securec.h"
#include "string_ex.h"

using namespace OHOS;
using std::string;

namespace {
const string TAG = "SaMain";

using ProcessNameSetFunc = std::function<void(const string&)>;

constexpr auto DEFAULT_XML = "/system/usr/default.xml";
// The pid name can be up to 16 bytes long, including the terminating null byte.
// So need to set the max length of pid name to 15 bytes.
constexpr size_t MAX_LEN_PID_NAME = 15;
}

static void SetProcName(const string& filePath, const ProcessNameSetFunc& setProcessName)
{
    std::vector<string> strVector;
    SplitStr(filePath, "/", strVector);
    auto vectorSize = strVector.size();
    if (vectorSize > 0) {
        auto& fileName = strVector[vectorSize - 1];
        auto dotPos = fileName.find(".");
        if (dotPos == string::npos) {
            return;
        }
        if (dotPos > MAX_LEN_PID_NAME) {
            dotPos = MAX_LEN_PID_NAME;
        }
        string profileName = fileName.substr(0, dotPos);
        int32_t ret = prctl(PR_SET_NAME, profileName.c_str());
        if (ret != 0) {
            HILOGI(TAG, "call the system API prctl failed!");
        }
        setProcessName(profileName);
    }
}

int main(int argc, char *argv[])
{
    HILOGI(TAG, "safwk main entry");

    auto setProcessName = [argc, argv](const string& name) -> void {
        uintptr_t start = reinterpret_cast<uintptr_t>(argv[0]);
        uintptr_t end = reinterpret_cast<uintptr_t>(strchr(argv[argc - 1], 0));
        uintptr_t argvSize = end - start;

        if (memset_s(argv[0], argvSize, 0, argvSize) != EOK) {
            HILOGW(TAG, "failed to clear argv:%s", strerror(errno));
            return;
        }
        if (strcpy_s(argv[0], argvSize, name.c_str()) != EOK) {
            HILOGW(TAG, "failed to set process name:%s", strerror(errno));
            return;
        }
        HILOGI(TAG, "Set process name to %s", argv[0]);
    };

    // Load system abilities related shared libraries from specific xml-format profile,
    // when this process starts.
    string profilePath(DEFAULT_XML);
    if (argc > 1) {
        string filePath(argv[1]);
        if (filePath.empty() || filePath.find(".xml") == string::npos) {
            HILOGE(TAG, "profile file path is invalid!");
            return 0;
        }
        SetProcName(filePath, setProcessName);
        profilePath = std::move(filePath);
    }

    LocalAbilityManager::GetInstance().DoStartSAProcess(profilePath);
    return 0;
}
