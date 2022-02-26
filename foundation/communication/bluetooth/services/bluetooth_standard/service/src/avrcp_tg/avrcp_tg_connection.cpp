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
#include <algorithm>
#include "avrcp_tg_connection.h"
#include "avrcp_tg_packet.h"
#include "avrcp_tg_pass_through.h"

namespace bluetooth {
AvrcTgConnectManager *AvrcTgConnectManager::g_instance = nullptr;

AvrcTgConnectInfo::AvrcTgConnectInfo(const std::string &btAddr, uint8_t connectId, uint8_t role, uint16_t controlMtu,
    uint16_t browseMtu, uint32_t companyId, uint16_t uidCounter, AvctChannelEventCallback eventCallback,
    AvctMsgCallback msgCallback)
    : eventCallback_(eventCallback),
      msgCallback_(msgCallback),
      connectId_(connectId),
      role_(role),
      controlMtu_(controlMtu),
      browseMtu_(browseMtu),
      companyId_(companyId),
      uidCounter_(uidCounter),
      btAddr_(btAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectInfo::%{public}s", __func__);

    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYBACK_STATUS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_REACHED_END, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_TRACK_REACHED_START, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYBACK_POS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_BATT_STATUS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_SYSTEM_STATUS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_PLAYER_APPLICATION_SETTING_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_NOW_PLAYING_CONTENT_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_AVAILABLE_PLAYERS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_ADDRESSED_PLAYER_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_UIDS_CHANGED, false));
    notes_.insert(std::make_pair(AVRC_EVENT_ID_VOLUME_CHANGED, false));

    ptInfo_ = {nullptr, nullptr};
    vdInfo_ = {nullptr, nullptr};
    brInfo_ = {nullptr, nullptr};
}

AvrcTgConnectInfo::~AvrcTgConnectInfo()
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectInfo::%{public}s", __func__);

    if (ptInfo_.timer_ != nullptr) {
        ptInfo_.timer_->Stop();
    }

    if (unInfo_.timer_ != nullptr) {
        unInfo_.timer_->Stop();
    }

    if (vdInfo_.timer_ != nullptr) {
        vdInfo_.timer_->Stop();
    }

    if (brInfo_.timer_ != nullptr) {
        brInfo_.timer_->Stop();
    }
}

AvrcTgConnectManager *AvrcTgConnectManager::GetInstance(void)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    if (g_instance == nullptr) {
        g_instance = new (std::nothrow) AvrcTgConnectManager();
    }

    return g_instance;
}

int AvrcTgConnectManager::Add(const RawAddress &rawAddr, uint8_t connectId, uint8_t role, uint16_t controlMtu,
    uint16_t browseMtu, uint32_t companyId, uint16_t uidCounter, AvctChannelEventCallback eventCallback,
    AvctMsgCallback msgCallback)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    int result = RET_NO_ERROR;

    if (GetConnectInfo(rawAddr.GetAddress()) == nullptr) {
        infos_.insert(std::make_pair(rawAddr.GetAddress(),
            AvrcTgConnectInfo(rawAddr.GetAddress(),
                connectId,
                role,
                controlMtu,
                browseMtu,
                companyId,
                uidCounter,
                eventCallback,
                msgCallback)));
    } else {
        result = RET_BAD_PARAM;
        LOG_DEBUG("[AVRCP TG] The connection information exists!");
    }

    return result;
}

void AvrcTgConnectManager::Delete(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    infos_.erase(rawAddr.GetAddress());
}

const AvrcTgConnectInfo *AvrcTgConnectManager::GetConnectInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    const AvrcTgConnectInfo *info = nullptr;

    auto iter = infos_.find(rawAddr.GetAddress());
    if (iter != infos_.end()) {
        info = &iter->second;
    }

    return info;
}

RawAddress AvrcTgConnectManager::GetRawAddress(uint8_t connectId) const
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::string addr(AVRC_TG_DEFAULT_BLUETOOTH_ADDRESS);

    for (auto &info : infos_) {
        if (info.second.connectId_ == connectId) {
            addr = info.second.btAddr_;
            break;
        }
    }

    return RawAddress(addr);
}

