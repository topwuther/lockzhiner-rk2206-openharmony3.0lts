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

#include "communication_provider_impl.h"
#include "log_print.h"

#undef LOG_TAG
#define LOG_TAG "CommunicationProviderImpl"

namespace OHOS {
namespace AppDistributedKv {
std::mutex CommunicationProviderImpl::mutex_;
CommunicationProviderImpl::CommunicationProviderImpl(AppPipeMgr &appPipeMgr, AppDeviceHandler &deviceHandler)
    : appPipeMgr_(appPipeMgr), appDeviceHandler_(deviceHandler)
{
}

CommunicationProviderImpl::~CommunicationProviderImpl()
{
    ZLOGD("destructor.");
}

Status CommunicationProviderImpl::Initialize()
{
    appDeviceHandler_.Init();
    return Status::SUCCESS;
}

Status CommunicationProviderImpl::StartWatchDeviceChange(const AppDeviceStatusChangeListener *observer,
    const PipeInfo &pipeInfo)
{
    return appDeviceHandler_.StartWatchDeviceChange(observer, pipeInfo);
}

Status CommunicationProviderImpl::StopWatchDeviceChange(const AppDeviceStatusChangeListener *observer,
    const PipeInfo &pipeInfo)
{
    return appDeviceHandler_.StopWatchDeviceChange(observer, pipeInfo);
}

DeviceInfo CommunicationProviderImpl::GetLocalDevice() const
{
    return appDeviceHandler_.GetLocalDevice();
}

std::vector<DeviceInfo> CommunicationProviderImpl::GetDeviceList() const
{
    return appDeviceHandler_.GetDeviceList();
}

Status CommunicationProviderImpl::StartWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    return appPipeMgr_.StartWatchDataChange(observer, pipeInfo);
}

Status CommunicationProviderImpl::StopWatchDataChange(const AppDataChangeListener *observer, const PipeInfo &pipeInfo)
{
    return appPipeMgr_.StopWatchDataChange(observer, pipeInfo);
}

Status CommunicationProviderImpl::SendData(const PipeInfo &pipeInfo, const DeviceId &deviceId, const uint8_t *ptr,
                                           int size, const MessageInfo &info)
{
    return appPipeMgr_.SendData(pipeInfo, deviceId, ptr, size, info);
}

Status CommunicationProviderImpl::Start(const PipeInfo &pipeInfo)
{
    return appPipeMgr_.Start(pipeInfo);
}

Status CommunicationProviderImpl::Stop(const PipeInfo &pipeInfo)
{
    return appPipeMgr_.Stop(pipeInfo);
}

bool CommunicationProviderImpl::IsSameStartedOnPeer(const PipeInfo &pipeInfo, const DeviceId &peer) const
{
    return appPipeMgr_.IsSameStartedOnPeer(pipeInfo, peer);
}

std::string CommunicationProviderImpl::GetUuidByNodeId(const std::string &nodeId) const
{
    return appDeviceHandler_.GetUuidByNodeId(nodeId);
}

DeviceInfo CommunicationProviderImpl::GetLocalBasicInfo() const
{
    return appDeviceHandler_.GetLocalBasicInfo();
}

std::vector<DeviceInfo> CommunicationProviderImpl::GetRemoteNodesBasicInfo() const
{
    return appDeviceHandler_.GetRemoteNodesBasicInfo();
}

std::string CommunicationProviderImpl::ToNodeId(const std::string &id) const
{
    std::string ret = appDeviceHandler_.ToNodeID(id, "");
    if (ret.empty()) {
        ZLOGD("toNodeId failed.");
    }
    return ret;
}

std::string CommunicationProviderImpl::GetUdidByNodeId(const std::string &nodeId) const
{
    return appDeviceHandler_.GetUdidByNodeId(nodeId);
}

void CommunicationProviderImpl::SetMessageTransFlag(const PipeInfo &pipeInfo, bool flag)
{
    appPipeMgr_.SetMessageTransFlag(pipeInfo, flag);
}
}  // namespace AppDistributedKv
}  // namespace OHOS
