# HiCollie<a name="EN-US_TOPIC_0000001077200880"></a>

-   [Introduction](#section11660541593)
-   [Architecture](#section342962219551)
-   [Directory Structure](#section55125489224)
-   [Constraints](#section161941989596)
-   [Compilation and Building](#section20568163942320)
-   [Available APIs](#section8725142134818)
-   [Usage](#section19959125052315)
-   [Repositories Involved](#section1134984213235)

## Introduction<a name="section11660541593"></a>

HiCollie provides the software watchdog capability. It implements a unified framework for fault detection and fault log generation to help you locate software timeout faults resulting from system service deadlock, application main thread blocking, and service process timeout. 

## Architecture<a name="section342962219551"></a>

## Directory Structure<a name="section55125489224"></a>

```
/base/hiviewdfx/hicollie                       #  HiCollie home directory
├── frameworks/native/                       # HiCollie Native code
├── interfaces/native/innerkits/include      # HiCollie Native header files
```

## Constraints<a name="section161941989596"></a>

A maximum of 128 timers can be registered for a single process using the HiCollie API. Excess timers are invalid and cannot be registered to implement timeout detection.

## Compilation and Building<a name="section20568163942320"></a>

For details, see the  _HiCollie Development Guide_.

## Available APIs<a name="section8725142134818"></a>

## Repositories Involved<a name="section1134984213235"></a>

[Hivew](https://gitee.com/openharmony)

