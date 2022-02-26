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

#include "distributeddb_communicator_common.h"
#include <gtest/gtest.h>
#include "securec.h"
#include "db_errno.h"
#include "log_print.h"
#include "message_transform.h"

using namespace std;
using namespace DistributedDB;

bool SetUpEnv(EnvHandle &inEnv, const string &inName)
{
    if (inEnv.adapterHandle != nullptr || inEnv.commAggrHandle != nullptr) {
        LOGI("[UT][Common][SetUp] Already Setup for %s", inName.c_str());
        return false;
    }

    inEnv.adapterHandle = new (nothrow) AdapterStub(inName);
    if (inEnv.adapterHandle == nullptr) {
        LOGI("[UT][Common][SetUp] Create AdapterStub fail for %s", inName.c_str());
        return false;
    }

    inEnv.commAggrHandle = new (nothrow) CommunicatorAggregator();
    if (inEnv.commAggrHandle == nullptr) {
        LOGI("[UT][Common][SetUp] Create CommunicatorAggregator fail for %s", inName.c_str());
        return false;
    }

    int errCode = inEnv.commAggrHandle->Initialize(inEnv.adapterHandle);
    if (errCode != E_OK) {
        LOGI("[UT][Common][SetUp] Init CommunicatorAggregator fail for %s", inName.c_str());
        return false;
    }

    return true;
}

void TearDownEnv(EnvHandle &inEnv)
{
    if (inEnv.commAggrHandle != nullptr) {
        inEnv.commAggrHandle->Finalize();
        inEnv.commAggrHandle->DecObjRef(inEnv.commAggrHandle);
        inEnv.commAggrHandle = nullptr;
    }

    if (inEnv.adapterHandle != nullptr) {
        delete inEnv.adapterHandle;
        inEnv.adapterHandle = nullptr;
    }
}

static void RegFuncForTinyMsg()
{
    TransformFunc funcForTinyMsg;
    funcForTinyMsg.computeFunc = [](const Message *inMsg)->uint32_t{return TINY_SIZE;};
    funcForTinyMsg.serializeFunc = [](uint8_t *buffer, uint32_t length, const Message *inMsg)->int{
        const RegedTinyObject *outObj = inMsg->GetObject<RegedTinyObject>();
        EXPECT_NE(outObj, nullptr);
        return E_OK;
    };
    funcForTinyMsg.deserializeFunc = [](const uint8_t *buffer, uint32_t length, Message *inMsg)->int{
        int errCode = inMsg->SetCopiedObject(RegedTinyObject());
        EXPECT_EQ(errCode, E_OK);
        return E_OK;
    };

    MessageTransform::RegTransformFunction(REGED_TINY_MSG_ID, funcForTinyMsg);
}

static void RegFuncForHugeMsg()
{
    TransformFunc funcForHugeMsg;
    funcForHugeMsg.computeFunc = [](const Message *inMsg)->uint32_t{return HUGE_SIZE;};
    funcForHugeMsg.serializeFunc = [](uint8_t *buffer, uint32_t length, const Message *inMsg)->int{
        const RegedHugeObject *outObj = inMsg->GetObject<RegedHugeObject>();
        EXPECT_NE(outObj, nullptr);
        return E_OK;
    };
    funcForHugeMsg.deserializeFunc = [](const uint8_t *buffer, uint32_t length, Message *inMsg)->int{
        int errCode = inMsg->SetCopiedObject(RegedHugeObject());
        EXPECT_EQ(errCode, E_OK);
        return E_OK;
    };

    MessageTransform::RegTransformFunction(REGED_HUGE_MSG_ID, funcForHugeMsg);
}

static void RegFuncForGiantMsg()
{
    TransformFunc funcForGiantMsg;
    funcForGiantMsg.computeFunc = [](const Message *inMsg)->uint32_t{
        const RegedGiantObject *outObj = inMsg->GetObject<RegedGiantObject>();
        if (outObj == nullptr) {
            return 0;
        }
        return outObj->rawData_.size();
    };
    funcForGiantMsg.serializeFunc = [](uint8_t *buffer, uint32_t length, const Message *inMsg)->int{
        const RegedGiantObject *outObj = inMsg->GetObject<RegedGiantObject>();
        if (outObj == nullptr) {
            return -E_INVALID_ARGS;
        }
        if (outObj->rawData_.size() != length) {
            return -E_LENGTH_ERROR;
        }
        errno_t errCode = memcpy_s(buffer, length, &(outObj->rawData_[0]), length);
        if (errCode != EOK) {
            return -E_SECUREC_ERROR;
        }
        return E_OK;
    };
    funcForGiantMsg.deserializeFunc = [](const uint8_t *buffer, uint32_t length, Message *inMsg)->int{
        RegedGiantObject *obj = new (nothrow) RegedGiantObject();
        if (obj == nullptr) {
            return -E_OUT_OF_MEMORY;
        }
        obj->rawData_.resize(length);
        errno_t retCode = memcpy_s(&(obj->rawData_[0]), length, buffer, length);
        if (retCode != EOK) {
            delete obj;
            return -E_SECUREC_ERROR;
        }
        int errCode = inMsg->SetExternalObject(obj);
        if (errCode != E_OK) {
            delete obj;
            return errCode;
        }
        return E_OK;
    };

    MessageTransform::RegTransformFunction(REGED_GIANT_MSG_ID, funcForGiantMsg);
}

