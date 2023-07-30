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
 * @addtogroup lockzhiner
 *
 * @file lz_hardware.h
 *
 */
#ifndef _LZ_HARDWARE_H_
#define _LZ_HARDWARE_H_

#include "lz_hardware/base.h"
#include "lz_hardware/device.h"
#include "lz_hardware/log.h"
#include "lz_hardware/errno.h"
#include "lz_hardware/gpio.h"
#include "lz_hardware/pinctrl.h"
#include "lz_hardware/delay.h"
#include "lz_hardware/file.h"
#include "lz_hardware/flash.h"
#include "lz_hardware/i2c.h"
#include "lz_hardware/spi.h"
#include "lz_hardware/pwm.h"
#include "lz_hardware/uart.h"
#include "lz_hardware/interrupt.h"
#include "lz_hardware/thread.h"
#include "lz_hardware/rand.h"
#include "lz_hardware/mutex.h"
#include "lz_hardware/spinlock.h"
#include "lz_hardware/wait.h"
#include "lz_hardware/saradc.h"
#include "lz_hardware/wifi.h"
#include "lz_hardware/vendor.h"
#include "lz_hardware/watchdog.h"
#include "lz_hardware/efuse.h"

unsigned int HalInit(void);
#endif
