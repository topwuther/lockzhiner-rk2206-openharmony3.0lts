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
include drivers/hdf/khdf/test/test_khdf.mk
HDF_FRAMWORK_TEST_ROOT = ../../../../framework/test/unittest

obj-y  += $(HDF_FRAMWORK_TEST_ROOT)/common/hdf_main_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_test_entry.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_all_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_list_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_file_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_work_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/osal/osal_get_case_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/manager/sample_driver_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/ability/config/hcs_parser/unittest/hcs_config_test.o \
          $(HDF_FRAMWORK_TEST_ROOT)/config/hdf_config_test.o

obj-$(CONFIG_DRIVERS_HDF_PLATFORM_GPIO) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/gpio_test.o \
                                           $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_gpio_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_I2C) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/i2c_test.o \
                                          $(HDF_FRAMWORK_TEST_ROOT)/platform/common/i2c_driver_test.o \
                                          $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_i2c_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_PWM) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/pwm_test.o \
                                          $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_pwm_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_RTC) += $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_rtc_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_SPI) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/spi_test.o \
                                          $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_spi_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_SDIO) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/sdio_test.o \
                                           $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_sdio_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_EMMC) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/emmc_test.o \
                                           $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_emmc_entry_test.o

obj-$(CONFIG_DRIVERS_HDF_PLATFORM_UART) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/uart_test.o \
                                           $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_uart_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_WATCHDOG) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/watchdog_test.o \
                                               $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_watchdog_entry_test.o
obj-$(CONFIG_DRIVERS_HDF_PLATFORM_MIPI_DSI) += $(HDF_FRAMWORK_TEST_ROOT)/platform/common/mipi_dsi_test.o \
                                               $(HDF_FRAMWORK_TEST_ROOT)/platform/hdf_mipi_dsi_entry_test.o

obj-$(CONFIG_DRIVERS_HDF_WIFI) += $(HDF_FRAMWORK_TEST_ROOT)/wifi/hdf_wifi_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/netdevice/net_device_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/module/hdf_module_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/net/hdf_netbuf_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/qos/flow_control_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/message/hdf_queue_test.o \
                                  $(HDF_FRAMWORK_TEST_ROOT)/model/network/wifi/unittest/message/hdf_single_node_message_test.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR) += $(HDF_FRAMWORK_TEST_ROOT)/sensor/hdf_sensor_test.o

obj-$(CONFIG_DRIVERS_HDF_AUDIO_TEST) += $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/hdf_audio_test.o \
                                        $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/audio_host_test.o \
                                        $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/audio_core_test.o \
                                        $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/audio_parse_test.o \
                                        $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/audio_sapm_test.o \
                                        $(HDF_FRAMWORK_TEST_ROOT)/model/audio/src/audio_stream_dispatch_test.o

ccflags-y += -Idrivers/hdf/framework/include \
             -Idrivers/hdf/framework/include/osal \
             -Idrivers/hdf/framework/include/utils \
             -Idrivers/hdf/framework/include \
             -Idrivers/hdf/framework/utils/include \
             -Idrivers/hdf/framework/include/core \
             -Idrivers/hdf/framework/ability/sbuf/include \
             -Idrivers/hdf/framework/core/host/include \
             -Idrivers/hdf/framework/core/manager/include \
             -Idrivers/hdf/framework/core/shared/include \
             -Idrivers/hdf/framework/core/common/include/host \
             -Idrivers/hdf/framework/core/common/include/manager \
             -Idrivers/hdf/framework/core/adapter/vnode/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include \
             -Idrivers/hdf/framework/ability/config/hcs_parser/include \
             -Idrivers/hdf/framework/test/unittest/config \
             -Idrivers/hdf/framework/test/unittest/ability/config/hcs_parser/unittest \
             -Idrivers/hdf/framework/include/config \
             -Idrivers/hdf/framework/ability/config/hcs_parser/include \
             -Idrivers/hdf/framework/test/unittest/osal \
             -Idrivers/hdf/khdf/osal/include \
             -Idrivers/hdf/framework/model/audio/include \
             -Idrivers/hdf/framework/model/audio/sapm/include \
             -Idrivers/hdf/framework/model/audio/dispatch/include \
             -Idrivers/hdf/framework/test/unittest/model/audio/include \
             -Idrivers/hdf/framework/../peripheral/audio/interfaces/include \
             -Idrivers/hdf/framework/model/audio/dispatch/include \
             -Idrivers/hdf/framework/model/audio/device/codec/include \
             -Idrivers/hdf/framework/model/audio/device/soc/include \
             -Idrivers/hdf/framework/model/audio/device/accessory/include
