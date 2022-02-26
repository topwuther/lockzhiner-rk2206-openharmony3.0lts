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
#include "sys_event_db_mgr.h"

#include <cinttypes>
#include <ctime>
#include <string>

#include "file_util.h"
#include "logger.h"
#include "sys_event.h"
#include "sys_event_dao.h"
namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_TAG("HiView-SysEventDbMgr");
#ifdef CUSTOM_MAX_FILE_SIZE_MB
const int MAX_FILE_SIZE = 1024 * 1204 * CUSTOM_MAX_FILE_SIZE_MB;
#else
const int MAX_FILE_SIZE = 1024 * 1204 * 100; // 100M
#endif // CUSTOM_MAX_FILE_SIZE
const int ONE_DAY_MILLISECONDS = 3600 * 24 * 1000;
const int MILLISECONDS = 1000;
using EventStore::SysEventDao;
using EventStore::SysEventQuery;
using EventStore::EventCol;
using EventStore::Op;
using EventStore::ResultSet;
static int64_t Get0ClockStampMs()
{
    time_t now = std::time(nullptr);
    int64_t zero = now;
    struct tm *l = std::localtime(&now);
    if (l != nullptr) {
        l->tm_hour = 0;
        l->tm_min = 0;
        l->tm_sec = 0;
        zero = std::mktime(l) * MILLISECONDS;  // time is 00:00:00
    }
    return zero;
}

void SysEventDbMgr::SaveToStore(std::shared_ptr<SysEvent> sysevent) const
{
    SysEventDao::Insert(sysevent);
    HIVIEW_LOGD("save sys event %{public}" PRId64 ", %{public}s", sysevent->GetSeq(), sysevent->eventName_.c_str());
}

void SysEventDbMgr::StartCheckStoreTask(std::shared_ptr<EventLoop> looper)
{
    if (looper == nullptr) {
        HIVIEW_LOGE("can not init check store task as looper null");
        return;
    }
    HIVIEW_LOGI("init check store task");
    auto statusTask = std::bind(&SysEventDbMgr::CheckStore, this);
    int delay = 60 * 10; // ten minute
    looper->AddTimerEvent(nullptr, nullptr, statusTask, delay, true);
}

void SysEventDbMgr::CheckStore()
{
    std::string dbFile = SysEventDao::GetDataFile();
    if (FileUtil::GetFileSize(dbFile) < MAX_FILE_SIZE) {
        HIVIEW_LOGD("does not need to clean db file");
        return;
    }
    int count = 0;
    int64_t zeroClock = Get0ClockStampMs();
    while (true) {
        count++;
        zeroClock = zeroClock - ONE_DAY_MILLISECONDS;
        SysEventQuery sysEventQuery = SysEventDao::BuildQuery();
        ResultSet result = sysEventQuery.Where(EventCol::TS, Op::LT, zeroClock).Execute(1);
        if (!result.HasNext()) {
            break;
        }
        if (count > 7) { // 7 days of week
            break;
        }
    }
    zeroClock = zeroClock + ONE_DAY_MILLISECONDS;
    SysEventQuery sysEventQuery = SysEventDao::BuildQuery();
    sysEventQuery.Where(EventCol::TS, Op::LT, zeroClock);
    int retCode = SysEventDao::Delete(sysEventQuery);
    if (retCode != 0) {
        HIVIEW_LOGI("clean the db file error at timestamp %{public}" PRId64 "", zeroClock);
        return;
    }
    HIVIEW_LOGI("clean the db file at timestamp %{public}" PRId64 " successful", zeroClock);
}
} // namespace HiviewDFX
} // namespace OHOS