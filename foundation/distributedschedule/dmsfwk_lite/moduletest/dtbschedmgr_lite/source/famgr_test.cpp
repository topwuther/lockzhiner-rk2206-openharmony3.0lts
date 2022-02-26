/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "ability_manager.h"
#include "dmsfwk_interface.h"
#include "dmslite_famgr.h"
#include "dmslite_packet.h"
#include "dmslite_parser.h"
#include "dmslite_tlv_common.h"

#include "ohos_errno.h"
#include "securec.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedSchedule {
class FamgrTest : public testing::Test {
protected:
    static void SetUpTestCase() { }
    static void TearDownTestCase() { }
    virtual void SetUp() { }
    virtual void TearDown() { }

    static int8_t FillWant(Want *want, const char *bundleName, const char *abilityName)
    {
        if (memset_s(want, sizeof(Want), 0x00, sizeof(Want)) != EOK) {
            return DMS_EC_FAILURE;
        }
        ElementName element;
        if (memset_s(&element, sizeof(ElementName), 0x00, sizeof(ElementName)) != EOK) {
            return DMS_EC_FAILURE;
        }

        if (!(SetElementBundleName(&element, bundleName)
            && SetElementAbilityName(&element, abilityName)
            && SetWantElement(want, element))) {
            ClearElement(&element);
            ClearWant(want);
            return DMS_EC_FAILURE;
        }
        ClearElement(&element);
        return DMS_EC_SUCCESS;
    }

    static void RunTest(const uint8_t *buffer, uint16_t bufferLen,
        const TlvParseCallback onTlvParseDone, const StartAbilityCallback onStartAbilityDone)
    {
        IDmsFeatureCallback dmsFeatureCallback = {
            .onTlvParseDone = onTlvParseDone,
            .onStartAbilityDone = onStartAbilityDone
        };

        CommuMessage commuMessage;
        commuMessage.payloadLength = bufferLen;
        commuMessage.payload = buffer;

        ProcessCommuMsg(&commuMessage, &dmsFeatureCallback);
    }
};

/**
 * @tc.name: StartRemoteAbility_001
 * @tc.desc: Start remote ability with bundle name and ability name
 * @tc.type: FUNC
 * @tc.require: SR000FKTLR AR000FKVSU
 */
HWTEST_F(FamgrTest, StartRemoteAbility_001, TestSize.Level1) {
    PreprareBuild();

    Want want;
    if (FillWant(&want, "ohos.dms.example", "MainAbility") != 0) {
        return;
    }

    CallerInfo callerInfo = {
        .uid = 0
    };
    StartRemoteAbility(&want, &callerInfo, nullptr);
    auto onTlvParseDone = [] (int8_t errCode, const void *dmsMsg) {
        const TlvNode *tlvHead = reinterpret_cast<const TlvNode *>(dmsMsg);
        EXPECT_EQ(errCode, DMS_TLV_SUCCESS);
        EXPECT_EQ(UnMarshallUint16(tlvHead, COMMAND_ID), 1);
        EXPECT_EQ(std::string(UnMarshallString(tlvHead, CALLEE_BUNDLE_NAME)), "ohos.dms.example");
        EXPECT_EQ(std::string(UnMarshallString(tlvHead, CALLEE_ABILITY_NAME)), "MainAbility");
    };
    RunTest((const uint8_t *)GetPacketBufPtr(), GetPacketSize(), onTlvParseDone, nullptr);

    ClearWant(&want);
    CleanBuild();
}

/**
 * @tc.name: StartRemoteAbility_002
 * @tc.desc: Start remote ability with bundle name and ability name
 * @tc.type: FUNC
 * @tc.require: SR000FKTD0 AR000FM5TN
 */
HWTEST_F(FamgrTest, StartRemoteAbility_002, TestSize.Level1) {
    PreprareBuild();

    Want want;
    if (FillWant(&want, "ohos.dms.example", "MainAbility") != 0) {
        return;
    }
    StartRemoteAbility(&want, nullptr, nullptr);
    auto onTlvParseDone = [] (int8_t errCode, const void *dmsMsg) {
        const TlvNode *tlvHead = reinterpret_cast<const TlvNode *>(dmsMsg);
        EXPECT_NE(errCode, DMS_TLV_SUCCESS);
    };
    RunTest((const uint8_t *)GetPacketBufPtr(), GetPacketSize(), onTlvParseDone, nullptr);

    ClearWant(&want);
    CleanBuild();
}
}
}
