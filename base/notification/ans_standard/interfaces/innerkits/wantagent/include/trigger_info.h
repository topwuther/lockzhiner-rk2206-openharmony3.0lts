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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_TRIGGER_INFO_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_TRIGGER_INFO_H

#include <string>
#include <memory>
#include "want.h"
#include "want_params.h"

namespace OHOS::Notification::WantAgent {
class TriggerInfo final : public std::enable_shared_from_this<TriggerInfo> {
    /**
     * Default constructor used to create a {@code TriggerInfo} instance.
     *
     */
public:
    TriggerInfo();
    virtual ~TriggerInfo() = default;

    /**
     * A constructor used to create a {@code TriggerInfo} instance based on the input parameters.
     *
     * @param permission Indicates the permission required for an {@link WantAgent} recipient.
     * This parameter is valid only when the {@link WantAgent} is triggered to send common events.
     * @param extraInfo Indicates the custom extra data you want to add for triggering an {@link WantAgent}.
     * @param want Indicates the extra {@link ohos.aafwk.content.Want}.
     * If {@code flags} in {@link WantAgentInfo} contains {@link WantAgentConstant.Flags#CONSTANT_FLAG},
     * this parameter is invalid. If flags contains {@link WantAgentConstant.Flags#REPLACE_ELEMENT},
     * {@link WantAgentConstant.Flags#REPLACE_ACTION}, {@link WantAgentConstant.Flags#REPLACE_URI},
     * {@link WantAgentConstant.Flags#REPLACE_ENTITIES}, and {@link WantAgentConstant.Flags#REPLACE_BUNDLE},
     * the {@code element}, {@code action}, {@code uri}, {@code entities}, and {@code bundleName} attributes of the
     * {@link ohos.aafwk.content.Want} specified in this parameter will be used to replace the
     * corresponding attributes in the original {@link ohos.aafwk.content.Want}, respectively.
     * If this parameter is null, the original {@link ohos.aafwk.content.Want} remains unchanged.
     * @param code Indicates the result code provided for the target of the {@link WantAgent}.
     */
    TriggerInfo(const std::string &permission, const std::shared_ptr<AAFwk::WantParams> &extraInfo,
        const std::shared_ptr<AAFwk::Want> &want, int resultCode);

    /**
     * A constructor used to create a {@code TriggerInfo} instance by copying parameters from an existing one.
     *
     * @param paramInfo Indicates the existing {@code TriggerInfo} object.
     */
    explicit TriggerInfo(const TriggerInfo &paramInfo);

    /**
     * A copy assignment operator used to create a {@code TriggerInfo} instance by copying parameters from an existing
     * one.
     *
     * @param paramInfo Indicates the existing {@code TriggerInfo} object.
     */
    const TriggerInfo &operator=(const TriggerInfo &paramInfo);

    /**
     * Obtains the permission from the current {@code TriggerInfo} object.
     *
     * @return Returns the permission name.
     */
    std::string GetPermission() const;

    /**
     * Obtains the extra data from the {@code TriggerInfo} object.
     *
     * @return Returns the extra data.
     */
    std::shared_ptr<AAFwk::WantParams> GetExtraInfo() const;

    /**
     * Obtains the {@link ohos.aafwk.content.Want} used for triggering an {@link WantAgent}.
     *
     * @return Returns an {@link ohos.aafwk.content.Want} object.
     */
    std::shared_ptr<AAFwk::Want> GetWant() const;

    /**
     * Obtains the result code provided for the target of the {@link WantAgent}.
     *
     * @return Returns the result code provided for the target of the {@link WantAgent}.
     */
    int GetResultCode() const;

    /**
     * A builder class for {@link TriggerInfo} objects.
     *
     */
public:
    class Builder final : public std::enable_shared_from_this<Builder> {
    private:
        std::string permission_;
        std::shared_ptr<AAFwk::WantParams> params_;
        std::shared_ptr<AAFwk::Want> want_;
        int resultCode_ = 0;

        /**
         * Default constructor used to create a {@code Builder} instance.
         *
         */
    public:
        Builder();
        virtual ~Builder() = default;

        /**
         * Sets the permission that the {@link WantAgent} recipient must have.
         *
         * @param permission Indicates the permission to set. This parameter is valid only when the {@link WantAgent}
         * to trigger is intended to send a common event.
         * @return Returns this {@code Builder} object with the specified permission.
         */
        std::shared_ptr<Builder> SetPermission(const std::string &permission);

        /**
         * Sets custom data.
         *
         * @param params Indicates the custom data to set.
         * @return Returns this {@code Builder} object with the custom data.
         */
        std::shared_ptr<Builder> SetWantParams(const std::shared_ptr<AAFwk::WantParams> &params);

        /**
         * Sets a custom {@link ohos.aafwk.content.Want}.
         *
         * @param want Indicates the custom {@code Want} to set. If the member variable {@code flags} of the
         * {@link WantAgentInfo} contains {@link WantAgentConstant.Flags#CONSTANT_FLAG}, this parameter does not
         * take effect. If {@code flags} contains {@link WantAgentConstant.Flags#REPLACE_ELEMENT},
         * {@link WantAgentConstant.Flags#REPLACE_ACTION}, {@link WantAgentConstant.Flags#REPLACE_URI},
         * {@link WantAgentConstant.Flags#REPLACE_ENTITIES}, and {@link WantAgentConstant.Flags#REPLACE_BUNDLE},
         * the {@code element}, {@code action}, {@code uri}, {@code entities}, and {@code bundleName} attributes of the
         * {@link ohos.aafwk.content.Want} specified in this parameter will be used to replace the corresponding
         * attributes in the original {@link ohos.aafwk.content.Want}, respectively. If this parameter is null, the
         * original {@link ohos.aafwk.content.Want} remains unchanged.
         * @return Returns this {@code Builder} object with the custom {@code Want}.
         */
        std::shared_ptr<Builder> SetWant(const std::shared_ptr<AAFwk::Want> &want);

        /**
         * Sets the result code provided for the target of the {@link WantAgent}.
         *
         * @param code Indicates the result code provided for the target of the {@link WantAgent}.
         * @return Returns this {@code Builder} object with the specified result code.
         */
        std::shared_ptr<Builder> SetResultCode(int resultCode);

        /**
         * Creates a {@link TriggerInfo} object using all of the settings.
         *
         * @return Returns the created {@code TriggerInfo} object.
         */
        std::shared_ptr<TriggerInfo> Build();
    };

private:
    std::string permission_;
    std::shared_ptr<AAFwk::WantParams> extraInfo_;
    std::shared_ptr<AAFwk::Want> want_;
    int resultCode_ = 0;
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_TRIGGER_INFO_H