/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef DEVSVC_RECORD_H
#define DEVSVC_RECORD_H

#include "hdf_object.h"
#include "hdf_slist.h"

struct DevSvcRecord {
    struct HdfSListNode entry;
    uint32_t key;
    struct HdfDeviceObject *value;
};

struct DevSvcRecord *DevSvcRecordNewInstance(void);
void DevSvcRecordFreeInstance(struct DevSvcRecord *inst);
void DevSvcRecordDelete(struct HdfSListNode *listEntry);

#endif /* DEVSVC_RECORD_H */

