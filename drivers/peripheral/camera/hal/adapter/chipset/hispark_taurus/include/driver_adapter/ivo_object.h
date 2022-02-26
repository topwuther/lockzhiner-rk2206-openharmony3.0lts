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

#ifndef HOS_CAMERA_IVO_OBJECT_H
#define HOS_CAMERA_IVO_OBJECT_H

#include "mpi_adapter.h"

namespace OHOS::Camera {
class IVoObject {
public:
    static std::shared_ptr<IVoObject> CreateVoObject();
    virtual ~IVoObject() {};
    virtual void ConfigVo(std::vector<DeviceFormat>& format) = 0;
    virtual void StartVo() = 0;
    virtual void StopVo() = 0;
};
}

#endif // OHOS::Camera

