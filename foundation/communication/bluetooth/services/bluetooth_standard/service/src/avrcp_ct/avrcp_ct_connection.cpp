/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "avrcp_ct_connection.h"
#include "avrcp_ct_packet.h"
#include "avrcp_ct_pass_through.h"

namespace bluetooth {
AvrcCtConnectManager *AvrcCtConnectManager::g_instance = nullptr;

AvrcCtConnectInfo::AvrcCtConnectInfo(const std::string &btAddr, uint8_t connectId, uint8_t role, uint16_t controlMtu,
    uint16_t browseMtu, uint32_t companyId, uint16_t uidCounter, bool absVolume, AvctChannelEventCallback eventCallback,
    AvctMsgCallback msgCallback)
    : eventCallback_(eventCallback),
      msgCallback_(msgCallback),
      connectId_(connectId),
      role_(role),
      controlMtu_(controlMtu),
      browseMtu_(browseMtu),
      companyId_(companyId),
      uidCounter_(uidCounter),
      absVolume_(absVolume),
      btAddr_(btAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectInfo::%{public}s", __func__);

    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYBACK_STATUS_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_REACHED_END, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_REACHED_START, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYBACK_POS_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_BATT_STATUS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_SYSTEM_STATUS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_AVAILABLE_PLAYERS_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_ADDRESSED_PLAYER_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_UIDS_CHANGED, true));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_VOLUME_CHANGED, true));

    ptInfo_ = {nullptr, nullptr};
    vdInfo_ = {nullptr, nullptr};
    brInfo_ = {nullptr, nullptr};

    brConnected_ = false;
}

AvrcCtConnectInfo::~AvrcCtConnectInfo()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectInfo::%{public}s", __func__);

    if (ptInfo_.timer_ != nullptr) {
        ptInfo_.timer_->Stop();
    }

    while (!ptInfo_.cmdQue_.empty()) {
        ptInfo_.cmdQue_.pop();
    }

    if (unInfo_.timer_ != nullptr) {
        unInfo_.timer_->Stop();
    }

    while (!unInfo_.cmdQue_.empty()) {
        unInfo_.cmdQue_.pop();
    }

    if (vdInfo_.timer_ != nullptr) {
        vdInfo_.timer_->Stop();
    }

    while (!vdInfo_.cmdQue_.empty()) {
        vdInfo_.cmdQue_.pop();
    }

    if (brInfo_.timer_ != nullptr) {
        brInfo_.timer_->Stop();
    }

    while (!brInfo_.cmdQue_.empty()) {
        brInfo_.cmdQue_.pop();
    }
}

AvrcCtConnectManager *AvrcCtConnectManager::GetInstance(void) 
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    if (g_instance == nullptr) {
       
        g_instance = new (std::nothrow) AvrcCtConnectManager();
    }

    return g_instance;
}

int AvrcCtConnectManager::Add(const RawAddress &rawAddr, uint8_t connectId, uint8_t role, uint16_t controlMtu,
    uint16_t browseMtu, uint32_t companyId, uint16_t uidCounter, bool absVolume, AvctChannelEventCallback eventCallback,
    AvctMsgCallback msgCallback)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    int result = RET_NO_ERROR;

    if (GetConnectInfo(rawAddr.GetAddress()) == nullptr) {
        infos_.insert(std::make_pair(rawAddr.GetAddress(),
            AvrcCtConnectInfo(rawAddr.GetAddress(),
                connectId,
                role,
                controlMtu,
                browseMtu,
                companyId,
                uidCounter,
                absVolume,
                eventCallback,
                msgCallback)));
    } else {
        result = RET_BAD_PARAM;
        LOG_DEBUG("[AVRCP CT] The connection information exists!");
    }

    return result;
}

void AvrcCtConnectManager::Delete(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    infos_.erase(rawAddr.GetAddress());
}

const AvrcCtConnectInfo *AvrcCtConnectManager::GetConnectInfo(const RawAddress &rawAddr) const
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    const AvrcCtConnectInfo *info = nullptr;

    auto iter = infos_.find(rawAddr.GetAddress());
    if (iter != infos_.end()) {
        info = &iter->second;
    }

    return info;
}

RawAddress AvrcCtConnectManager::GetRawAddress(uint8_t connectId)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::string addr(AVRC_CT_DEFAULT_BLUETOOTH_ADDRESS);

    for (auto &info : infos_) {
        if (info.second.connectId_ == connectId) {
            addr = info.second.btAddr_;
            break;
        }
    }

    return RawAddress(addr);
}

std::list<std::string> AvrcCtConnectManager::GetDeviceAddresses(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    std::list<std::string> addrs;

    std::transform(infos_.begin(), infos_.end(), std::inserter(addrs,addrs.end()),
        [](const std::pair<std::string, AvrcCtConnectInfo> &info){return info.second.btAddr_;});

    return addrs;
}

void AvrcCtConnectManager::AddDisconnectedDevice(const std::string &addr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    disconnectedDevices_.push_back(addr);
}

