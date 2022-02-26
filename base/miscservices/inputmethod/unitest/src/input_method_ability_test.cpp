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
#include <functional>
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <sys/time.h>
#include <thread>
#include <string>
#include "global.h"
#include "input_method_ability.h"
#include "input_method_agent_stub.h"
#include "input_method_core_stub.h"
#include "input_control_channel_stub.h"
#include "input_attribute.h"
#include "message_handler.h"

using namespace testing::ext;
namespace OHOS {
namespace MiscServices {
    class InputMethodAbilityTest : public testing::Test {
    public:
        static void SetUpTestCase(void);
        static void TearDownTestCase(void);
        void SetUp();
        void TearDown();
    };

    void InputMethodAbilityTest::SetUpTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUpTestCase");
    }

    void InputMethodAbilityTest::TearDownTestCase(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDownTestCase");
    }

    void InputMethodAbilityTest::SetUp(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::SetUp");
    }

    void InputMethodAbilityTest::TearDown(void)
    {
        IMSA_HILOGI("InputMethodAbilityTest::TearDown");
    }

    /**
    * @tc.name: testReadWriteIInputMethodAgent
    * @tc.desc: Checkout IInputMethodAgent.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputMethodAgent, TestSize.Level0)
    {
        sptr<InputMethodAgentStub> mInputMethodAgentStub = new InputMethodAgentStub();
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputMethodAgentStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputMethodAgent> iface = iface_cast<IInputMethodAgent>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
    * @tc.name: testReadWriteIInputMethodCore
    * @tc.desc: Checkout IInputMethodCore.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputMethodCore, TestSize.Level0)
    {
        sptr<InputMethodCoreStub> mInputMethodCoreStub = new InputMethodCoreStub(0);
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputMethodCoreStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputMethodCore> iface = iface_cast<IInputMethodCore>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
    * @tc.name: testReadWriteIInputControlChannel
    * @tc.desc: Checkout IInputControlChannel.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testReadWriteIInputControlChannel, TestSize.Level0)
    {
        sptr<InputControlChannelStub> mInputControlChannelStub = new InputControlChannelStub(0);
        MessageParcel data;
        auto ret =  data.WriteRemoteObject(mInputControlChannelStub->AsObject());
        EXPECT_TRUE(ret);
        auto remoteObject = data.ReadRemoteObject();
        sptr<IInputControlChannel> iface = iface_cast<IInputControlChannel>(remoteObject);
        EXPECT_TRUE(iface != nullptr);
    }

    /**
    * @tc.name: testGetInputMethodAbilityInstance
    * @tc.desc: Checkout the function of getInstance.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testGetInputMethodAbilityInstance, TestSize.Level0)
    {
        auto ima = InputMethodAbility::GetInstance();
        EXPECT_TRUE(ima != nullptr);
    }

    /**
    * @tc.name: testSerializedInputAttribute
    * @tc.desc: Checkout the serialization of InputAttribute.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testSerializedInputAttribute, TestSize.Level0)
    {
        sptr<InputAttribute> mInputAttribute = new InputAttribute();
        mInputAttribute->SetInputPattern(InputAttribute::PATTERN_PASSWORD);
        MessageParcel data;
        auto ret =  data.WriteParcelable(mInputAttribute);
        EXPECT_TRUE(ret);
        sptr<InputAttribute> deserialization = data.ReadParcelable<InputAttribute>();
        EXPECT_TRUE(deserialization != nullptr);
        EXPECT_TRUE(deserialization->GetSecurityFlag());
    }

    /**
    * @tc.name: testSerializedKeyboardType
    * @tc.desc: Checkout the serialization of KeyboardType.
    * @tc.type: FUNC
    */
    HWTEST_F(InputMethodAbilityTest, testSerializedKeyboardType, TestSize.Level0)
    {
        int32_t def_value = 2021;
        sptr<KeyboardType> mKeyboardType = new KeyboardType();
        mKeyboardType->setId(def_value);
        MessageParcel data;
        auto ret =  data.WriteParcelable(mKeyboardType);
        EXPECT_TRUE(ret);
        sptr<KeyboardType> deserialization = data.ReadParcelable<KeyboardType>();
        EXPECT_TRUE(deserialization != nullptr);
        EXPECT_TRUE(deserialization->getId() == def_value);
    }
}
}