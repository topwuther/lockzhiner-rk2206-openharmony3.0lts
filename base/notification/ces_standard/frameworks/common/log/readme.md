/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

## How To Use Log

# C++
1. Add a statement to your gn file

```
    deps = [
        "${ces_common_path}/
        log:eventfwk_log_source_set",
    ]

    defines = [
        "EVENT_LOG_TAG = \"eventfwk_Core\"",
        "LOG_DOMAIN = 0xD001110",
    ]
```
2. Include header file `#include "event_log_wrapper.h"`
3. Control log print level `EventLogWrapper::SetLogLevel(EventLogLevel::DEBUG);` default is DEBUG
4. Example output format `[fileName(functionName)] string`
```
    // dynamic control log level
    EventLogWrapper::SetLogLevel
    (EventLogLevel::FATAL);

    // The following log statement will not print
    EVENT_LOGD("one %{public}d", 1234);
    EVENT_LOGI("two %{public}s", "1234");
    EVENT_LOGW("three %{private}s", "1234");
    EVENT_LOGE("four %{private}s", "1234");

    // The following log statement will print
    EventLogWrapper::SetLogLevel
    (EventLogLevel::DEBUG);
    EVENT_LOGD("five %{public}d", 1234);
    EVENT_LOGI("six %{public}s", "1234");
    EVENT_LOGW("seven %{private}s", "1234");
    EVENT_LOGE("eight %{private}s", "1234");
```

# Java

1. import dependent in your class
```
import ohosos.eventfwk.utils.eventLog;
import ohosos.hiviewdfx.HiLogLabel;
```

2. custom HiLogLabel (if you don't custom lable, the default will be used)
```
private static final HiLogLabel LABEL = new HiLogLabel(HiLog.LOG_CORE, 0xD001110, "eventKit");
```

3. use EventLog interface
```
Event_Log.d(LABEL, "log %{public}s", "123");
Event_Log.i("log %{public}d", 123);
```