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

#include "hfp_ag_audio_connection.h"

#include <cstring>

#include "btstack.h"
#include "hfp_ag_profile_event_sender.h"
#include "raw_address.h"
#include "securec.h"

namespace bluetooth {
std::string HfpAgAudioConnection::g_activeAddr {NULL_ADDRESS};
std::vector<HfpAgAudioConnection::AudioDevice> HfpAgAudioConnection::g_audioDevices {};

BtmScoCallbacks HfpAgAudioConnection::g_cbs = {
    &HfpAgAudioConnection::OnConnectCompleted,
    &HfpAgAudioConnection::OnConnectionChanged,
    &HfpAgAudioConnection::OnDisconnectCompleted,
    &HfpAgAudioConnection::OnConnectRequest,
    &HfpAgAudioConnection::OnWriteVoiceSettingCompleted,
};

void HfpAgAudioConnection::SetRemoteAddr(const std::string &addr)
{
    remoteAddr_ = addr;
}

void HfpAgAudioConnection::SetActiveDevice(const std::string &address)
{
    g_activeAddr = address;
}

bool HfpAgAudioConnection::IsAudioConnected(const std::string &address)
{
    auto dev = GetDeviceByAddr(address);
    if (dev != g_audioDevices.end()) {
        if (dev->lastConnectResult == CONNECT_SUCCESS) {
            return true;
        }
    }

    return false;
}

std::string HfpAgAudioConnection::GetActiveDevice()
{
    return g_activeAddr;
}

BtAddr HfpAgAudioConnection::ConvertToBtAddr(std::string address)
{
    RawAddress rawAddr(address);
    BtAddr btAddr;
    rawAddr.ConvertToUint8(btAddr.addr);
    btAddr.type = BT_PUBLIC_DEVICE_ADDRESS;
    return btAddr;
}

int HfpAgAudioConnection::Register()
{
    LOG_DEBUG("[HFP AG]%{public}s(): Audio Register start", __FUNCTION__);
    g_activeAddr = NULL_ADDRESS;
    std::vector<HfpAgAudioConnection::AudioDevice>().swap(g_audioDevices);
    int ret = BTM_RegisterScoCallbacks(&g_cbs, nullptr);
    HFP_AG_RETURN_IF_FAIL(ret);
    return ret;
}

int HfpAgAudioConnection::Deregister()
{
    LOG_DEBUG("[HFP AG]%{public}s(): Audio Deregister start", __FUNCTION__);
    g_activeAddr = NULL_ADDRESS;
    std::vector<HfpAgAudioConnection::AudioDevice>().swap(g_audioDevices);
    int ret = BTM_DeregisterScoCallbacks(&g_cbs);
    HFP_AG_RETURN_IF_FAIL(ret);
    return ret;
}

void HfpAgAudioConnection::SetSupportFeatures(bool escoSupport, bool escoS4Support, int inUseCodec)
{
    escoSupport_ = escoSupport;
    escoS4Support_ = escoS4Support;
    inUseCodec_ = inUseCodec;
    LOG_DEBUG("[HFP AG]%{public}s(): escoSupport_[%{public}d], escoS4Support_[%{public}d], inUseCodec_[%{public}d]",
        __FUNCTION__,
        escoSupport_,
        escoS4Support_,
        inUseCodec_);
}

std::vector<HfpAgAudioConnection::AudioDevice>::iterator HfpAgAudioConnection::GetDeviceByAddr(const std::string &addr)
{
    std::vector<HfpAgAudioConnection::AudioDevice>::iterator it;
    for (it = g_audioDevices.begin(); it != g_audioDevices.end(); ++it) {
        if (it->addr == addr) {
            break;
        }
    }
    return it;
}

std::vector<HfpAgAudioConnection::AudioDevice>::iterator HfpAgAudioConnection::GetDeviceByHandle(uint16_t handle)
{
    std::vector<HfpAgAudioConnection::AudioDevice>::iterator it;
    for (it = g_audioDevices.begin(); it != g_audioDevices.end(); ++it) {
        if (it->handle == handle) {
            break;
        }
    }
    return it;
}

int HfpAgAudioConnection::ConnectByMsbc(AudioDevice &dev, BtAddr btAddr) const
{
    int ret = BTM_WriteVoiceSetting(BTM_VOICE_SETTING_TRANS);
    HFP_AG_RETURN_IF_FAIL(ret);

    dev.linkType = LINK_TYPE_ESCO;

    if (BTM_IsSecureConnection(&btAddr)) {
        LOG_DEBUG("[HFP AG]%{public}s():Try connect by MSBC T2.", __FUNCTION__);
        BtmCreateEscoConnectionParam param = MSBC_T2_PARAM;
        param.addr = btAddr;
        ret = BTM_CreateEscoConnection(&param);
        HFP_AG_RETURN_IF_FAIL(ret);
        dev.lastParam = MSBC_ESCO_T2;
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():Try connect by MSBC T1.", __FUNCTION__);
        BtmCreateEscoConnectionParam param = MSBC_T1_PARAM;
        param.addr = btAddr;
        ret = BTM_CreateEscoConnection(&param);
        HFP_AG_RETURN_IF_FAIL(ret);
        dev.lastParam = MSBC_ESCO_T1;
    }
    HFP_AG_RETURN_IF_FAIL(ret);

    HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(remoteAddr_, HFP_AG_AUDIO_CONNECTING_EVT);
    return ret;
}

int HfpAgAudioConnection::ConnectByCvsd(AudioDevice &dev, BtAddr btAddr, bool cvsdEscoFailed) const
{
    int ret = BTM_WriteVoiceSetting(BTM_VOICE_SETTING_CVSD);
    HFP_AG_RETURN_IF_FAIL(ret);

    if (escoSupport_ && !cvsdEscoFailed) {
        dev.linkType = LINK_TYPE_ESCO;
        if (escoS4Support_) {
            LOG_DEBUG("[HFP AG]%{public}s():Try connect by CVSD ESCO S4.", __FUNCTION__);
            dev.lastParam = CVSD_ESCO_S4;
            BtmCreateEscoConnectionParam param = CVSD_ESCO_S4_PARAM;
            param.addr = btAddr;
            ret = BTM_CreateEscoConnection(&param);
            HFP_AG_RETURN_IF_FAIL(ret);
        } else {
            LOG_DEBUG("[HFP AG]%{public}s():Try connect by CVSD ESCO S1.", __FUNCTION__);
            dev.lastParam = CVSD_ESCO_S1;
            BtmCreateEscoConnectionParam param = CVSD_ESCO_S1_PARAM;
            param.addr = btAddr;
            ret = BTM_CreateEscoConnection(&param);
            HFP_AG_RETURN_IF_FAIL(ret);
        }
    } else {
        if (!BTM_IsSecureConnection(&btAddr)) {
            dev.linkType = LINK_TYPE_SCO;
            LOG_DEBUG("[HFP AG]%{public}s():Try connect by CVSD SCO.", __FUNCTION__);
            dev.lastParam = CVSD_SCO;
            BtmCreateScoConnectionParam param = CVSD_SCO_PARAM;
            param.addr = btAddr;
            ret = BTM_CreateScoConnection(&param);
            HFP_AG_RETURN_IF_FAIL(ret);
        } else {
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(remoteAddr_, HFP_AG_AUDIO_CONNECT_FAILED_EVT);
            return ret;
        }
    }

    HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(remoteAddr_, HFP_AG_AUDIO_CONNECTING_EVT);
    return ret;
}

int HfpAgAudioConnection::ConnectAudio() const
{
    LOG_DEBUG("[HFP AG]%{public}s():Connect SCO to %{public}s", __FUNCTION__, remoteAddr_.c_str());

    if (remoteAddr_ != g_activeAddr) {
        LOG_WARN("[HFP AG]%{public}s():remoteAddr[%{public}s] and g_activeAddr[%{public}s] match failed!",
            __FUNCTION__,
            remoteAddr_.c_str(),
            g_activeAddr.c_str());
        HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(remoteAddr_, HFP_AG_AUDIO_CONNECT_FAILED_EVT);
        return BT_BAD_PARAM;
    }

    auto dev = GetDeviceByAddr(remoteAddr_);
    if (dev == g_audioDevices.end()) {
        HfpAgAudioConnection::AudioDevice audioDev;
        audioDev.role = ROLE_INITIATOR;
        audioDev.addr = remoteAddr_;
        g_audioDevices.push_back(audioDev);
        dev = GetDeviceByAddr(remoteAddr_);
        LOG_DEBUG("[HFP AG]%{public}s():Create Audio device for [%{public}s]", __FUNCTION__, remoteAddr_.c_str());
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():Audio device [%{public}s] already in device list ", __FUNCTION__, remoteAddr_.c_str());
    }

    bool msbcEscoFailed = false;
    bool cvsdEscoFailed = false;
    if (dev->lastConnectResult == CONNECT_FAIL) {
        if (dev->lastParam == MSBC_ESCO_T2 || dev->lastParam == MSBC_ESCO_T1) {
            msbcEscoFailed = true;
        } else if (dev->lastParam == CVSD_ESCO_S4 || dev->lastParam == CVSD_ESCO_S1) {
            cvsdEscoFailed = true;
        } else {
            LOG_DEBUG(
                "[HFP AG]%{public}s():Audio device [%{public}s], lastParam[%{public}d]", __FUNCTION__, remoteAddr_.c_str(), dev->lastParam);
        }
    }

    BtAddr btAddr = ConvertToBtAddr(remoteAddr_);
    if (inUseCodec_ == HFP_AG_CODEC_MSBC) {
        if (!msbcEscoFailed && escoSupport_) {
            return ConnectByMsbc(*dev, btAddr);
        } else {
            LOG_WARN("[HFP AG]%{public}s():Need re-negotiate codec.", __FUNCTION__);
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_SETUP_CODEC_CVSD);
            return BT_BAD_PARAM;
        }
    } else if (inUseCodec_ == HFP_AG_CODEC_CVSD) {
        return ConnectByCvsd(*dev, btAddr, cvsdEscoFailed);
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():RemoteAddr [%{public}s], invalid codec[%{public}d]", __FUNCTION__, remoteAddr_.c_str(), inUseCodec_);
        return BT_BAD_PARAM;
    }

