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
#ifndef FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FILE_SHARE_ABILITY_H
#define FOUNDATION_FILEMANAGER_STANDARD_FRAMEWORKS_FILE_MANAGER_INCLUDE_FILE_SHARE_ABILITY_H

#include <cstdio>
#include <string>

#include "ability.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"

#include "shared_path_strategy.h"

#include "values_bucket.h"

namespace OHOS {
namespace FileManager {
const std::string ROOT_PATH = "root-path";
const std::string FILES_PATH = "files-path";
const std::string CACHE_PATH = "cache-path";
const std::string EXTERNAL_FILES_PATH = "external-files-path";
const std::string EXTERNAL_CACHE_PATH = "external-cache-path";
const std::string ATTR_TYPE = "type";
const std::string ATTR_NAME = "name";
const std::string ATTR_PATH = "path";
class FileShareAbility : public AppExecFwk::Ability {
public:
    /**
     * @brief Constructor.
     *
     */
    FileShareAbility();
    virtual ~FileShareAbility() = default;

    static FileShareAbility *Instance();
    /**
     * Will be called when ability start. You should override this function
     *
     * @param want ability start information
     */
    void OnStart(const AppExecFwk::Want &want) override;
    /**
     * @brief Deletes one or more data records from the database.
     *
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter
     * is null.
     *
     * @return Returns the query result.
     */
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri,
        const std::vector<std::string> &columns,
        const NativeRdb::DataAbilityPredicates &predicates) override;
    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method
     * should be implemented by a Data ability. Data abilities supports general data types, including text,
     * HTML, and JPEG.
     *
     * @param uri Indicates the URI of the data.
     *
     * @return Returns the MIME type that matches the data specified by uri.
     */
    virtual std::string GetType(const Uri &uri) override;
    /**
     * @brief Deletes one or more data records. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the database table storing the data to delete.
     * @param predicates Indicates filter criteria. If this parameter is null, all data records will be
     * deleted by default.
     *
     * @return Returns the number of data records deleted.
     */
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;
    /**
     * @brief Opens a file. This method should be implemented by a Data ability.
     *
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing
     * file, "wa" for write-only access to append to any existing data, "rw" for read and write access on any
     * existing data, or "rwt" for read and write access that truncates any existing file.
     *
     * @return Returns the FileDescriptor object of the file descriptor.
     */
    virtual int OpenFile(const Uri &uri, const std::string &mode) override;
    /**
     *  lifecycle callback
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
              const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
              std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
              const sptr<IRemoteObject> &token) override;
    /**
     *  A FileShareAbility can only return a uri for file paths defined in their
     * meta-data element.
     *
     * @param context Pointer to the global communication context structure of the server
     * @param deviceId Equipment number
     * @param authority The authority of a FileShareAbility defined in element in your app's manifest.
     * @param file A  File pointing to the filename for which you want .
     * @param displayName The filename to be displayed. This can be used if the original filename
     * is undesirable.
     * @return Uri( scheme://authority/path1/path2/path3/updateIDNumber....)
     */
    static Uri FuzzyFileToUri(AppExecFwk::Ability *context,
                              std::string &deviceId,
                              std::string &authority,
                              std::string &file,
                              std::string &displayName);

private:
    AppExecFwk::Want originWant_;
    std::string mPath;
    std::vector<std::vector<std::string>> fileNames;
    std::vector<std::string> fileName;
    static std::unique_ptr<FileShareAbility> mInstance;
    const std::string SchemeOhos = "dataability";
    std::shared_ptr<AppExecFwk::AbilityInfo> mAbilityInfo = nullptr;
    std::vector<std::string> types_;
    static SharedPathStrategy GetPathStrategy(const AppExecFwk::Ability *context,
                                              const std::string &authority);
    static SharedPathStrategy ParsePathStrategy(const AppExecFwk::Ability *context,
                                                const std::string &authority);
    static SharedPathStrategy ParsePathStrategy();
    SharedPathStrategy mStrategy;
};
} // namespace FileManager
} // namespace OHOS
#endif // FILE_SHARE_ABILITY_H