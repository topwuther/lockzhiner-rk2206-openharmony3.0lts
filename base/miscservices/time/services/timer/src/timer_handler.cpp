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

#include <sys/epoll.h>
#include <iostream>
#include <cstring>
#include <sys/timerfd.h>
#include <unistd.h>
#include "timer_handler.h"
#include <linux/rtc.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/time.h>
#include <sstream>
#include <fstream>

namespace OHOS {
namespace MiscServices {
namespace {
static const uint32_t ALARM_TIME_CHANGE_MASK = 1 << 16;
static const clockid_t alarm_to_clock_id[N_TIMER_FDS] = {
    CLOCK_REALTIME_ALARM,
    CLOCK_REALTIME,
    CLOCK_BOOTTIME_ALARM,
    CLOCK_BOOTTIME,
    CLOCK_MONOTONIC,
    CLOCK_REALTIME,
};
}
std::shared_ptr<TimerHandler> TimerHandler::Create()
{
    int epollfd;
    TimerFds fds;

    epollfd = epoll_create(fds.size());
    if (epollfd < 0) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "epoll_create %{public}d failed: %{public}s",
                static_cast<int>(fds.size()), strerror(errno));
            return nullptr;
    }

    for (size_t i = 0; i < fds.size(); i++) {
        fds[i] = timerfd_create(alarm_to_clock_id[i], 0);
        if (fds[i] < 0) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "timerfd_create %{public}d  failed: %{public}s",
                static_cast<int>(i), strerror(errno));
            close(epollfd);
            for (size_t j = 0; j < i; j++) {
                close(fds[j]);
            }
            return nullptr;
        }
    }

    std::shared_ptr<TimerHandler> handler = std::shared_ptr<TimerHandler>(new TimerHandler(fds, epollfd));
    for (size_t i = 0; i < fds.size(); i++) {
        epoll_event event {};
        event.events = EPOLLIN | EPOLLWAKEUP;
        event.data.u32 = i;

        int err = epoll_ctl(epollfd, EPOLL_CTL_ADD, fds[i], &event);
        if (err < 0) {
            TIME_HILOGE(TIME_MODULE_SERVICE, "epoll_ctl(EPOLL_CTL_ADD) failed: %{public}s", strerror(errno));
            return nullptr;
        }
    }
    itimerspec spec {};

    int err = timerfd_settime(fds[ALARM_TYPE_COUNT], TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &spec, nullptr);
    if (err < 0) {
        TIME_HILOGE(TIME_MODULE_SERVICE, "timerfd_settime() failed: %{public}s", strerror(errno));
        return nullptr;
    }

    return handler;
}

TimerHandler::TimerHandler(const TimerFds &fds, int epollfd)
    : fds_ {fds}, epollFd_ {epollfd}
{
}

TimerHandler::~TimerHandler()
{
    for (auto fd : fds_) {
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
    }
    close(epollFd_);
}

int TimerHandler::Set(uint32_t type, std::chrono::nanoseconds when)
{
    TIME_HILOGI(TIME_MODULE_SERVICE, "type= %{public}d, when= %{public}lld", type, when.count());
    if (static_cast<size_t>(type) > ALARM_TYPE_COUNT) {
        errno = EINVAL;
        return -1;
    }

    auto second = std::chrono::duration_cast<std::chrono::seconds>(when);
    timespec ts {second.count(), (when - second).count()};
    itimerspec spec {timespec {}, ts};
    return timerfd_settime(fds_[type], TFD_TIMER_ABSTIME, &spec, nullptr);
}

uint32_t TimerHandler::WaitForAlarm()
{
    epoll_event events[N_TIMER_FDS];

    int nevents = epoll_wait(epollFd_, events, N_TIMER_FDS, -1);
    if (nevents < 0) {
        return nevents;
    }

    uint32_t result = 0;
    for (int i = 0; i < nevents; i++) {
        uint32_t alarm_idx = events[i].data.u32;
        uint64_t unused;
        ssize_t err = read(fds_[alarm_idx], &unused, sizeof(unused));
        if (err < 0) {
            if (alarm_idx == ALARM_TYPE_COUNT && errno == ECANCELED) {
                result |= ALARM_TIME_CHANGE_MASK;
            } else {
                return err;
            }
        } else {
            result |= (1 << alarm_idx);
        }
    }
    return result;
}
} // MiscServices
} // OHOS