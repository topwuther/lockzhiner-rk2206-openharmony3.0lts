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

#include <sstream>
#include <iomanip>

#include "msg_request_auth.h"
#include "device_manager_log.h"
#include "constants.h"
#include "encrypt_utils.h"
#include "device_manager_errno.h"
#include "device_manager_log.h"
#include "encrypt_utils.h"
#include "msg_request_auth.h"
#include "parameter.h"
#include "softbus_session.h"

namespace OHOS {
namespace DistributedHardware {
MsgRequestAuth::MsgRequestAuth(std::string &token, std::string hostPkgName, std::string targetPkgName,
    const int32_t groupVisibility, const DmDeviceInfo& devReqInfo, const DmAppImageInfo &imageInfo,
    std::string &extras)
{
    DMLOG(DM_LOG_INFO, "MsgRequestAuth construction started");
    nlohmann::json jsonObject = nlohmann::json::parse(extras, nullptr, false);
    if (jsonObject.is_discarded()) {
        DMLOG(DM_LOG_ERROR, "MsgRequestAuth error");
        return;
    }
    if (!jsonObject.contains(APP_NAME_KEY) || !jsonObject.contains(APP_DESCRIPTION_KEY)) {
        DMLOG(DM_LOG_ERROR, "MsgRequestAuth, err json string");
        return;
    }

    mAuthType_ = jsonObject.contains(AUTH_TYPE) ? (int32_t)jsonObject[AUTH_TYPE] : AUTH_TYPE_QR;
    mHead_ = std::make_shared<MsgHead>((mAuthType_ == AUTH_TYPE_QR) ? (DmMsgType::MSG_TYPE_REQ_AUTH) :
        (DmMsgType::MSG_TYPE_AUTH_BY_PIN));
    char localDeviceId[DEVICE_UUID_LENGTH] = {0};
    GetDevUdid(localDeviceId, DEVICE_UUID_LENGTH);
    mDeviceName_ = devReqInfo.deviceName;
    mDeviceId_ = localDeviceId;
    mToken_ = token;
    mHostPkg_ = hostPkgName;
    mTargetPkg_ = targetPkgName;
    mGroupVisibility_ = groupVisibility;
    mAppName_ = jsonObject[APP_NAME_KEY];
    mAppDescription_ = jsonObject[APP_DESCRIPTION_KEY];
    mImageInfo_ = imageInfo;
    mThumbnailSize_ = mImageInfo_.GetAppThumbnailLen();
    mAppIconSize_ = mImageInfo_.GetAppIconLen();
    mDeviceType_ = ToHexString(devReqInfo.deviceTypeId);
    DMLOG(DM_LOG_INFO, "MsgRequestAuth construction completed");
}

int32_t MsgRequestAuth::GetEncodedAppInfo(const uint8_t *dataSrc, int32_t srcLen, std::string &outString)
{
    DMLOG(DM_LOG_INFO, "MsgRequestAuth GetEncodedAppInfo started");
    if (srcLen <= 0 || dataSrc == nullptr) {
        DMLOG(DM_LOG_ERROR, "data string is empty");
        return DEVICEMANAGER_OK;
    }

    int32_t tempBufLen = ((srcLen / BASE64_BYTE_LEN_3) + 1) * BASE64_BYTE_LEN_4 + 1;
    char *tmpBuf = (char *)calloc(sizeof(char), tempBufLen);
    if (tmpBuf == nullptr) {
        DMLOG(DM_LOG_ERROR, "getEncodedAppInfoString: malloc mem error, size %d", tempBufLen);
        return DEVICEMANAGER_MALLOC_ERROR;
    }

    size_t outLen = 0;
    int32_t ret = EncryptUtils::MbedtlsBase64Encode((uint8_t *)tmpBuf, tempBufLen, &outLen, dataSrc, (size_t)srcLen);
    if (ret != 0) {
        DMLOG(DM_LOG_ERROR, "MbedtlsBase64Encode error");
        free(tmpBuf);
        return ENCODE_DATA_ERROR;
    }
    outString = tmpBuf;
    free(tmpBuf);
    tmpBuf = nullptr;
    DMLOG(DM_LOG_INFO, "MsgRequestAuth GetEncodedAppInfo completed");
    return DEVICEMANAGER_OK;
}

void MsgRequestAuth::GetDecodeAppInfo(const std::string appString, uint8_t **outBuffer, int32_t &outBufferLen)
{
    DMLOG(DM_LOG_INFO, "MsgRequestAuth GetDecodeAppInfo started");
    int32_t tempBufLen = appString.length() + 1;
    uint8_t *buffer = (uint8_t *)calloc(sizeof(char), tempBufLen);
    if (buffer == nullptr) {
        DMLOG(DM_LOG_ERROR, "GetDecodeAppInfo: malloc mem error, tempBufLen %d", tempBufLen);
        return;
    }

    size_t outLen = 0;
    int32_t ret = EncryptUtils::MbedtlsBase64Decode(buffer, tempBufLen, &outLen,
        (const uint8_t*)appString.c_str(), appString.length());
    if (ret != 0 || outLen > tempBufLen) {
        DMLOG(DM_LOG_ERROR, "MbedtlsBase64Decode failed, ret %d, outLen %d, tempBufLen %d",
            ret, outLen, tempBufLen);
        outBufferLen = 0;
        *outBuffer = nullptr;
        free(buffer);
        return;
    }

    DMLOG(DM_LOG_INFO, "MsgRequestAuth GetDecodeAppInfo outBufferLen %d", outBufferLen);
    outBufferLen = outLen;
    *outBuffer = buffer;
}

std::string MsgRequestAuth::EncodeDevInfo()
{
    DMLOG(DM_LOG_INFO, "MsgRequestAuth EncodeDevInfo started");
    nlohmann::json jsonObj;
    mHead_->Encode(jsonObj);
    jsonObj[TAG_SLICE_NUM] = mMsgSlice_;
    jsonObj[TAG_INDEX] = 0;
    jsonObj[TAG_REQUESTER] = mDeviceName_;
    jsonObj[TAG_DEVICE_ID] = mDeviceId_;
    jsonObj[TAG_DEVICE_TYPE] = mDeviceType_;
    jsonObj[TAG_TOKEN] = mToken_;
    jsonObj[TAG_VISIBILITY] = mGroupVisibility_;
    if (mGroupVisibility_ == GROUP_VISIBILITY_IS_PRIVATE) {
        jsonObj[TAG_TARGET] = mTargetPkg_;
        jsonObj[TAG_HOST] = mHostPkg_;
    }
    jsonObj[TAG_APP_NAME] = mAppName_;
    jsonObj[TAG_APP_DESCRIPTION] = mAppDescription_;

    std::string appIconStr = "";
    GetEncodedAppInfo(mImageInfo_.GetAppIcon(), mImageInfo_.GetAppIconLen(), appIconStr);
    jsonObj[TAG_APP_ICON] = appIconStr;
    jsonObj[TAG_THUMBNAIL_SIZE] = mThumbnailSize_;
    jsonObj[TAG_AUTH_TYPE] = mAuthType_;
    DMLOG(DM_LOG_INFO, "MsgRequestAuth EncodeDevInfo completed");
    return jsonObj.dump();
}

void MsgRequestAuth::DecodeDeviceInfo(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg)
{
    msg->mDeviceName_ = json[TAG_REQUESTER];
    msg->mDeviceId_ = json[TAG_DEVICE_ID];
    msg->mDeviceType_ = json[TAG_DEVICE_TYPE];
    msg->mToken_ = json[TAG_TOKEN];
    msg->mGroupVisibility_ = json[TAG_VISIBILITY];
    if (msg->mGroupVisibility_ == GROUP_VISIBILITY_IS_PRIVATE) {
        msg->mTargetPkg_ = json[TAG_TARGET];
        msg->mHostPkg_ = json[TAG_HOST];
    }
    msg->mAppName_ = json[TAG_APP_NAME];
    msg->mAppDescription_ = json[TAG_APP_DESCRIPTION];

    const std::string iconStr = json[TAG_APP_ICON];
    uint8_t *appIcon = nullptr;
    int32_t appIconLen = 0;
    msg->GetDecodeAppInfo(iconStr, &appIcon, appIconLen);
    if (appIcon != nullptr) {
        msg->mImageInfo_.ResetIcon(appIcon, appIconLen);
        free(appIcon);
    }

    SetThumbnailSize(json, msg);
    msg->mAuthType_ = json[TAG_AUTH_TYPE];
}

std::vector<std::string> MsgRequestAuth::Encode()
{
    DMLOG(DM_LOG_INFO, "MsgRequestAuth encode started");
    std::vector<std::string> jsonStrs;
    int32_t thumbnailSlice =
        ((mThumbnailSize_ / MSG_MAX_SIZE) + (mThumbnailSize_ % MSG_MAX_SIZE) == 0 ? 0 : 1);
    mMsgSlice_ = thumbnailSlice + 1;
    jsonStrs.push_back(EncodeDevInfo());
    for (int32_t idx = 0; idx < thumbnailSlice; idx++) {
        nlohmann::json jsonObj;
        mHead_->Encode(jsonObj);
        jsonObj[TAG_SLICE_NUM] = mMsgSlice_;
        jsonObj[TAG_INDEX] = idx + 1;
        jsonObj[TAG_DEVICE_ID] = mDeviceId_;
        jsonObj[TAG_THUMBNAIL_SIZE] = mThumbnailSize_;

        // frag  thumbnail by 45KB
        std::string thumbnailStr = "";
        int32_t leftLen = mImageInfo_.GetAppThumbnailLen() - idx * MSG_MAX_SIZE;
        int32_t sliceLen = (leftLen > MSG_MAX_SIZE) ? MSG_MAX_SIZE : leftLen;

        DMLOG(DM_LOG_INFO, "TAG_APP_THUMBNAIL encode, idx %d, encodeLen %d, mThumbnailSize_ %d",
            idx, sliceLen, mThumbnailSize_);

        const uint8_t *thumbnail = mImageInfo_.GetAppThumbnail();
        GetEncodedAppInfo(thumbnail + idx * MSG_MAX_SIZE, sliceLen, thumbnailStr);
        jsonObj[TAG_APP_THUMBNAIL] = thumbnailStr;

        jsonStrs.push_back(jsonObj.dump());
    }
    DMLOG(DM_LOG_INFO, "MsgRequestAuth encode completed");
    return jsonStrs;
}

std::shared_ptr<MsgRequestAuth> MsgRequestAuth::Decode(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msgIn)
{
    if (!json.contains(TAG_INDEX) || !json.contains(TAG_DEVICE_ID) || !json.contains(TAG_SLICE_NUM)) {
        DMLOG(DM_LOG_ERROR, "err json string, first time");
        return nullptr;
    }
    int32_t idx = json[TAG_INDEX];
    std::string deviceId = json[TAG_DEVICE_ID];
    if (!IsMsgValid(msgIn, json, deviceId, idx)) {
        auto inValidMsg = std::make_shared<MsgRequestAuth>();
        inValidMsg->mMsgSlice_ = FAIL;
        return inValidMsg;
    }

    std::shared_ptr<MsgRequestAuth> msg = msgIn;
    if (msgIn == nullptr || msgIn->mMsgCnt_ == msgIn->mMsgSlice_) {
        msg = std::make_shared<MsgRequestAuth>();
    }
    msg->mHead_ = MsgHead::Decode(json);
    msg->mMsgSlice_ = json[TAG_SLICE_NUM];
    if (idx == 0) {
        DecodeDeviceInfo(json, msg);
    } else {
        SetThumbnailSize(json, msg);
        msg->mDeviceId_ = deviceId;
        if (!json.contains(TAG_APP_THUMBNAIL)) {
            DMLOG(DM_LOG_ERROR, "err json string, TAG_APP_THUMBNAIL not exit");
            return nullptr;
        }

        std::string thumbnailStr = json[TAG_APP_THUMBNAIL];
        uint8_t *thumbnail = nullptr;
        int32_t thumbnailLen = 0;
        msg->GetDecodeAppInfo(thumbnailStr, &thumbnail, thumbnailLen);
        if (thumbnail == nullptr) {
            DMLOG(DM_LOG_ERROR, "TAG_APP_THUMBNAIL Decode error");
            return nullptr;
        }

        DMLOG(DM_LOG_INFO, "TAG_APP_THUMBNAIL decode, idx %d, decodeLen %d, mThumbnailSize_ %d",
            idx, thumbnailLen, msg->mThumbnailSize_);
        if (msg->mThumbnailSize_ < thumbnailLen + (idx - 1) * MSG_MAX_SIZE) {
            auto inValidReqMsg = std::make_shared<MsgRequestAuth>();
            inValidReqMsg->mMsgSlice_ = FAIL;
            free(thumbnail);
            return inValidReqMsg;
        }
        msg->mImageInfo_.SetThumbnailData(thumbnail, thumbnailLen, (idx - 1) * MSG_MAX_SIZE, thumbnailLen);
        free(thumbnail);
    }
    msg->mMsgCnt_++;
    return msg;
}

int32_t MsgRequestAuth::GetMsgSlice()
{
    return mMsgSlice_;
}

int32_t MsgRequestAuth::GetMsgCnt()
{
    return mMsgCnt_;
}

std::string MsgRequestAuth::GetRequestDeviceId()
{
    return mDeviceId_;
}

bool MsgRequestAuth::IsMsgValid(std::shared_ptr<MsgRequestAuth> msgIn, nlohmann::json &json,
    std::string &deviceId, int32_t index)
{
    if (msgIn != nullptr && msgIn->mMsgCnt_ != msgIn->mMsgSlice_ && deviceId.compare(msgIn->mDeviceId_)) {
        DMLOG(DM_LOG_ERROR, "IsMsgValid, msgIn error");
        return false;
    }

    if (!json.contains(TAG_THUMBNAIL_SIZE)) {
        DMLOG(DM_LOG_ERROR, "IsMsgValid, err json string");
        return false;
    }

    if (index < 0 || index >= DES_SLICE_MAX_NUM) {
        DMLOG(DM_LOG_ERROR, "index err");
        return false;
    }

    if (index == 0) {
        return IsAppInfoValid(json);
    } else {
        int32_t thumbnailSlice = json[TAG_THUMBNAIL_SIZE];
        if (thumbnailSlice > THUMB_MAX_LEN || thumbnailSlice < 0) {
            DMLOG(DM_LOG_ERROR, "IsMsgValid, thumbnailSlice  error");
            return false;
        }
    }
    return true;
}

bool MsgRequestAuth::IsAppInfoValid(nlohmann::json &json)
{
    if (!json.contains(TAG_REQUESTER) || !json.contains(TAG_DEVICE_TYPE) || !json.contains(TAG_TOKEN) ||
        !json.contains(TAG_VISIBILITY) || !json.contains(TAG_APP_NAME) || !json.contains(TAG_APP_DESCRIPTION) ||
        !json.contains(TAG_APP_ICON) || !json.contains(TAG_AUTH_TYPE) || !json.contains(TAG_DEVICE_ID)) {
        DMLOG(DM_LOG_ERROR, "IsAppInfoValid:: err json string");
        return false;
    }

    int32_t groupVisibility = json[TAG_VISIBILITY];
    if (groupVisibility == GROUP_VISIBILITY_IS_PRIVATE) {
        if (!json.contains(TAG_TARGET) || !json.contains(TAG_HOST)) {
            DMLOG(DM_LOG_ERROR, "IsAppInfoValid:: err json string, TAG_TARGET or TAG_HOST not contain");
            return false;
        }
    }

    if (json[TAG_APP_ICON].size() > ICON_MAX_LEN) {
        DMLOG(DM_LOG_ERROR, "IsAppInfoValid, appIcon size error");
        return false;
    }

    int32_t thumbnailSize = json[TAG_THUMBNAIL_SIZE];
    if (thumbnailSize > THUMB_MAX_LEN || thumbnailSize < 0) {
        DMLOG(DM_LOG_ERROR, "IsAppInfoValid, thumbnailSize  error");
        return false;
    }
    return true;
}

void MsgRequestAuth::SetAuthType(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg)
{
    int32_t authType = json.contains(TAG_AUTH_TYPE) ? (int32_t)json[TAG_AUTH_TYPE] : AUTH_TYPE_QR;
    if (authType != AUTH_TYPE_QR && authType != AUTH_TYPE_PIN) {
        authType = AUTH_TYPE_QR;
    }
    msg->mAuthType_ = authType;
}

void MsgRequestAuth::SetThumbnailSize(nlohmann::json &json, std::shared_ptr<MsgRequestAuth> msg)
{
    if (!json.contains(TAG_THUMBNAIL_SIZE)) {
        DMLOG(DM_LOG_ERROR, "SetThumbnailSize, err json string");
        return;
    }
    int32_t thumbnailSlice = json[TAG_THUMBNAIL_SIZE];
    if (msg->mThumbnailSize_ == 0) {
        msg->mImageInfo_.InitThumbnail(thumbnailSlice);
        msg->mThumbnailSize_ = msg->mImageInfo_.GetAppThumbnailLen();
        DMLOG(DM_LOG_INFO, "thumbnailSlice %d, mThumbnailSize_ is, %d", thumbnailSlice, msg->mThumbnailSize_);
    }
}

std::string MsgRequestAuth::ToHexString(int32_t value)
{
    std::stringstream ioss;
    std::string tmpStr;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << value;
    ioss >> tmpStr;
    return tmpStr;
}
}
}
