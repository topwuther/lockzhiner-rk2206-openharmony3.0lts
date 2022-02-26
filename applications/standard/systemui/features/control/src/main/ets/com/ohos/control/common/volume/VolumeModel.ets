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

import audio from '@ohos.multimedia.audio';
import Log from '../../../../../../../../../../common/src/main/ets/default/Log.ets';

let TAG = 'Control-VolumeModel';

export default class VolumeModel {
    constructor() {
    }

    setVolume(volumeValue, volumeType){
        Log.showInfo(TAG, `setVolume volumeValue: ${JSON.stringify(volumeValue)} volumeType：${JSON.stringify(volumeType)} `);

        audio.getAudioManager().setVolume(volumeType, volumeValue).then(() => {
            Log.showInfo(TAG,'Promise returned to indicate a successful volume setting.')
        });
    }

    getVolume(callback, volumeType) {
        Log.showInfo(TAG, `getVolume volumeType：${volumeType} `);
        audio.getAudioManager().getVolume(volumeType, (err, value) => {

            if (err) {
                Log.showInfo(TAG, `Failed to obtain the volume. ${err.message}`);
                return;
            }
            Log.showInfo(TAG, 'getVolume Callback invoked to indicate that the volume is obtained.' + value);
            callback.value = value;
        })
    }

    getMaxVolume(callback, volumeType) {
        Log.showInfo(TAG, `getMaxVolume volumeType：${volumeType} `);
        audio.getAudioManager().getMaxVolume(volumeType, (err, value) => {
            if (err) {
                Log.showInfo(TAG, `Failed to obtain the volume. ${err.message}`);
                return;
            }
            Log.showInfo(TAG, 'getMaxVolume Callback invoked to indicate that the volume is obtained.' + value);
            callback.maxValue = value;
        })
    }

    getMinVolume(callback, volumeType) {
        Log.showInfo(TAG, `getMaxVolume volumeType：${volumeType} `);
        audio.getAudioManager().getMinVolume(volumeType, (err, value) => {
            if (err) {
                Log.showInfo(TAG, `Failed to obtain the volume. ${err.message}`);
                return;
            }
            Log.showInfo(TAG, 'getMinVolume Callback invoked to indicate that the volume is obtained.' + value);
            callback.minValue = value;
        })
    }
}