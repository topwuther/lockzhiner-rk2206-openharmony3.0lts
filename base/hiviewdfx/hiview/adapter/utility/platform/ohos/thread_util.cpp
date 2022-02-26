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
#include "thread_util.h"

#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>

namespace OHOS {
namespace HiviewDFX {
namespace Thread {
pid_t GetTid()
{
    return gettid();
}

void SetThreadDescription(const std::string &name)
{
    prctl(PR_SET_NAME, name.c_str());
}
}
}
}