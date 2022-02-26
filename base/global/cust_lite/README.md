# cust<a name="EN-US_TOPIC_0000001126254525"></a>

-   [Introduction](#section1881113251316)
-   [Directory Structure](#section196561842161316)
-   [Usage](#section1799421112165)
-   [Constraints](#section1811111510182)
-   [Repositories Involved](#section170262901818)

## Introduction<a name="section1881113251316"></a>

The customization framework, namely, cust, provides APIs for each service module to obtain the configuration directories at different levels or the configuration file paths.

## Directory Structure<a name="section196561842161316"></a>

The directory structure for the customization framework is as follows:

```
/base/global/
├── cust_lite                 # Code repository for the customization framework
│   ├── frameworks            # Core code of the customization framework
│   │   ├── cust_lite         # Customization framework
│   │   │   ├── src           # Implementation code
│   │   │   └── test          # Test code
│   ├── interfaces            # APIs of the customization framework
│   │   └── innerkits         # APIs of the customization framework for internal subsystems
```

## Usage<a name="section1799421112165"></a>

Call the APIs of the customization framework to obtain the configuration directories at different levels or the configuration file paths.

```
#include <gtest/gtest.h>
#include "cust_utils.h"

const char *testPathSuffix = "user.xml"; // Set the name of the configuration file.
char buf[MAX_PATH_LEN];
char *filePath = GetOneCfgFile(testPathSuffix, CUST_TYPE_CONFIG, buf, MAX_PATH_LEN); // Obtain the path of the configuration file with the highest priority.
```

## Constraints<a name="section1811111510182"></a>

**Programming language**: C/C++

## Repositories Involved<a name="section170262901818"></a>

[Globalization subsystem](https://gitee.com/openharmony/docs/blob/master/en/readme/globalization.md)

[global\_resmgr\_lite](https://gitee.com/openharmony/global_resmgr_lite/blob/master/README.md)

[global\_i18n\_lite](https://gitee.com/openharmony/global_i18n_lite/blob/master/README.md)

global\_cust\_lite

