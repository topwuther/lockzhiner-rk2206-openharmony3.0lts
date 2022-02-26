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

#ifndef DISTRIBUTEDDATAFWK_SRC_PIPE_HANDLER_H
#define DISTRIBUTEDDATAFWK_SRC_PIPE_HANDLER_H

#include <set>
#include <mutex>
#include <string>
#include <map>
#include "app_types.h"
#include "kv_scheduler.h"
#include "log_print.h"
#include "reporter.h"
#include "app_data_change_listener.h"
#include "softbus_adapter.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "AppPipeHandler"

namespace OHOS {
namespace AppDistributedKv {
class AppPipeHandler {
public:
    ~AppPipeHandler();
    explicit AppPipeHandler(const PipeInfo &pipeInfo);

    // add DataChangeListener to watch data change;
    Status StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);
    // stop DataChangeListener to watch data change;
    Status StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo);
    // Send data to other device, function will be called back after sent to notify send result.
    Status SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr, int size,
                    const MessageInfo &info);
    bool IsSameStartedOnPeer(const struct PipeInfo &pipeInfo, const struct DeviceId &peer);

    void SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag);

    int CreateSessionServer(const std::string &sessionName) const;

    int RemoveSessionServer(const std::string &sessionName) const;
private:
    PipeInfo pipeInfo_;
    std::shared_ptr<SoftBusAdapter> softbusAdapter_ {};
};
}  // namespace AppDistributedKv
}  // namespace OHOS
#undef LOG_TAG
#endif /* DISTRIBUTEDDATAFWK_SRC_PIPE_HANDLER_H */
