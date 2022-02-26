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

obj-$(CONFIG_DRIVERS_HDF_WIFI)  += net_device_adapter.o netbuf_adapter.o

ccflags-$(CONFIG_DRIVERS_HDF_WIFI) += \
             -Idrivers/hdf/framework/model/network/common/netdevice \
             -Idrivers/hdf/framework/include/net\
             -Idrivers/hdf/khdf/network/include \
             -Idrivers/hdf/khdf/osal/include \
             -Iinclude/hdf/wifi \
             -Iinclude/hdf/utils \
             -Iinclude/hdf/osal \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
