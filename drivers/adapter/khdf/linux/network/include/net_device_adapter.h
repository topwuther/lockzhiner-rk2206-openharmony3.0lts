/*
 * net_device_adapter.h
 *
 * net device adapter of linux
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

#ifndef HDF_NET_DEVICE_ADAPTER_FULL_H
#define HDF_NET_DEVICE_ADAPTER_FULL_H

#include <linux/netdevice.h>
#include <uapi/linux/if.h>

#include "net_device_impl.h"

/**
 * data sending results, just define locked here, ok and busy reference linux definition for enum netdev_tx.
 */
#define NETDEV_TX_LOCKED 0x20

int32_t RegisterNetDeviceImpl(struct NetDeviceImpl *ndImpl);
int32_t UnRegisterNetDeviceImpl(struct NetDeviceImpl *ndImpl);

struct FullNetDevicePriv {
    struct net_device *dev;
    struct NetDeviceImpl *impl;
};

#endif /* HDF_NET_DEVICE_ADAPTER_FULL_H */

