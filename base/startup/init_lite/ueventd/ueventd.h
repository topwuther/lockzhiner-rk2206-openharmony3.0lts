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

#ifndef BASE_STARTUP_INITLITE_UEVENTD_H
#define BASE_STARTUP_INITLITE_UEVENTD_H
#include <unistd.h>

// Refer to linux kernel kobject.h
typedef enum ACTION {
    ACTION_ADD,
    ACTION_REMOVE,
    ACTION_CHANGE,
    ACTION_MOVE,
    ACTION_ONLINE,
    ACTION_OFFLINE,
    ACTION_BIND,
    ACTION_UNBIND,
    ACTION_UNKNOWN,
} ACTION;

struct UidGid {
    uid_t uid;
    gid_t gid;
};

struct Uevent {
    const char *subsystem;
    const char *syspath;
    // DEVNAME may has slash
    const char *deviceName;
    const char *partitionName;
    const char *firmware;
    ACTION action;
    int partitionNum;
    int major;
    int minor;
    struct UidGid ug;
    // for usb device.
    int busNum;
    int devNum;
};

typedef enum SUBYSTEM {
    SUBSYSTEM_EMPTY = -1,
    SUBSYSTEM_BLOCK = 0,
    SUBSYSTEM_PLATFORM = 1,
    SUBSYSTEM_FIRMWARE = 2,
    SUBSYSTEM_OTHERS = 3,
} SUBSYSTEMTYPE;

const char *ActionString(ACTION action);
#endif // BASE_STARTUP_INITLITE_UEVENTD_H