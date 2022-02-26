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

KHDF_AUDIO_BASE_ROOT_DIR = ../../../../../..
KHDF_AUDIO_KHDF_ROOT_DIR = ../drivers/hdf/khdf
KHDF_FRAMEWORK_ROOT_DIR = ../drivers/hdf/framework
KHDF_AUDIO_ROOT_DIR = ../../../../../framework/model/audio
KHDF_AUDIO_PERIPHERAL_ROOT_DIR = $(KHDF_AUDIO_BASE_ROOT_DIR)/drivers/peripheral
KHDF_AUDIO_CODEC_DIR = $(KHDF_AUDIO_PERIPHERAL_ROOT_DIR)/audio/chipsets

obj-y += \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_core.o \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_host.o \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_parse.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_accessory_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_codec_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_platform_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/sapm/src/audio_sapm.o \
          $(KHDF_AUDIO_ROOT_DIR)/dispatch/src/audio_stream_dispatch.o \
          $(KHDF_AUDIO_ROOT_DIR)/dispatch/src/audio_control_dispatch.o \
          $(KHDF_AUDIO_CODEC_DIR)/tfa9879/accessory/src/tfa9879_accessory_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/tfa9879/accessory/src/tfa9879_accessory_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_ops.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/dsp/src/dsp_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_dai_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_aiao_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_platform_ops.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_platform_adapter.o

obj-$(CONFIG_ARCH_HI3516DV300) += \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_core.o \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_host.o \
          $(KHDF_AUDIO_ROOT_DIR)/core/src/audio_parse.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_accessory_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_codec_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/common/src/audio_platform_base.o \
          $(KHDF_AUDIO_ROOT_DIR)/sapm/src/audio_sapm.o \
          $(KHDF_AUDIO_ROOT_DIR)/dispatch/src/audio_stream_dispatch.o \
          $(KHDF_AUDIO_ROOT_DIR)/dispatch/src/audio_control_dispatch.o \
          $(KHDF_AUDIO_CODEC_DIR)/tfa9879/accessory/src/tfa9879_accessory_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/tfa9879/accessory/src/tfa9879_accessory_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/src/hi3516_codec_ops.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/dsp/src/dsp_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_dai_adapter.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_aiao_impl.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_platform_ops.o \
          $(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/src/hi3516_platform_adapter.o

ccflags-y += -lm -lc  -lgcc -std=gnu99 -Werror \
             -Iinclude/$(KHDF_AUDIO_BASE_ROOT_DIR)/drivers/framework/include/platform \
             -Iinclude/$(KHDF_AUDIO_BASE_ROOT_DIR)/utils/native/base/include \
             -Iinclude/$(KHDF_AUDIO_KHDF_ROOT_DIR)/osal/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/support/platform/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/utils/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/core \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/core/common/include/host \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/utils \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/utils \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/osal \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/ability/sbuf/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/platform \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/config \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/core/host/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/core/shared/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/core/manager/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/include/audio \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/model/audio/core/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/model/audio/sapm/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/model/audio/dispatch/include \
             -Iinclude/$(KHDF_FRAMEWORK_ROOT_DIR)/model/audio/common/include \
             -Iinclude/$(KHDF_AUDIO_CODEC_DIR)/tfa9879/accessory/include \
             -Iinclude/$(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/codec/include \
             -Iinclude/$(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/soc/include \
             -Iinclude/$(KHDF_AUDIO_CODEC_DIR)/hi3516dv300/include \
             -Iinclude/$(KHDF_AUDIO_PERIPHERAL_ROOT_DIR)/audio/interfaces/include \
             -Iinclude/$(KHDF_AUDIO_CODEC_DIR)/include