std::list<std::string> AvrcTgConnectManager::GetDeviceAddresses(void)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    std::list<std::string> addrs;
    std::transform(infos_.begin(), infos_.end(), std::inserter(addrs, addrs.end()), 
        [](const std::pair<std::string, AvrcTgConnectInfo>& info){return info.second.btAddr_;});
    return addrs;
}

void AvrcTgConnectManager::SetActiveDevice(const std::string addr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] rawAddr[%{public}s]", addr.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    activeAddr_ = addr;
}

const std::string &AvrcTgConnectManager::GetActiveDevice(void)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    LOG_DEBUG("[AVRCP TG] rawAddr[%{public}s]", activeAddr_.c_str());

    return activeAddr_;
}

void AvrcTgConnectManager::AddDisconnectedDevice(const std::string addr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    disconnectedDevices_.push_back(addr);
}

void AvrcTgConnectManager::DeleteDisconnectedDevice(const std::string addr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    for (std::list<std::string>::iterator it = disconnectedDevices_.begin(); it != disconnectedDevices_.end(); it++) {
        if (it->compare(addr) == 0x00) {
            it = disconnectedDevices_.erase(it);
        }
    }
}

const std::list<std::string> &AvrcTgConnectManager::GetAllDisconnectedDevices(void)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::lock_guard<std::mutex> lock(mutex_);

    return disconnectedDevices_;
}

bool AvrcTgConnectManager::IsConnectInfoEmpty(void) const
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    return infos_.empty();
}

uint8_t AvrcTgConnectManager::GetConnectId(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    uint8_t connectId = 0x00;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        connectId = info->connectId_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return connectId;
}

void AvrcTgConnectManager::SetConnectId(const RawAddress &rawAddr, uint8_t connectId)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] connectId[%u]", connectId);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->connectId_ = connectId;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

uint16_t AvrcTgConnectManager::GetControlMtu(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    uint16_t mtu = AVRC_TG_DEFAULT_CONTROL_MTU_SIZE;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        mtu = info->controlMtu_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return mtu;
}

void AvrcTgConnectManager::SetControlMtu(const RawAddress &rawAddr, uint16_t mtu)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] mtu[%u]", mtu);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->controlMtu_ = mtu;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

uint16_t AvrcTgConnectManager::GetBrowseMtu(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    uint16_t mtu = AVRC_TG_DEFAULT_BROWSE_MTU_SIZE;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        mtu = info->browseMtu_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return mtu;
}

void AvrcTgConnectManager::SetBrowseMtu(const RawAddress &rawAddr, uint16_t mtu)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] mtu[%u]", mtu);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->browseMtu_ = mtu;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::EnableNotifyState(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] notification[%u]", notification);

    std::lock_guard<std::mutex> lock(mutex_);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            iter->second = true;
        }
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::DisableNotifyState(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] notification[%{public}d]", notification);

    std::lock_guard<std::mutex> lock(mutex_);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            iter->second = false;
        }
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

bool AvrcTgConnectManager::IsNotifyStateEnabled(const RawAddress &rawAddr, uint8_t notification)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] notification[%{public}d]", notification);

    std::lock_guard<std::mutex> lock(mutex_);

    bool result = false;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        auto iter = info->notes_.find(notification);
        if (iter != info->notes_.end()) {
            result = iter->second;
        }
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    LOG_DEBUG("[AVRCP TG] result[%{public}d]", result);

    return result;
}

uint32_t AvrcTgConnectManager::GetCompanyId(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    uint32_t companyId = AVRC_TG_DEFAULT_BLUETOOTH_SIG_COMPANY_ID;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        companyId = info->companyId_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return companyId;
}

uint16_t AvrcTgConnectManager::GetUidCounter(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    uint16_t uidCounter = 0x00;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        uidCounter = info->uidCounter_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return uidCounter;
}

