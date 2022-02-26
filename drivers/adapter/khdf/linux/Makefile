#
# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
#

export HDF_ROOT := drivers/hdf
obj-$(CONFIG_DRIVERS_HDF)  += osal/
obj-$(CONFIG_DRIVERS_HDF)  += network/
obj-$(CONFIG_DRIVERS_HDF)  += config/

SUPPORT_LEVEL_STD_H := $(shell [ "$(CONFIG_HDF_SUPPORT_LEVEL)" -ge 2 ] && echo true)

$(warning PRODUCT_PATH=$(PRODUCT_PATH))
ifeq ($(PRODUCT_PATH),)
$(error PRODUCT_PATH not)
endif

# for L2+, hcs config should in vendor/product_company/product_name/config/khdf
ifeq ($(SUPPORT_LEVEL_STD_H), true)
SUB_DIR:=khdf/
endif

ifeq ($(CONFIG_DRIVERS_HDF), y)
ifeq ($(CONFIG_DRIVERS_HDF_TEST), y)
obj-$(CONFIG_DRIVERS_HDF) += ../../../../$(PRODUCT_PATH)/hdf_config/$(SUB_DIR)/hdf_test/
obj-$(CONFIG_DRIVERS_HDF) += test/
else
obj-$(CONFIG_DRIVERS_HDF) += ../../../../$(PRODUCT_PATH)/hdf_config/$(SUB_DIR)
endif
endif

obj-$(CONFIG_DRIVERS_HDF) += manager/
obj-$(CONFIG_DRIVERS_HDF_PLATFORM) += platform/
obj-$(CONFIG_DRIVERS_HDF_DISP) += model/display/
obj-$(CONFIG_DRIVERS_HDF_INPUT) += model/input/
obj-$(CONFIG_DRIVERS_HDF_WIFI) += model/network/wifi/
obj-$(CONFIG_DRIVERS_HDF_USB_PNP_NOTIFY) += model/usb/host/
obj-$(CONFIG_DRIVERS_HDF_USB_F_GENERIC) += model/usb/device/
obj-$(CONFIG_DRIVERS_HDF_SENSOR) += model/sensor/
obj-$(CONFIG_DRIVERS_HDF_STORAGE) += model/storage/
obj-$(CONFIG_DRIVERS_HDF_BT) += model/network/bluetooth/
obj-$(CONFIG_DRIVERS_HDF_VIBRATOR) += model/misc/vibrator/
obj-$(CONFIG_DRIVERS_HDF_AUDIO) += model/audio/
obj-$(CONFIG_DRIVERS_HDF_DSOFTBUS) += model/misc/dsoftbus/
