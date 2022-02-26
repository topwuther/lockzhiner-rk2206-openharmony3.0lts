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

#ifndef REMOTE_COMMAND_MANAGER_H
#define REMOTE_COMMAND_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <mutex>

#include "base_remote_command.h"
#include "constant.h"
#include "distributed_data_validator.h"
#include "remote_command_executor.h"
#include "rpc_channel.h"

#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class RemoteCommandManager final {
public:
    ~RemoteCommandManager();

    /**
     * @brief Singleton instance get method.
     *
     * @since 1.0
     * @version 1.0
     */
    static RemoteCommandManager &GetInstance();

    /**
     * @brief Init method.
     *
     * @see
     * @since 1.0
     * @version 1.0
     */
    void Init();

    /**
     * @brief Execute a command now.
     *
     * @param udid The udid of a device which you want to execute on. if udid is empty, return -1.
     * @param command A command extend BaseRemoteCommand. if command is nullptr, return -1.
     * @return The execute result, returned from RemoteCommandExecutor.
     * @see RemoteCommandExecutor.ExecuteOneCommand
     * @since 1.0
     * @version 1.0
     */
    int ExecuteCommand(const std::string &udid, const std::shared_ptr<BaseRemoteCommand> &command);

    /**
     * @brief Add a command to buffer.
     *
     * @param udid The udid of a device which you want to execute on.
     * @param command A command extend BaseRemoteCommand.
     * @return The add result, returned from RemoteCommandExecutor. by now, SUCCESS: 0. INVALID_COMMAND: -14
     * @see RemoteCommandExecutor.AddCommand
     * @since 1.0
     * @version 1.0
     */
    int AddCommand(const std::string &udid, const std::shared_ptr<BaseRemoteCommand> &command);

    /**
     * @brief Execute all buffered commands for given device.
     *
     * @param udid The udid of a device which you want to execute on.
     * @return The execute result. SUCCESS: 0; FAILURE: -1.
     * @see RemoteCommandExecutor.ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    int ProcessDeviceCommandImmediately(const std::string &udid);

    /**
     * @brief Execute all buffered commands for all device asynchronized.
     *
     * @return The loop result. SUCCESS: 0.
     * @see RemoteCommandExecutor.ProcessBufferedCommandsWithThread
     * @since 1.0
     * @version 1.0
     */
    int Loop();

    /**
     * @brief Clear buffered commands.
     *
     * @since 1.0
     * @version 1.0
     */
    void Clear();

    /**
     * @brief Remove a command from buffer.
     *
     * @param udid The udid of a device which you want to remove.
     */
    void RemoveCommand(const std::string &udid);

    /**
     * @brief For event of device online, prepare channel and build connection with peer device.
     *
     * @param peerNodeId The udid of peer device.
     * @return Result code indicates if notify successfully. SUCCESS: 0, FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int NotifyDeviceOnline(const std::string &peerNodeId);

    /**
     * @brief For event of device offline, clean caches related to peer device.
     *
     * @param peerNodeId The peer device's nodeId, maybe uuid or udid .
     * @return Result code indicates if notify successfully. SUCCESS: 0, FAILURE: -1.
     * @since 1.0
     * @version 1.0
     */
    int NotifyDeviceOffline(const std::string &peerNodeId);

    /**
     * @brief Get remote command executor's channel for given nodeId.
     *
     * @param nodeId The peer device's nodeId, maybe uuid or udid or networkId.
     * @return Channel instance if remote command executor has been created, null otherwise.
     */
    std::shared_ptr<RpcChannel> GetExecutorChannel(const std::string &nodeId);

private:
    RemoteCommandManager();

    // executors buffer
    std::map<std::string, std::shared_ptr<RemoteCommandExecutor>> executors_;
    // executors buffer mutex
    std::mutex mutex_;

    /**
     * @brief Fetch a executor from executors buffer. If not found, create one and cache it to buffer.
     *
     * @param nodeId The udid of a device which you want to get executor.
     * @see void
     * @since 1.0
     * @version 1.0
     */
    std::shared_ptr<RemoteCommandExecutor> GetOrCreateRemoteCommandExecutor(const std::string &nodeId);
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS

#endif
