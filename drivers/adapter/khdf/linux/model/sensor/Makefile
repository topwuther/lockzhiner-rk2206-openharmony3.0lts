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

SEC_LIB_SOURCES = memcpy_s.c memmove_s.c memset_s.c securecutil.c secureinput_a.c secureprintoutput_a.c snprintf_s.c sprintf_s.c  strcat_s.c strcpy_s.c strncat_s.c strncpy_s.c strtok_s.c  vsnprintf_s.c vsprintf_s.c 

PWD:=$(shell pwd)
SEC_OBJECTS:=$(patsubst %.c,%.o,$(SEC_LIB_SOURCES))

SENSOR_ROOT_DIR = ../../../../../framework/model/sensor/driver

obj-$(CONFIG_DRIVERS_HDF_SENSOR) += \
               $(SENSOR_ROOT_DIR)/common/src/sensor_config_controller.o \
               $(SENSOR_ROOT_DIR)/common/src/sensor_config_parser.o \
               $(SENSOR_ROOT_DIR)/common/src/sensor_device_manager.o \
               $(SENSOR_ROOT_DIR)/common/src/sensor_platform_if.o 
               

obj-$(CONFIG_DRIVERS_HDF_SENSOR_ACCEL) += $(SENSOR_ROOT_DIR)/accel/sensor_accel_driver.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR_ACCEL_BMI160) += $(SENSOR_ROOT_DIR)/chipset/accel/accel_bmi160.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR_GYRO) += $(SENSOR_ROOT_DIR)/gyro/sensor_gyro_driver.o \
               $(SENSOR_ROOT_DIR)/chipset/gyro/gyro_bmi160.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR_BAROMETER) += $(SENSOR_ROOT_DIR)/barometer/sensor_barometer_driver.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR_BAROMETER_BMP180) += $(SENSOR_ROOT_DIR)/chipset/barometer/barometer_bmp180.o

obj-$(CONFIG_DRIVERS_HDF_SENSOR_HALL) += $(SENSOR_ROOT_DIR)/hall/sensor_hall_driver.o 

obj-$(CONFIG_DRIVERS_HDF_SENSOR_HALL_AK8789) += $(SENSOR_ROOT_DIR)/chipset/hall/hall_ak8789.o

ccflags-y += -Idrivers/hdf/framework/model/sensor/driver/include \
             -Idrivers/hdf/framework/model/sensor/driver/common/include \
             -Idrivers/hdf/framework/model/sensor/driver/chipset/accel \
             -Idrivers/hdf/framework/model/sensor/driver/accel \
             -Idrivers/hdf/framework/model/sensor/driver/chipset/gyro \
             -Idrivers/hdf/framework/model/sensor/driver/gyro \
             -Idrivers/hdf/framework/model/sensor/driver/barometer \
             -Idrivers/hdf/framework/model/sensor/driver/chipset/barometer \
             -Idrivers/hdf/framework/model/sensor/driver/hall \
             -Idrivers/hdf/framework/model/sensor/driver/chipset/hall \
             -Idrivers/hdf/framework/include/core \
             -Idrivers/hdf/framework/core/common/include/host \
             -Idrivers/hdf/framework/include/utils \
             -Idrivers/hdf/framework/include/osal \
             -Idrivers/hdf/framework/ability/sbuf/include \
             -Idrivers/hdf/framework/include/platform \
             -Idrivers/hdf/framework/include/config \
             -Idrivers/hdf/khdf/osal/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include