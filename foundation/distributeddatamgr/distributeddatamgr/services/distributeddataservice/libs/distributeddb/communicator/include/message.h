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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <new>
#include <string>
#include <cstdint>
#include "db_errno.h"
#include "macro_utils.h"
#include "object_holder.h"
#include "object_holder_typed.h"
#include "communicator_type_define.h"

namespace DistributedDB {
constexpr uint32_t INVALID_MESSAGE_ID = 0;
constexpr uint16_t TYPE_INVALID = 0;
constexpr uint16_t TYPE_REQUEST = 1;
constexpr uint16_t TYPE_RESPONSE = 2;
constexpr uint16_t TYPE_NOTIFY = 3;
constexpr uint32_t NO_ERROR = 0;
constexpr uint16_t MSG_VERSION_BASE = 0;
constexpr uint16_t MSG_VERSION_EXT = 1;

class Message {
public:
    Message() = default;

    Message(uint32_t inMsgId)
    {
        messageId_ = inMsgId;
    }

    ~Message()
    {
        if (holderPtr_ != nullptr) {
            delete holderPtr_;
            holderPtr_ = nullptr;
        }
    }

    DISABLE_COPY_ASSIGN_MOVE(Message);

    // For user convenience, inObj can be a stack object, provided that it supports copy construct
    // Set Object again will delete object that set before if successfully, otherwise impact no change
    template<typename T>
    int SetCopiedObject(const T &inObj)
    {
        T *copiedObject = new (std::nothrow) T(inObj);
        if (copiedObject == nullptr) {
            return -E_OUT_OF_MEMORY;
        }
        ObjectHolder *tmpHolderPtr = new (std::nothrow) ObjectHolderTyped<T>(copiedObject);
        if (tmpHolderPtr == nullptr) {
            delete copiedObject;
            return -E_OUT_OF_MEMORY;
        }
        if (holderPtr_ != nullptr) {
            delete holderPtr_;
        }
        holderPtr_ = tmpHolderPtr;
        return E_OK;
    }

    // By calling this method successfully, The ownership of inObj will be taken up by this class
    // Thus this class is responsible for delete the inObj
    // If calling this method unsuccessfully, The ownership of inObj is not changed
    // Set Object again will delete object that set before if successfully, otherwise impact no change
    template<typename T>
    int SetExternalObject(T *&inObj)
    {
        if (inObj == nullptr) {
            return -E_INVALID_ARGS;
        }
        ObjectHolder *tmpHolderPtr = new (std::nothrow) ObjectHolderTyped<T>(inObj);
        if (tmpHolderPtr == nullptr) {
            return -E_OUT_OF_MEMORY;
        }
        if (holderPtr_ != nullptr) {
            delete holderPtr_;
        }
        holderPtr_ = tmpHolderPtr;
        inObj = nullptr;
        return E_OK;
    }

    // Calling this method in form of GetObject<T>() to specify return type based on the MessageId
    template<typename T>
    const T *GetObject() const
    {
        if (holderPtr_ == nullptr) {
            return nullptr;
        }
        ObjectHolderTyped<T> *realHolderPtr = static_cast<ObjectHolderTyped<T> *>(holderPtr_);
        return realHolderPtr->GetObject();
    }

    int SetMessageType(uint16_t inMsgType)
    {
        if (inMsgType != TYPE_REQUEST && inMsgType != TYPE_RESPONSE && inMsgType != TYPE_NOTIFY) {
            return -E_INVALID_ARGS;
        }
        messageType_ = inMsgType;
        return E_OK;
    }

    void SetMessageId(uint32_t inMessageId)
    {
        messageId_ = inMessageId;
    }

    void SetSessionId(uint32_t inSessionId)
    {
        sessionId_ = inSessionId;
    }

    void SetSequenceId(uint32_t inSequenceId)
    {
        sequenceId_ = inSequenceId;
    }

    void SetErrorNo(uint32_t inErrorNo)
    {
        errorNo_ = inErrorNo;
    }

    void SetTarget(const std::string &inTarget)
    {
        target_ = inTarget;
    }

    void SetPriority(Priority inPriority)
    {
        prio_ = inPriority;
    }

    void SetVersion(uint16_t inVersion)
    {
        if (inVersion != MSG_VERSION_BASE && inVersion != MSG_VERSION_EXT) {
            return;
        }
        version_ = inVersion;
    }

    uint16_t GetMessageType() const
    {
        return messageType_;
    }

    uint32_t GetMessageId() const
    {
        return messageId_;
    }

    uint32_t GetSessionId() const
    {
        return sessionId_;
    }

    uint32_t GetSequenceId() const
    {
        return sequenceId_;
    }

    uint32_t GetErrorNo() const
    {
        return errorNo_;
    }

    std::string GetTarget() const
    {
        return target_;
    }

    Priority GetPriority() const
    {
        return prio_;
    }

    uint16_t GetVersion() const
    {
        return version_;
    }
private:
    // Field or content that will be serialized for bytes transfer
    uint16_t version_ = MSG_VERSION_BASE;
    uint16_t messageType_ = TYPE_INVALID;
    uint32_t messageId_ = INVALID_MESSAGE_ID;
    uint32_t sessionId_ = 0;    // Distinguish different conversation
    uint32_t sequenceId_ = 0;   // Distinguish different message even in same session with same content in retry case
    uint32_t errorNo_ = NO_ERROR;
    ObjectHolder *holderPtr_ = nullptr;
    // Field carry supplemental info
    std::string target_;
    Priority prio_ = Priority::LOW;
};
} // namespace DistributedDB

#endif // MESSAGE_H
