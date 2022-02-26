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

#ifndef ICOMMUNICATORAGGREGATOR_H
#define ICOMMUNICATORAGGREGATOR_H

#include <cstdint>
#include "iadapter.h"
#include "ref_object.h"
#include "communicator_type_define.h"

namespace DistributedDB {
class ICommunicator; // Forward Declaration
// Return E_OK to indicate to retain received frame. Do not block during callback.
using CommunicatorLackCallback = std::function<int(const LabelType &commLabel)>;

class ICommunicatorAggregator : public virtual RefObject {
public:
    // Return 0 as success. Return negative as error
    // The caller is the owner of inAdapter and responsible for manage its lifecycle.
    // The ICommunicatorAggregator is only the user of inAdapter
    // If Initialize fail, the ICommunicatorAggregator will rollback what had done to inAdapter so it can be reuse.
    virtual int Initialize(IAdapter *inAdapter) = 0;

    // Call this method after Initialize successfully and before destroy the ICommunicatorAggregator
    // Emphasize again : DO NOT CALL Finalize IF Initialize FAIL.
    // Must not call any other functions if Finalize had been called.
    // More likely, The Finalize has no chance to be called. since it is process level.
    virtual void Finalize() = 0;

    // If not success, return nullptr and set outErrorNo
    virtual ICommunicator *AllocCommunicator(uint64_t commLabel, int &outErrorNo) = 0;
    virtual ICommunicator *AllocCommunicator(const LabelType &commLabel, int &outErrorNo) = 0;

    virtual void ReleaseCommunicator(ICommunicator *inCommunicator) = 0;

    virtual int RegCommunicatorLackCallback(const CommunicatorLackCallback &onCommLack, const Finalizer &inOper) = 0;
    virtual int RegOnConnectCallback(const OnConnectCallback &onConnect, const Finalizer &inOper) = 0;

    virtual ~ICommunicatorAggregator() {};
};
} // namespace DistributedDB

#endif // ICOMMUNICATORAGGREGATOR_H
