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

SEC_LIB_SOURCES = memcpy_s.c memmove_s.c memset_s.c securecutil.c secureinput_a.c secureprintoutput_a.c snprintf_s.c sprintf_s.c  strcat_s.c strcpy_s.c strncat_s.c strncpy_s.c strtok_s.c  vsnprintf_s.c vsprintf_s.c 

PWD:=$(shell pwd)
SEC_OBJECTS:=$(patsubst %.c,%.o,$(SEC_LIB_SOURCES))

STORAGE_ROOT_DIR = ../../../../../framework/model/storage

obj-$(CONFIG_DRIVERS_HDF_STORAGE) += \
               $(STORAGE_ROOT_DIR)/src/common/storage_block.o \
			   ./storage_block_linux.o

ccflags-y += -Idrivers/hdf/framework/model/storage/include/common \
             -Idrivers/hdf/framework/include/core \
             -Idrivers/hdf/framework/core/common/include/host \
             -Idrivers/hdf/framework/include/utils \
             -Idrivers/hdf/framework/utils/include \
             -Idrivers/hdf/framework/include/osal \
             -Idrivers/hdf/framework/ability/sbuf/include \
             -Idrivers/hdf/framework/include/platform \
             -Idrivers/hdf/framework/support/platform/include/common \
             -Idrivers/hdf/framework/include/config \
             -Idrivers/hdf/khdf/osal/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
