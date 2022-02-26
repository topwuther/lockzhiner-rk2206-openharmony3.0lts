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

include drivers/hdf/khdf/platform/platform.mk

obj-y  += $(HDF_PLATFORM_FRAMEWORKS_ROOT)/src/mipi_dsi_core.o \
          $(HDF_PLATFORM_FRAMEWORKS_ROOT)/src/mipi_dsi_if.o \
          mipi_dsi_adapter.o

obj-y  += mipi_tx_dev.o \
          mipi_tx_hi35xx.o

EXTRA_CFLAGS += -Wno-error=date-time
