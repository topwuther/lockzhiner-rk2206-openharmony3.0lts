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

MODULE_NAME := hi3881

HDF_VENDOR_PREFIX := ../../../../../../../../../

include drivers/hdf/khdf/model/network/wifi/hdfwifi.mk

INC_TOP_PATH := $(PROJECT_ROOT)
VENDOR_WIFI_PATH := device/hisilicon/drivers/wifi/driver
WIFI_DRIVER_DIR := hi3881

##################path of compile file :start###############
#####wifi#####
OAL_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/oal/
OAM_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/oam/
FRW_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/frw/
MAC_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/mac/common/
HMAC_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/mac/hmac/
HCC_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/hcc/
WAL_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/wal/
ADPATER_PATH := $(HDF_WIFI_VENDOR_ROOT)/$(WIFI_DRIVER_DIR)/hdfadapt/
##################path of compile file :end###############

ccflags-$(CONFIG_DRIVERS_HDF_WIFI) += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_WIFI_FRAMEWORKS_INC) \
    $(HDF_WIFI_ADAPTER_INC) \
    $(HDF_WIFI_VENDOR_INC) \
    -I$(INC_TOP_PATH)/$(VENDOR_WIFI_PATH)/$(WIFI_DRIVER_DIR)/linuxkpi/include \
    $(SECURE_LIB_INC)

include drivers/hdf/khdf/model/network/wifi/vendor/hi3881/env_config.mk

HI1131_HOST_CFLAGS += -Wextra -fsigned-char -fno-strict-aliasing -Wdate-time \
                    -Wformat=2 -Wshadow -fvisibility=hidden -fno-common -fstack-protector-strong

HI1131_HOST_CFLAGS += -Wall
HI1131_HOST_CFLAGS += $(HI1131_WIFI_CFLAGS)

obj-$(CONFIG_DRIVERS_HI3881) += $(MODULE_NAME).o

####################################### platform module start #########################################
oal-objs := oal_util.o oal_net.o oal_main.o oal_mem.o
oal-objs += oal_cfg80211.o oal_sdio_host.o oal_kernel_file.o exception_rst.o \
            plat_firmware.o plat_pm.o plat_pm_wlan.o plat_sdio.o oal_chr.o
oal-objs := $(addprefix $(OAL_PATH),$(oal-objs))

adapter-objs := hdfinit_3881.o net_adpater.o hdf_driver_register.o
adapter-objs := $(addprefix $(ADPATER_PATH),$(adapter-objs))

oam-objs := oam_main.o oam_log.o
oam-objs := $(addprefix $(OAM_PATH),$(oam-objs))

board-objs := plat_board_common_sdio.o
board-objs := $(addprefix $(OAL_PATH),$(board-objs))

frw-objs += frw_event.o frw_event_sched.o frw_main.o frw_task.o frw_timer.o
frw-objs := $(addprefix $(FRW_PATH),$(frw-objs))

plat-objs = $(oal-objs) $(oam-objs) $(board-objs) $(frw-objs) $(adapter-objs)
####################################### platform module end #########################################

####################################### wifi module start #########################################
hmac-objs := hmac_11i.o hmac_blockack.o hmac_chan_mgmt.o hmac_config.o hmac_crypto_tkip.o hmac_event.o hmac_uapsd.o \
             hmac_device.o hmac_encap_frame.o hmac_encap_frame_ap.o hmac_encap_frame_sta.o hmac_frag.o hmac_fsm.o \
             hmac_mgmt_ap.o hmac_mgmt_bss_comm.o hmac_mgmt_classifier.o hmac_mgmt_sta.o hmac_protection.o hmac_user.o \
             hmac_rx_data.o hmac_tx_data.o hmac_rx_filter.o hmac_scan.o hmac_sme_sta.o hmac_vap.o \
             hmac_main.o

ifeq ($(CFG_EDCA_OPT_AP), y)
hmac-objs += hmac_edca_opt.o
endif
ifeq ($(CFG_P2P), y)
hmac-objs += hmac_p2p.o
endif
ifeq ($(CFG_TCP_OPT), y)
hmac-objs += hmac_tcp_opt.o
endif
ifeq ($(CFG_TX_CLASSIFY_LAN_TO_WLAN), y)
hmac-objs += hmac_traffic_classify.o
endif
ifeq ($(CFG_WOW), y)
hmac-objs += hmac_wow.o
endif
ifeq ($(CFG_ANY), y)
hmac-objs += hmac_any.o
endif
ifeq ($(CFG_WAPI), y)
hmac-objs += hmac_wapi.o hmac_wapi_wpi.o hmac_wapi_sms4.o
endif
hmac-objs := $(addprefix $(HMAC_PATH),$(hmac-objs))

wal-objs  := wal_customize.o wal_cfg80211.o wal_11d.o wal_hipriv.o wal_event_msg.o hi_wifi_driver_api.o wal_linux_flowctl.o wal_ioctl.o wal_event.o wal_scan.o wal_main.o wal_regdb.o wal_mac80211.o
wal-objs  := $(addprefix $(WAL_PATH),$(wal-objs))

mac-objs  := mac_data.o mac_device.o mac_frame.o mac_ie.o mac_user.o mac_vap.o mac_mib.o mac_resource.o mac_regdomain.o mac_pm_driver.o
mac-objs  := $(addprefix $(MAC_PATH),$(mac-objs))

hcc-objs  := hcc_comm.o hcc_task.o hcc_host.o hcc_hmac.o hmac_hcc_adapt.o
hcc-objs  := $(addprefix $(HCC_PATH),$(hcc-objs))

wifi-objs += $(mac-objs) $(hmac-objs) $(wal-objs) $(hcc-objs)
####################################### wifi module end #########################################

$(MODULE_NAME)-objs := $(wifi-objs) $(plat-objs)

ccflags-$(CONFIG_DRIVERS_HI3881) += $(HI1131_HOST_CFLAGS)