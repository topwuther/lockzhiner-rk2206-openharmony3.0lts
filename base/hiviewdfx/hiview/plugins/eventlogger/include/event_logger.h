/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H
#define HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H

#include <ctime>
#include <map>
#include <memory>
#include <string>

#include "event.h"
#include "event_loop.h"
#include "log_store_ex.h"
#include "logger.h"
#include "plugin.h"
#include "sys_event.h"
namespace OHOS {
namespace HiviewDFX {
struct BinderInfo {
    int client;
    int server;
    unsigned long wait;
};

class EventLogger : public Plugin {
public:
    EventLogger() : logStore_(std::make_unique<LogStoreEx>(LOGGER_FAULT_LOG_PATH, true)),
        startTime_(time(nullptr)) {};
    ~EventLogger() {};
    bool OnEvent(std::shared_ptr<Event> &event) override;
    void OnLoad() override;
    void OnUnload() override;
    bool CanProcessEvent(std::shared_ptr<Event> event) override;
private:
    static const inline std::string LOGGER_FAULT_LOG_PATH = "/data/log/eventlog";
    static constexpr int EVENT_MAX_ID = 1000000;
    static constexpr int MAX_FILE_NUM = 500;
    static constexpr int MAX_FOLDER_SIZE = 50 * 1024 * 1024;

    std::unique_ptr<LogStoreEx> logStore_;
    uint64_t startTime_;
    std::map<std::string, std::time_t> eventTagTime_;

    void StartLogCollect(std::shared_ptr<SysEvent> event);
    bool JudgmentRateLimiting(std::shared_ptr<SysEvent> event);
    std::string GetFormatTime(unsigned long timestamp) const;
    bool WriteCommonHead(int fd, std::shared_ptr<SysEvent> event);
    bool PostEvent(std::shared_ptr<SysEvent> event);
    bool UpdateDB(std::shared_ptr<SysEvent> event, std::string logFile);
    bool NeedNoAction(std::shared_ptr<SysEvent> event);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIVIEW_PLUGIN_EVENT_LOG_COLLECTOR_H
