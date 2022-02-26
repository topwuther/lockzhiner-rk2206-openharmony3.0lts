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

#include "hfp_ag_sdp_server.h"

#include "hfp_ag_data_connection.h"
#include "hfp_ag_defines.h"
#include "raw_address.h"
#include "rfcomm.h"

namespace bluetooth {
HfpAgSdpServer &HfpAgSdpServer::GetInstance()
{
    static HfpAgSdpServer instance;
    return instance;
}

int HfpAgSdpServer::RegisterSdpService(uint8_t scn)
{
    if (scn == 0) {
        LOG_ERROR("[HFP AG]%{public}s():Unavailable server channel number", __FUNCTION__);
        return HFP_AG_FAILURE;
    }

    // Create and register service record
    sdpHandle_ = SDP_CreateServiceRecord();

    int ret = AddServiceClassId();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddProtocol(scn);
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddProfile();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddServiceName();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddNetwork();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddFeatures();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = AddBrowseGroupList();
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = SDP_RegisterServiceRecord(sdpHandle_);
    HFP_AG_RETURN_IF_FAIL(ret);
    return ret;
}

int HfpAgSdpServer::DeregisterSdpService()
{
    int ret = SDP_DeregisterServiceRecord(sdpHandle_);
    HFP_AG_RETURN_IF_FAIL(ret);

    ret = SDP_DestroyServiceRecord(sdpHandle_);
    HFP_AG_RETURN_IF_FAIL(ret);

    sdpHandle_ = 0;
    return ret;
}

int HfpAgSdpServer::AddServiceClassId() const
{
    BtUuid classId[HFP_AG_SERVER_CLASSID_NUM];
    classId[0].type = BT_UUID_16;
    classId[0].uuid16 = HFP_AG_UUID_SERVCLASS_HFP_AG;
    classId[1].type = BT_UUID_16;
    classId[1].uuid16 = HFP_AG_UUID_SERVCLASS_GENERIC_AUDIO;
    return SDP_AddServiceClassIdList(sdpHandle_, classId, HFP_AG_SERVER_CLASSID_NUM);
}

int HfpAgSdpServer::AddProtocol(uint8_t scn) const
{
    SdpProtocolDescriptor protocol[HFP_AG_SERVER_PROTOCOL_NUM];
    protocol[0].protocolUuid.type = BT_UUID_16;
    protocol[0].protocolUuid.uuid16 = UUID_PROTOCOL_L2CAP;
    protocol[0].parameterNumber = 0;
    protocol[1].protocolUuid.type = BT_UUID_16;
    protocol[1].protocolUuid.uuid16 = UUID_PROTOCOL_RFCOMM;
    protocol[1].parameterNumber = 1;
    protocol[1].parameter[0].type = SDP_TYPE_UINT_8;
    protocol[1].parameter[0].value = scn;
    return SDP_AddProtocolDescriptorList(sdpHandle_, protocol, HFP_AG_SERVER_PROTOCOL_NUM);
}

int HfpAgSdpServer::AddProfile() const
{
    SdpProfileDescriptor profile[HFP_AG_SERVER_PROFILE_NUM];
    profile[0].profileUuid.type = BT_UUID_16;
    profile[0].profileUuid.uuid16 = HFP_AG_UUID_SERVCLASS_HFP_AG;
    profile[0].versionNumber = HFP_AG_HFP_VERSION_1_7;
    return SDP_AddBluetoothProfileDescriptorList(sdpHandle_, profile, HFP_AG_SERVER_PROFILE_NUM);
}

int HfpAgSdpServer::AddServiceName() const
{
    return SDP_AddServiceName(sdpHandle_, SDP_ATTRIBUTE_PRIMARY_LANGUAGE_BASE,
        HFP_AG_SERVER_SERVICE_NAME.c_str(), HFP_AG_SERVER_SERVICE_NAME.length());
}

int HfpAgSdpServer::AddNetwork() const
{
    uint32_t features = HfpAgDataConnection::GetLocalFeatures();
    uint8_t networkValue = (features & HFP_AG_FEATURES_REJECT_CALL) ? 1 : 0;
    SdpAttribute network;
    network.attributeId = HFP_AG_SDP_ATTRIBUTE_DATA_STORES_OR_NETWORK;
    network.type = SDP_TYPE_UINT_8;
    network.attributeValueLength = HFP_AG_SERVER_NETWORK_LENGTH;
    network.attributeValue = &networkValue;
    return AddAttribute(network);
}

int HfpAgSdpServer::AddFeatures() const
{
    uint32_t features = HfpAgDataConnection::GetLocalFeatures();
    bool codecMSBC = (features & HFP_AG_FEATURES_CODEC_NEGOTIATION) ? true : false;
    features &= HFP_AG_FEATURES_SDP_SPEC;
    if (codecMSBC) {
        // Codec bit position is different in SDP (bit 5) and in BRSF (bit 7)
        features |= HFP_AG_HF_FEATURES_SUPPORT_WBS;
    }
    SdpAttribute featuresAttr;
    featuresAttr.attributeId = HFP_AG_SDP_ATTRIBUTE_SUPPORTED_FEATURES;
    featuresAttr.type = SDP_TYPE_UINT_16;
    featuresAttr.attributeValueLength = HFP_AG_SERVER_FEATURES_LENGTH;
    featuresAttr.attributeValue = &features;
    return AddAttribute(featuresAttr);
}

int HfpAgSdpServer::AddBrowseGroupList() const
{
    BtUuid browseGroupList[HFP_AG_SERVER_BROWSE_LIST_NUM];
    browseGroupList[0].type = BT_UUID_16;
    browseGroupList[0].uuid16 = SDP_PUBLIC_BROWSE_GROUP_ROOT_UUID;
    return SDP_AddBrowseGroupList(sdpHandle_, browseGroupList, HFP_AG_SERVER_BROWSE_LIST_NUM);
}

int HfpAgSdpServer::AddAttribute(const SdpAttribute &attribute) const
{
    return SDP_AddAttribute(
        sdpHandle_, attribute.attributeId, attribute.type, attribute.attributeValue, attribute.attributeValueLength);
}
}  // namespace bluetooth