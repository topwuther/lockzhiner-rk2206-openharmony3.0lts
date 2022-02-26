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

#ifndef DISTRIBUTEDDATAMGR_DFX_CODE_CONSTANT_H
#define DISTRIBUTEDDATAMGR_DFX_CODE_CONSTANT_H

class DfxCodeConstant {
public:
    static inline const int SERVICE_FAULT = 950001100;
    static inline const int RUNTIME_FAULT = 950001101;
    static inline const int DATABASE_FAULT = 950001102;
    static inline const int COMMUNICATION_FAULT = 950001103;
    static inline const int DATABASE_STATISTIC = 950001104;
    static inline const int VISIT_STATISTIC = 950001105;
    static inline const int TRAFFIC_STATISTIC = 950001106;
    static inline const int DATABASE_PERFORMANCE_STATISTIC = 950001107;
    static inline const int API_PERFORMANCE_STATISTIC = 950001110;
    static inline const int API_PERFORMANCE_INTERFACE = 950001111;
};
#endif // DISTRIBUTEDDATAMGR_DFX_CODE_CONSTANT_H
