/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

#ifndef FF_GEN_DRV_H
#define FF_GEN_DRV_H

#include "stdint.h"
#include "diskio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct {
    DSTATUS (*disk_initialize)(BYTE);
    DSTATUS (*disk_status)(BYTE);
    DSTATUS (*disk_read)(BYTE, BYTE *, DWORD, UINT);
    DSTATUS (*disk_write)(BYTE, const BYTE *, DWORD, UINT);
    DSTATUS (*disk_ioctl)(BYTE, BYTE, void *);
} DiskioDrvTypeDef;

typedef struct {
    uint8_t initialized[FF_VOLUMES];
    const DiskioDrvTypeDef *drv[FF_VOLUMES];
    uint8_t lun[FF_VOLUMES];
    volatile uint8_t nbr;
} DiskDrvTypeDef;

extern DiskDrvTypeDef g_diskDrv;
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* FS_CONFIG_H */
