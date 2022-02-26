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

MODULE_NAME := hdf_vendor_wifi

HDF_VENDOR_PREFIX := ../../../../../../../../

include drivers/hdf/khdf/model/network/wifi/hdfwifi.mk

obj-$(CONFIG_DRIVERS_HDF_WIFI) += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(HDF_WIFI_VENDOR_ROOT)/core/hdf_wlan_sdio_adapt.o

ccflags-$(CONFIG_DRIVERS_HDF_WIFI) += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_WIFI_FRAMEWORKS_INC) \
    $(HDF_WIFI_ADAPTER_INC) \
    $(HDF_WIFI_VENDOR_INC) \
    $(SECURE_LIB_INC)

HDF_WIFI_FLAGS +=-D_PRE_OS_VERSION_LINUX=1
HDF_WIFI_FLAGS +=-D_PRE_OS_VERSION_LITEOS=2
HDF_WIFI_FLAGS +=-D_PRE_OS_VERSION=_PRE_OS_VERSION_LINUX

ccflags-$(CONFIG_DRIVERS_HDF_WIFI) += $(HDF_WIFI_FLAGS)

obj-$(CONFIG_DRIVERS_HI3881) += hi3881/