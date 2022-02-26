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

#ifndef HOS_CAMERA_IVI_OBJECT_H
#define HOS_CAMERA_IVI_OBJECT_H

#include "mpi_adapter.h"
#include "camera_metadata_info.h"

namespace OHOS::Camera {
class IViObject {
public:
    static std::shared_ptr<IViObject> CreateViObject();
    virtual ~IViObject() {};
    virtual void ConfigVi(std::vector<DeviceFormat>& format) = 0;
    virtual RetCode StartVi() = 0;
    virtual RetCode StopVi() = 0;
    virtual RetCode SetFlashlight(FlashMode mode, bool enable) = 0;
    virtual RetCode UpdateSetting(const camera_device_metadata_tag_t command, const void* args) = 0;
    virtual RetCode QuerySetting(const camera_device_metadata_tag_t command, void* args) = 0;
};
}
#endif // namespace OHOS::Camera