    return BT_NO_ERROR;
}

int HfpAgAudioConnection::DisconnectAudio() const
{
    LOG_DEBUG("[HFP AG]%{public}s():Disconnect SCO from %{public}s", __FUNCTION__, remoteAddr_.c_str());

    int ret;
    auto dev = GetDeviceByAddr(remoteAddr_);
    if (dev != g_audioDevices.end()) {
        ret = BTM_DisconnectScoConnection(dev->handle, REMOTE_USER_TERMINATED_CONNECTION);
        HFP_AG_RETURN_IF_FAIL(ret);
        HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(remoteAddr_, HFP_AG_AUDIO_DISCONNECTING_EVT);
    } else {
        LOG_WARN("[HFP AG]%{public}s():Invalid Address", remoteAddr_.c_str());
        ret = BT_DEVICE_ERROR;
    }
    return ret;
}

int HfpAgAudioConnection::AcceptByMsbc(BtAddr btAddr)
{
    int ret = BTM_WriteVoiceSetting(BTM_VOICE_SETTING_TRANS);
    HFP_AG_RETURN_IF_FAIL(ret);

    if (BTM_IsSecureConnection(&btAddr)) {
        LOG_DEBUG("[HFP AG]%{public}s():Accept by MSBC T2.", __FUNCTION__);
        BtmCreateEscoConnectionParam param = MSBC_T2_PARAM;
        param.addr = btAddr;
        ret = BTM_AcceptEscoConnectionRequest(&param);
        HFP_AG_RETURN_IF_FAIL(ret);
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():Accept by MSBC T1.", __FUNCTION__);
        BtmCreateEscoConnectionParam param = MSBC_T1_PARAM;
        param.addr = btAddr;
        ret = BTM_AcceptEscoConnectionRequest(&param);
        HFP_AG_RETURN_IF_FAIL(ret);
    }

    return ret;
}

