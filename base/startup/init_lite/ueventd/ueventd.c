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

#include "ueventd.h"
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ueventd_device_handler.h"
#include "ueventd_firmware_handler.h"
#include "ueventd_read_cfg.h"
#include "ueventd_socket.h"
#include "ueventd_utils.h"
#include "securec.h"
#define INIT_LOG_TAG "ueventd"
#include "init_log.h"

// buffer size refer to kernel kobject uevent
#define UEVENT_BUFFER_SIZE (2048 + 1)

static const char *actions[] = {
    [ACTION_ADD] = "add",
    [ACTION_REMOVE] = "remove",
    [ACTION_CHANGE] = "change",
    [ACTION_MOVE] = "move",
    [ACTION_ONLINE] = "online",
    [ACTION_OFFLINE] = "offline",
    [ACTION_BIND] = "bind",
    [ACTION_UNBIND] = "unbind",
    [ACTION_UNKNOWN] = "unknow",
};

static SUBSYSTEMTYPE GetSubsystemType(const char *subsystem)
{
    if (subsystem == NULL || *subsystem == '\0') {
        return SUBSYSTEM_EMPTY;
    }

    if (strcmp(subsystem, "block") == 0) {
        return SUBSYSTEM_BLOCK;
    } else if (strcmp(subsystem, "platform") == 0) {
        return SUBSYSTEM_PLATFORM;
    } else if (strcmp(subsystem, "firmware") == 0) {
        return SUBSYSTEM_FIRMWARE;
    } else {
        return SUBSYSTEM_OTHERS;
    }
}

const char *ActionString(ACTION action)
{
    return actions[action];
}

static ACTION GetUeventAction(const char *action)
{
    if (action == NULL || *action == '\0') {
        return ACTION_UNKNOWN;
    }

    if (STRINGEQUAL(action, "add")) {
        return ACTION_ADD;
    } else if (STRINGEQUAL(action, "remove")) {
        return ACTION_REMOVE;
    } else if (STRINGEQUAL(action, "change")) {
        return ACTION_CHANGE;
    } else if (STRINGEQUAL(action, "move")) {
        return ACTION_MOVE;
    } else if (STRINGEQUAL(action, "online")) {
        return ACTION_ONLINE;
    } else if (STRINGEQUAL(action, "offline")) {
        return ACTION_OFFLINE;
    } else if (STRINGEQUAL(action, "bind")) {
        return ACTION_BIND;
    } else if (STRINGEQUAL(action, "unbind")) {
        return ACTION_UNBIND;
    } else {
        return ACTION_UNKNOWN;
    }
}

static void HandleUevent(const struct Uevent *uevent)
{
    if (uevent->action == ACTION_ADD || uevent->action == ACTION_CHANGE || uevent->action == ACTION_ONLINE) {
        ChangeSysAttributePermissions(uevent->syspath);
    }

    SUBSYSTEMTYPE type = GetSubsystemType(uevent->subsystem);
    switch (type) {
        case SUBSYSTEM_BLOCK:
            HandleBlockDeviceEvent(uevent);
            break;
        case SUBSYSTEM_FIRMWARE:
            HandleFimwareDeviceEvent(uevent);
            break;
        case SUBSYSTEM_OTHERS:
            HandleOtherDeviceEvent(uevent);
            break;
        default:
            break;
    }
}

static void AddUevent(struct Uevent *uevent, const char *event, size_t len)
{
    if (uevent == NULL || event == NULL || len == 0) {
        return;
    }

    if (STARTSWITH(event, "DEVPATH=")) {
        uevent->syspath = event + strlen("DEVPATH=");
    } else if (STARTSWITH(event, "SUBSYSTEM=")) {
        uevent->subsystem = event + strlen("SUBSYSTEM=");
    } else if (STARTSWITH(event, "ACTION=")) {
        uevent->action = GetUeventAction(event + strlen("ACTION="));
    } else if (STARTSWITH(event, "DEVNAME=")) {
        uevent->deviceName = event + strlen("DEVNAME=");
    } else if (STARTSWITH(event, "PARTNAME=")) {
        uevent->partitionName = event + strlen("PARTNAME=");
    } else if (STARTSWITH(event, "PARTN=")) {
        uevent->partitionNum = StringToInt(event + strlen("PARTN="), -1);
    } else if (STARTSWITH(event, "MAJOR=")) {
        uevent->major = StringToInt(event + strlen("MAJOR="), -1);
    } else if (STARTSWITH(event, "MINOR=")) {
        uevent->minor = StringToInt(event + strlen("MINOR="), -1);
    } else if (STARTSWITH(event, "DEVUID")) {
        uevent->ug.uid = (uid_t)StringToInt(event + strlen("DEVUID="), 0);
    } else if (STARTSWITH(event, "DEVGID")) {
        uevent->ug.gid = (gid_t)StringToInt(event + strlen("DEVGID="), 0);
    } else if (STARTSWITH(event, "FIRMWARE=")) {
        uevent->firmware = event + strlen("FIRMWARE=");
    } else if (STARTSWITH(event, "BUSNUM=")) {
        uevent->busNum = StringToInt(event + strlen("BUSNUM="), -1);
    } else if (STARTSWITH(event, "DEVNUM=")) {
        uevent->devNum = StringToInt(event + strlen("DEVNUM="), -1);
    }
    // Ignore other events
}

