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

#ifndef _LINK_H
#define _LINK_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PART_BLOCK_SIZE         0X200  // 512Byte
#define PART_SYSTEM_ADDR        0x00
#define PART_SYSTEM_BLOCKS      0x80   // 64K
#define PART_LOADER_ADDR        (PART_SYSTEM_ADDR + PART_SYSTEM_BLOCKS * PART_BLOCK_SIZE)
#define PART_LOADER_BLOCKS      0x80   // 64K
#define PART_LITEOS_ADDR        (PART_LOADER_ADDR + PART_LOADER_BLOCKS * PART_BLOCK_SIZE)
#define PART_LITEOS_BLOCKS      0x0F00 // (1M - 128K)
#define PART_ROOTFS_ADDR        (PART_LITEOS_ADDR + PART_LITEOS_BLOCKS * PART_BLOCK_SIZE)
#define PART_ROOTFS_BLOCKS      0X2000 // 4M
#define PART_USERFS_ADDR        (PART_ROOTFS_ADDR + PART_ROOTFS_BLOCKS * PART_BLOCK_SIZE)
#define PART_USERFS_BLOCKS      0x1000 // 2M

#define FIRMWARE_VENDOR_OFFSET  (PART_LITEOS_ADDR + 1 * PART_BLOCK_SIZE);
#define FIRMWARE_WIFI_OFFSET    0x20c00
#define FIRMWARE_WIFI_SIZE      0x40000

/* if GRF_SOC_CON0.remap==1’b1, ICACHE access: 0X00080000 or 0x00000000 */
#define SRAM0_ICACHE_ADDR       0x00080000
#define SRAM0_DCACHE_ADDR       0x20000000
#define SRAM0_SIZE              0x20000 // 128K

#define SRAM1_ICACHE_ADDR       0x000A0000
#define SRAM1_DCACHE_ADDR       0x20020000
#define SRAM1_SIZE              0x20000 // 128K

#define XIP_ADDR                0x10000000
#define XIP_SIZE                0x800000 // 8M

#define SRAM_CODE_ADDR          SRAM0_ICACHE_ADDR
#define SRAM_CODE_SIZE          0x26000 // 128K

#define SRAM_DATA_ADDR          (SRAM0_DCACHE_ADDR + SRAM_CODE_SIZE)
#define SRAM_DATA_SIZE          (SRAM0_SIZE + SRAM1_SIZE - SRAM_CODE_SIZE)

#define PSRAM_ADDR              0x38000000
#define PSRAM_SIZE              0x00800000

#define SYS_STACK_SIZE          0x100000

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LINK_H */
