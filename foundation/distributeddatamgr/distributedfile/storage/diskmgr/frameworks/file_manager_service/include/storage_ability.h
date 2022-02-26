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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_STORAGE_ABILITY_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_STORAGE_ABILITY_H

#include <string>
#include "file_info.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "root_info.h"
#include "system_ability_definition.h"
#include <uri.h>

namespace OHOS {
namespace FileManager {
struct SaveFileInfo {
    bool status = false;
    std::string srcUri = "";
    std::string dstUri = "";
};
class StorageAbility {
public:
    /**
     * @brief Constructor.
     */
    StorageAbility();
    virtual ~StorageAbility() = default;

    static StorageAbility *Instance();
    /**
     * @param  uriStr Indicates the path of data to query.
     * @param  scheme Be used to check the validity of uri.
     * @param  absoluteUri Save the path of data to query.
     * 
     * @return Return a status code which indicates the result of execute function.
     */
    int CheckUri(std::string &uriStr, const std::string &scheme, std::string &absoluteUri) const;
    /**
     * @param  uri Indicates the path of data to query.
     * @param  result Save all the files which contained in the requested directory.
     * 
     * @return Return a status code which indicates the result of execute function.
     */
    int QueryFiles(const Uri &uri, std::vector<FileInfo> &result);
    /**
     * @param  result Save user's primary device storageinfo and other storage devices info,such as
     * deviceName,volume description ,diskInfo.
     * user's primary device : We currently only support a single emulated volume per user mounted at a
     * timeand it's always considered the primary.
     * Use device name for the volume since this is likely same thing.
     * User sees when they mount their phone on another device.
     * Device name should always be set. In case it isn't, though,all back to a localized "Internal Storage"
     * string.
     * Other storage devices： like an SD card or USB OTG drive plugged in.
     * 
     * @return  Return a status code which indicates the result of execute function.
     */
    int QueryDeviceInfo(std::vector<RootInfo> &result);
    /**
     * @param uri Indicates the path of data to query.
     * @param fileName Query all the files which include "filename" in the directory.
     * @param result Save the children files contained in the requested directory.
     * 
     * @return Return a status code which indicates the result of execute function.
     */
    int SearchFiles(const Uri &uri, const std::string &fileName, std::vector<FileInfo> &result);
    /**
     * @param uri Indicates the target path which the file to be saved.
     * @param srcPath Indicates the source path which the files to be saved.
     * @param result Save the result whether one or more files saved successful.
     * 
     * @return Return a status code which indicates the result of execute function.
     */
    int SaveFiles(const Uri &uri, const std::vector<std::string> &srcPath, std::vector<SaveFileInfo> &result);
    /**
     * @param uri Indicates the path which the directory to be created.
     * @param result Save the result whether make directory successful.
     * 
     * @return Return a status code which indicates the result of execute function.
     */
    int CreateDir(const Uri &uri);

private:
    const std::string SCHEMEOHOS = "dataability";
    static std::unique_ptr<StorageAbility> mInstance;
};
} // namespace FileManager
} // namespace OHOS

#endif // FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_STORAGE_ABILITY_H