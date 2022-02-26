/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HDF_DEVMGR_H__
#define __HDF_DEVMGR_H__

#if defined(__KERNEL__)

#elif defined(CONFIG_LITE_OS)

#else
#include "devhost/hdf_attribute.h"
#include "devhost/hdf_device.h"
#include "devhost/hdf_device_full.h"
#include "devhost/hdf_driver.h"
#include "devhost/dm_service_if.h"
#include "devhost/ds_manager_if.h"

#include "hidl/hdf_dev_token.h"
#include "hidl/hdf_ds_manager_clnt.h"

#include "mock/hdf_sbuf.h"
#include "mock/hdf_remote_object.h"
#include "mock/hdf_service_registry.h"

#include "../third_party/utils/include/of/fdt_convertor.h"
#include "../third_party/utils/include/of/of.h"
#endif

#endif /* __HDF_DEVMGR_H__ */
