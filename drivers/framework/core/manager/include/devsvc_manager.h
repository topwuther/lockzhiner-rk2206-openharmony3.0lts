/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef DEVICE_SERVICE_MANAGER_H
#define DEVICE_SERVICE_MANAGER_H

#include "devsvc_manager_if.h"
#include "hdf_service_observer.h"
#include "hdf_slist.h"
#include "osal_mutex.h"

struct DevSvcManager {
    struct IDevSvcManager super;
    struct HdfSList services;
    struct HdfServiceObserver observer;
    struct OsalMutex mutex;
};

struct HdfObject *DevSvcManagerCreate(void);
bool DevSvcManagerConstruct(struct DevSvcManager *inst);
void DevSvcManagerRelease(struct HdfObject *object);
struct IDevSvcManager *DevSvcManagerGetInstance(void);
int DevSvcManagerAddService(struct IDevSvcManager *manager, const char *svcName, struct HdfDeviceObject *service);
struct HdfObject *DevSvcManagerGetService(struct IDevSvcManager *manager, const char *svcName);
void DevSvcManagerRemoveService(struct IDevSvcManager *manager, const char *svcName);

int DevSvcManagerClntSubscribeService(const char *svcName, struct SubscriberCallback callback);
int DevSvcManagerClntUnsubscribeService(const char *svcName);

#endif /* DEVICE_SERVICE_MANAGER_H */
