/*
* Copyright (C) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import {ErrorCallback, AsyncCallback} from './basic';

/**
 * @name audio
 * @sysCap SystemCapability.Multimedia.Audio
 * @import import audio from '@ohos.multimedia.audio';
 * This part of the function is temporarily unavailable due to permission issues in the standard system,
 * waiting for update
 */
declare namespace audio {

    /**
     * get the audiomanager of the audio
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    function getAudioManager(): AudioManager;

    /**
     * the type of audio stream
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    enum AudioVolumeType {
        /**
         * the media stream
         */
        MEDIA = 1,
        /**
         * the ringtone stream
         */
        RINGTONE = 2,
    }

    /**
     * the flag type of device
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    enum DeviceFlag {
        /**
         * the device flag of output
         */
        OUTPUT_DEVICES_FLAG = 1,
        /**
         * the device flag of input
         */
        INPUT_DEVICES_FLAG = 2,
        /**
         * the device flag of all devices
         */
        ALL_DEVICES_FLAG = 3,
    }
    /**
     * the role of device
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    enum DeviceRole {
        /**
         * the role of input devices
         */
        INPUT_DEVICE = 1,
        /**
         * the role of output devices
         */
        OUTPUT_DEVICE = 2,
    }
    /**
     * the type of device
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    enum DeviceType {
        /**
         * invalid
         */
        INVALID = 0,
        /**
         * speaker
         */
        SPEAKER = 1,
        /**
         * wired headset
         */
        WIRED_HEADSET = 2,
        /**
         * bluetooth sco
         */
        BLUETOOTH_SCO = 3,
        /**
         * bluetooth a2dp
         */
        BLUETOOTH_A2DP = 4,
        /**
         * mic
         */
        MIC = 5,
    }
    /**
     * the audiomanager of the audio
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    interface AudioManager {
        /**
         * set the volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        setVolume(audioType: AudioVolumeType,volume: number,callback: AsyncCallback<void>): void;
        /**
         * set the volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        setVolume(audioType: AudioVolumeType,volume: number): Promise<void>;
        /**
         * get the volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getVolume(audioType: AudioVolumeType, callback: AsyncCallback<number>): void;
        /**
         * get the volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getVolume(audioType: AudioVolumeType): Promise<number>;
        /**
         * get the min volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getMinVolume(audioType: AudioVolumeType, callback: AsyncCallback<number>): void
        /**
         * get the min volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getMinVolume(audioType: AudioVolumeType): Promise<number>;
        /**
         * get the max volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getMaxVolume(audioType: AudioVolumeType, callback: AsyncCallback<number>): void
        /**
         * get the max volume of the audiovolumetype
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getMaxVolume(audioType: AudioVolumeType): Promise<number>;
        /**
         * get the device list of the audio devices by the audio flag
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getDevices(deviceFlag: DeviceFlag, callback: AsyncCallback<AudioDeviceDescriptors>): void;
        /**
         * get the device list of the audio devices by the audio flag
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        getDevices(deviceFlag: DeviceFlag): Promise<AudioDeviceDescriptors>;
    }

    /**
     * the Descriptor of the device
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    interface AudioDeviceDescriptor {
        /**
         * the role of device
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        readonly deviceRole: DeviceRole;
        /**
         * the type of device
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Audio
         */
        readonly deviceType: DeviceType;
    }

    /**
     * the Descriptor list of the devices
     * @devices phone
     * @sysCap SystemCapability.Multimedia.Audio
     */
    type AudioDeviceDescriptors = Array<Readonly<AudioDeviceDescriptor>>;
}

export default audio;