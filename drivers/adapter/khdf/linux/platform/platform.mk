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

HDF_PLATFORM_FRAMEWORKS_ROOT = ../../../../../framework/support/platform
ccflags-$(CONFIG_DRIVERS_HDF_PLATFORM) += -Idrivers/hdf/framework/include/platform \
    -Idrivers/hdf/framework/support/platform/include \
    -Idrivers/hdf/framework/support/platform/include/mmc \
    -Idrivers/hdf/framework/support/platform/include/common \
    -Idrivers/hdf/framework/model/storage/include/common \
    -Iinclude/hdf \
    -Iinclude/hdf/osal \
    -Iinclude/hdf/utils \
    -Idrivers/hdf/khdf/osal/include \
    -Idrivers/hdf/framework/include \
    -Idrivers/hdf/framework/include/utils \
    -Idrivers/hdf/framework/include/config \
    -Idrivers/hdf/khdf/config/include \
    -Idrivers/hdf/framework/core/manager/include \
    -Idrivers/hdf/framework/core/host/include \
    -Idrivers/hdf/framework/core/shared/include \
    -Idrivers/hdf/framework/include/core \
    -Idrivers/hdf/framework/core/common/include/host \
    -Idrivers/hdf/framework/ability/sbuf/include \
    -Idrivers/hdf/framework/utils/include \
    -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include
