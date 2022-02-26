/*
 * netbuf_adapter.h
 *
 * net buffer adapter of linux
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

/**
 * @addtogroup WLAN
 * @{
 *
 * @brief Provides cross-OS migration, component adaptation, and modular assembly and compilation.
 *
 * Based on the unified APIs provided by the WLAN module, developers of the Hardware Driver Interface
 * (HDI) are capable of creating, disabling, scanning for, and connecting to WLAN hotspots, managing WLAN chips,
 * network devices, and power, and applying for, releasing, and moving network data buffers.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef _HDF_NETBUF_ADAPTER_H
#define _HDF_NETBUF_ADAPTER_H

#include <linux/skbuff.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 * @brief Enumerates the segments of a network data buffer.
 *
 * The entire network data buffer is divided into three segments: a header, data, and a tail.
 * The header and tail are used to extend both ends of the data segment.
 *
 * @since 1.0
 */
enum {
    E_HEAD_BUF, /**< Header buffer segment */
    E_DATA_BUF, /**< Data segment */
    E_TAIL_BUF, /**< Tail buffer segment */
    MAX_BUF_NUM /**< Maximum number of buffer segments */
};

typedef struct sk_buff NetBuf;
typedef struct sk_buff_head NetBufQueue;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
/** @} */