int HfpAgAudioConnection::AcceptByCvsd(const AudioDevice &dev, BtAddr btAddr) const
{
    int ret = BTM_WriteVoiceSetting(BTM_VOICE_SETTING_CVSD);
    HFP_AG_RETURN_IF_FAIL(ret);

    if (dev.linkType == LINK_TYPE_ESCO && escoSupport_) {
        if (escoS4Support_) {
            LOG_DEBUG("[HFP AG]%{public}s():Accept by CVSD ESCO S4.", __FUNCTION__);
            BtmCreateEscoConnectionParam param = CVSD_ESCO_S4_PARAM;
            param.addr = btAddr;
            ret = BTM_AcceptEscoConnectionRequest(&param);
            HFP_AG_RETURN_IF_FAIL(ret);
        } else {
            LOG_DEBUG("[HFP AG]%{public}s():Accept by CVSD ESCO S1.", __FUNCTION__);
            BtmCreateEscoConnectionParam param = CVSD_ESCO_S1_PARAM;
            param.addr = btAddr;
            ret = BTM_AcceptEscoConnectionRequest(&param);
            HFP_AG_RETURN_IF_FAIL(ret);
        }
    } else if (dev.linkType == LINK_TYPE_SCO) {
        LOG_DEBUG("[HFP AG]%{public}s():Accept by CVSD SCO.", __FUNCTION__);
        BtmCreateScoConnectionParam param = CVSD_SCO_PARAM;
        param.addr = btAddr;
        ret = BTM_AcceptScoConnectionRequest(&param);
        HFP_AG_RETURN_IF_FAIL(ret);
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():CVSD ESCO connection fail, linktype[%hhu] and escoSupport[%{public}d] are not matched!",
            __FUNCTION__,
            dev.linkType,
            escoSupport_);
        return BT_BAD_PARAM;
    }

    return ret;
}

