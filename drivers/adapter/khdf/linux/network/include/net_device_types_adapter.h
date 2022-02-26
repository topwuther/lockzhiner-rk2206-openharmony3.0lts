/*
 * net_device_types_adapter.h
 *
 * net device types adapter of linux
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

#ifndef HDF_NET_DEVICE_TYPTES_ADAPTER_FULL_H
#define HDF_NET_DEVICE_TYPTES_ADAPTER_FULL_H

#include <linux/netdevice.h>
#include <linux/types.h>

typedef int32_t NetDevTxResult;

/**
 * data sending results, just define locked here, ok and busy reference linux definition for enum netdev_tx.
 */
#define NETDEV_TX_LOCKED 0x20

#endif /* HDF_NET_DEVICE_TYPTES_ADAPTER_FULL_H */