void AvrcCtConnectManager::DeleteDisconnectedDevice(const std::string &addr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (std::list<std::string>::iterator it = disconnectedDevices_.begin(); it != disconnectedDevices_.end(); ++it) {
        if (it->compare(addr) == 0x00) {
            it = disconnectedDevices_.erase(it);
        }
    }
}

const std::list<std::string> &AvrcCtConnectManager::GetAllDisconnectedDevices(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    return disconnectedDevices_;
}

bool AvrcCtConnectManager::IsConnectInfoEmpty(void)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return infos_.empty();
}

uint8_t AvrcCtConnectManager::GetConnectId(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint8_t connectId = 0x00;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        connectId = info->connectId_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return connectId;
}

void AvrcCtConnectManager::SetConnectId(const RawAddress &rawAddr, uint8_t connectId)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] connectId[%{public}d]", connectId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->connectId_ = connectId;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::EnableNotifyState(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] notification[%{public}d]", notification);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            iter->second = true;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::DisableNotifyState(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] notification[%{public}d]", notification);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            iter->second = false;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

bool AvrcCtConnectManager::IsNotifyStateEnabled(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] notification[%{public}d]", notification);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    bool result = false;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            result = iter->second;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    LOG_DEBUG("[AVRCP CT] result[%{public}d]", result);

    return result;
}

void AvrcCtConnectManager::DisableExcludeEvents(const RawAddress &rawAddr, std::vector<uint8_t> events)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info == nullptr) {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
        return;
    }
    for (auto pos = info->notes_.begin(); pos != info->notes_.end(); ) {
        for (auto event : events) {
            if (pos->first != event) {
                pos->second = false;
                break;
            }
        }
        pos++;
    }
}

uint32_t AvrcCtConnectManager::GetCompanyId(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint32_t companyId = AVRC_CT_DEFAULT_BLUETOOTH_SIG_COMPANY_ID;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        companyId = info->companyId_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return companyId;
}

uint16_t AvrcCtConnectManager::GetUidCounter(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint16_t uidCounter = 0x00;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        uidCounter = info->uidCounter_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return uidCounter;
}

void AvrcCtConnectManager::SetUidCounter(const RawAddress &rawAddr, uint16_t uidCounter)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP CT] uidCounter[%{public}d]", uidCounter);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->uidCounter_ = uidCounter;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

/******************************************************************
 * PASS THROUGH COMMAND                                           *
 ******************************************************************/

std::shared_ptr<AvrcCtPassPacket> AvrcCtConnectManager::GetPassPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtPassPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->ptInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

void AvrcCtConnectManager::SetPassPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtPassPacket> &pkt) 
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

bool AvrcCtConnectManager::IsPassPacketEmpty(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool result = true;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr && info->ptInfo_.pkt_ != nullptr) {
        result = false;
    } else {
        LOG_DEBUG("[AVRCP CT] The PASS THROUGH packet is empty!");
    }

    return result;
}

