/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "usb_init.h"
#include "usb_api_pri.h"
#include "devsvc_manager_clnt.h"
#include "hdf_device_desc.h"

typedef struct usb_info {
	bool b_init;
	controller_type ctype;
	device_type dtype;
} usb_info_t;

static char *dev_name = NULL;
static bool uvc_enable = false;
static usb_info_t usb_info = { false, (controller_type)0xFFFF, (device_type)0xFFFF };
static struct mtx usb_mtx = PTHREAD_MUTEX_INITIALIZER;

static struct driver_module_data* usb_driver_mode_list[] = {
#ifdef LOSCFG_DRIVERS_USB_HOST_EHCI
	/* xxx controller modules */
	/* usb generial controller modules */
	&usbus_ehci_driver_mod,
#endif

#if defined (LOSCFG_DRIVERS_USB_HOST_XHCI)
	&usbus_xhci_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_HOST_DRIVER
	/* xxx driver modules */
	&uhub_uhub_driver_mod,
	&uhub_usbus_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_4G_MODEM
#ifndef LOSCFG_DRIVERS_HDF_USB_DDK_HOST
	&cdce_uhub_driver_mod,
#endif
	//&bsd_u3g_uhub_driver_mod,
#endif

#if defined (LOSCFG_DRIVERS_USB_SERIAL) || defined (LOSCFG_DRIVERS_USB_4G_MODEM)
	&u3g_uhub_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_MASS_STORAGE
	&umass_uhub_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_ETHERNET
	&axe_uhub_driver_mod,
	&axge_uhub_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_RNDIS_HOST
	&urndis_uhub_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_USB_WIRELESS
	&usb_linux_uhub_driver_mod,
#endif

#if defined (LOSCFG_DRIVERS_USB_HID_CLASS) && defined (LOSCFG_DRIVERS_HDF_INPUT)
	&uhid_uhub_driver_mod,
#endif

#ifdef LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE
	&composite_hiudc3_driver_mod,
#endif
	NULL
};

extern device_t
bus_get_device(device_t dev, const char *name);

void
usbinfo_clean(void)
{
	dev_name = NULL;
	uvc_enable = false;
	usb_info.ctype = (controller_type)0xFFFF;
	usb_info.dtype = (device_type)0xFFFF;
	usb_info.b_init = false;
	return;
}

char *
dev_name_get(void)
{
	return (dev_name);
}

static uint32_t
usb_loadonce(void)
{
	struct driver_module_data *data;
	uint32_t i;

	dprintf("usb %s\n",fetach_usbversion());

#ifdef LOSCFG_DRIVERS_USB
	/* init quirk */
	usb_quirk_init(NULL);

	for (i = 0; (data = usb_driver_mode_list[i]) && (data != NULL); i++) {
		driver_module_handler(NULL, MOD_LOAD, data);
	}
#endif

#ifdef LOSCFG_USB_DEBUG
	usb_debug_module_regsiter();
#endif

	return (0);
}

static void
usb_unloadonce(void)
{
	uint32_t i;

#ifdef LOSCFG_USB_DEBUG
	usb_debug_module_unregsiter();
#endif

#ifdef LOSCFG_DRIVERS_USB
	for (i = 0; usb_driver_mode_list[i] != NULL; i++) {
		driver_module_handler(NULL, MOD_UNLOAD, usb_driver_mode_list[i]);
	}
#endif

	return;
}
#ifdef LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE
static int composite_add(void)
{
    device_t udc;
    device_t composite;

	udc = bus_get_device(nexus, "hiudc3");
	if (udc == NULL) {
		return -1;
	}

    composite = device_add_child(udc, "composite", -1);
    if (composite == NULL) {
    	return -1;
    }

    if (device_probe_and_attach(composite)) {
        device_printf(composite, "WARNING: Probe and attach failed!\n");
        return -1;
    }
    struct HdfDeviceObject *devObj = HdfRegisterDevice("usbfn_master", "usbfn_master", NULL);
    if (devObj == NULL) {
		dprintf("%s register usbfn_master may failed\n", __func__);
    }

	devObj = HdfRegisterDevice("usbfn_cdcacm", "usbfn_cdcacm", NULL);
    if (devObj == NULL) {
		dprintf("%s register usbfn_cdcacm may failed\n", __func__);
    }
    
	devObj = HdfRegisterDevice("usbfn_cdcecm", "usbfn_cdcecm", NULL);
    if (devObj == NULL) {
		dprintf("%s register usbfn_cdcecm may failed\n", __func__);
    }
	dprintf("%s success\n", __func__);
    return 0;
}
#endif
/*
 * step1: modify DRIVER_MODULE,register all driver module
 * step2: make ehci/ohci device (direct skip pci bus)
 * step3: insert ehci/ohci device into usb controller
 * step4: creat ehci/ohci root hub device
 * step5: ehci/ohci transfer setup/start
 */
