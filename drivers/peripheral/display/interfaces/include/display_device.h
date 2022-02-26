/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
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
 * @addtogroup Display
 * @{
 *
 * @brief Defines driver functions of the display module.
 *
 * This module provides driver functions for the graphics subsystem, including graphics layer management,
 * device control, graphics hardware acceleration, display memory management, and callbacks.
 * @since 1.0
 * @version 2.0
 */

 /**
 * @file display_device.h
 *
 * @brief Declares control functions of the display device.
 *
 * @since 1.0
 * @version 2.0
 */

#ifndef DISPLAY_DEVICE_H
#define DISPLAY_DEVICE_H
#include "display_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Defines pointers to the functions of the display device.
 */
typedef struct {
    /**
     * @brief Sets the power status.
     *
     * When the OS enters the sleep mode or wakes up from the sleep mode, the display service or
     * the power management module can set the power status of the display device, so that the driver IC
     * of the device can normally enter the specified state.
     *
     * @param devId Indicates the ID of a display device. The value ranges from 0 to 4, where 0 indicates
     * the first display device and 4 indicates the last display device.
     * @param status Indicates the power status to set. The display service determines whether to set the
     * display device to the on or off state based on this setting. For details, see @link PowerStatus}.
     *
     * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
     * @since 1.0
     * @version 1.0
     */
    int32_t (*SetDisplayPowerStatus)(uint32_t devId, DispPowerStatus status);

    /**
     * @brief Obtains the power status.
     *
     * You can use this function to obtain the power status of the display device specified by devId.
     *
     * @param devId Indicates the ID of a display device. The value ranges from 0 to 4, where 0 indicates
     * the first display device and 4 indicates the last display device.
     * @param status Indicates the power status of the display device specified by devId. For details,
     * see {@link PowerStatus}.
     *
     * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
     * @since 1.0
     * @version 1.0
     */
    int32_t (*GetDisplayPowerStatus)(uint32_t devId, DispPowerStatus *status);

    /**
     * @brief Sets the backlight level.
     *
     * You can use this function to set the backlight level of the display device specified by devId.
     *
     * @param devId Indicates the ID of a display device. The value ranges from 0 to 4, where 0 indicates
     * the first display device and 4 indicates the last display device.
     * @param level Indicates the backlight level to set.
     *
     * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
     * @since 1.0
     * @version 1.0
     */
    int32_t (*SetDisplayBacklight)(uint32_t devId, uint32_t level);

    /**
     * @brief Obtains the backlight level.
     *
     * You can use this function to obtain the backlight level of the display device specified by devId.
     *
     * @param devId Indicates the ID of a display device. The value ranges from 0 to 4, where 0 indicates
     * the first display device and 4 indicates the last display device.
     * @param level Indicates the backlight level.
     *
     * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
     * @since 1.0
     * @version 1.0
     */
    int32_t (*GetDisplayBacklight)(uint32_t devId, uint32_t *level);
} DeviceFuncs;

/**
 * @brief Initializes the control functions of the display device. You can apply for the resources for
 * using control functions and then operate the display device by using the control functions.
 *
 * @param funcs Indicates the double pointer to the control functions of the display device. The caller obtains
 * the double pointer to operate the display device. The memory is allocated during initialization, and therefore
 * the caller does not need to allocate the memory.
 *
 * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
 *
 * @since 1.0
 * @version 1.0
 */
int32_t DeviceInitialize(DeviceFuncs **funcs);

/**
 * @brief Uninitializes control functions of the display device. The resources used by
 * the control functions will be released.
 *
 * @param funcs Indicates the double pointer to control functions of the display device. It is used to release
 * the memory allocated during initialization of the control functions.
 *
 * @return Returns 0 if the operation is successful; returns an error code defined in {@link DispErrCode} otherwise.
 * @since 1.0
 * @version 1.0
 */
int32_t DeviceUninitialize(DeviceFuncs *funcs);

#ifdef __cplusplus
}
#endif
#endif
/* @} */