static void RegFuncForOverSizeMsg()
{
    TransformFunc funcForOverSizeMsg;
    funcForOverSizeMsg.computeFunc = [](const Message *inMsg)->uint32_t{return OVER_SIZE;};
    funcForOverSizeMsg.serializeFunc = [](uint8_t *buffer, uint32_t length, const Message *inMsg)->int{
        const RegedOverSizeObject *outObj = inMsg->GetObject<RegedOverSizeObject>();
        EXPECT_NE(outObj, nullptr);
        return E_OK;
    };
    funcForOverSizeMsg.deserializeFunc = [](const uint8_t *buffer, uint32_t length, Message *inMsg)->int{
        int errCode = inMsg->SetCopiedObject(RegedOverSizeObject());
        EXPECT_EQ(errCode, E_OK);
        return E_OK;
    };

    MessageTransform::RegTransformFunction(REGED_OVERSIZE_MSG_ID, funcForOverSizeMsg);
}

void DoRegTransformFunction()
{
    RegFuncForTinyMsg();
    RegFuncForHugeMsg();
    RegFuncForGiantMsg();
    RegFuncForOverSizeMsg();
}

Message *BuildRegedTinyMessage()
{
    RegedTinyObject *obj = new (nothrow) RegedTinyObject();
    if (obj == nullptr) {
        return nullptr;
    }

    Message *outMsg = new (nothrow) Message(REGED_TINY_MSG_ID);
    if (outMsg == nullptr) {
        delete obj;
        return nullptr;
    }

    int errCode = outMsg->SetExternalObject(obj);
    if (errCode != E_OK) {
        delete obj;
        obj = nullptr;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    outMsg->SetMessageType(TYPE_REQUEST);
    outMsg->SetSessionId(FIXED_SESSIONID);
    outMsg->SetSequenceId(FIXED_SEQUENCEID);

    return outMsg;
}

Message *BuildRegedHugeMessage()
{
    RegedHugeObject *obj = new (nothrow) RegedHugeObject();
    if (obj == nullptr) {
        return nullptr;
    }

    Message *outMsg = new (nothrow) Message(REGED_HUGE_MSG_ID);
    if (outMsg == nullptr) {
        delete obj;
        return nullptr;
    }

    int errCode = outMsg->SetExternalObject(obj);
    if (errCode != E_OK) {
        delete obj;
        obj = nullptr;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    outMsg->SetMessageType(TYPE_RESPONSE);
    outMsg->SetSessionId(FIXED_SESSIONID);
    outMsg->SetSequenceId(FIXED_SEQUENCEID);

    return outMsg;
}

// length should be a multiple of four
Message *BuildRegedGiantMessage(uint32_t length)
{
    uint32_t count = length / sizeof(uint32_t);
    if (count == 0) {
        return nullptr;
    }

    RegedGiantObject *obj = new (nothrow) RegedGiantObject();
    if (obj == nullptr) {
        return nullptr;
    }

    Message *outMsg = new (nothrow) Message(REGED_GIANT_MSG_ID);
    if (outMsg == nullptr) {
        delete obj;
        return nullptr;
    }

    obj->rawData_.resize(count * sizeof(uint32_t));
    auto dataPtr = reinterpret_cast<uint32_t *>(&(obj->rawData_[0]));
    uint32_t value = 0;
    while (value < count) {
        *dataPtr++ = value++;
    }

    int errCode = outMsg->SetExternalObject(obj);
    if (errCode != E_OK) {
        delete obj;
        obj = nullptr;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    outMsg->SetMessageType(TYPE_NOTIFY);
    outMsg->SetSessionId(FIXED_SESSIONID);
    outMsg->SetSequenceId(FIXED_SEQUENCEID);

    return outMsg;
}

Message *BuildRegedOverSizeMessage()
{
    RegedOverSizeObject *obj = new (nothrow) RegedOverSizeObject();
    if (obj == nullptr) {
        return nullptr;
    }

    Message *outMsg = new (nothrow) Message(REGED_OVERSIZE_MSG_ID);
    if (outMsg == nullptr) {
        delete obj;
        return nullptr;
    }

    int errCode = outMsg->SetExternalObject(obj);
    if (errCode != E_OK) {
        delete obj;
        obj = nullptr;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    outMsg->SetMessageType(TYPE_NOTIFY);
    outMsg->SetSessionId(FIXED_SESSIONID);
    outMsg->SetSequenceId(FIXED_SEQUENCEID);

    return outMsg;
}

Message *BuildUnRegedTinyMessage()
{
    UnRegedTinyObject *obj = new (nothrow) UnRegedTinyObject();
    if (obj == nullptr) {
        return nullptr;
    }

    Message *outMsg = new (nothrow) Message(UNREGED_TINY_MSG_ID);
    if (outMsg == nullptr) {
        delete obj;
        return nullptr;
    }

    int errCode = outMsg->SetExternalObject(obj);
    if (errCode != E_OK) {
        delete obj;
        obj = nullptr;
        delete outMsg;
        outMsg = nullptr;
        return nullptr;
    }
    outMsg->SetMessageType(TYPE_NOTIFY);
    outMsg->SetSessionId(FIXED_SESSIONID);
    outMsg->SetSequenceId(FIXED_SEQUENCEID);

    return outMsg;
}

bool RegedGiantObject::CheckEqual(const RegedGiantObject &inLeft, const RegedGiantObject &inRight)
{
    if (inLeft.rawData_.size() != inRight.rawData_.size()) {
        return false;
    }
    uint32_t index = 0;
    for (auto &left : inLeft.rawData_) {
        uint8_t right = inRight.rawData_[index];
        if (left != right) {
            LOGE("[RegedGiantObject][CheckEqual] RawData unequal at index=%u", index);
            return false;
        }
        index++;
    }
    return true;
}