/*
 * pwm_hi35xx.h
 *
 * hi35xx pwm driver implement.
 *
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef PWM_HI35XX_H
#define PWM_HI35XX_H

#include "hdf_base.h"

#define PWM_CLK_HZ     3000000 // 3MHz
#define PWM_CLK_PERIOD 333     // 333ns

#define PWM_MAX_HZ     1500000 // 1.5MHz
#define PWM_MIN_PERIOD 666     // 666ns

#define PWM_MIN_HZ     0.045       // 0.045Hz
#define PWM_MAX_PERIOD 22222222222 // 22222222222ns > 4294967295ns (UINT32_MAX)

#define PWM_ENABLE  1
#define PWM_DISABLE 0

#define PWM_INV_OFFSET  1
#define PWM_KEEP_OFFSET 2

#define PWM_DEFAULT_PERIOD     0x018F
#define PWM_DEFAULT_POLARITY   0
#define PWM_DEFAULT_DUTY_CYCLE 0x00C7

struct HiPwmRegs {
    volatile uint32_t cfg0;
    volatile uint32_t cfg1;
    volatile uint32_t cfg2;
    volatile uint32_t ctrl;
    volatile uint32_t state0;
    volatile uint32_t state1;
    volatile uint32_t state2;
};

static inline void HiPwmDisable(struct HiPwmRegs *reg)
{
    if (reg == NULL) {
        return;
    }
    reg->ctrl = PWM_DISABLE;
}

static inline void HiPwmAlwaysOutput(struct HiPwmRegs *reg)
{
    if (reg == NULL) {
        return;
    }
    /* keep the pwm always output */
    reg->ctrl |= ((1 << PWM_KEEP_OFFSET) | PWM_ENABLE);
}

static inline void HiPwmOutputNumberSquareWaves(struct HiPwmRegs *reg, uint32_t number)
{
    uint32_t mask;

    if (reg == NULL) {
        return;
    }
    /* pwm output number square waves */
    reg->cfg2 = number;
    mask = ~(1 << PWM_KEEP_OFFSET);
    reg->ctrl &= mask;
    reg->ctrl |= PWM_ENABLE;
}

static inline void HiPwmSetPolarity(struct HiPwmRegs *reg, uint8_t polarity)
{
    uint32_t mask;

    if (reg == NULL) {
        return;
    }
    mask = ~(1 << PWM_INV_OFFSET);
    reg->ctrl &= mask;
    reg->ctrl |= (polarity << PWM_INV_OFFSET);
}

static inline void HiPwmSetPeriod(struct HiPwmRegs *reg, uint32_t period)
{
    if (reg == NULL) {
        return;
    }
    reg->cfg0 = period / PWM_CLK_PERIOD;
}

static inline void HiPwmSetDuty(struct HiPwmRegs *reg, uint32_t duty)
{
    if (reg == NULL) {
        return;
    }
    reg->cfg1 = duty / PWM_CLK_PERIOD;
}

#endif /* PWM_HI35XX_H */
