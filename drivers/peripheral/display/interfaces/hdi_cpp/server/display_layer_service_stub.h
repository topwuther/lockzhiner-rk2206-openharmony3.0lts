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

#ifndef OHOS_HDI_DISPLAY_LAYER_V1_0_INTERFACE_SERVICE_STUB_H
#define OHOS_HDI_DISPLAY_LAYER_V1_0_INTERFACE_SERVICE_STUB_H

#include <iremote_stub.h>
#include <message_parcel.h>
#include <message_option.h>
#include "idisplay_layer.h"
#include "display_layer_service_impl.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace V1_0 {

enum {
    CMD_DISPLAY_LAYER_INIT_DISPLAY = 0,
    CMD_DISPLAY_LAYER_DEINIT_DISPLAY,
    CMD_DISPLAY_LAYER_GET_DISPLAY_INFO,
    CMD_DISPLAY_LAYER_CREATE_LAYER,
    CMD_DISPLAY_LAYER_CLOSE_LAYER,
    CMD_DISPLAY_LAYER_SET_LAYER_VISIBLE,
    CMD_DISPLAY_LAYER_GET_LAYER_VISIBLE_STATE,
    CMD_DISPLAY_LAYER_SET_LAYER_RECT,
    CMD_DISPLAY_LAYER_GET_LAYER_RECT,
    CMD_DISPLAY_LAYER_SET_LAYER_ZORDER,
    CMD_DISPLAY_LAYER_GET_LAYER_ZORDER,
    CMD_DISPLAY_LAYER_SET_TRANSFORM_MODE,
    CMD_DISPLAY_LAYER_SET_LAYER_BUFFER,
};

class DisplayLayerStub {
public:
    DisplayLayerStub();
    virtual ~DisplayLayerStub() {}

    DispErrCode Init();
    int32_t LayerStubInitDisplay(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubDeinitDisplay(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubGetDisplayInfo(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubCreateLayer(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubCloseLayer(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubSetLayerVisible(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubGetLayerVisibleState(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubSetLayerRect(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubGetLayerRect(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubSetLayerZorder(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubGetLayerZorder(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubSetLayerTransformMode(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubSetLayerBuffer(MessageParcel& data, MessageParcel& reply, MessageOption& option);
    int32_t LayerStubOnRemoteRequest(int cmdId, MessageParcel& data, MessageParcel& reply, MessageOption& option);

private:
    std::shared_ptr<DisplayLayerService> layerService_ = nullptr;
};

} // namespace V1_0
} // namespace Display
} // namespace HDI
} // namespace OHOS

void *LayerStubInstance();

void DestroyLayerStub(void *obj);

int32_t LayerServiceOnRemoteRequest(void *stub, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply);

#endif // OHOS_HDI_DISPLAY_LAYER_V1_0_INTERFACE_SERVICE_STUB_H