int HfpAgAudioConnection::AcceptAudioConnection() const
{
    int ret = BT_NO_ERROR;
    BtAddr btAddr = ConvertToBtAddr(remoteAddr_);

    // Only accpet CVSD sco from remote device.
    auto dev = GetDeviceByAddr(remoteAddr_);
    if (dev != g_audioDevices.end()) {
        if (inUseCodec_ == HFP_AG_CODEC_MSBC) {
            if (dev->linkType == LINK_TYPE_ESCO && escoSupport_) {
                return AcceptByMsbc(btAddr);
            } else {
                LOG_DEBUG("[HFP AG]%{public}s():MSBC ESCO connection fail, linktype[%hhu] and escoSupport[%{public}d] are not matched!",
                    __FUNCTION__,
                    dev->linkType,
                    escoSupport_);
                return BT_BAD_PARAM;
            }
        } else if (inUseCodec_ == HFP_AG_CODEC_CVSD) {
            return AcceptByCvsd(*dev, btAddr);
        } else {
            LOG_DEBUG("[HFP AG]%{public}s():Invalid Codec[%{public}d]!", __FUNCTION__, inUseCodec_);
            return BT_BAD_PARAM;
        }
    } else {
        LOG_WARN("[HFP AG]%{public}s:Invalid Address", remoteAddr_.c_str());
        return BT_BAD_PARAM;
    }

    return ret;
}

int HfpAgAudioConnection::RejectAudioConnection() const
{
    LOG_DEBUG("[HFP AG]%{public}s():Reject sco connect request from %{public}s", __FUNCTION__, remoteAddr_.c_str());
    BtAddr btAddr = ConvertToBtAddr(remoteAddr_);
    BtmRejectScoConnectionRequestParam param = {btAddr, REJECT_DUE_TO_LIMITED_RESOURCES};

    int ret = BTM_RejectScoConnectionRequest(&param);
    HFP_AG_RETURN_IF_FAIL(ret);

    return ret;
}

void HfpAgAudioConnection::OnConnectRequest(const BtmScoConnectionRequestParam *param, void *context)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);
    HfpScoConnectionRequestParam parameters;
    parameters.linkType = param->linkType;
    (void)memcpy_s(&parameters.addr, sizeof(BtAddr), param->addr, sizeof(BtAddr));
    HfpAgProfileEventSender::GetInstance().GetDispatchter()->PostTask(
        std::bind(&HfpAgAudioConnection::ProcessOnConnectRequest, parameters));
}

void HfpAgAudioConnection::ProcessOnConnectRequest(HfpScoConnectionRequestParam parameters)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);

    RawAddress btAddr = RawAddress::ConvertToString(parameters.addr.addr);
    std::string address = btAddr.GetAddress();

    auto dev = GetDeviceByAddr(address);
    if (dev == g_audioDevices.end()) {
        HfpAgAudioConnection::AudioDevice audioDev;
        audioDev.role = ROLE_ACCEPTOR;
        audioDev.addr = address;
        audioDev.linkType = parameters.linkType;
        g_audioDevices.push_back(audioDev);
        LOG_DEBUG("[HFP AG]%{public}s():Create Audio device for [%{public}s]", __FUNCTION__, address.c_str());
    } else {
        dev->linkType = parameters.linkType;
        LOG_DEBUG("[HFP AG]%{public}s():Audio device [%{public}s] already in device list ", __FUNCTION__, address.c_str());
    }
    HfpAgProfileEventSender::GetInstance().ScoConnectRequest(
        btAddr.GetAddress(), HFP_AG_AUDIO_CONNECT_REQUEST_EVT, parameters.linkType);
}

void HfpAgAudioConnection::OnConnectCompleted(const BtmScoConnectionCompleteParam *param, void *context)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);
    HfpScoConnectionCompleteParam parameters;
    parameters.status = param->status;
    parameters.connectionHandle = param->connectionHandle;
    (void)memcpy_s(&parameters.addr, sizeof(BtAddr), param->addr, sizeof(BtAddr));
    HfpAgProfileEventSender::GetInstance().GetDispatchter()->PostTask(
        std::bind(&HfpAgAudioConnection::ProcessOnConnectCompleted, parameters));
}

