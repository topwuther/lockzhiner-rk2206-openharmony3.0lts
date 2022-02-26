# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ifeq ($(LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE), y)
LITEOS_BASELIB += -L$(LITEOSTOPDIR)/../../device/hisilicon/hispark_taurus/sdk_liteos/drivers
LITEOS_BASELIB += -lusb_dwc3
endif

ifeq ($(LOSCFG_DRIVERS_HDF), y)
    LITEOS_BASELIB += --whole-archive
    LITEOS_DRIVERS_HDF := $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos
    LITEOS_SOURCE_ROOT := $(LITEOSTOPDIR)/../..
    LIB_SUBDIRS        += $(LITEOS_DRIVERS_HDF)
    HDF_FRAMEWORKS_PATH:= $(LITEOSTOPDIR)/../../drivers/framework
    LITEOS_BASELIB += -lhdf
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/core/common/include/manager
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/support/platform/include
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/support/platform/include/common
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/support/platform/include/mmc
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/model/storage/include/mtd
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/include/platform
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/include/utils
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/include/osal
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/utils/include
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/osal/include
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(LITEOSTOPDIR)/../../drivers/adapter/khdf/model/storage/include

# models
ifeq ($(LOSCFG_DRIVERS_HDF_WIFI), y)
    LITEOS_BASELIB += -lhdf_wifi_model
    LIB_SUBDIRS    +=  $(LITEOS_DRIVERS_HDF)/model/network/wifi
endif

ifeq ($(LOSCFG_DRIVERS_HIETH_SF), y)
    LITEOS_BASELIB += -lhdf_eth_model
    LIB_SUBDIRS    +=  $(LITEOS_DRIVERS_HDF)/model/network/ethernet
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USB), y)
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(LITEOS_DRIVERS_HDF)/model/bus/usb/include
    LITEOS_BASELIB += -lhdf_usb
    LIB_SUBDIRS    +=  $(LITEOS_DRIVERS_HDF)/model/bus/usb
endif

ifeq ($(LOSCFG_DRIVERS_HDF_DISP), y)
    LITEOS_BASELIB += -lhdf_display
    LIB_SUBDIRS    +=  $(LITEOS_DRIVERS_HDF)/model/display
endif

ifeq ($(LOSCFG_DRIVERS_HDF_INPUT), y)
    LITEOS_BASELIB += -lhdf_input_driver
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/input
endif

ifeq ($(LOSCFG_DRIVERS_HDF_SENSOR), y)
    LITEOS_BASELIB += -lhdf_sensor_driver
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/sensor
endif

ifeq ($(LOSCFG_DRIVERS_HDF_STORAGE), y)
    LITEOS_BASELIB += -lhdf_storage_driver
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/storage
endif

ifeq ($(LOSCFG_DRIVERS_HDF_VIBRATOR), y)
    LITEOS_BASELIB += -lhdf_vibrator_driver
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/misc/vibrator
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USB_DDK_HOST), y)
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(HDF_FRAMEWORKS_PATH)/model/usb/include
    LITEOS_DRIVERS_HDF_INCLUDE += -I $(LITEOS_DRIVERS_HDF)/model/usb/host/include
    LITEOS_BASELIB += -lhdf_usb_ddk_host
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/usb/host
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE), y)
    LITEOS_BASELIB += -lhdf_usb_ddk_device
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/model/usb/device
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USB_DDK_DEVICE), y)
ifeq ($(LOSCFG_DRIVERS_HDF_USBFN_MASTER), y)
    LITEOS_BASELIB += -lhdf_usbfn_master
    LIB_SUBDIRS    += $(LITEOS_SOURCE_ROOT)/drivers/peripheral/usb/gadget/function/master
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USBFN_CDCACM), y)
    LITEOS_BASELIB += -lhdf_usbfn_cdcacm
    LIB_SUBDIRS    += $(LITEOS_SOURCE_ROOT)/drivers/peripheral/usb/gadget/function/acm
endif

ifeq ($(LOSCFG_DRIVERS_HDF_USBFN_CDCECM), y)
    LITEOS_BASELIB += -lhdf_usbfn_cdcecm
    LIB_SUBDIRS    += $(LITEOS_SOURCE_ROOT)/drivers/peripheral/usb/gadget/function/ecm
endif
endif

SOURCE_ROOT := $(abspath $(LITEOSTOPDIR)/../../)
PRODUCT_CONFIG := $(PRODUCT_PATH)/hdf_config
DEVICE_CONFIG := $(DEVICE_PATH)/hdf_config
HAVE_PRODUCT_CONFIG := $(shell if [ -e $(PRODUCT_CONFIG)/Makefile ]; then echo y; else echo n; fi)

ifeq ($(LOSCFG_DRIVERS_HDF_TEST), y)
    # for test
    include $(LITEOS_DRIVERS_HDF)/test/test_lite.mk
    LITEOS_BASELIB += -lhdf_test
    LIB_SUBDIRS    += $(LITEOS_DRIVERS_HDF)/test

    LITEOS_BASELIB += -lhdf_test_config
    LIB_SUBDIRS += $(PRODUCT_CONFIG)/hdf_test
else
    LITEOS_BASELIB += -lhdf_config
    ifeq ($(HAVE_PRODUCT_CONFIG), y)
        LIB_SUBDIRS += $(PRODUCT_CONFIG)
    else
        LIB_SUBDIRS += $(DEVICE_CONFIG)
    endif
endif

# vendor lib
COMPANY_OF_SOC := $(patsubst "%",%,$(LOSCFG_DEVICE_COMPANY))
-include $(LITEOSTOPDIR)/../../device/$(COMPANY_OF_SOC)/drivers/lite.mk
    LITEOS_BASELIB += --no-whole-archive
endif

