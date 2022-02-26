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

#include "constant.h"
#include <dirent.h>
#include <unistd.h>
#include <cerrno>

namespace OHOS {
namespace DistributedKv {
// the Key Prefix for Meta data of KvStore.
const std::string KvStoreMetaRow::KEY_PREFIX = "KvStoreMetaData";

const std::string SecretMetaRow::KEY_PREFIX = "SecretKey";

/* version for distributed kv data service. */
const std::string Constant::VERSION = "1";

/* meta name for distributed kv data service. */
const std::string Constant::META_DIR_NAME = "Meta";

/* name for distributed kv data service. */
const std::string Constant::SERVICE_NAME = "mdds";

/* root path for distributed kv data service. */
const std::string Constant::ROOT_PATH = "/data/misc_de/0";

/* root path for distributeddata service and system services. */
const std::string Constant::ROOT_PATH_DE = "/data/misc_de/0";

/* root path for self-developed and non-self-developed app. */
const std::string Constant::ROOT_PATH_CE = "/data/misc_ce/0";

// the max length for key is 1024.
const size_t Constant::MAX_KEY_LENGTH = 1024;

// the max length for StoreId is 128.
const size_t Constant::MAX_STORE_ID_LENGTH = 128;

// the max length for value is 4M.
const size_t Constant::MAX_VALUE_LENGTH = 4 * 1024 * 1024;

// the max batch for putBatch is 128.
const size_t Constant::MAX_BATCH_SIZE = 128;

// the max capacity for ipc is 800K.
const size_t Constant::MAX_IPC_CAPACITY = 800 * 1024;

// the default mode is 0755, stands for r/w/x for user, r/x for group, r/x for others.
const mode_t Constant::DEFAULT_MODE = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

// the mode for dir is 0755, r/w/x for user, r/-/x for group, r/-/x for others.
const mode_t Constant::DEFAULT_MODE_DIR = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

// the mode for file is 0600, r/w/- for user, -/-/- for group, -/-/- for others.
const mode_t Constant::DEFAULT_MODE_FILE = S_IRUSR | S_IWUSR;

// Size threshold of switching to large data is a little smaller than MAX_IPC_CAPACITY.
const int Constant::SWITCH_RAW_DATA_SIZE = 700 * 1024;

const int Constant::MAX_OPEN_KVSTORES = 16;

// default group id for synchronization.
const std::string Constant::DEFAULT_GROUP_ID = "default";

// true indicates the ownership of distributed data is DEVICE, otherwise, ACCOUNT
const bool Constant::STOREID_ONLY_FLAG = true;

// service meta db name.
const std::string Constant::SERVICE_META_DB_NAME = "service_meta";

const std::string Constant::KEY_SEPARATOR = "###";

const std::string Constant::PROCESS_LABEL = "distributeddata";

const std::string Constant::ROOT_KEY_GENERATED = "RootKeyGenerated";

std::vector<uint8_t> KvStoreMetaRow::GetKeyFor(const std::string &key)
{
    std::string str = Constant::Concatenate({KvStoreMetaRow::KEY_PREFIX, Constant::KEY_SEPARATOR, key });
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::vector<uint8_t> SecretMetaRow::GetKeyFor(const std::string &key)
{
    std::string str = Constant::Concatenate({SecretMetaRow::KEY_PREFIX, Constant::KEY_SEPARATOR, key });
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string Constant::Concatenate(std::initializer_list<std::string> stringList)
{
    std::string result;
    size_t result_size = 0;
    for (const std::string &str : stringList) {
        result_size += str.size();
    }
    result.reserve(result_size);
    for (const std::string &str : stringList) {
        result.append(str.data(), str.size());
    }
    return result;
}

std::string Constant::GetDefaultDeviceAccountId()
{
    return "0";
}

std::string Constant::GetDefaultHarmonyAccountName()
{
    return "default";
}
}  // namespace DistributedKv
}  // namespace OHOS
