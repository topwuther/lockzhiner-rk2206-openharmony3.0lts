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

#include "server_socket.h"

#include <cerrno>
#include <iostream>
#include <sys/socket.h>

#include "client_socket.h"
#include "hilog/log.h"
#include "securec.h"

namespace OHOS {
namespace AppSpawn {
using namespace OHOS::HiviewDFX;
static constexpr HiLogLabel LABEL = {LOG_CORE, 0, "MockServerSocket"};

ServerSocket::ServerSocket(const std::string &server) : AppSpawnSocket(server)
{}

ServerSocket::~ServerSocket()
{}

int ServerSocket::VerifyConnection(int connectFd)
{
    HiLog::Info(LABEL, "VerifyConnection");
    return 0;
}

void ServerSocket::CloseConnection(int connectFd)
{
    HiLog::Info(LABEL, "CloseConnection");
}

void ServerSocket::SaveConnection(int connectFd)
{
    HiLog::Info(LABEL, "CloseConnection");
}

void ServerSocket::CloseServer()
{
    HiLog::Info(LABEL, "CloseServer");
}

void ServerSocket::CloseServerMonitor()
{
    HiLog::Info(LABEL, "CloseServerMonitor");
}

int ServerSocket::BindSocket(int connectFd)
{
    HiLog::Info(LABEL, "BindSocket");
    return 0;
}

int ServerSocket::RegisterServerSocket(int &connectFd)
{
    HiLog::Info(LABEL, "RegisterServerSocket connectFd");
    return 0;
}

int ServerSocket::RegisterServerSocket()
{
    HiLog::Info(LABEL, "RegisterServerSocket");
    return 0;
}

int ServerSocket::WaitForConnection(int connectFd)
{
    HiLog::Info(LABEL, "WaitForConnection connectFd");
    return 0;
}

int ServerSocket::WaitForConnection()
{
    HiLog::Info(LABEL, "WaitForConnection");
    return 0;
}
}  // namespace AppSpawn
}  // namespace OHOS
