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

describe('RecorderLocalTestAudioAPI', function () {
    let audioRecorder = media.createAudioRecorder();
    let isTimeOut = false;
    const END_STATE = 0;
    const PRE_STATE = 1;
    const START_STATE = 2;
    const PAUSE_STATE = 3;
    const RESUME_STATE = 4;
    const STOP_STATE = 5;
    const RESET_STATE = 6;
    const RELEASE_STATE = 7;
    const ERROR_STATE = 8;
    const TIME_OUT = 15000; // 15s
    const FORMAT_M4A = 6;
    const SOURCE_TYPE = 1;
    const ENCORDER_AACLC = 3;
    const CHANNEL_TWO = 2;
    let audioConfig = {
        audioSourceType : SOURCE_TYPE,
        audioEncoder : ENCORDER_AACLC,
        audioEncodeBitRate : 22050,
        audioSampleRate : 22050,
        numberOfChannels : CHANNEL_TWO,
        format : FORMAT_M4A,
        uri : 'file:///data/accounts/account_0/appdata/recorder/testAPI.m4a',
        location : { latitude : 1, longitude : 1 },
    }

    function initAudioRecorder() {
        if (typeof (audioRecorder) != 'undefined') {
            audioRecorder.release();
            audioRecorder = undefined;
        }
        audioRecorder = media.createAudioRecorder();
    }

    function nextStep(mySteps) {
        if (mySteps[0] == END_STATE) {
            console.info('case end!!!');
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
            console.info('case err returned in error() case callback');
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
            audioRecorder = undefined;
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

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_createAudioRecorder_API_0100
        * @tc.name      : Create an AudioRecoder Object by function of createAudioRecorder
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_createAudioRecorder_API_0100', 0, async function (done) {
        let testAudioRecorder;
        expect(testAudioRecorder).assertNull();
        testAudioRecorder= media.createAudioRecorder();
        expect(testAudioRecorder != undefined).assertTrue();
        done();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0100
        * @tc.name      : 01.creatAudioRecorder->prepare
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0100', 0, async function (done) {
        let testAudioRecorder= media.createAudioRecorder();
        expect(testAudioRecorder != null).assertTrue();
        testAudioRecorder.prepare(audioConfig);
        done();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0200
        * @tc.name      : 02.start->prepare
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0200', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(START_STATE, ERROR_STATE, PRE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.start();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0500
        * @tc.name      : 05.stop->prepare
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0500', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, RESET_STATE, PRE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0600
        * @tc.name      : 06.reset->prepare
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0600', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, PRE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0800
        * @tc.name      : 08.all steps->prepare
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0800', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, PRE_STATE, ERROR_STATE, STOP_STATE,
            PRE_STATE, RESET_STATE, PRE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0900
        * @tc.name      : 09.prepare called three times
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Prepare_API_0900', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, PRE_STATE, ERROR_STATE, PRE_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0100
        * @tc.name      : 01.creatAudioRecorder->start
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0100', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(START_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.start();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0200
        * @tc.name      : 02.prepare->start
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0200', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0500
        * @tc.name      : 05.stop->start
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0500', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, START_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0600
        * @tc.name      : 06.reset->start
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0600', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, START_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0800
        * @tc.name      : 08.all steps->start
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0800', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, START_STATE, ERROR_STATE,
            PRE_STATE, START_STATE, RESET_STATE, START_STATE, ERROR_STATE, PRE_STATE, START_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0900
        * @tc.name      : 09.start called three times
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Start_API_0900', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, START_STATE, START_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0100
        * @tc.name      : 01.creatAudioRecorder->stop
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0100', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(STOP_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.stop();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0200
        * @tc.name      : 02.prepare->stop
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0200', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, STOP_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0300
        * @tc.name      : 03.start->stop
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0300', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0600
        * @tc.name      : 06.reset->stop
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0600', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, STOP_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0800
        * @tc.name      : 08.all steps->stop
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0800', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, STOP_STATE, ERROR_STATE, RESET_STATE, PRE_STATE, START_STATE, STOP_STATE,
            RESET_STATE, PRE_STATE, RESET_STATE, STOP_STATE, ERROR_STATE, PRE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0900
        * @tc.name      : 09.stop called three times
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Stop_API_0900', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, STOP_STATE, ERROR_STATE,
            STOP_STATE, ERROR_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0100
        * @tc.name      : 01.creatAudioRecorder->reset
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0100', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.reset();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0200
        * @tc.name      : 02.prepare->reset
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0200', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0300
        * @tc.name      : 03.start->reset
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0300', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0600
        * @tc.name      : 06.stop->reset
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0600', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0800
        * @tc.name      : 08.all steps->reset
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0800', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, RESET_STATE, PRE_STATE, START_STATE, RESET_STATE, PRE_STATE, START_STATE,
            STOP_STATE, RESET_STATE, PRE_STATE, START_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0900
        * @tc.name      : 09.reset callend three times
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Reset_API_0900', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, RESET_STATE, RESET_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0100
        * @tc.name      : 01.creatAudioRecorder->release
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0100', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.release();
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0200
        * @tc.name      : 02.prepare->release
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0200', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0300
        * @tc.name      : 03.start->release
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0300', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0600
        * @tc.name      : 06.stop->release
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0600', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, STOP_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })

    /* *
        * @tc.number    : SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0700
        * @tc.name      : 07.reset->release
        * @tc.desc      : Reliability Test
        * @tc.size      : MediumTest
        * @tc.type      : Reliability
        * @tc.level     : Level2
    */
    it('SUB_MEDIA_RECORDER_AudioRecorder_Release_API_0700', 0, async function (done) {
        initAudioRecorder();
        let mySteps = new Array(PRE_STATE, START_STATE, RESET_STATE, RELEASE_STATE, END_STATE);
        setCallback(mySteps, done);
        audioRecorder.prepare(audioConfig);
    })
})