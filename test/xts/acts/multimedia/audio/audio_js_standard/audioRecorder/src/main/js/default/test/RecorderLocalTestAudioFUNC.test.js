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

import media from '@ohos.multimedia.media'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('RecorderLocalTestAudioFUNC', function () {
    let audioRecorder = media.createAudioRecorder();
    let isTimeOut = false;
    const BASIC_PATH = 'file:///data/accounts/account_0/appdata/recorder/'
    const END_STATE = 0;
    const PRE_STATE = 1;
    const START_STATE = 2;
    const PAUSE_STATE = 3;
    const RESUME_STATE = 4;
    const STOP_STATE = 5;
    const RESET_STATE = 6;
    const RELEASE_STATE = 7;
    const ERROR_STATE = 8;
    const RECORDER_TIME = 5000;
    const TIME_OUT = 20000; // 20s
    const CHANNEL_ONE = 1;
    const CHANNEL_TWO = 2;
    const FORMAT_MP4 = 2;
    const FORMAT_M4A = 6;
    const SOURCE_TYPE = 1;
    const ENCORDER_AACLC = 3;
    let audioConfig = {
        audioSourceType : SOURCE_TYPE,
        audioEncoder : ENCORDER_AACLC,
        audioEncodeBitRate : 22050,
        audioSampleRate : 22050,
        numberOfChannels : CHANNEL_TWO,
        format : FORMAT_M4A,
        uri : 'file:///data/accounts/account_0/appdata/appdata/recorder/test.m4a',
        location : { latitude : 1, longitude : 1 },
    }

    function sleep(time) {
        for(let t = Date.now();Date.now() - t <= time;);
    };

    function initAudioRecorder() {
        if (typeof (audioRecorder) != 'undefined') {
            audioRecorder.release();
            audioRecorder = undefined;
        }
        audioRecorder = media.createAudioRecorder();
    }

    beforeAll(function () {
        console.info('beforeAll case');
    })

    beforeEach(function () {
        isTimeOut = false;
        console.info('beforeEach case');
    })

    afterEach(function () {
        console.info('afterEach case');
    })

    afterAll(function () {
        console.info('afterAll case');
    })

    function nextStep(mySteps) {
        if (mySteps[0] == END_STATE) {
            isTimeOut = true;
            return;
        }
        switch (mySteps[0]) {
            case PRE_STATE:
                console.info('case to prepare');
                audioRecorder.prepare(audioConfig);
                break;
            case START_STATE:
                console.info('case to start');
                audioRecorder.start();
                break;
            case PAUSE_STATE:
                console.info('case to pause');
                audioRecorder.pause();
                break;
            case RESUME_STATE:
                console.info('case to resume');
                audioRecorder.resume();
                break;
            case STOP_STATE:
                console.info('case to stop');
                audioRecorder.stop();
                break;
            case RESET_STATE:
                console.info('case to reset');
                audioRecorder.reset();
                break;
            case RELEASE_STATE:
                console.info('case to release');
                audioRecorder.release();
                break;
            case ERROR_STATE:
                console.info('case to wait error callback');
                break;
            default:
                break;
        }
    }
	
	function setErrorCallback(mySteps) {
        audioRecorder.on('error', (err) => {
            console.info(`case error called,errName is ${err.name}`);
            console.info(`case error called,errCode is ${err.code}`);
            console.info(`case error called,errMessage is ${err.message}`);
            mySteps.shift();
            expect(mySteps[0]).assertEqual(ERROR_STATE);
            mySteps.shift();
            nextStep(mySteps);
        });  
	}

    function setCallback(mySteps, done) {
        audioRecorder.on('prepare', () => {
            console.info('setCallback prepare() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('start', () => {
            console.info('setCallback start() case callback is called');
            sleep(RECORDER_TIME);
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('pause', () => {
            console.info('setCallback pause() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('resume', () => {
            console.info('setCallback resume() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('stop', () => {
            console.info('setCallback stop() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('reset', () => {
            console.info('setCallback reset() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

        audioRecorder.on('release', () => {
            console.info('setCallback release() case callback is called');
            mySteps.shift();
            nextStep(mySteps);
        });

		setErrorCallback(mySteps);
        setTimeout(function() {
            if (!isTimeOut) {
                console.info('case is time out!');
                expect(isTimeOut).assertTrue();
            }
            mySteps = undefined;
            done();
        }, TIME_OUT);
    }

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0100
        * @tc.name      : 01.AAC
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0100', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test1.m4a';
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0210
        * @tc.name      : 02.AAC_Different Channels 1
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0210', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test2.m4a';
        audioConfig.numberOfChannels = CHANNEL_ONE;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0220
        * @tc.name      : 02.AAC_Different Channels 2
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0220', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test3.m4a';
        audioConfig.numberOfChannels = CHANNEL_TWO;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0310
        * @tc.name      : 03.AAC_DifferentSampleRate 8000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0310', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test4.m4a';
        audioConfig.audioSampleRate = 8000; // samplerate 8000
        audioConfig.numberOfChannels = CHANNEL_TWO;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0320
        * @tc.name      : 03.AAC_DifferentSampleRate 32000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0320', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test5.mp4';
        audioConfig.audioSampleRate = 32000; // samplerate 32000
        audioConfig.numberOfChannels = CHANNEL_ONE;
        audioConfig.fileFormat = FORMAT_MP4;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0330
        * @tc.name      : 03.AAC_DifferentSampleRate 44100
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0330', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test6.mp4';
        audioConfig.audioSampleRate = 44100; // samplerate 44100
        audioConfig.numberOfChannels = CHANNEL_TWO;
        audioConfig.fileFormat = FORMAT_MP4;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0340
        * @tc.name      : 03.AAC_DifferentSampleRate 64000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0340', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test7.m4a';
        audioConfig.audioSampleRate = 64000; // samplerate 64000
        audioConfig.numberOfChannels = CHANNEL_ONE;
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0350
        * @tc.name      : 03.AAC_DifferentSampleRate 96000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0340', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test8.m4a';
        audioConfig.audioSampleRate = 96000; // samplerate 96000
        audioConfig.numberOfChannels = CHANNEL_TWO;
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0410
        * @tc.name      : 04.AAC_DifferentBitrate 8000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0410', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test9.mp4';
        audioConfig.audioSampleRate = 22050; // samplerate 22050
        audioConfig.audioEncodeBitRate = 8000; // encodeBitrate 8000
        audioConfig.numberOfChannels = CHANNEL_TWO;
        audioConfig.fileFormat = FORMAT_MP4;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0420
        * @tc.name      : 04.AAC_DifferentBitrate 16000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0420', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test10.mp4';
        audioConfig.audioSampleRate = 44100; // samplerate 44100
        audioConfig.audioEncodeBitRate = 16000; // encodeBitrate 16000
        audioConfig.numberOfChannels = CHANNEL_ONE;
        audioConfig.fileFormat = FORMAT_MP4;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0430
        * @tc.name      : 04.AAC_DifferentBitrate 32000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0430', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test11.m4a';
        audioConfig.audioSampleRate = 44100; // samplerate 44100
        audioConfig.audioEncodeBitRate = 32000; // encodeBitrate 32000
        audioConfig.numberOfChannels = CHANNEL_ONE;
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0440
        * @tc.name      : 04.AAC_DifferentBitrate 64000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0440', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test12.m4a';
        audioConfig.audioSampleRate = 44100; // samplerate 44100
        audioConfig.audioEncodeBitRate = 64000; // encodeBitrate 64000
        audioConfig.numberOfChannels = CHANNEL_TWO;
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Codec_AAC_Function_0450
        * @tc.name      : 04.AAC_DifferentBitrate 112000
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Codec_AAC_Function_0450', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test13.m4a';
        audioConfig.audioSampleRate = 44100; // samplerate 44100
        audioConfig.audioEncodeBitRate = 112000; // encodeBitrate 112000
        audioConfig.numberOfChannels = CHANNEL_TWO;
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Format_MP4_Function_0200
        * @tc.name      : 02.AAC,mp4
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Format_MP4_Function_0200', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test14.mp4';
        audioConfig.audioEncodeBitRate = 22050; // samplerate 22050
        audioConfig.audioSampleRate = 22050; // encodeBitrate 22050
        audioConfig.fileFormat = FORMAT_MP4;
        audioConfig.numberOfChannels = CHANNEL_TWO;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_Format_M4A_Function_0200
        * @tc.name      : 02.AAC,mp4
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_Format_MP4_Function_0200', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test15.m4a';
        audioConfig.audioEncodeBitRate = 22050; // samplerate 22050
        audioConfig.audioSampleRate = 22050; // encodeBitrate 22050
        audioConfig.fileFormat = FORMAT_M4A;
        audioConfig.numberOfChannels = CHANNEL_TWO;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_0100
        * @tc.name      : 001.start
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_0100', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test16.m4a';
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_0500
        * @tc.name      : 005.reset
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_0500', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test17.m4a';
        audioConfig.fileFormat = FORMAT_M4A;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_1100
        * @tc.name      : 011.start->reset->start->stop
        * @tc.desc      : Audio recordr control test
        * @tc.size      : MediumTest
        * @tc.type      : Function
        * @tc.level     : Level0
    */
    it('SUB_MEDIA_RECORDER_LOCAL_AUDIO_Function_06_1100', 0, async function (done) {
        audioConfig.uri = BASIC_PATH + 'test18.mp4';
        audioConfig.fileFormat = FORMAT_MP4;
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, PRE_STATE,
            START_STATE, STOP_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })
})