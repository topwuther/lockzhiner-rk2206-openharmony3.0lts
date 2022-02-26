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

#ifndef REMOTE_COMMAND_EXECUTOR_H
#define REMOTE_COMMAND_EXECUTOR_H

#include <mutex>
#include <deque>
#include "base_remote_command.h"
#include "remote_command_factory.h"
#include "rpc_channel.h"
#include "permission_log.h"

namespace OHOS {
namespace Security {
namespace Permission {
class RemoteCommandExecutor final {
public:
    RemoteCommandExecutor(const std::string &targetNodeId);
    virtual ~RemoteCommandExecutor();

    const std::shared_ptr<RpcChannel> &GetChannel() const
    {
        return ptrChannel_;
    }
    void SetChannel(const std::shared_ptr<RpcChannel> &ptrChannel)
    {
        ptrChannel_ = ptrChannel;
    }

    /**
     * @brief Factory method to create a rpc channel. we will only create SoftBusChannel by now.
     *
     * @param targetNodeId target device node id(udid)
     * @return Returns <b>a shared_ptr</b> if the operation is successful, returns <b>nullptr</b> otherwise.
     * @see SoftBusChannel
     * @since 1.0
     * @version 1.0
     */
    static const std::shared_ptr<RpcChannel> CreateChannel(const std::string &targetNodeId);

    /**
     * @brief Process one command given.
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @return Returns <b>SUCCESS</b> if the operation is successful, returns <b>minus integer</b> otherwise.
     * @see void
     * @since 1.0
     * @version 1.0
     */
    int ProcessOneCommand(const std::shared_ptr<BaseRemoteCommand> &ptrCommand);

    /**
     * @brief Add one command into the buffer
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @return Returns <b>SUCCESS</b> if the operation is successful, returns <b>INVALID_COMMAND</b> otherwise.
     * @see ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    int AddCommand(const std::shared_ptr<BaseRemoteCommand> &ptrCommand);

    /**
     * @brief Process all the command in the buffer
     *
     * @param standalone true if run in a new thread or event runner, otherwise false.
     * @return Returns <b>SUCCESS</b> if the operation is successful, returns <b>FAILURE</b> otherwise.
     * @see AddCommand ProcessOneCommand
     * @since 1.0
     * @version 1.0
     */
    int ProcessBufferedCommands(bool standalone = false);

    /**
     * @brief Process all the command in the buffer within a new thread. in deconstruct, we need to join this thread if
     * needed.
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @return void
     * @see ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    void ProcessBufferedCommandsWithThread();

private:
    /**
     * @brief execute a command in a specific place.
     * for remote command, transfor the command json string by channel to softbus, and wait for softbus to response a
     * json string. while remote response a json string, construct a remote command and finish it.
     * if command buffer is empty, close the rpc channel.
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @param isRemote where to run. true for remote, false for local.
     * @return Returns <b>SUCCESS</b> if the operation is successful, returns <b>FAILURE</b> otherwise.
     * @see ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    int ExecuteRemoteCommand(const std::shared_ptr<BaseRemoteCommand> &ptrCommand, bool isRemote);

    /**
     * @brief create a rpc channel if not exist.
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @param isRemote where to run. true for remote, false for local.
     * @return void
     * @see ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    void CreateChannelIfNeeded();

    /**
     * @brief finish a command
     *
     * @param ptrCommand BaseRemoteCommand to execute.
     * @return Returns <b>SUCCESS</b> if the operation is successful, returns <b>FAILURE</b> otherwise.
     * @see ProcessBufferedCommands
     * @since 1.0
     * @version 1.0
     */
    int ClientProcessResult(const std::shared_ptr<BaseRemoteCommand> &ptrCommand);

private:
    // target device node id(udid)
    std::string targetNodeId_;

    // cached channel for buffered commands
    std::shared_ptr<RpcChannel> ptrChannel_;

    // mutex to lock commands buffer for concurrent access.
    std::recursive_mutex mutex_;

    // commands buffer
    std::deque<std::shared_ptr<BaseRemoteCommand> > commands_;

    // consumer running flag, true if the consumer is RUNNING, false otherwise. @see ProcessBufferedCommands
    bool running_;
};
}  // namespace Permission
}  // namespace Security
}  // namespace OHOS
#endif  // REMOTE_COMMAND_EXECUTOR_H
