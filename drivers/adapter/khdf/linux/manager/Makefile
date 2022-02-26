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

#SOURCES=$(wildcard *.c)
SEC_LIB_SOURCES =   memcpy_s.c memmove_s.c memset_s.c securecutil.c secureinput_a.c secureprintoutput_a.c snprintf_s.c sprintf_s.c  strcat_s.c strcpy_s.c strncat_s.c strncpy_s.c strtok_s.c  vsnprintf_s.c vsprintf_s.c

#SEC_LIB_SOURCES += fwscanf_s.c secureinput_w.c secureprintoutput_w.c swprintf_s.c swscanf_s.c vfwscanf_s.c vswprintf_s.c vswscanf_s.c vwscanf_s.c wcscat_s.c wcscpy_s.c wcsncat_s.c wcsncpy_s.c wcstok_s.c wmemcpy_s.c wmemmove_s.c wscanf_s.c
#SEC_LIB_SOURCES:=$(wildcard ../../framework/libc_sec/src/*.c)
#$(warning SEC_LIB_SOURCES $(SEC_LIB_SOURCES))
PWD:=$(shell pwd)
SEC_OBJECTS:=$(patsubst %.c,%.o,$(SEC_LIB_SOURCES))
OBJECTS:=$(addprefix ../../../../../third_party/bounds_checking_function/src/,${SEC_OBJECTS})
obj-y += \
              ../../../../framework/utils/src/hdf_cstring.o \
              ../../../../framework/utils/src/hdf_slist.o \
              ../../../../framework/utils/src/hdf_sref.o \
              ../../../../framework/utils/src/hdf_map.o \
              ../../../../framework/ability/sbuf/src/hdf_sbuf.o \
              ../../../../framework/ability/sbuf/src/hdf_sbuf_impl_raw.o \
              ../../../../framework/core/host/src/devhost_service.o \
              ../../../../framework/core/host/src/devmgr_service_clnt.o \
              ../../../../framework/core/host/src/devsvc_manager_clnt.o \
              ../../../../framework/core/host/src/hdf_device.o \
              ../../../../framework/core/host/src/hdf_device_node.o \
              ../../../../framework/core/host/src/hdf_device_token.o \
              ../../../../framework/core/host/src/hdf_driver_loader.o \
              ../../../../framework/core/host/src/hdf_observer_record.o \
              ../../../../framework/core/host/src/hdf_service_subscriber.o \
              ../../../../framework/core/host/src/hdf_device_object.o \
              ../../../../framework/core/host/src/hdf_service_observer.o \
              ../../../../framework/core/host/src/power_state_token.o \
              ../../../../framework/core/manager/src/devhost_service_clnt.o \
              ../../../../framework/core/manager/src/device_token_clnt.o \
              ../../../../framework/core/manager/src/devmgr_service.o \
              ../../../../framework/core/manager/src/devsvc_manager.o \
              ../../../../framework/core/manager/src/hdf_driver_installer.o \
              ../../../../framework/core/manager/src/hdf_host_info.o \
              ../../../../framework/core/manager/src/power_state_token_clnt.o \
              ../../../../framework/core/shared/src/hdf_service_record.o \
              ../../../../framework/core/shared/src/hdf_device_info.o \
              ../../../../framework/core/shared/src/hdf_object_manager.o \
              ../../../../framework/core/shared/src/hdf_io_service.o \
              ../../../../framework/core/common/src/devlite_object_config.o \
              ../../../../framework/core/common/src/hdf_attribute.o \
              ../../../../framework/core/common/src/load_driver_entry.o \
              ../../../../framework/core/common/src/devmgr_service_start.o \
              ../../../../framework/core/common/src/hdf_device_node_ext.o \
              ../../../../framework/core/adapter/vnode/src/hdf_vnode_adapter.o \
              ./src/devmgr_load.o \
              $(OBJECTS)

ccflags-y += -Iinclude/../drivers/hdf/framework/include \
             -Iinclude/../drivers/hdf/framework/include/osal \
             -Iinclude/../drivers/hdf/framework/include/utils \
             -Iinclude/../drivers/hdf/framework/include \
             -Iinclude/../drivers/hdf/framework/utils/include \
             -Iinclude/../drivers/hdf/framework/include/core \
             -Iinclude/../drivers/hdf/framework/ability/sbuf/include \
             -Iinclude/../drivers/hdf/framework/core/host/include \
             -Iinclude/../drivers/hdf/framework/core/manager/include \
             -Iinclude/../drivers/hdf/framework/core/shared/include \
             -Iinclude/../drivers/hdf/framework/core/common/include/host \
             -Iinclude/../drivers/hdf/framework/core/common/include/manager \
             -Iinclude/../drivers/hdf/framework/core/adapter/vnode/include \
             -I$(PROJECT_ROOT)/third_party/bounds_checking_function/include \
             -Iinclude/../drivers/hdf/framework/ability/config/hcs_parser/include \
             -Iinclude/../drivers/hdf/framework/include/config \
             -Iinclude/../drivers/hdf/framework/ability/config/hcs_parser/include \
             -Iinclude/../drivers/hdf/khdf/osal/include \
             -Iinclude/uapi/linux

