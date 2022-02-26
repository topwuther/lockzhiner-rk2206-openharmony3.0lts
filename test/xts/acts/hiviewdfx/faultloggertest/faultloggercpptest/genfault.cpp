/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef TEST_COMMON_H
#define TEST_COMMON_H
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

typedef enum {
    FAULT_HEAP = 1,
    FAULT_STACK = 2,
    FAULT_NULLPTR = 3,
}FaultType;

void FaultHeap(int value)
{
    
    int* arraytest = new int[123];
    delete []arraytest;
    printf("%d", arraytest[value]);
}

void FaultStack(int value)
{
    int arraytest[123];
    printf("value of i is %d", arraytest[value]);
}

void FaultNullPointer()
{
    const int num = 80;
    int *i = nullptr;
    *i = num;
}

void GenFault(int faulttype)
{
    printf("genFault, param is %d\r\n", faulttype);
    switch (faulttype) {
        case FAULT_HEAP:
            FaultHeap(125);
        case FAULT_STACK:
            FaultStack(125);
        case FAULT_NULLPTR:
            FaultNullPointer();
            break;
    }
}
#endif