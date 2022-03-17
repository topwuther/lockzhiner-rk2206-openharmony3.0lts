# Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TARGET = liteos
BUILD_DIR = $(OUTDIR)
BOARD_DIR = $(shell pwd)

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=soft

# mcu
MCU = $(CPU) -mthumb #$(FPU) $(FLOAT-ABI)

#######################################
# LDFLAGS
#######################################
LDSCRIPT = board.ld

boot_LIBS = -lbootstrap -lbroadcast
hardware_LIBS = -lhal_iothardware -lhardware
hdf_LIBS = -lhdf_config -lhdf_core -lhdf_osal_lite -lhdf_platform_lite \
#-lsample_driver -lgpio_driver -lpwm_driver -li2c_driver -ladc_driver -lspi_driver
app_LIBS = 
xts_LIBS = -lmodule_ActsBootstrapTest -lmodule_ActsDfxFuncTest -lmodule_ActsHieventLiteTest -lhuks_test_common -lmodule_ActsHuksHalFunctionTest -lmodule_ActsKvStoreTest \
-lmodule_ActsLwipTest -lmodule_ActsParameterTest -lmodule_ActsSamgrTest -lmodule_ActsUpdaterFuncTest -lmodule_ActsUtilsFileTest \
-lmodule_ActsWifiIotTest -lmodule_ActsWifiServiceTest -lhctest -lmbedtls -lhal_update_static -lhota

common_LIBS = \
-larch         -lcjson_static  -ldump_static       -lhal_wifiaware  -lhuks_3.0_sdk     -lnative_file    -lsec_static      -lwifiaware \
-lauthmanager  -lcmsis         -lexchook                            -lkernel           -lpm             -lsysparam        -lwifiservice \
-lbacktrace    -lcppsupport    -lhal_file_static   -lhichainsdk     -lposix          -ltoken_static \
-lboard        -lcpup          -lhievent_lite   -lmbedtls          -lsamgr          -ltrans_service \
-ldiscovery     -lhal_sysparam      -lhilog_lite     -lmusl-c           -lsamgr_adapter  -lutils \
-llwip          -lhal_token_static  -lhiview_lite    -lmusl-m           -lsamgr_source   -lutils_kv_store \
-lpahomqtt_static -llzlittlefs
LIBS= -Wl,--start-group \
      -Wl,--whole-archive $(boot_LIBS) $(hardware_LIBS) $(hdf_LIBS) $(app_LIBS) -Wl,--no-whole-archive \
      $(common_LIBS) \
      -Wl,--end-group

LIB_DIR = -L$(BUILD_DIR)/libs

LDFLAGS = $(MCU) \
	  --specs=nosys.specs \
	  -T$(LDSCRIPT) \
	  $(LIB_DIR) \
	  -Wl,--start-group $(LIBS) -Wl,--end-group \
	  -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
	  -Wl,--gc-sections

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
$(BUILD_DIR)/$(TARGET).elf:
	$(CC) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(BIN) $< $@

-include $(wildcard $(BUILD_DIR)/*.d)
