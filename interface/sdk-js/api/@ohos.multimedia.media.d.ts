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

import {ErrorCallback, AsyncCallback, Callback} from './basic';

/**
 * @name media
 * @sysCap SystemCapability.Multimedia.Media
 * @import import media from '@ohos.multimedia.media';
 * This part of the function is temporarily unavailable due to permission issues in the standard system,
 * waiting for update
 */
declare namespace media {
    /**
     * Create the AudioPlayer instance to manage audio play
     * @sysCap SystemCapability.Multimedia.Media
     */
    function createAudioPlayer(): AudioPlayer;

    /**
     * Audio playback state
     */
    type AudioState = 'playing' | 'paused' | 'stopped';

    interface AudioPlayer {
        /**
         * start to play audio resource
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        play(): void;

        /**
         * pause playing
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        pause(): void;

        /**
         * stop playing
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        stop(): void;

        /**
         * jump to a specified location
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         * @param time position to seek
         */
        seek(timeMs: number):void;

        /**
         * change the audioplayer volume
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         * @param volume to set
         */
        setVolume(vol: number): void;

        /**
         * release audio resource
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        release(): void;

        /**
         * audio resource URI
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        src: string;

        /**
         * whether audio is single looping
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         * @option true "The audio looping function is enabled."
         * @option false "The audio looping function is disabled."
         */
        loop: boolean;

         /**
         * audio property of current progress
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        readonly currentTime: number;

         /**
         * audio property of audio playback duration
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        readonly duration: number;

        /**
         * audio property of playback status
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        readonly state: AudioState;

        /**
         * audio callback function for listening to event
         * @devices phone
         * @sysCap SystemCapability.Multimedia.Media
         */
        on(type: 'play' | 'pause' | 'stop' | 'dataLoad' | 'finish' | 'volumeChange' | 'timeUpdate', callback: ()=>{}):void;
        on(type:'error', callback: ErrorCallback):void;
    }
}
export default media;
