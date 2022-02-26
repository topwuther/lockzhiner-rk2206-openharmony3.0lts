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

#ifndef DATABASE_OPER_H
#define DATABASE_OPER_H

#include <string>

#include "kvdb_properties.h"
#include "generic_kvdb.h"

namespace DistributedDB {
class DatabaseOper {
public:
    virtual ~DatabaseOper() {};

    virtual int Rekey(const CipherPassword &passwd) = 0;

    virtual int Import(const std::string &filePath, const CipherPassword &passwd) = 0;

    virtual int Export(const std::string &filePath, const CipherPassword &passwd) const = 0;

    void SetLocalDevId(const std::string &deviceId);

    int RekeyRecover(const KvDBProperties &property);

    int ClearImportTempFile(const KvDBProperties &property) const;

    int ClearExportedTempFiles(const KvDBProperties &property) const;

protected:
    int ExecuteRekey(const CipherPassword &passwd, const KvDBProperties &property);

    virtual bool RekeyPreHandle(const CipherPassword &passwd, int &errCode) = 0;

    virtual int BackupDb(const CipherPassword &passwd) const = 0;

    virtual int CloseStorages() = 0;

    virtual int RekeyPostHandle(const CipherPassword &passwd) = 0;

    int GetCtrlFilePrefix(const KvDBProperties &property, std::string &filePrefix) const;

    virtual int ExportAllDatabases(const std::string &currentDir, const CipherPassword &passwd,
        const std::string &dbDir) const = 0;

    static int RemoveFile(const std::string &fileName);

    // import begin
    int ExecuteImport(const std::string &filePath, const CipherPassword &passwd, const KvDBProperties &property) const;

    virtual int BackupCurrentDatabase(const ImportFileInfo &info) const = 0;

    virtual int ImportUnpackedDatabase(const ImportFileInfo &info, const CipherPassword &srcPasswd) const = 0;

    virtual int ImportPostHandle() const = 0;

    // export begin
    int ExecuteExport(const std::string &filePath, const CipherPassword &passwd, const KvDBProperties &property) const;

private:
    int CreateStatusCtrlFile(const KvDBProperties &property, std::string &orgCtrlFile, std::string &newCtrlFile);

    static int RenameStatusCtrlFile(const std::string &orgCtrlFile, const std::string &newCtrlFile);

    int RecoverPrehandle(int dbType, const std::string &dir, const std::string &fileName);

    int RemoveDbDir(const std::string &dir, int dbType, bool isNeedDelDir = true);

    static int GetWorkDir(const KvDBProperties &property, std::string &workDir);

    int RemoveDbFiles(const std::string &dir, const std::vector<std::string> &dbNameList, bool isNeedDelDir = true);

    static void InitImportFileInfo(ImportFileInfo &info, const KvDBProperties &property);

    int UnpackAndCheckImportedFile(const std::string &srcFile, const ImportFileInfo &info,
        const KvDBProperties &property) const;

    int RecoverImportedBackFiles(const std::string &dir, const std::string &fileName, int dbType) const;

    int RemoveImportedBackFiles(const std::string &backupDir, const std::string &ctrlFileName, int dbType) const;

    int PackExportedDatabase(const std::string &fileDir, const std::string &packedFile,
        const KvDBProperties &property) const;

    int CheckSecurityOption(const std::string &filePath, const KvDBProperties &property) const;

    int CreateBackupDirForExport(const KvDBProperties &property, std::string &currentDir, std::string &backupDir) const;

    std::string deviceId_;
};
} // namespace DistributedDB

#endif // DATABASE_OPER_H