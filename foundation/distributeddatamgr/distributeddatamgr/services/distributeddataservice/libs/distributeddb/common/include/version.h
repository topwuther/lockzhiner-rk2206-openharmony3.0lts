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

#ifndef VERSION_H
#define VERSION_H

#include <string>
#include <cstdint>

namespace DistributedDB {
// Version Regulation:
// Module version is always equal to the max version of its submodule.
// If a module or submodule upgrade to higher version, DO NOT simply increase current version by 1.
//      First: you have to preserve current version by renaming it as a historical version.
//      Second: Update the current version to the version to be release.
//      Finally: Update its parent module's version if exist.
// Why we update the current version to the version to be release? For example, if module A has submodule B and C,
// if now version of B is 105, and C is 101, thus version of A is 105; if now release version is 106 and we upgrade
// submodule C, if we simply change version of C to 102 then version of A is still 105, but if we change version of C
// to 106 then version of A is now 106, so we can know that something had changed for module A.
const std::string SOFTWARE_VERSION_STRING = "1.1.3"; // DistributedDB current version string.
constexpr uint32_t SOFTWARE_VERSION_BASE = 100; // Software version base value, do not change it
constexpr uint32_t SOFTWARE_VERSION_RELEASE_1_0 = SOFTWARE_VERSION_BASE + 1; // 1 for first released version
constexpr uint32_t SOFTWARE_VERSION_RELEASE_2_0 = SOFTWARE_VERSION_BASE + 2; // 2 for second released version
constexpr uint32_t SOFTWARE_VERSION_RELEASE_3_0 = SOFTWARE_VERSION_BASE + 3; // 3 for second released version
constexpr uint32_t SOFTWARE_VERSION_EARLIEST = SOFTWARE_VERSION_RELEASE_1_0;
constexpr uint32_t SOFTWARE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_3_0;
constexpr int VERSION_INVALID = INT32_MAX;

// Storage Related Version
// LocalNaturalStore Related Version
constexpr int LOCAL_STORE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;
// SingleVerNaturalStore Related Version
constexpr int SINGLE_VER_STORE_VERSION_V1 = SOFTWARE_VERSION_RELEASE_1_0;
constexpr int SINGLE_VER_STORE_VERSION_V2 = SOFTWARE_VERSION_RELEASE_2_0;
constexpr int SINGLE_VER_STORE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_3_0;
// MultiVerNaturalStore Related Version
constexpr uint32_t VERSION_FILE_VERSION_CURRENT = 1;
constexpr uint32_t MULTI_VER_STORE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;
constexpr int MULTI_VER_COMMIT_STORAGE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;
constexpr int MULTI_VER_DATA_STORAGE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;
constexpr int MULTI_VER_METADATA_STORAGE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;
constexpr int MULTI_VER_VALUESLICE_STORAGE_VERSION_CURRENT = SOFTWARE_VERSION_RELEASE_1_0;

// Syncer Related Version
constexpr int TIME_SYNC_VERSION_V1 = SOFTWARE_VERSION_RELEASE_1_0; // time sync proctol added in version 101.
constexpr int ABILITY_SYNC_VERSION_V1 = SOFTWARE_VERSION_RELEASE_2_0; // Ability sync proctol added in version 102.
constexpr uint32_t SINGLE_VER_SYNC_PROCTOL_V1 = SOFTWARE_VERSION_RELEASE_1_0; // The 1st version num
constexpr uint32_t SINGLE_VER_SYNC_PROCTOL_V2 = SOFTWARE_VERSION_RELEASE_2_0; // The 2nd version num
constexpr uint32_t SINGLE_VER_SYNC_PROCTOL_V3 = SOFTWARE_VERSION_RELEASE_3_0; // The third version num
} // namespace DistributedDB

#endif // VERSION_H