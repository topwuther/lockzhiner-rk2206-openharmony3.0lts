/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef DFX_FAULTLOGGERD_H
#define DFX_FAULTLOGGERD_H

#include <cinttypes>

int32_t StartServer(int argc, char *argv[]);

namespace OHOS {
namespace HiviewDFX {
class FaultLoggerDaemon {
public:
    FaultLoggerDaemon() : currentLogCounts_(0) {};
    ~FaultLoggerDaemon() {};
    bool InitEnvironment();
    void HandleRequest(int32_t fd);

private:
    int32_t CreateFileForRequest(int32_t type, int32_t pid) const;
    void RemoveTempFileIfNeed();

private:
    int32_t currentLogCounts_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif