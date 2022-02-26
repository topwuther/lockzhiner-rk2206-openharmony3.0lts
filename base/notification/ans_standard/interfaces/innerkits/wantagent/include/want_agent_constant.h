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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_CONSTANT_H
#define BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_CONSTANT_H

namespace OHOS::Notification::WantAgent {
/// <summary>
/// Provides enumerated constants that are used for setting object attributes in the methods provided by
/// <seealso cref="WantAgentInfo"/>.
///
/// </summary>
class WantAgentConstant final {
    /// <summary>
    /// Enumerates flags for using an <seealso cref="WantAgent"/>.
    ///
    /// </summary>
public:
    enum class Flags {
        /// <summary>
        /// Indicates that the <seealso cref="WantAgent"/> can be used only once.
        /// </summary>
        ONE_TIME_FLAG,

        /// <summary>
        /// Indicates that {@code null} is returned if the <seealso cref="WantAgent"/> does not exist.
        /// </summary>
        NO_BUILD_FLAG,

        /// <summary>
        /// Indicates that the existing <seealso cref="WantAgent"/> should be canceled before the new object is
        /// generated.
        /// </summary>
        CANCEL_PRESENT_FLAG,

        /// <summary>
        /// Indicates that the system only replaces the extra data of the existing <seealso cref="WantAgent"/>
        /// with that of the new object.
        /// </summary>
        UPDATE_PRESENT_FLAG,

        /// <summary>
        /// Indicates that the created <seealso cref="WantAgent"/> should be immutable.
        /// </summary>
        CONSTANT_FLAG,

        /// <summary>
        /// Indicates that the current value of {@code element} can be replaced
        /// when the <seealso cref="WantAgent"/> is triggered.
        /// </summary>
        REPLACE_ELEMENT,

        /// <summary>
        /// Indicates that the current value of {@code action} can be replaced
        /// when the <seealso cref="WantAgent"/> is triggered.
        /// </summary>
        REPLACE_ACTION,

        /// <summary>
        /// Indicates that the current value of {@code uri} can be replaced when the <seealso cref="WantAgent"/> is
        /// triggered.
        /// </summary>
        REPLACE_URI,

        /// <summary>
        /// Indicates that the current value of {@code entities} can be replaced
        /// when the <seealso cref="WantAgent"/> is triggered.
        /// </summary>
        REPLACE_ENTITIES,

        /// <summary>
        /// Indicates that the current value of {@code bundleName} can be replaced
        /// when the <seealso cref="WantAgent"/> is triggered.
        /// </summary>
        REPLACE_BUNDLE
    };

    /// <summary>
    /// Identifies the operation for using an <seealso cref="WantAgent"/>, such as starting an ability or sending a
    /// common event.
    ///
    /// </summary>
public:
    enum class OperationType {
        /// <summary>
        /// Unknown operation.
        /// </summary>
        UNKNOWN_TYPE,

        /// <summary>
        /// Starts an ability with a UI.
        /// </summary>
        START_ABILITY,

        /// <summary>
        /// Starts multiple abilities.
        /// </summary>
        START_ABILITIES,

        /// <summary>
        /// Starts an ability without a UI.
        /// </summary>
        START_SERVICE,

        /// <summary>
        /// Sends a common event.
        /// </summary>
        SEND_COMMON_EVENT,

        /// <summary>
        /// Starts a foreground ability without a UI.
        /// </summary>
        START_FOREGROUND_SERVICE
    };

private:
    WantAgentConstant(){};
    virtual ~WantAgentConstant() = default;
};
}  // namespace OHOS::Notification::WantAgent
#endif  // BASE_NOTIFICATION_ANS_STANDARD_KITS_NATIVE_WANTAGENT_INCLUDE_WANT_AGENT_CONSTANT_H