uint32_t
usb_init(controller_type ctype, device_type dtype)
{
	uint32_t ret;
	static int usb_loaded = 0;

	dprintf("\n******** %s in **********\n", __FUNCTION__);

	mtx_lock(&usb_mtx);
	if (usb_info.b_init) {
		dprintf("\n duplicate usb_init %s, ctype:%d dtype:%d\n", __FUNCTION__, usb_info.ctype, usb_info.dtype);
		mtx_unlock(&usb_mtx);
		return (LOS_NOK);
	}

	if (usb_loaded == 0) {
		ret = usb_loadonce();
		if (ret) {
			goto err;
		}
		usb_loaded = 1;
	}

	usb_dev_init(NULL);

	if (ctype == HOST) {
#if defined (LOSCFG_DRIVERS_USB_HOST_XHCI)
		ret = hixhci_init();
#endif
#ifdef LOSCFG_DRIVERS_USB_HOST_EHCI
		ret = hiehci_init();
#endif
	} else {
#ifdef LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE
		ret = usbd_load_driver();
		if (ret != LOS_OK) {
			dprintf("usbd_load_driver failed ,ret = %d\n", ret);
			goto err;
		}
		ret = composite_add();
		if (ret != LOS_OK) {
			dprintf("composite_add failed ,ret = %d\n", ret);
			goto err;
		}
		ret = usbd_enable_interrupt();
		if (ret != LOS_OK) {
			dprintf("usbd_enable_interrupt failed, ret = %d\n", ret);
			goto err;
		}
#endif
	}

	usb_info.b_init = true;
	usb_info.ctype = ctype;
	usb_info.dtype = dtype;

	mtx_unlock(&usb_mtx);
	dprintf("******** %s ok**********\n\n", __FUNCTION__);
	return (LOS_OK);

err:
	mtx_unlock(&usb_mtx);
	if (!usb_loaded) {
		usb_unloadonce();
	}
	dprintf("******** %s fail**********\n\n", __FUNCTION__);

	return (LOS_NOK);
}

uint32_t
usb_deinit(void)
{
	uint32_t ret = LOS_OK;

	dprintf("******** %s in **********\n\n", __FUNCTION__);
	mtx_lock(&usb_mtx);

	if (usb_info.b_init == false) {
		dprintf("******** %s out, no init **********\n\n", __FUNCTION__);
		goto err;
	}

	if (usb_info.ctype == HOST) {
		dprintf("******** %s fail, host not support **********\n\n", __FUNCTION__);
		goto err;
	}

	if (ret) {
		dprintf("******** %s fail, %d **********\n\n", __FUNCTION__, ret);
		goto err;
	}

	usb_dev_uninit(NULL);

	dprintf(" ** %s uninit success **\n", dev_name_get());
	usbinfo_clean();
	mtx_unlock(&usb_mtx);
	return (LOS_OK);

err:
	mtx_unlock(&usb_mtx);
	return (LOS_NOK);
}

bool
usb_is_devicemode(void)
{
	return (HiUsbIsDeviceMode());
}

