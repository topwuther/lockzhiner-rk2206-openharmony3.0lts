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

#include "hfp_ag_system_interface.h"

#include "hfp_ag_service.h"
#include "log.h"
#include "stub/telephone_service.h"

namespace bluetooth {
HfpAgSystemInterface::HfpAgSystemInterface()
{}

HfpAgSystemInterface &HfpAgSystemInterface::GetInstance()
{
    static HfpAgSystemInterface hfpAgSystemInterface;
    return hfpAgSystemInterface;
}

void HfpAgSystemInterface::Start()
{
    QueryAgIndicator();
    return;
}

void HfpAgSystemInterface::Stop()
{
    serviceState_ = 0;
    signalStrength_ = 0;
    roamState_ = 0;
    batteryLevel_ = 0;
    activeNum_ = 0;
    heldNum_ = 0;
    callState_ = HFP_AG_CALL_STATE_DISCONNECTED;
    return;
}

void HfpAgSystemInterface::DialOutCall(const std::string &address, const std::string &number) const
{
    stub::TelephoneService::GetInstance()->DialOutCall(number);
    return;
}

void HfpAgSystemInterface::HangupCall(const std::string &address) const
{
    stub::TelephoneService::GetInstance()->HangupCall();
    return;
}

void HfpAgSystemInterface::AnswerCall(const std::string &address) const
{
    stub::TelephoneService::GetInstance()->AnswerCall();
}

bool HfpAgSystemInterface::SendDtmf(int dtmf, const std::string &address) const
{
    stub::TelephoneService::GetInstance()->SendDtmf(dtmf);
    return true;
}

bool HfpAgSystemInterface::HoldCall(int chld) const
{
    stub::TelephoneService::GetInstance()->ProcessChld(chld);
    return true;
}

std::string HfpAgSystemInterface::GetNetworkOperator() const
{
    std::string name = stub::TelephoneService::GetInstance()->GetNetworkOperator();
    return name;
}

std::string HfpAgSystemInterface::GetSubscriberNumber() const
{
    std::string number = stub::TelephoneService::GetInstance()->GetSubscriberNumber();
    return number;
}

bool HfpAgSystemInterface::QueryCurrentCallsList() const
{
    return stub::TelephoneService::GetInstance()->ListCurrentCalls();
}

void HfpAgSystemInterface::QueryPhoneState() const
{
    stub::TelephoneService::GetInstance()->QueryPhoneState();
    return;
}

bool HfpAgSystemInterface::StartVoiceRecognition(const std::string &address) const
{
    return stub::TelephoneService::GetInstance()->StartVoiceRecognition(address);
}

bool HfpAgSystemInterface::StopVoiceRecogition(const std::string &address) const
{
    return stub::TelephoneService::GetInstance()->StopVoiceRecogition(address);
}

bool HfpAgSystemInterface::SetAudioParameters(const std::string &parameters) const
{
    return stub::TelephoneService::GetInstance()->SetAudioParameters(parameters);
}

void HfpAgSystemInterface::SetStreamVolume(int streamType, int volume, int flag) const
{
    stub::TelephoneService::GetInstance()->SetStreamVolume(streamType, volume, flag);
}

std::string HfpAgSystemInterface::GetLastDialNumber() const
{
    std::string number = stub::TelephoneService::GetInstance()->GetLastDialNumber();
    return number;
}

int HfpAgSystemInterface::GetServiceState() const
{
    return serviceState_;
}

int HfpAgSystemInterface::GetSignalStrength() const
{
    return signalStrength_;
}

int HfpAgSystemInterface::GetRoamState() const
{
    return roamState_;
}

int HfpAgSystemInterface::GetBatteryLevel() const
{
    return batteryLevel_;
}

void HfpAgSystemInterface::QueryAgIndicator()
{
    // query ag indicator for the other module.

    serviceState_ = stub::TelephoneService::GetInstance()->GetRegistrationStatus();
    batteryLevel_ = stub::TelephoneService::GetInstance()->GetBatteryLevel();
    signalStrength_ = stub::TelephoneService::GetInstance()->GetSignalStrength();
    roamState_ = stub::TelephoneService::GetInstance()->GetRoamingStatus();
    SendServiceStateToService();
    SendRoamStateToService();
    SendSignalStrengthToService();
    SendBatteryLevelToService();
    return;
}

void HfpAgSystemInterface::SendHfIndicator(const std::string &address, int indId, int indValue) const
{
    RawAddress device(address);
    if (indId == HFP_AG_HF_INDICATOR_ENHANCED_DRIVER_SAFETY_ID) {
        (*HfpAgService::GetService()).NotifyHfEnhancedDriverSafety(device, indValue);
        stub::TelephoneService::GetInstance()->NotifyHfEnhancedDriverSafety(indValue);
    } else if (indId == HFP_AG_HF_INDICATOR_BATTERY_LEVEL_ID) {
        (*HfpAgService::GetService()).NotifyHfBatteryLevel(device, indValue);
        stub::TelephoneService::GetInstance()->NotifyHfBatteryLevel(indValue);
    } else {
        LOG_DEBUG("[HFP AG]%{public}s():HF Indicator id:%{public}d", __FUNCTION__, indId);
    }
    return;
}

void HfpAgSystemInterface::SetActiveCallNumber(int number)
{
    activeNum_ = number;
}

int HfpAgSystemInterface::GetActiveCallNumber() const
{
    return activeNum_;
}

void HfpAgSystemInterface::SetHeldCallNumber(int number)
{
    heldNum_ = number;
}

int HfpAgSystemInterface::GetHeldCallNumber() const
{
    return heldNum_;
}

void HfpAgSystemInterface::SetCallState(int state)
{
    callState_ = state;
}

int HfpAgSystemInterface::GetCallState() const
{
    return callState_;
}

void HfpAgSystemInterface::SendServiceStateToService() const
{
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_SERVICE_STATE, serviceState_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendRoamStateToService() const
{
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_ROAM_STATE, roamState_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendSignalStrengthToService() const
{
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_SIGNAL_STRENGTH, signalStrength_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::SendBatteryLevelToService() const
{
    HfpAgService *service = HfpAgService::GetService();
    if (service != nullptr) {
        service->NotifyAgIndicatorStateChanged(HFP_AG_NOTIFY_BATTERY_LEVEL, batteryLevel_);
    } else {
        LOG_ERROR("[HFP AG]%{public}s():service is null!", __FUNCTION__);
    }
}

void HfpAgSystemInterface::OnSubscriptionStateChanged(int state)
{
    int preService = serviceState_;
    serviceState_ = state;
    if (preService != serviceState_) {
        SendServiceStateToService();
    }
}

void HfpAgSystemInterface::OnSignalStrengthChanged(int signal)
{
    int preSignal = signalStrength_;
    signalStrength_ = signal;
    if (preSignal != signalStrength_) {
        SendSignalStrengthToService();
    }
}

void HfpAgSystemInterface::OnRoamStateChanged(int state)
{
    int preRoam = roamState_;
    roamState_ = state;
    if (preRoam != roamState_) {
        SendRoamStateToService();
    }
}

void HfpAgSystemInterface::OnBatteryLevel(int level)
{
    int preBattery = batteryLevel_;
    batteryLevel_ = level;
    if (preBattery != batteryLevel_) {
        SendBatteryLevelToService();
    }
}

bool HfpAgSystemInterface::IsInCall() const
{
    return ((activeNum_ > 0) || (heldNum_ > 0) ||
            ((callState_ != HFP_AG_CALL_STATE_IDLE) && (callState_ != HFP_AG_CALL_STATE_INCOMING) &&
                (callState_ != HFP_AG_CALL_STATE_DISCONNECTED)));
}

bool HfpAgSystemInterface::IsRinging() const
{
    return (callState_ == HFP_AG_CALL_STATE_INCOMING);
}
}  // namespace bluetooth
