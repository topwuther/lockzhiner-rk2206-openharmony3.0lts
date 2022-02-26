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

SEC_LIB_SOURCES = memcpy_s.c memmove_s.c memset_s.c securecutil.c secureinput_a.c secureprintoutput_a.c snprintf_s.c sprintf_s.c  strcat_s.c strcpy_s.c strncat_s.c strncpy_s.c strtok_s.c  vsnprintf_s.c vsprintf_s.c 

PWD:=$(shell pwd)
SEC_OBJECTS:=$(patsubst %.c,%.o,$(SEC_LIB_SOURCES))

VIBRATOR_ROOT_DIR = ../../../../../../framework/model/misc/vibrator/driver

obj-$(CONFIG_DRIVERS_HDF_VIBRATOR) += \
               $(VIBRATOR_ROOT_DIR)/src/vibrator_driver.o \
               $(VIBRATOR_ROOT_DIR)/src/vibrator_haptic.o
obj-$(CONFIG_DRIVERS_HDF_VIBRATOR_LINEAR) += \
              $(VIBRATOR_ROOT_DIR)/chipset/vibrator_linear_driver.o


ccflags-y += -Iinclude/../drivers/hdf/framework/model/misc/vibrator/driver/include \
             -Iinclude/../drivers/hdf/framework/model/misc/vibrator/driver/chipset \
             -Iinclude/../drivers/hdf/framework/include/core \
             -Iinclude/../drivers/hdf/framework/core/common/include/host \
             -Iinclude/../drivers/hdf/framework/include/utils \
             -Iinclude/../drivers/hdf/framework/include/osal \
             -Iinclude/../drivers/hdf/framework/ability/sbuf/include \
             -Iinclude/../drivers/hdf/framework/include/platform \
             -Iinclude/../drivers/hdf/framework/include/config \
             -Iinclude/../drivers/hdf/framework/core/host/include \
             -Iinclude/../drivers/hdf/framework/core/shared/include \
             -Iinclude/../drivers/hdf/framework/utils/include \
             -Iinclude/../drivers/hdf/khdf/osal/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
