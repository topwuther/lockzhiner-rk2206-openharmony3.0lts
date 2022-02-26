/*
 * pwm_hi35xx_linux.c
 *
 * pwm driver of hi35xx
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

#include <asm/uaccess.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/proc_fs.h>
#include <linux/pwm.h>
#include <linux/version.h>
#include "pwm_hi35xx.h"

#define PWM_ENABLE_MASK 0x1
#define PWM_HI35XX_N_CELLS 2

struct Hi35xxPwmChip {
    struct pwm_chip chip;
    struct HiPwmRegs *reg;
    void __iomem *base;
    struct clk *clk;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static int Hi35xxPwmApply(struct pwm_chip *chip, struct pwm_device *pwm, const struct pwm_state *state)
#else
static int Hi35xxPwmApply(struct pwm_chip *chip, struct pwm_device *pwm, struct pwm_state *state)
#endif
{
    struct HiPwmRegs *reg = NULL;
    struct Hi35xxPwmChip *hi35xxChip = (struct Hi35xxPwmChip *)chip;

    if (hi35xxChip == NULL || pwm == NULL || state == NULL) {
        pr_err("%s: parameter is null\n", __func__);
        return -EINVAL;
    }
    reg = (struct HiPwmRegs *)hi35xxChip->base;
    if (state->polarity != PWM_POLARITY_NORMAL && state->polarity != PWM_POLARITY_INVERSED) {
        pr_err("%s: polarity %u is invalid", __func__, state->polarity);
        return -EINVAL;
    }

    if (state->period < PWM_MIN_PERIOD) {
        pr_err("%s: period %u is not support, min period %u", __func__, state->period, PWM_MIN_PERIOD);
        return -EINVAL;
    }
    if (state->duty_cycle < 1 || state->duty_cycle > state->period) {
        pr_err("%s: duty_cycle %u is not support, min duty_cycle 1 max duty_cycle %u",
               __func__, state->duty_cycle , state->period);
        return -EINVAL;
    }

    HiPwmDisable(reg);
    if (pwm->state.polarity != state->polarity) {
        HiPwmSetPolarity(reg, state->polarity);
    }
    if (pwm->state.period != state->period) {
        HiPwmSetPeriod(reg, state->period);
    }
    if (pwm->state.duty_cycle != state->duty_cycle) {
        HiPwmSetDuty(reg, state->duty_cycle);
    }
    if (state->enabled) {
        HiPwmAlwaysOutput(reg);
    }
    return 0;
}


static void Hi35xxGetState(struct pwm_chip *chip, struct pwm_device *pwm, struct pwm_state *state)
{
    struct HiPwmRegs *reg = NULL;
    struct Hi35xxPwmChip *hi35xxChip = (struct Hi35xxPwmChip *)chip;

    if (hi35xxChip == NULL || pwm == NULL || state == NULL) {
        pr_err("%s: parameter is null\n", __func__);
        return;
    }
    reg = (struct HiPwmRegs *)hi35xxChip->base;
    state->period = reg->cfg0;
    state->duty_cycle = reg->cfg1;
    state->enabled = reg->ctrl & PWM_ENABLE_MASK;
}

static struct pwm_ops Hi35xxPwmOps = {
    .apply = Hi35xxPwmApply,
    .get_state = Hi35xxGetState,
    .owner = THIS_MODULE,
};

static int PwmProbe(struct platform_device *pdev)
{
    int ret;
    struct resource *r = NULL;
    struct Hi35xxPwmChip *chip = NULL;
    struct device_node *np = pdev->dev.of_node;

    if (!np) {
        dev_err(&pdev->dev, "invalid devicetree node\n");
        return -EINVAL;
    }

    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (chip == NULL) {
        return -ENOMEM;
    }
    chip->chip.dev = &pdev->dev;
    chip->chip.ops = &Hi35xxPwmOps;
    chip->chip.of_xlate = NULL;
    chip->chip.of_pwm_n_cells = PWM_HI35XX_N_CELLS;
    chip->chip.base = -1;
    chip->chip.npwm = 1;
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    chip->base = devm_ioremap_resource(&pdev->dev, r);
    if (IS_ERR(chip->base)) {
        return PTR_ERR(chip->base);
    }
    chip->reg = (struct HiPwmRegs *)chip->base;
    chip->clk = devm_clk_get(&pdev->dev, NULL);
    if (IS_ERR(chip->clk)) {
        dev_err(&pdev->dev, "failed to get clock\n");
        return PTR_ERR(chip->clk);
    }
    ret = clk_prepare_enable(chip->clk);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to enable clock\n");
        return ret;
    }
    ret = pwmchip_add(&chip->chip);
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to add PWM chip\n");
        return ret;
    }

    platform_set_drvdata(pdev, chip);
    return ret;
}

static int PwmRemove(struct platform_device *pdev)
{
    int ret;
    struct Hi35xxPwmChip *chip = NULL;

    chip = platform_get_drvdata(pdev);
    if (chip == NULL) {
        return -ENODEV;
    }
    ret = pwmchip_remove(&chip->chip);
    if (ret < 0) {
        return ret;
    }
    clk_disable_unprepare(chip->clk);
    return 0;
}

static const struct of_device_id g_pwmMatch[] = {
    { .compatible = "hisilicon,pwm" },
    {},
};

static struct platform_driver g_pwmDriver = {
    .probe = PwmProbe,
    .remove = PwmRemove,
    .driver = {
        .name = "pwm",
        .of_match_table = g_pwmMatch,
    }
};
module_platform_driver(g_pwmDriver);
MODULE_LICENSE("GPL");
