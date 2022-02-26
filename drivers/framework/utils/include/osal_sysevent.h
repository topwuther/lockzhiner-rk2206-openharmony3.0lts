/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#ifndef OSAL_SYSEVENT_H
#define OSAL_SYSEVENT_H

#include "hdf_dlist.h"

#define HDF_SYSEVENT 0xFADE

/* hdf sys event class definition */
#define HDF_SYSEVENT_CLASS_POWER 0x00000001

/* hdf power event definition */
enum PowerKeventId {
    KEVENT_POWER_SUSPEND,
    KEVENT_POWER_DISPLAY_OFF,
    KEVENT_POWER_RESUME,
    KEVENT_POWER_DISPLAY_ON,
    KEVENT_POWER_EVENT_MAX,
};

struct HdfSysEvent {
    uint64_t eventClass;
    uint32_t eventid;
    const char *content;
    uint64_t syncToken;
};

struct HdfSysEventNotifyNode;

typedef int (*HdfSysEventNotifierFn)(
    struct HdfSysEventNotifyNode *self, uint64_t eventClass, uint32_t event, const char *content);

struct HdfSysEventNotifyNode {
    HdfSysEventNotifierFn callback;
    struct DListHead listNode;
    uint64_t classFilter;
};

int HdfSysEventNotifyRegister(struct HdfSysEventNotifyNode *notifierNode, uint64_t classSet);
void HdfSysEventNotifyUnregister(struct HdfSysEventNotifyNode *notifierNode);
int HdfSysEventSend(uint64_t eventClass, uint32_t event, const char *content, bool sync);

#endif // #ifndef OSAL_SYSEVENT_H