static void ParseUeventMessage(const char *buffer, ssize_t length, struct Uevent *uevent)
{
    if (buffer == NULL || uevent == NULL || length == 0) {
        // Ignore invalid buffer
        return;
    }

    // reset parititon number, major and minor.
    uevent->partitionNum = -1;
    uevent->major = -1;
    uevent->minor = -1;
    uevent->busNum = -1;
    uevent->devNum = -1;
    ssize_t pos = 0;
    while (pos < length) {
        const char *event = buffer + pos;
        size_t len = strlen(event);
        if (len == 0) {
            break;
        }
        AddUevent(uevent, event, len);
        pos += (ssize_t)len + 1;
    }
}

static void ProcessUevent(int sockFd)
{
    // One more bytes for '\0'
    char ueventBuffer[UEVENT_BUFFER_SIZE] = {};
    ssize_t n = 0;
    struct Uevent uevent = {};
    while ((n = ReadUeventMessage(sockFd, ueventBuffer, sizeof(ueventBuffer) - 1)) > 0) {
        ParseUeventMessage(ueventBuffer, n, &uevent);
        if (uevent.syspath == NULL) {
            INIT_LOGD("Ignore unexpected uevent");
            return;
        }
        HandleUevent(&uevent);
    }
}

static int g_triggerDone = 0;
static void DoTrigger(const char *ueventPath, int sockFd)
{
    if (ueventPath == NULL || ueventPath[0] == '\0') {
        return;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(ueventPath, realPath) == NULL) {
        if (errno != ENOENT) {
            INIT_LOGE("Fail resolve %s real path err=%d", ueventPath, errno);
            return;
        }
    }
    int fd = open(realPath, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        INIT_LOGE("Open \" %s \" failed, err = %d", realPath, errno);
    } else {
        ssize_t n = write(fd, "add\n", strlen("add\n"));
        if (n < 0) {
            INIT_LOGE("Write \" %s \" failed, err = %d", realPath, errno);
            close(fd);
        } else {
            close(fd);
            // uevent triggered, now handle it.
            if (sockFd >= 0) {
                ProcessUevent(sockFd);
            }
        }
    }
}

static void Trigger(const char *path, int sockFd)
{
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return;
    }
    struct dirent *dirent = NULL;
    while ((dirent = readdir(dir)) != NULL) {
        if (dirent->d_name[0] == '.') {
            continue;
        }
        if (dirent->d_type == DT_DIR) {
            char pathBuffer[PATH_MAX];
            if (snprintf_s(pathBuffer, PATH_MAX, PATH_MAX - 1, "%s/%s", path, dirent->d_name) == -1) {
                continue;
            }
            Trigger(pathBuffer, sockFd);
        } else {
            if (strcmp(dirent->d_name, "uevent") != 0) {
                continue;
            }
            char ueventBuffer[PATH_MAX];
            if (snprintf_s(ueventBuffer, PATH_MAX, PATH_MAX - 1, "%s/%s", path, "uevent") == -1) {
                INIT_LOGW("Cannnot build uevent path under %s", path);
                continue;
            }
            DoTrigger(ueventBuffer, sockFd);
        }
    }
    closedir(dir);
}

static void RetriggerUevent(int sockFd)
{
    if (!g_triggerDone) {
        Trigger("/sys/block", sockFd);
        Trigger("/sys/class", sockFd);
        Trigger("/sys/devices", sockFd);
        g_triggerDone = 1;
    }
}

int main(int argc, char **argv)
{
    char *ueventdConfigs[] = {"/etc/ueventd.config", NULL};
    int i = 0;
    while (ueventdConfigs[i] != NULL) {
        ParseUeventdConfigFile(ueventdConfigs[i++]);
    }
    int ueventSockFd = UeventdSocketInit();
    if (ueventSockFd < 0) {
        INIT_LOGE("Failed to create uevent socket");
        return -1;
    }

    RetriggerUevent(ueventSockFd);
    struct pollfd pfd = {};
    pfd.events = POLLIN;
    pfd.fd = ueventSockFd;

    while (1) {
        pfd.revents = 0;
        int ret = poll(&pfd, 1, -1);
        if (ret <= 0) {
            continue;
        }
        if (pfd.revents & POLLIN) {
            ProcessUevent(ueventSockFd);
        }
    }
    return 0;
}
