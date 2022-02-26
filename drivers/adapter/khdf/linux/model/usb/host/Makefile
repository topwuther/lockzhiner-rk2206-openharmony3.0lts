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

USB_PNP_NOTIFY_ROOT_DIR = ./
obj-$(CONFIG_DRIVERS_HDF_USB_PNP_NOTIFY) += \
         $(USB_PNP_NOTIFY_ROOT_DIR)/src/usb_pnp_notify.o

ccflags-y += -lm -lc -lgcc \
             -Idrivers/hdf/khdf/model/usb/host/include \
             -Idrivers/hdf/framework/include/utils \
             -Idrivers/hdf/framework/utils/include \
             -Idrivers/hdf/khdf/osal/include \
             -Idrivers/hdf/framework/include/core \
             -Idrivers/hdf/framework/core/common/include/host \
             -Idrivers/hdf/framework/core/shared/include \
             -Idrivers/hdf/framework/ability/sbuf/include \
             -Idrivers/hdf/../../../../../../../../../../../../../third_party/bounds_checking_function/include \
             -Idrivers/hdf/framework/include \
             -Idrivers/hdf/framework/include/osal \
             -Idrivers/hdf/framework/include/platform \
             -Idrivers/hdf/framework/include/config \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
