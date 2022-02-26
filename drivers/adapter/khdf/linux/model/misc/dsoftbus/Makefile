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

DSOFTBUS_ROOT_DIR = ../../../../../../framework/model/misc/dsoftbus

obj-$(CONFIG_DRIVERS_HDF_DSOFTBUS) += \
               $(DSOFTBUS_ROOT_DIR)/src/hdf_dsoftbus_driver.o \
               $(DSOFTBUS_ROOT_DIR)/src/module_manager.o
ifeq ($(CONFIG_DSOFTBUS_WLAN_PARAM_MONITOR), y)
obj-$(CONFIG_DRIVERS_HDF_DSOFTBUS) += $(DSOFTBUS_ROOT_DIR)/src/wlan_param_monitor.o
ccflags-$(CONFIG_DRIVERS_HDF_DSOFTBUS) +=-DENABLE_WLAN_PARAM_MONITOR
endif

ccflags-y += -Iinclude/../drivers/hdf/framework/model/misc/dsoftbus/include \
             -Iinclude/../drivers/hdf/framework/core/common/include/host \
             -Iinclude/../drivers/hdf/framework/include/core \
             -Iinclude/../drivers/hdf/framework/include/osal \
             -Iinclude/../drivers/hdf/framework/include/utils \
             -Iinclude/../drivers/hdf/framework/ability/sbuf/include \
             -Iinclude/../drivers/hdf/framework/include/platform \
             -Iinclude/../drivers/hdf/framework/include/config \
             -Iinclude/../drivers/hdf/framework/core/host/include \
             -Iinclude/../drivers/hdf/framework/core/shared/include \
             -Iinclude/../drivers/hdf/framework/utils/include \
             -Iinclude/../drivers/hdf/khdf/osal/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
