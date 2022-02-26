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

HDF_FRAMEWORK_TEST_ROOT = drivers/hdf/framework/test/unittest
HDF_FRAMEWORK_ROOT = drivers/hdf/framework
#$(error HDF_FRAMEWORK_ROOT is $(HDF_FRAMEWORK_ROOT))

ccflags-$(CONFIG_DRIVERS_HDF_TEST) += -Idrivers/hdf/framework/include/platform \
    -I$(HDF_FRAMEWORK_ROOT)/support/platform/include \
    -I$(HDF_FRAMEWORK_ROOT)/support/platform/include/platform \
    -Iinclude/hdf \
    -Iinclude/hdf/osal \
    -Iinclude/hdf/utils \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/common \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/manager \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/osal \
    -Idrivers/hdf/khdf/test/adapter/osal/include \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/osal \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/wifi \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/netdevice \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/module \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/net \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/qos \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/message \
    -Idrivers/hdf/khdf/network/include \
    -Idrivers/hdf/khdf/osal/include \
    -Idrivers/hdf/khdf/test/osal/include \
    -I$(HDF_FRAMEWORK_ROOT)/include \
    -I$(HDF_FRAMEWORK_ROOT)/include/utils \
    -I$(HDF_FRAMEWORK_ROOT)/include/config \
    -Idrivers/hdf/khdf/config/include \
    -I$(HDF_FRAMEWORK_ROOT)/core/manager/include \
    -I$(HDF_FRAMEWORK_ROOT)/core/host/include \
    -I$(HDF_FRAMEWORK_ROOT)/core/shared/include \
    -I$(HDF_FRAMEWORK_ROOT)/include/core \
    -I$(HDF_FRAMEWORK_ROOT)/core/common/include/host \
    -I$(HDF_FRAMEWORK_ROOT)/ability/sbuf/include \
    -I$(HDF_FRAMEWORK_ROOT)/ability/config/hcs_parser/include \
    -I$(HDF_FRAMEWORK_ROOT)/utils/include \
    -I$(HDF_FRAMEWORK_ROOT)/include/wifi \
    -I$(HDF_FRAMEWORK_ROOT)/include/net \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/include \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/common/netdevice \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/core/module \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/platfrom/src/qos \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/core/components/softap \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/core/components/sta \
    -I$(HDF_FRAMEWORK_ROOT)/model/network/wifi/platform/include \
    -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/platform \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/wifi \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/platform/common \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/netdevice \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/module \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/net \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/qos \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/model/network/wifi/unittest/message \
    -I$(HDF_FRAMEWORK_TEST_ROOT)/sensor \
    -I$(HDF_FRAMEWORK_ROOT)/model/sensor/driver/include \
    -I$(HDF_FRAMEWORK_ROOT)/model/sensor/driver/common/include
