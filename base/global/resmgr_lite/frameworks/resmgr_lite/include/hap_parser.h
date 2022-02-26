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

#ifndef RESOURCE_MANAGER_ZIPARCHIVE_H
#define RESOURCE_MANAGER_ZIPARCHIVE_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "res_desc.h"
#include "res_config_impl.h"

namespace OHOS {
namespace Global {
namespace Resource {
class HapParser {
public:
    /**
     * Read specified file in zip to buffer
     * @param zipFile
     * @param fileName  file name in zip which we will read
     * @param buffer    bytes will write to buffer
     * @param bufLen    the file length in bytes
     * @param errInfo
     * @return
     */
    static int32_t ReadFileFromZip(const char *zipFile, const char *fileName, void **buffer,
                                  size_t &bufLen, std::string &errInfo);

    /**
     * Read resource.index in hap to buffer
     * @param zipFile hap file path
     * @param buffer  bytes will write to buffer
     * @param bufLen  length in bytes
     * @param errInfo
     * @return
     */
    static int32_t ReadIndexFromFile(const char *zipFile, void **buffer,
                                     size_t &bufLen, std::string &errInfo);

    static int32_t ParseResHex(const char *buffer, const size_t bufLen, ResDesc &resDesc,
                               const ResConfigImpl *defaultConfig = nullptr);

    static ResConfigImpl *CreateResConfigFromKeyParams(const std::vector<KeyParam *> &keyParams);

    static std::string ToFolderPath(const std::vector<KeyParam *> &keyParams);

    static ScreenDensity GetScreenDensity(uint32_t value);

    static DeviceType GetDeviceType(uint32_t value);
private:
    static const char *RES_FILE_NAME;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif
