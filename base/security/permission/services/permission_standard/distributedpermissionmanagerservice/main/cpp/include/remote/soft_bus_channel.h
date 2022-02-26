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

#ifndef SOFT_BUS_CHANNEL_H
#define SOFT_BUS_CHANNEL_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <map>
#include <string>
#include <zlib.h>
#include "nlohmann/json.hpp"
#include "rpc_channel.h"
#include "session.h"
#include "soft_bus_session.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class SoftBusChannel final : public RpcChannel, public std::enable_shared_from_this<SoftBusChannel> {
public:
    SoftBusChannel(const std::string &deviceId);
    virtual ~SoftBusChannel();

    /**
     * @brief Build connection with peer device.
     *
     * @return Result code, 0 indicated build successfully, -1 indicates failure.
     * @since 1.0
     * @version 1.0
     * @see Release
     */
    int BuildConnection() override;

    /**
     * @brief Execute BaseRemoteCommand at peer device.
     *
     * @param commandName The name of Command.
     * @param jsonPayload The json payload of command.
     * @return Executed result response string.
     * @since 1.0
     * @version 1.0
     */
    std::string ExecuteCommand(const std::string &commandName, const std::string &jsonPayload) override;

    /**
     * @brief Handle data received. This interface only use for soft bus channel.
     *
     * @param session Session with peer device.
     * @param bytes Data sent from the peer device.
     * @param length Data length sent from the peer device.
     * @since 1.0
     * @version 1.0
     */
    void HandleDataReceived(int session, const unsigned char *bytes, const int length) override;

    /**
     * @brief Close rpc connection when no data is being transmitted. it will run in a delayed task.
     *
     * @since 1.0
     * @version 1.0
     */
    void CloseConnection() override;

    /**
     * @brief Release resources when the device offline.
     *
     * @since 1.0
     * @version 1.0
     */
    void Release() override;

private:
    /**
     * @brief compress json command to char array command.
     *
     * @param type request or response
     * @param id unique message id
     * @param commandName command name
     * @param jsonPayload command notated by json string
     * @param bytes transfer data array
     * @param bytesLength transfer data length
     * @return The execute result, SUCCESS: 0; FAILURE: -1.
     * @see Compress
     * @since 1.0
     * @version 1.0
     */
    int PrepareBytes(const std::string &type, const std::string &id, const std::string &commandName,
        const std::string &jsonPayload, const unsigned char *bytes, int &bytesLength);

    /**
     * @brief compress string to char array.
     *
     * @param json string to be compressed
     * @param compressedBytes compressed data array
     * @param compressedLength compressed data length
     * @return The execute result, SUCCESS: 0; FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int Compress(const std::string &json, const unsigned char *compressedBytes, int &compressedLength);

    /**
     * @brief decompress char array to string.
     *
     * @param bytes compressed data array
     * @param length compressed data length
     * @return decompressed string
     * @since 1.0
     * @version 1.0
     */
    std::string Decompress(const unsigned char *bytes, const int length);

    /**
     * @brief transfer request data to soft bus.
     *
     * @param bytes data array to transfer
     * @param bytesLength data length
     * @return The execute result, SUCCESS: 0; FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int SendRequestBytes(const unsigned char *bytes, const int bytesLength);

    /**
     * @brief transfer response data to soft bus.
     *
     * @param session response session id
     * @param bytes data array to transfer
     * @param bytesLength data length
     * @return The execute result, SUCCESS: 0; FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int SendResponseBytes(int session, const unsigned char *bytes, const int bytesLength);

    /**
     * @brief enforce session is available. if session is opened, reopen it.
     *
     * @return The execute result, SUCCESS: 0; FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int CheckSessionMayReopenLocked();

    /**
     * @brief check session is available.
     *
     * @return The execute result, available: true, otherwise: false.
     * @since 1.0
     * @version 1.0
     */
    bool IsSessionAvailable();

    /**
     * @brief cancel closing connection.
     *
     * @since 1.0
     * @version 1.0
     */
    void CancelCloseConnectionIfNeeded();

    /**
     * @brief request callback for HandleDataReceived
     *
     * @param id unique message id
     * @param commandName command name
     * @param jsonPayload command notated by json string
     * @return decompressed string
     * @see HandleDataReceived
     * @since 1.0
     * @version 1.0
     */
    void HandleRequest(
        int session, const std::string &id, const std::string &commandName, const std::string &jsonPayload);

    /**
     * @brief response callback for HandleDataReceived
     *
     * @param id unique message id
     * @param jsonPayload command notated by json string
     * @return decompressed string
     * @see HandleDataReceived
     * @since 1.0
     * @version 1.0
     */
    void HandleResponse(const std::string &id, const std::string &jsonPayload);

    /**
     * @brief temp function to generate uuid.
     *
     * @param buf uuid string
     * @since 1.0
     * @version 1.0
     */
    void random_uuid(char buf[37])
    {
        const int xbase = 15;
        const int bbase = 255;
        const int index6 = 6;
        const int index8 = 8;
        const int index3 = 3;
        const int index5 = 5;
        const int index7 = 7;
        const int index9 = 9;
        const int uuidlen = 16;
        const int blen = 2;
        const char *c = "89ab";
        char *p = buf;
        int n;

        for (n = 0; n < uuidlen; ++n) {
            int b = rand() % bbase;
            switch (n) {
                case index6:
                    sprintf(p, "4%x", b % xbase);
                    break;
                case index8:
                    sprintf(p, "%c%x", c[rand() % strlen(c)], b % xbase);
                    break;
                default:
                    sprintf(p, "%02x", b);
                    break;
            }
            p += blen;
            if (n == index3 || n == index5 || n == index7 || n == index9) {
                *p++ = '-';
                break;
            }
        }
        *p = 0;
    }

    // bind device id for this channel
    std::string deviceId_;

    // channel mutex
    std::mutex mutex_;

    // connection closing state. true: in closing, false: otherwise
    bool isDelayClosing_;

    // soft bus session mutex
    std::mutex sessionMutex_;

    // soft bus session id, -1 for invalid session id.
    int session_;

    // soft bus session busy flag, true: busy, false: otherwise
    bool isSessionUsing_;

    // communication callbacks map. key: unique message id, value: response callback.
    std::map<std::string, std::function<void(std::string)>> callbacks_;

    // callback function arguments: response string variable
    std::string responseResult_;
    // callback function execute variable
    std::condition_variable loadedCond_;
};

class SoftBusMessage {
public:
    SoftBusMessage(
        const std::string &type, const std::string &id, const std::string &commandName, const std::string &jsonPayload)
        : type_(type), id_(id), commandName_(commandName), jsonPayload_(jsonPayload)
    {}
    ~SoftBusMessage() = default;

    bool IsValid() const
    {
        if (this->type_.empty()) {
            return false;
        }
        if (this->id_.empty()) {
            return false;
        }
        if (this->commandName_.empty()) {
            return false;
        }
        return !(this->jsonPayload_.empty());
    }

    /**
     * Convert SoftBusMessage object to corresponding json string.
     *
     * @return Soft bus message json string.
     */
    std::string ToJson() const
    {
        nlohmann::json json;
        json["type"] = this->type_;
        json["id"] = this->id_;
        json["commandName"] = this->commandName_;
        json["jsonPayload"] = this->jsonPayload_;
        return json.dump();
    }

    const std::string &GetType() const
    {
        return type_;
    }
    const std::string &GetId() const
    {
        return id_;
    }
    const std::string &GetCommandName() const
    {
        return commandName_;
    }
    const std::string &GetJsonPayload() const
    {
        return jsonPayload_;
    }

    static std::shared_ptr<SoftBusMessage> FromJson(const std::string &jsonString)
    {
        nlohmann::json json;
        if (!json.accept(jsonString)) {
            return nullptr;
        }
        json = json.parse(jsonString);
        std::shared_ptr<SoftBusMessage> message = std::make_shared<SoftBusMessage>(
            json.at("type"), json.at("id"), json.at("commandName"), json.at("jsonPayload"));
        return message;
    }

private:
    std::string type_;
    std::string id_;
    std::string commandName_;
    std::string jsonPayload_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif  // SOFT_BUS_CHANNEL_H
