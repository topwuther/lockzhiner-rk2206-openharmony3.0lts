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

import PreviewModel from '../default/model/PreviewModel.js';
import RemoteDeviceModel from './model/RemoteDeviceModel.js';
import KvStoreModel from './model/KvStoreModel.js';
import LogUtil from '../default/common/utils/LogUtil.js';

let mLogUtil = new LogUtil();

export default {
    data: {
        previewModel: new PreviewModel(),
        remoteDeviceModel: new RemoteDeviceModel(),
        kvStoreModel: new KvStoreModel()
    },
    onCreate() {
        mLogUtil.cameraInfo('AceApplication onCreate');
    },
    onDestroy() {
        mLogUtil.cameraInfo('AceApplication onDestroy');
    }
};
