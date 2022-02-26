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

#ifndef RPC_CHANNEL_H
#define RPC_CHANNEL_H

namespace OHOS {
namespace Security {
namespace Permission {
/*
 * Channel used for communicate with peer devices.
 */
class RpcChannel {
public:
    /**
     * @brief Build connection with peer device.
     *
     * @return Result code represent if build successfully. 0 indicates success, -1 indicates failure.
     * @since 1.0
     * @version 1.0
     */
    virtual int BuildConnection() = 0;

    /**
     * @brief Execute BaseRemoteCommand at peer device.
     *
     * @param commandName The name of Command.
     * @param jsonPayload The json payload of command.
     * @return Executed result response string.
     * @since 1.0
     * @version 1.0
     */
    virtual std::string ExecuteCommand(const std::string &commandName, const std::string &jsonPayload) = 0;

    /**
     * @brief Handle data received. This interface only use for soft bus channel.
     *
     * @param session Session with peer device.
     * @param bytes Data sent from the peer device.
     * @param length Data length sent from the peer device.
     * @since 1.0
     * @version 1.0
     */
    virtual void HandleDataReceived(int session, const unsigned char *bytes, int length)
    {}

    /**
     * @brief Close rpc connection when no data is being transmitted.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual void CloseConnection()
    {}

    /**
     * @brief Release resources when the device offline.
     *
     * @since 1.0
     * @version 1.0
     */
    virtual void Release(){};
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif