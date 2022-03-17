/*
 * Copyright (c) 2022 FuZhou Lockzhiner Electronic Co., Ltd. All rights reserved.
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

/**
 * @addtogroup Lockzhiner
 *
 * @file LZ_HARDWARE_errno.h
 */

#ifndef LZ_HARDWARE_ERRNO_H
#define LZ_HARDWARE_ERRNO_H

/**
 * @brief Defines a module-level return value to indicate a successful operation.
 *
 */
#define LZ_HARDWARE_SUCCESS    0

/**
 * @brief Defines a module-level return value to indicate an operation failure.
 *
 */
#define LZ_HARDWARE_FAILURE   (1)

/**
 * @brief Defines a module-level return value to indicate param invalid.
 *
 */
#define LZ_HARDWARE_INVAILD_PARAMS    (2)

#define LZ_HARDWARE_ALREADY_INIT      (3)
#endif
/** @} */
