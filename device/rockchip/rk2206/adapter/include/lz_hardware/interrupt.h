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
 * @file interrupt.h
 */

#ifndef LZ_HARDWARE_INTERRUPT_H
#define LZ_HARDWARE_INTERRUPT_H

typedef void (*ProcFunc)(void *arg);

/**
 * @brief Create a hardware interrupt.
 *
 *
 *
 * @param irq Indicates hardware interrupt number.
 * @param irq Indicates interrupt handler used when a hardware interrupt is triggered.
 * @param arg Indicates input parameter of the interrupt handler used when a hardware interrupt is triggered.
 * @return Returns {@link HAL_SUCCESS} if the irq is requested successfully;
 * returns {@link HAL_FAILURE} otherwise. For details about other return values.
 */
int RequestIrq(unsigned int irq, ProcFunc isr, void *arg);

/**
 * @brief Delete hardware interrupt.
 *
 *
 *
 * @param irq Indicates hardware interrupt number.
 * @return Returns {@link HAL_SUCCESS} if the irq is free successfully;
 * returns {@link HAL_FAILURE} otherwise. For details about other return values, see the chip description.
 */

int FreeIrq(unsigned int irq);

#endif
/** @} */
