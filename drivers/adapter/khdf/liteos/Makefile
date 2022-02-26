# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include $(LITEOSTOPDIR)/config.mk
include ./lite.mk

MODULE_NAME := hdf

HDF_FRAMEWORKS = ../../../framework

LOCAL_SRCS := ./osal/src/osal_timer.c  \
              ./osal/src/osal_time.c  \
              ./osal/src/osal_thread.c  \
              ./osal/src/osal_spinlock.c  \
              ./osal/src/osal_irq.c  \
              ./osal/src/osal_firmware.c \
              ./osal/src/osal_file.c \
              ./osal/src/osal_workqueue.c \
              ./osal/src/osal_cdev.c \
              $(HDF_FRAMEWORKS)/utils/src/hdf_cstring.c \
	      $(HDF_FRAMEWORKS)/utils/src/hdf_map.c \
              $(HDF_FRAMEWORKS)/utils/src/hdf_slist.c \
              $(HDF_FRAMEWORKS)/utils/src/hdf_sref.c \
              $(HDF_FRAMEWORKS)/ability/sbuf/src/hdf_sbuf.c \
	      $(HDF_FRAMEWORKS)/ability/sbuf/src/hdf_sbuf_impl_raw.c \
              ./osal/src/osal_mem.c \
              ./osal/src/osal_mutex.c \
              ./osal/src/osal_sem.c \
              $(HDF_FRAMEWORKS)/core/host/src/devhost_service.c \
              $(HDF_FRAMEWORKS)/core/host/src/devmgr_service_clnt.c \
              $(HDF_FRAMEWORKS)/core/host/src/devsvc_manager_clnt.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_device.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_device_node.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_device_token.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_driver_loader.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_observer_record.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_service_subscriber.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_device_object.c \
              $(HDF_FRAMEWORKS)/core/host/src/hdf_service_observer.c \
              $(HDF_FRAMEWORKS)/core/host/src/power_state_token.c \
              $(HDF_FRAMEWORKS)/core/manager/src/devhost_service_clnt.c \
              $(HDF_FRAMEWORKS)/core/manager/src/device_token_clnt.c \
              $(HDF_FRAMEWORKS)/core/manager/src/devmgr_service.c \
              $(HDF_FRAMEWORKS)/core/manager/src/devsvc_manager.c \
              $(HDF_FRAMEWORKS)/core/manager/src/hdf_driver_installer.c \
              $(HDF_FRAMEWORKS)/core/manager/src/hdf_host_info.c \
              $(HDF_FRAMEWORKS)/core/manager/src/power_state_token_clnt.c \
              $(HDF_FRAMEWORKS)/core/shared/src/hdf_service_record.c \
              $(HDF_FRAMEWORKS)/core/shared/src/hdf_device_info.c \
              $(HDF_FRAMEWORKS)/core/shared/src/hdf_object_manager.c \
              $(HDF_FRAMEWORKS)/core/shared/src/hdf_io_service.c \
              $(HDF_FRAMEWORKS)/core/adapter/vnode/src/hdf_vnode_adapter.c \
              $(HDF_FRAMEWORKS)/core/common/src/devlite_object_config.c \
              $(HDF_FRAMEWORKS)/core/common/src/hdf_attribute.c \
              $(HDF_FRAMEWORKS)/core/common/src/load_driver_entry.c \
              $(HDF_FRAMEWORKS)/core/common/src/devmgr_service_start.c \
              $(HDF_FRAMEWORKS)/core/common/src/hdf_device_node_ext.c \
              $(HDF_FRAMEWORKS)/ability/config/hcs_parser/src/hcs_blob_if.c \
              $(HDF_FRAMEWORKS)/ability/config/hcs_parser/src/hcs_parser.c \
              $(HDF_FRAMEWORKS)/ability/config/hcs_parser/src/hcs_generate_tree.c \
              $(HDF_FRAMEWORKS)/ability/config/hcs_parser/src/hcs_tree_if.c \
              $(HDF_FRAMEWORKS)/ability/config/device_resource_if.c

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM), y)
              LOCAL_SRCS += ./platform/src/plat_common.c \
              $(HDF_FRAMEWORKS)/support/platform/src/common/platform_device.c \
              $(HDF_FRAMEWORKS)/support/platform/src/common/platform_manager.c \
              $(HDF_FRAMEWORKS)/support/platform/src/common/platform_queue.c \
              $(HDF_FRAMEWORKS)/support/platform/src/common/platform_common.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_I2C), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/i2c_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/i2c_core.c
              LOCAL_SRCS += ./platform/src/i2c_dev.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_ADC), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/adc_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/adc_core.c
endif


ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_GPIO), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/gpio_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/gpio_core.c
              LOCAL_SRCS += ./platform/src/gpio_dev.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_WATCHDOG), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/watchdog_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/watchdog_core.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_SPI), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/spi_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/spi_core.c
              LOCAL_SRCS += ./platform/src/spi_dev.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_PWM), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/pwm_core.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_MMC), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_block.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_core.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_dispatch.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_protocol.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_sdio.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mmc/mmc_emmc.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_SDIO), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/mmc/sdio_if.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_EMMC), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/mmc/emmc_if.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_RTC), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/rtc_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/rtc_core.c \
              $(HDF_FRAMEWORKS)/support/platform/src/rtc_base.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_MIPI_DSI), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/mipi_dsi_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/mipi_dsi_core.c
#              LOCAL_SRCS += ./platform/src/mipi_tx_dev.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_DMAC), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/dmac_core.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_UART), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/uart_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/uart_core.c
              LOCAL_SRCS += ./platform/src/uart_dev.c
endif

ifeq ($(LOSCFG_DRIVERS_HDF_PLATFORM_I2S), y)
              LOCAL_SRCS += $(HDF_FRAMEWORKS)/support/platform/src/i2s_if.c \
              $(HDF_FRAMEWORKS)/support/platform/src/i2s_core.c
endif

ifeq ($(LOSCFG_NET_LWIP_SACK), y)
              LOCAL_SRCS += ./network/src/netbuf_adapter.c
ifeq ($(LOSCFG_DRIVERS_HDF_WIFI), y)
              LOCAL_SRCS += ./network/src/net_device_adapter.c
endif
endif

LOCAL_FLAGS += $(LITEOS_GCOV_OPTS)

include $(HDF_DRIVER)