void AvrcTgConnectManager::SetUidCounter(const RawAddress &rawAddr, uint16_t uidCounter)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);
    LOG_DEBUG("[AVRCP TG] uidCounter[%u]", uidCounter);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->uidCounter_ = uidCounter;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

/******************************************************************
 * PASS THROUGH COMMAND                                           *
 ******************************************************************/

std::shared_ptr<AvrcTgPassPacket> AvrcTgConnectManager::GetPassPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::shared_ptr<AvrcTgPassPacket> pkt = nullptr;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->ptInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return pkt;
}

void AvrcTgConnectManager::SetPassPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcTgPassPacket> &pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

bool AvrcTgConnectManager::IsPassTimerEmpty(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    bool result = true;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr && info->ptInfo_.timer_ != nullptr) {
        result = false;
    } else {
        LOG_DEBUG("[AVRCP TG] The PASS THROUGH timer is empty!");
    }

    return result;
}

void AvrcTgConnectManager::SetPassTimer(
    const RawAddress &rawAddr, std::function<void()> callback, int ms, bool isPeriodic)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.timer_ = std::make_shared<utility::Timer>(callback);
        info->ptInfo_.timer_->Start(ms, isPeriodic);
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearPassPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->ptInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearPassTimer(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        if (info->ptInfo_.timer_ != nullptr) {
            info->ptInfo_.timer_->Stop();
            info->ptInfo_.timer_ = nullptr;
        }
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearPassInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    ClearPassTimer(rawAddr);
    ClearPassPacket(rawAddr);
}

/******************************************************************
 * UNIT INFO / SUB UNIT INFO COMMAND                              *
 ******************************************************************/

std::shared_ptr<AvrcTgUnitPacket> AvrcTgConnectManager::GetUnitPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::shared_ptr<AvrcTgUnitPacket> pkt = nullptr;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->unInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return pkt;
}

void AvrcTgConnectManager::SetUnitPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcTgUnitPacket> &pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearUnitPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->unInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearUnitInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    ClearUnitPacket(rawAddr);
}

/******************************************************************
 * VENDOR DEPENDENT COMMAND                                       *
 ******************************************************************/

std::shared_ptr<AvrcTgVendorPacket> AvrcTgConnectManager::GetVendorPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::shared_ptr<AvrcTgVendorPacket> pkt = nullptr;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->vdInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return pkt;
}

void AvrcTgConnectManager::SetVendorPacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcTgVendorPacket> &pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcCtConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearVendorPacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->vdInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearVendorInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    ClearVendorPacket(rawAddr);
}

/******************************************************************
 * BROWSING COMMAND                                               *
 ******************************************************************/

std::shared_ptr<AvrcTgBrowsePacket> AvrcTgConnectManager::GetBrowsePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    std::shared_ptr<AvrcTgBrowsePacket> pkt = nullptr;

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        pkt = info->brInfo_.pkt_;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }

    return pkt;
}

void AvrcTgConnectManager::SetBrowsePacket(const RawAddress &rawAddr, const std::shared_ptr<AvrcTgBrowsePacket> &pkt)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.pkt_ = pkt;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearBrowsePacket(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = GetConnectInfo(rawAddr.GetAddress());
    if (info != nullptr) {
        info->brInfo_.pkt_ = nullptr;
    } else {
        LOG_DEBUG("[AVRCP TG] The connection information does not exist!");
    }
}

void AvrcTgConnectManager::ClearBrowseInfo(const RawAddress &rawAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    ClearBrowsePacket(rawAddr);
}

AvrcTgConnectManager::~AvrcTgConnectManager()
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    infos_.clear();

    g_instance = nullptr;
}

AvrcTgConnectInfo *AvrcTgConnectManager::GetConnectInfo(const std::string &btAddr)
{
    LOG_DEBUG("[AVRCP TG] AvrcTgConnectManager::%{public}s", __func__);

    AvrcTgConnectInfo *info = nullptr;

    auto iter = infos_.find(btAddr);
    if (iter != infos_.end()) {
        info = &iter->second;
    }

    return info;
}
}  // namespace bluetooth