void HfpAgAudioConnection::ProcessOnConnectCompletedFail(
    std::vector<HfpAgAudioConnection::AudioDevice>::iterator dev, const std::string &address)
{
    dev->lastConnectResult = CONNECT_FAIL;
    if (dev->role == ROLE_INITIATOR) {
        if (dev->lastParam == MSBC_ESCO_T2 || dev->lastParam == MSBC_ESCO_T1) {
            LOG_DEBUG("[HFP AG]%{public}s():MSBC ESCO failed, try CVSD ESCO.", __FUNCTION__);
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_SETUP_CODEC_CVSD);
        } else if (dev->lastParam == CVSD_ESCO_S4 || dev->lastParam == CVSD_ESCO_S1) {
            LOG_DEBUG("[HFP AG]%{public}s():CVSD ESCO failed, try CVSD SCO", __FUNCTION__);
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_RETRY_CONNECT_AUDIO_EVT);
        } else if (dev->lastParam == CVSD_SCO) {
            LOG_DEBUG("[HFP AG]%{public}s():CVSD SCO failed, report fail event to service", __FUNCTION__);
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_AUDIO_CONNECT_FAILED_EVT);
        } else {
            LOG_DEBUG("[HFP AG]%{public}s():Invalidaddress[%{public}s] lastParam[%{public}d]", __FUNCTION__, address.c_str(), dev->lastParam);
        }
    } else {
        // As acceptor, report connect failed event to service directly.
        HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_AUDIO_CONNECT_FAILED_EVT);
        LOG_DEBUG("[HFP AG]%{public}s(): Accept SCO from address[%{public}s]failed", __FUNCTION__, address.c_str());
    }
}

void HfpAgAudioConnection::ProcessOnConnectCompleted(HfpScoConnectionCompleteParam parameters)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);

    RawAddress btAddr = RawAddress::ConvertToString(parameters.addr.addr);
    std::string address = btAddr.GetAddress();
    auto dev = GetDeviceByAddr(address);
    if (dev != g_audioDevices.end()) {
        dev->addr = address;
        dev->handle = parameters.connectionHandle;
        if (!parameters.status) {
            LOG_DEBUG("[HFP AG]%{public}s(): SCO connect successfully!", __FUNCTION__);
            dev->lastConnectResult = CONNECT_SUCCESS;
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(dev->addr, HFP_AG_AUDIO_CONNECTED_EVT);
        } else {
            ProcessOnConnectCompletedFail(dev, address);
        }
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():Invalid audio device", address.c_str());
    }
}

void HfpAgAudioConnection::OnDisconnectCompleted(const BtmScoDisconnectionCompleteParam *param, void *context)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);
    HfpScoDisconnectionCompleteParam parameters;
    parameters.connectionHandle = param->connectionHandle;
    parameters.reason = param->reason;
    parameters.status = param->status;
    HfpAgProfileEventSender::GetInstance().GetDispatchter()->PostTask(
        std::bind(&HfpAgAudioConnection::ProcessOnDisconnectCompleted, parameters));
}

void HfpAgAudioConnection::ProcessOnDisconnectCompleted(HfpScoDisconnectionCompleteParam parameters)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);

    auto it = GetDeviceByHandle(parameters.connectionHandle);
    if (it != g_audioDevices.end()) {
        if (!parameters.status) {
            LOG_DEBUG("[HFP AG]%{public}s(): Disconnect SCO from address[%{public}s] successfully.", __FUNCTION__, it->addr.c_str());
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(it->addr, HFP_AG_AUDIO_DISCONNECTED_EVT);
            g_audioDevices.erase(it);
        } else {
            LOG_DEBUG("[HFP AG]%{public}s(): Disconnect SCO from address[%{public}s] failed.", __FUNCTION__, it->addr.c_str());
            HfpAgProfileEventSender::GetInstance().UpdateScoConnectState(
                it->addr, HFP_AG_AUDIO_DISCONNECT_FAILED_EVT);
        }
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():Invalid audio device", __FUNCTION__);
    }
}

void HfpAgAudioConnection::OnConnectionChanged(const BtmScoConnectionChangedParam *param, void *context)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);
    LOG_DEBUG("[HFP AG]%{public}s():connectionHandle[%hu]", __FUNCTION__, param->connectionHandle);
}

void HfpAgAudioConnection::OnWriteVoiceSettingCompleted(uint8_t status, void *context)
{
    LOG_DEBUG("[HFP AG]%{public}s():", __PRETTY_FUNCTION__);
    LOG_DEBUG("[HFP AG]%{public}s():status[%hhu]", __FUNCTION__, status);
}
}  // namespace bluetooth