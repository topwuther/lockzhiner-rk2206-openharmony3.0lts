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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_INFO_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_INFO_H

#include <vector>
#include <memory>
#include "want.h"
#include "want_agent_constant.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
/**
 * A parametric class that contains the parameters required by WantAgentHelper GetWantAgent.
 *
 * This class is used to encapsulate parameters requestCode, operationType,
 * flag, Wants, and extraInfo. It is used as the input parameter for
 * the WantAgentHelper GetWantAgent method.
 *
 */
class WantAgentInfo final : public std::enable_shared_from_this<WantAgentInfo> {
private:
    int requestCode_ = 0;
    WantAgentConstant::OperationType operationType_ = WantAgentConstant::OperationType::UNKNOWN_TYPE;
    std::vector<WantAgentConstant::Flags> flags_ = std::vector<WantAgentConstant::Flags>();
    std::vector<std::shared_ptr<AAFwk::Want>> wants_ = std::vector<std::shared_ptr<AAFwk::Want>>();
    std::shared_ptr<AAFwk::WantParams> extraInfo_;

    /**
     * Default constructor used to create an empty WantAgentInfo instance.
     *
     */
public:
    WantAgentInfo();
    virtual ~WantAgentInfo() = default;

    /**
     * A constructor used to create an WantAgentInfo instance based on the input parameters.
     *
     * @param requestCode Indicates the request code to set. It is a private value defined by the user.
     * @param operationType Indicates the type of the operation to be performed by the WantAgent object.
     * For details about the value range, see WantAgentConstant.OperationType.
     * @param flag Indicates the flag for handling the WantAgent.
     * For details about the value range, see WantAgentConstant.Flags.
     * @param Wants Indicates the collection of Want objects to be used for creating the WantAgent
     * object. The number of Wants in the collection is determined by WantAgentConstant.OperationType.
     * @param extraInfo Indicates the extra information to be used for creating the WantAgent object.
     */
    WantAgentInfo(int requestCode, const WantAgentConstant::OperationType &operationType, WantAgentConstant::Flags flag,
        std::vector<std::shared_ptr<AAFwk::Want>> &Wants, const std::shared_ptr<AAFwk::WantParams> &extraInfo);

    /**
     * A constructor used to create an WantAgentInfo instance based on the input parameters.
     *
     * @param requestCode Indicates the request code to set. It is a private value defined by the user.
     * @param operationType Indicates the type of the operation to be performed by the WantAgent object.
     * For details about the value range, see WantAgentConstant.OperationType.
     * @param flags Indicates the flags for handling the WantAgent.
     * For details about the value range, see WantAgentConstant.Flags.
     * @param Wants Indicates the collection of Want objects to be used for creating the WantAgent
     * object. The number of Wants in the collection is determined by WantAgentConstant.OperationType.
     * @param extraInfo Indicates the extra information to be used for creating the WantAgent object.
     */
    WantAgentInfo(int requestCode, const WantAgentConstant::OperationType &operationType,
        const std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<AAFwk::Want>> &Wants,
        const std::shared_ptr<AAFwk::WantParams> &extraInfo);

    /**
     * A constructor used to create an WantAgentInfo instance by copying parameters from an existing one.
     *
     * @param paramInfo Indicates the existing WantAgentInfo object.
     */
    explicit WantAgentInfo(const std::shared_ptr<WantAgentInfo> &paramInfo);

    /**
     * Obtains the requestCode of the WantAgent object.
     *
     * @return Returns the requestCode of the WantAgent object.
     */
    int GetRequestCode() const;

    /**
     * Obtains the operationType of the WantAgent object.
     *
     * @return Returns the operationType of the WantAgent object.
     */
    WantAgentConstant::OperationType GetOperationType() const;

    /**
     * Obtains the flag of the WantAgent object.
     *
     * @return Returns the flag of the WantAgent object.
     */
    std::vector<WantAgentConstant::Flags> GetFlags() const;

    /**
     * Obtains the collection of all Wants of the WantAgent object.
     *
     * @return Returns the collection of all Wants of the WantAgent object.
     */
    std::vector<std::shared_ptr<AAFwk::Want>> GetWants() const;

    /**
     * Obtains the extra information of the WantAgent object.
     *
     * @return Returns the extra information of the WantAgent object.
     */
    std::shared_ptr<AAFwk::WantParams> GetExtraInfo() const;
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_INFO_H