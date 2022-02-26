# Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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

include $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/lite.mk

MODULE_NAME := hdf_storage_driver

LOCAL_INCLUDE := $(LITEOSTOPDIR)/../../base/hiviewdfx/interfaces/kits \
                 $(LITEOSTOPDIR)/../../third_party/mtd-utils/include \
                 $(LITEOSTOPDIR)/../../drivers/khdf/liteos/model/storage/include \

LOCAL_SRCS += ./storage_block_lite.c
LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/framework/model/storage/src/common/storage_block.c

ifeq ($(LOSCFG_DRIVERS_HDF_STORAGE_MTD), y)
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/framework/model/storage/src/mtd/mtd_core.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/framework/model/storage/src/mtd/mtd_spi_common.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/framework/model/storage/src/mtd/mtd_char.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/framework/model/storage/src/mtd/mtd_block.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/model/storage/mtd_legacy_lite.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/model/storage/mtd_char_lite.c
    LOCAL_SRCS += $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/model/storage/mtd_block_lite.c
endif

include $(HDF_DRIVER)
