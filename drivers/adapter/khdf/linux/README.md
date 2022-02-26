# linux\_khdf<a name="EN-US_TOPIC_0000001078489630"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

This repository stores the code and compilation scripts for the OpenHarmony driver subsystem to adapt to the Linux kernel and to deploy the hardware driver foundation \(HDF\).

## Directory Structure<a name="section161941989596"></a>

```
/drivers/adapter/khdf/linux
├── config               # Compilation scripts for building and configuring the parsing code
├── hcs                  # HDF configuration and management
├── manager              # Code for starting and adapting to the HDF
├── model                # Code for adapting to Linux
│   ├── display          # Display driver model
│   ├── input            # Input driver model
│   ├── network          # WLAN driver model
│   ├── sensor           # Sensor driver model
├── network              # Code for adapting to the Linux kernel network
├── osal                 # POSIX APIs for adapting to the Linux kernel
├── platform             # Code for adapting the platform APIs to the Linux kernel
│   ├── emmc             # EMMC APIs
│   ├── gpio             # GPIO APIs
│   ├── i2c              # I2C APIs
│   ├── mipi_dsi         # MIPI DSI APIs
│   ├── pwm              # PWM APIs
│   ├── rtc              # RTC APIs
│   ├── sdio             # SDIO APIs
│   ├── spi              # SPI APIs
│   ├── uart             # UART APIs
│   └── watchdog         # WATCHDOG APIs
```

## Repositories Involved<a name="section1371113476307"></a>

[Driver subsystem](https://gitee.com/openharmony/docs/blob/master/en/readme/driver-subsystem.md)

[drivers\_framework](https://gitee.com/openharmony/drivers_framework/blob/master/README.md)

[drivers\_adapter](https://gitee.com/openharmony/drivers_adapter/blob/master/README.md)

drivers\_adapter\_khdf\_linux

[drivers\_peripheral](https://gitee.com/openharmony/drivers_peripheral/blob/master/README.md)

