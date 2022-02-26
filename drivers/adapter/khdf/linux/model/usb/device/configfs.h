/*
 * configfs.h
 *
 * usb configfs adapter of linux
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

#ifndef USB__GADGET__CONFIGFS__H
#define USB__GADGET__CONFIGFS__H

#include <linux/configfs.h>

void unregister_gadget_item(struct config_item *item);

struct config_group *usb_os_desc_prepare_interf_dir(
		struct config_group *parent,
		int n_interf,
		struct usb_os_desc **desc,
		char **names,
		struct module *owner);

static inline struct usb_os_desc *to_usb_os_desc(struct config_item *item)
{
	return container_of(to_config_group(item), struct usb_os_desc, group);
}

#endif /*  USB__GADGET__CONFIGFS__H */
