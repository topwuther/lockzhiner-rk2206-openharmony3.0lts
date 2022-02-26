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

#ifndef STARTUP_SYSPARAM_PARAMETER_API_H
#define STARTUP_SYSPARAM_PARAMETER_API_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/**
 * @brief Obtains a system parameter matching the specified <b>key</b>.
 *
 * If no system parameter is found, the <b>def</b> parameter will be returned.\n
 *
 * @param key Indicates the key for the system parameter to query.
 * The value can contain lowercase letters, digits, underscores (_), and dots (.).
 * Its length cannot exceed 32 bytes (including the end-of-text character in the string).
 * @param def Indicates the default value to return when no query result is found.
 * This parameter is specified by the caller.
 * @param value Indicates the data buffer that stores the query result.
 * This parameter is applied for and released by the caller and can be used as an output parameter.
 * @param len Indicates the length of the data in the buffer.
 * @return Returns the number of bytes of the system parameter if the operation is successful;
 * returns <b>-9</b> if a parameter is incorrect; returns <b>-1</b> in other scenarios.
 * @since 1
 * @version 1
 */
int GetParameter(const char *key, const char *def, char *value, unsigned int len);

/**
 * @brief Sets or updates a system parameter.
 *
 * You can use this function to set a system parameter that matches <b>key</b> as <b>value</b>.\n
 *
 * @param key Indicates the key for the parameter to set or update.
 * The value can contain lowercase letters, digits, underscores (_), and dots (.).
 * Its length cannot exceed 32 bytes (including the end-of-text character in the string).
 * @param value Indicates the system parameter value.
 * Its length cannot exceed 128 bytes (including the end-of-text character in the string).
 * @return Returns <b>0</b> if the operation is successful;
 * returns <b>-9</b> if a parameter is incorrect; returns <b>-1</b> in other scenarios.
 * @since 1
 * @version 1
 */
int SetParameter(const char *key, const char *value);

const char *GetSecurityPatchTag();
const char *GetOSFullName();
const char *GetVersionId();
const char *GetBuildRootHash();
const char *GetOsReleaseType();
int GetSdkApiVersion();

const char *GetDeviceType();
const char *GetProductModel();
const char *GetManufacture();
const char *GetBrand();
const char *GetMarketName();
const char *GetProductSeries();
const char *GetSoftwareModel();
const char *GetHardwareModel();
const char *GetHardwareProfile();
const char *GetSerial();
const char *GetAbiList();
const char *GetDisplayVersion();
const char *GetIncrementalVersion();
const char *GetBootloaderVersion();
const char *GetBuildType();
const char *GetBuildUser();
const char *GetBuildHost();
const char *GetBuildTime();
int GetFirstApiVersion();
int GetDevUdid(char *udid, int size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // STARTUP_SYSPARAM_PARAMETER_API_H
