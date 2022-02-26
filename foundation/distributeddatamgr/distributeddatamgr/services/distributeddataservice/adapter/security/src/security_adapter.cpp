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

#include "security_adapter.h"
#include "log_print.h"
#include "security.h"
#include "1.0/dev_slinfo_mgr.h"
#undef LOG_TAG
#define LOG_TAG "SecurityAdapter"

namespace OHOS::DistributedKv {
namespace {
class InstallDevsl {
public:
    InstallDevsl();
    ~InstallDevsl();
    void Initialize();
private:
    std::shared_ptr<Security> security_ = nullptr;
};

InstallDevsl::InstallDevsl()
{
    (void)DEVSL_OnStart(0);
    security_ = std::make_shared<Security>("distributeddata", "default", "/data/misc_de/0/mdds/Meta");
    if (security_ == nullptr) {
        ZLOGD("Security is nullptr.");
        return;
    }

    auto status = DistributedDB::KvStoreDelegateManager::SetProcessSystemAPIAdapter(security_);
    ZLOGD("set distributed db system api adapter: %d.", static_cast<int>(status));
}

InstallDevsl::~InstallDevsl()
{
    DEVSL_ToFinish();
}

void InstallDevsl::Initialize()
{
    return;
}
__attribute__((used)) InstallDevsl g_installDevsl;
}

KVSTORE_API void InitSecurityAdapter()
{
    if (!Security::IsFirstInit()) {
        ZLOGD("Security is already inited.");
        return;
    }

    g_installDevsl.Initialize();
    ZLOGD("Security init finished!");
}
}
