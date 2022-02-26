/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef DEVSVC_MANAGER_CLNT_H
#define DEVSVC_MANAGER_CLNT_H

#include "devsvc_manager_if.h"

struct DevSvcManagerClnt {
    struct IDevSvcManager *devSvcMgrIf;
};

struct DevSvcManagerClnt *DevSvcManagerClntGetInstance(void);
struct HdfDeviceObject *DevSvcManagerClntGetDeviceObject(const char *svcName);
int DevSvcManagerClntAddService(const char *svcName, struct HdfDeviceObject *service);
void DevSvcManagerClntRemoveService(const char *svcName);
int DevSvcManagerClntSubscribeService(const char *svcName, struct SubscriberCallback callback);
int DevSvcManagerClntUnsubscribeService(const char *svcName);
struct HdfDeviceObject *HdfRegisterDevice(const char *moduleName, const char *serviceName, const void *privateData);
void HdfUnregisterDevice(const char *moduleName, const char *serviceName);

#endif /* DEVSVC_MANAGER_CLNT_H */
