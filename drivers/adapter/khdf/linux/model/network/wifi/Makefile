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

MODULE_NAME := hdf_wifi_model

HDF_DIR_PREFIX := ../../../../../../

include drivers/hdf/khdf/model/network/wifi/hdfwifi.mk

WAP_PATH := core/components/softap
WSTA_PATH := core/components/sta
WEAPOL_PATH := core/components/eapol
NETDEV_PATH := ../common/netdevice
MODULE_PATH := core/module
QOS_PATH := platform/src/qos
MESSAGE_PATH := platform/src/message
PLATFORM_PATH := platform/src
CORE_PATH := core
IBUS_PATH := bus

obj-$(CONFIG_DRIVERS_HDF_WIFI) += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(HDF_WIFI_FRAMEWORKS_ROOT)/$(WAP_PATH)/ap.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(WEAPOL_PATH)/eapol.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(WSTA_PATH)/sta.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(NETDEV_PATH)/net_device.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MODULE_PATH)/wifi_module.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MODULE_PATH)/wifi_base.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MODULE_PATH)/wifi_feature.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(CORE_PATH)/hdf_wifi_core.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(QOS_PATH)/flow_control.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(QOS_PATH)/flow_control_task.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MESSAGE_PATH)/nodes/local_node.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MESSAGE_PATH)/message_dispatcher.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MESSAGE_PATH)/message_router.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(MESSAGE_PATH)/sidecar.o \
                       $(HDF_WIFI_KHDF_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_queue.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_priority_queue.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wifi_event.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_chipdriver_manager.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_config_parser.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_power_manager.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_product.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_reset_manager.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(PLATFORM_PATH)/hdf_wlan_utils.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(IBUS_PATH)/hdf_ibus_intf.o \
                       $(HDF_WIFI_FRAMEWORKS_ROOT)/$(IBUS_PATH)/hdf_sdio_intf.o \

ccflags-$(CONFIG_DRIVERS_HDF_WIFI) += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_WIFI_FRAMEWORKS_INC) \
    $(HDF_WIFI_ADAPTER_INC) \
    $(SECURE_LIB_INC)

obj-$(CONFIG_DRIVERS_HDF_WIFI) += vendor/