void AvrcCtConnectManager::SetPassTimer(
    const RawAddress &rawAddr, const std::function<void()> callback, int ms, bool isPeriodic)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.timer_ = std::make_shared<utility::Timer>(callback);
        info->ptInfo_.timer_->Start(ms, isPeriodic);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearPassPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearPassTimer(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        if (info->ptInfo_.timer_ != nullptr) {
            info->ptInfo_.timer_->Stop();
            info->ptInfo_.timer_ = nullptr;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearPassInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    ClearPassTimer(rawAddr);
    ClearPassPacket(rawAddr);
}

void AvrcCtConnectManager::PushPassQueue(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtPassPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.cmdQue_.push(pkt);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

std::shared_ptr<AvrcCtPassPacket> AvrcCtConnectManager::PopPassQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    std::shared_ptr<AvrcCtPassPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->ptInfo_.cmdQue_.front();
        info->ptInfo_.cmdQue_.pop();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

uint8_t AvrcCtConnectManager::GetSizeOfPassQueue(const RawAddress &rawAddr)
{
    uint8_t result = 0x00;

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->ptInfo_.cmdQue_.size();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

/******************************************************************
 * UNIT INFO / SUB UNIT INFO COMMAND                              *
 ******************************************************************/

std::shared_ptr<AvrcCtUnitPacket> AvrcCtConnectManager::GetUnitPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcTgConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtUnitPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->unInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

void AvrcCtConnectManager::SetUnitPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtUnitPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcTgConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::SetUnitTimer(
    const RawAddress &rawAddr, std::function<void()> callback, int ms, bool isPeriodic)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.timer_ = std::make_shared<utility::Timer>(callback);
        info->unInfo_.timer_->Start(ms, isPeriodic);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearUnitPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcTgConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearUnitTimer(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        if (info->unInfo_.timer_ != nullptr) {
            info->unInfo_.timer_->Stop();
            info->unInfo_.timer_ = nullptr;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearUnitInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcTgConnectManager::%{public}s", __func__);

    ClearUnitTimer(rawAddr);
    ClearUnitPacket(rawAddr);
}

void AvrcCtConnectManager::PushUnitQueue(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtUnitPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.cmdQue_.push(pkt);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

std::shared_ptr<AvrcCtUnitPacket> AvrcCtConnectManager::PopUnitQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtUnitPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->unInfo_.cmdQue_.front();
        info->unInfo_.cmdQue_.pop();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

uint8_t AvrcCtConnectManager::GetSizeOfUnitQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint8_t result = 0x00;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->unInfo_.cmdQue_.size();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

/******************************************************************
 * VENDOR DEPENDENT COMMAND                                       *
 ******************************************************************/

std::shared_ptr<AvrcCtVendorPacket> AvrcCtConnectManager::GetVendorPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtVendorPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->vdInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

std::shared_ptr<AvrcCtVendorPacket> AvrcCtConnectManager::GetVendorContinuePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtVendorPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->vdInfo_.continuePkt_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

void AvrcCtConnectManager::SetVendorPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtVendorPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::SetVendorContinuePacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtVendorPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.continuePkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::SetVendorTimer(
    const RawAddress &rawAddr, std::function<void()> callback, int ms, bool isPeriodic)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.timer_ = std::make_shared<utility::Timer>(callback);
        info->vdInfo_.timer_->Start(ms, isPeriodic);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearVendorPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearVendorContinuePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.continuePkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearVendorTimer(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        if (info->vdInfo_.timer_ != nullptr) {
            info->vdInfo_.timer_->Stop();
            info->vdInfo_.timer_ = nullptr;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearVendorInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    ClearVendorTimer(rawAddr);
    ClearVendorPacket(rawAddr);
    ClearVendorContinuePacket(rawAddr);
}

void AvrcCtConnectManager::PushVendorQueue(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtVendorPacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.cmdQue_.push(pkt);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

std::shared_ptr<AvrcCtVendorPacket> AvrcCtConnectManager::PopVendorQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtVendorPacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->vdInfo_.cmdQue_.front();
        info->vdInfo_.cmdQue_.pop();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

uint8_t AvrcCtConnectManager::GetSizeOfVendorQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint8_t result = 0x00;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->vdInfo_.cmdQue_.size();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

/******************************************************************
 * BROWSING COMMAND                                               *
 ******************************************************************/

std::shared_ptr<AvrcCtBrowsePacket> AvrcCtConnectManager::GetBrowsePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtBrowsePacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->brInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

void AvrcCtConnectManager::SetBrowsePacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtBrowsePacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::SetBrowseTimer(
    const RawAddress &rawAddr, std::function<void()> callback, int ms, bool isPeriodic)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.timer_ = std::make_shared<utility::Timer>(callback);
        info->brInfo_.timer_->Start(ms, isPeriodic);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearBrowsePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearBrowseTimer(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        if (info->brInfo_.timer_ != nullptr) {
            info->brInfo_.timer_->Stop();
            info->brInfo_.timer_ = nullptr;
        }
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

void AvrcCtConnectManager::ClearBrowseInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    ClearBrowseTimer(rawAddr);
    ClearBrowsePacket(rawAddr);
}

void AvrcCtConnectManager::PushBrowseQueue(const RawAddress &rawAddr, const std::shared_ptr<AvrcCtBrowsePacket> &pkt)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.cmdQue_.push(pkt);
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

std::shared_ptr<AvrcCtBrowsePacket> AvrcCtConnectManager::PopBrowseQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::shared_ptr<AvrcCtBrowsePacket> pkt = nullptr;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->brInfo_.cmdQue_.front();
        info->brInfo_.cmdQue_.pop();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return pkt;
}

uint8_t AvrcCtConnectManager::GetSizeOfBrowseQueue(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    uint8_t result = 0x00;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->brInfo_.cmdQue_.size();
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

bool AvrcCtConnectManager::IsDisableAbsoluteVolume(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool result = false;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->absVolume_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

bool AvrcCtConnectManager::IsBrowsingConnected(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    bool result = false;

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        result = info->brConnected_;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }

    return result;
}

void AvrcCtConnectManager::SetBrowsingState(const RawAddress &rawAddr, bool state)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AvrcCtConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brConnected_ = state;
    } else {
        LOG_DEBUG("[AVRCP CT] The connection information does not exist!");
    }
}

AvrcCtConnectManager::~AvrcCtConnectManager()
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);

    infos_.clear();

    g_instance = nullptr;
}

AvrcCtConnectInfo *AvrcCtConnectManager::GetConnectInfo(const std::string &btAddr)
{
    LOG_DEBUG("[AVRCP CT] AvrcCtConnectManager::%{public}s", __func__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AvrcCtConnectInfo *info = nullptr;

    auto iter = infos_.find(btAddr);
    if (iter != infos_.end()) {
        info = &iter->second;
    }

    return info;
}
}  // namespace bluetooth