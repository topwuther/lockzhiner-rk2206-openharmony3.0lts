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

#ifndef CLIENT_STUB_H
#define CLIENT_STUB_H
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "gtest/gtest.h"
#include "log.h"
#include "update_client.h"
#include "update_helper.h"

constexpr int32_t TEST_NVALUE_TYPE_CONTEXT = 1000;
constexpr int32_t TEST_NVALUE_TYPE_UPGRADE = 1001;
constexpr int32_t TEST_NVALUE_TYPE_UPDATE_POLICY = 1002;

class TestNApiValue {
public:
    TestNApiValue(int type, const void *value);
    ~TestNApiValue() {}

    int GetType()
    {
        return type_;
    }
public:
    int type_;
    bool bValue;
    int intValue;
    int64_t int64Value;
    std::string strValue;
};

struct TestNApiEnv {
    int testStage = 0;
    bool testAsyncorPermose = true;
    int engineType = 0;
    bool noneClient = false;
    OHOS::update_engine::UpdateContext clientContext;
    OHOS::update_engine::VersionInfo pakcageInfo;
    OHOS::update_engine::Progress progress;
    std::string errorMsg;
    std::string eventType;
    // Save the applied memory and release it at last
    std::vector<TestNApiValue*> testValueList;
};

updateClient::UpdateSession *TestGetUpdateSession();
void FreeAllNapiValues();
#endif // CLIENT_STUB_H
