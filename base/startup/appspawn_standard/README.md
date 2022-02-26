# Startup<a name="EN-US_TOPIC_0000001078883578"></a>

-   [Introduction](#section11660541593)
-   [Directory Structure](#section161941989596)
-   [Repositories Involved](#section1371113476307)

## Introduction<a name="section11660541593"></a>

Appspawn is responsible for creating application process and setting process information function.

## Directory Structure<a name="section161941989596"></a>

```
base/startup/appspawn_standard
├── include                    # include directory
├── parameter                  # system parameters
├── src                        # source code
│   └── socket                 # encapsulation of socket basic library
└── test                       # test code
```

## Repositories Involved<a name="section1371113476307"></a>

Startup subsystem

hmf/startup/syspara\_lite

**hmf/startup/appspawn_standard**

hmf/startup/appspawn\_lite

hmf/startup/bootstrap\_lite

hmf/startup/startup

hmf/startup/systemrestore

