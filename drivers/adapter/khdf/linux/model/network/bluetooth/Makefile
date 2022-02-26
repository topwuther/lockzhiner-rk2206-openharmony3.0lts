#
# Copyright (c) 2021 Huawei Device Co., Ltd.
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

MODULE_NAME := hdf_bt_model

include drivers/hdf/khdf/model/network/bluetooth/env_config.mk

HDF_BT_FRAMEWORKS_ROOT := ../../../../../../framework/model/network/bluetooth

obj-$(CONFIG_DRIVERS_HDF_BT) += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(HDF_BT_FRAMEWORKS_ROOT)/hdf_bt_core.o \
                       $(HDF_BT_FRAMEWORKS_ROOT)/hdf_chip.o \
                       $(HDF_BT_FRAMEWORKS_ROOT)/hdf_reset.o \
                       $(HDF_BT_FRAMEWORKS_ROOT)/hdf_power.o

ccflags-$(CONFIG_DRIVERS_HDF_BT) += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_BT_FRAMEWORKS_INC) \
    $(SECURE_LIB_INC) 

