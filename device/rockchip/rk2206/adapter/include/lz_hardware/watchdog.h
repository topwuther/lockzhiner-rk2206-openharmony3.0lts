#ifndef _LZ_HARDWARE_WATCHDOG_H_
#define _LZ_HARDWARE_WATCHDOG_H_

/* watchdog reboot mode */
enum {
    LZ_WATCHDOG_REBOOT_MODE_FIRST = 0,  /* Immediately reset system if WDT timeout */
    LZ_WATCHDOG_REBOOT_MODE_SECOND,     /* First generate an interrupt and if the interrupt is not cleard by the time, a second timeout occur then generate a system reset */
    LZ_WATCHDOG_REBOOT_MODE_MAX
};

/* watchdog init */
unsigned int LzWatchdogInit();

/* 以1.3981013秒为档位，每个档位以2^X递进，如果你设置为2，则实际配置为1.3981013 * (2 ^ 1)
 * 1档位：1.3981013 * (2 ^ 0)
 * 2档位：1.3981013 * (2 ^ 1)
 * .......
 */
unsigned int LzWatchdogSetTimeout(unsigned int timeout_sec);

/* Get watchdog timeout, uint: sec */
unsigned int LzWatchdogGetTimeout();

/* start watchdog */
unsigned int LzWatchdogStart(unsigned int reboot_mode);

/* watchdog freedog */
unsigned int LzWatchdogKeepAlive();

#endif /* _LZ_HARDWARE_WATCHDOG_H_ */