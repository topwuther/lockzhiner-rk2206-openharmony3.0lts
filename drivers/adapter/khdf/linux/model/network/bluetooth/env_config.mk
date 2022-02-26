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

HDF_FRAMEWORKS_INC := \
    -I./ \
    -Idrivers/hdf/framework/ability/sbuf/include \
    -Idrivers/hdf/framework/core/common/include/host \
    -Idrivers/hdf/framework/core/host/include \
    -Idrivers/hdf/framework/core/manager/include \
    -Idrivers/hdf/framework/core/shared/include \
    -Idrivers/hdf/framework/include \
    -Idrivers/hdf/framework/include/config \
    -Idrivers/hdf/framework/include/core \
    -Idrivers/hdf/framework/include/platform \
    -Idrivers/hdf/framework/include/utils \
    -Idrivers/hdf/framework/support/platform/include \
    -Idrivers/hdf/framework/support/platform/include/platform \
    -Idrivers/hdf/framework/utils/include \
    -Idrivers/hdf/khdf/osal/include \
    -Idrivers/hdf/khdf/config/include \
    -Iinclude/hdf \
    -Iinclude/hdf/osal \
    -Iinclude/hdf/utils \
    -Idrivers/hdf/framework/include/bluetooth

HDF_BT_FRAMEWORKS_INC := \
    -Idrivers/hdf/framework/model/network/bluetooth

SECURE_LIB_INC := \
    -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include