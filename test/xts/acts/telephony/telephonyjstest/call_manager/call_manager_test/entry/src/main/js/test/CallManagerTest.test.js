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

import call from '@ohos.telephony.call';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';
describe('CallManagerTest', function () {
    const ERROR_CALLID_999 = 999;
    const GETMAIN_CALLID_ERRO = -1;
    const STATUS_ON = 1;
    const SLOT_0 = 0;
    const ERR_SLOT_ID = -1;

   /*
    * @tc.number  Telephony_CallManager_getCallState_Async_0100
    * @tc.name    To get the idle call status, call getCallState() to get the current call status.
    *             call.CALL_STATE_IDLE is returned
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getCallState_Async_0100', 0, async function (done) {
       call.getCallState((err, data) => {
           if (err) {
               console.log('Telephony_CallManager_getCallState_Async_0100 : err = ' + err.message);
               expect().assertFail();
               done();
               return;
           }
           expect(data === call.CALL_STATE_IDLE).assertTrue();
           console.log('Telephony_CallManager_getCallState_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_getCallState_Promise_0100
    * @tc.name    To get the idle call status, call getCallState() to get the current call status.
    *             call.CALL_STATE_IDLE is returned
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getCallState_Promise_0100', 0, async function (done) {
       try {
           var data = await call.getCallState();
           expect(data === call.CALL_STATE_IDLE).assertTrue();
           console.log('Telephony_CallManager_getCallState_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_getCallState_Promise_0100 : err = ' + err.message);
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_dial_Async_0200
    * @tc.name    Set the mandatory phone number to empty ('), call dial() back to dial, and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_dial_Async_0200', 0, async function (done) {
       call.dial('', (err) => {
           if (err) {
               console.log('Telephony_CallManager_dial_Async_0200 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_dial_Async_0200 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_dial_Async_0300
    * @tc.name    Set will choose parameter number phone number more than 30 characters
    *             (' 12345678901234567890012345678901 ', separate phone number did not take ', '),
    *             the callback way call dial () to dial, capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_dial_Async_0300', 0, async function (done) {
       call.dial('12345678901234567890012345678901', (err) => {
           if (err) {
               console.log('Telephony_CallManager_dial_Async_0300 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_dial_Async_0300 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_dial_Promise_0200
    * @tc.name    Set mandatory phone number to empty (' ') and optional options to
    *             {accountId: 1, videoState: 0, dialScene: 0, dialType: 0}. Call dial() to dial and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_dial_Promise_0200', 0, async function (done) {
       try {
           await call.dial('', { accountId: 1 })
           console.log('Telephony_CallManager_dial_Promise_0200 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_dial_Promise_0200 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_dial_Promise_0900
    * @tc.name    Will choose parameter number more than 30 Numbers (12345678901234567890012345678901),
    *             with no optional parameters, call dial () to dial, capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_dial_Promise_0900', 0, async function (done) {
       try {
           await call.dial('12345678901234567890012345678901');
           expect().assertFail();
           console.log('Telephony_CallManager_dial_Promise_0900 fail');
       } catch (err) {
           console.log('Telephony_CallManager_dial_Promise_0900 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_dial_Async_0900
    * @tc.name    Set mandatory phone number to empty (' ') and optional parameters options to
    *             {accountId: 0, videoState: 0, dialScene: 0, dialType: 0}. Call dial() in callback mode to capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_dial_Async_0900', 0, async function (done) {
       let obj = { accountId: 0, videoState: 0, dialScene: 0, dialType: 0 };
       call.dial('', obj, (err) => {
           if (err) {
               console.log('Telephony_CallManager_dial_Async_0900 finish  err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_dial_Async_0900 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_holdCall_Async_0100
    * @tc.name    Set the mandatory callId parameter to non-existent callId, call holdCall() to enable call hold,
    *             and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_holdCall_Async_0100', 0, async function (done) {
       call.holdCall(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_holdCall_Async_0100 finish  err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_holdCall_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_holdCall_Promise_0100
    * @tc.name    Set the mandatory callId parameter to non-existent callId and call holdCall() to enable call hold.
    *             Capture the err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_holdCall_Promise_0100', 0, async function (done) {
       try {
           await call.holdCall(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_holdCall_Promise_0100 fail');
       } catch (err) {
           console.log('Telephony_CallManager_holdCall_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_unHoldCall_Async_0100
    * @tc.name    Set the mandatory parameter callId to non-existent callId, call unholdCall() to activate the call,
    *             and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_unHoldCall_Async_0100', 0, async function (done) {
       call.unHoldCall(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_unHoldCall_Async_0100 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_unHoldCall_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_unHoldCall_Promise_0100
    * @tc.name    Set the mandatory parameter callId to non-existent callId, call unholdCall() to deactivate the call,
    *             and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_unHoldCall_Promise_0100', 0, async function (done) {
       try {
           await call.unHoldCall(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_unHoldCall_Promise_0100 fail');
           done();
       } catch (err) {
           console.log('Telephony_CallManager_unHoldCall_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
   })

   /*
    * @tc.number  Telephony_CallManager_switchCall_Async_0100
    * @tc.name    When a call is switched to a non-existent callId, switchCall() is called in callback mode to
    *             switch the call and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_switchCall_Async_0100', 0, async function (done) {
       call.switchCall(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_switchCall_Async_0100 finish err = ' + err);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_switchCall_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_switchCall_Promise_0100
    * @tc.name    Switch to a non-existent callId during a call. Call switchCall() to switch the call and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_switchCall_Promise_0100', 0, async function (done) {
       try {
           await call.switchCall(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_switchCall_Promise_0100 fail');
       } catch (err) {
           console.log('Telephony_CallManager_switchCall_Promise_0100 finish err = ' + err);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_hasCall_Async_0100
    * @tc.name    After an empty call is automatically hung up, the callback method calls hasCall() to confirm that
    *             there is no current call, returning false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hasCall_Async_0100', 0, async function (done) {
       call.dial('', (err) => {
           if (err) {
               console.log('Telephony_CallManager_hasCall_Async_0100 dial finish err = ' + err.message);
               call.hasCall((err, data) => {
                   if (err) {
                       console.log('Telephony_CallManager_hasCall_Async_0400 fail');
                       expect().assertFail();
                       done();
                       return;
                   }
                   expect(data === false).assertTrue();
                   console.log('Telephony_CallManager_hasCall_Async_0100 finish data = ' + data);
               })
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_hasCall_Async_0100 dial fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_hasCall_Async_0400
    * @tc.name    When idle, hasCall() is called to confirm that there is no current call,returning false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hasCall_Async_0400', 0, async function (done) {
       call.hasCall((err, data) => {
           if (err) {
               console.log('Telephony_CallManager_hasCall_Async_0400 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_hasCall_Async_0400 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_hasCall_Promise_0100
    * @tc.name    Call hasCall() to confirm that there is no call, returns false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hasCall_Promise_0100', 0, async function (done) {
       try {
           var data = await call.dial('');
           expect().assertFail();
           console.log('Telephony_CallManager_hasCall_Promise_0100 dail fail');
       } catch (err) {
           console.log('Telephony_CallManager_hasCall_Promise_0100 dail finish err = ' + err.message);
           try {
               var data = await call.hasCall();
               expect(data === false).assertTrue();
               console.log('Telephony_CallManager_hasCall_Promise_0100 finish data = ' + data);
           } catch (err) {
               console.log('Telephony_CallManager_hasCall_Promise_0100 fail');
               expect().assertFail();
               done();
               return;
           }
           done();
       }
   })

   /*
    * @tc.number  Telephony_CallManager_hasCall_Promise_0400
    * @tc.name    When idle, hasCall() is called to confirm that there is no current call, returning false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hasCall_Promise_0400', 0, async function (done) {
       try {
           var data = await call.hasCall();
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_hasCall_Promise_0400 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_hasCall_Promise_0400 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_combineConference_Async_0100
    * @tc.name    If mainCallId is 999, call combineConference() to merge the conference call and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_combineConference_Async_0100', 0, async function (done) {
       call.combineConference(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_combineConference_Async_0100 finish err = ' + err.message);
               done();
               return;
           }
           console.log('Telephony_CallManager_combineConference_Async_0100 fail');
           expect().assertFail();
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_combineConference_Promise_0100
    * @tc.name    If mainCallId is 999, call combineConference() to merge the conference call and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_combineConference_Promise_0100', 0, async function (done) {
       try {
           await call.combineConference(ERROR_CALLID_999);
           console.log('Telephony_CallManager_combineConference_Promise_0100 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_combineConference_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_getSubCallIdList_Async_0100
    * @tc.name    If callId is 999, call getSubCallIdList() to get the call list of subcalls and return the empty list
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getSubCallIdList_Async_0100', 0, async function (done) {
       call.getSubCallIdList(ERROR_CALLID_999, (err, data) => {
           if (err) {
               expect().assertFail();
               console.log('Telephony_CallManager_getSubCallIdList_Async_0100 fail');
               done();
               return;
           }
           expect(data.length === 0).assertTrue();
           console.log('Telephony_CallManager_getSubCallIdList_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_getSubCallIdList_Promise_0100
    * @tc.name    If callId is 999, call getSubCallIdList() to get the sub-call list
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getSubCallIdList_Promise_0100', 0, async function (done) {
       try {
           var data = await call.getSubCallIdList(ERROR_CALLID_999);
           expect(data.length === 0).assertTrue();
           console.log('Telephony_CallManager_getSubCallIdList_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_getSubCallIdList_Promise_0100 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_getCallIdListForConference_Async_0100
    * @tc.name    CallId for 999, the callback call getCallIdListForConference () to obtain a list all the phone call
    *             in the conference call, returns an empty list
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getCallIdListForConference_Async_0100', 0, async function (done) {
       call.getCallIdListForConference(ERROR_CALLID_999, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_getCallIdListForConference_Async_0100 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data.length === 0).assertTrue();
           console.log('Telephony_CallManager_getCallIdListForConference_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_getCallIdListForConference_Promise_0100
    * @tc.name    GetCallIdListForConference callId for 999, call () to obtain a list all the phone call in the
    *             conference call, returns an empty list
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getCallIdListForConference_Promise_0100', 0, async function (done) {
       try {
           var data = await call.getCallIdListForConference(ERROR_CALLID_999);
           expect(data.length === 0).assertTrue();
           console.log('Telephony_CallManager_getCallIdListForConference_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_getCallIdListForConference_Promise_0100 fail');
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_startDTMF_Async_1000
    * @tc.name    CallId is 999, character is C, startDTMF() is called as a callback to startDTMF and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_startDTMF_Async_1000', 0, async function (done) {
       call.startDTMF(ERROR_CALLID_999, 'C', (err) => {
           if (err) {
               console.log('Telephony_CallManager_startDTMF_Async_1000 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_startDTMF_Async_1000 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_startDTMF_Promise_1000
    * @tc.name    If callId is 999 and character is C, call startDTMF() to startDTMF and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_startDTMF_Promise_1000', 0, async function (done) {
       try {
           await call.startDTMF(ERROR_CALLID_999, 'C');
           console.log('Telephony_CallManager_startDTMF_Promise_1000 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_startDTMF_Promise_1000 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_stopDTMF_Async_0100
    * @tc.name    If callId is 999, call stopDTMF() to start DTMF and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_stopDTMF_Async_0100', 0, async function (done) {
       call.stopDTMF(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_stopDTMF_Async_0100 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_stopDTMF_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_stopDTMF_Promise_0100
    * @tc.name    If callId is 999, call stopDTMF() to start DTMF and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_stopDTMF_Promise_0100', 0, async function (done) {
       try {
           await call.stopDTMF(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_stopDTMF_Promise_0100 fail');
       } catch (err) {
           console.log('Telephony_CallManager_stopDTMF_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_reject_Async_0100
    * @tc.name    When callId is 999, call Reject () to reject the call and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_reject_Async_0100', 0, async function (done) {
       call.reject(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_reject_Async_0100 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_reject_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_reject_Promise_0100
    * @tc.name    CallId is 999, reject() is called, and ERR is captured
    */
   it('Telephony_CallManager_reject_Promise_0100', 0, async function (done) {
       try {
           await call.reject(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_reject_Promise_0100 fail');
       } catch (err) {
           console.log('Telephony_CallManager_reject_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_hangup_Async_0100
    * @tc.name    If callId is 999, call hangup() to hangup the call and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hangup_Async_0100', 0, async function (done) {
       call.hangup(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_hangup_Async_0100 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_hangup_Async_0100 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_hangup_Promise_0100
    * @tc.name    If callId is 999, call hangup() to hangup and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_hangup_Promise_0100', 0, async function (done) {
       try {
           await call.hangup(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_hangup_Promise_0100 fail');
       } catch (err) {
           console.log('Telephony_CallManager_hangup_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_getMainCallId_Async_0100
    * @tc.name    If the mainCallId is 999, call getMainCallId() to get the call and return -1
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getMainCallId_Async_0100', 0, async function (done) {
       call.getMainCallId(ERROR_CALLID_999, (err, data) => {
           if (err) {
               expect().assertFail();
               console.log('Telephony_CallManager_getMainCallId_Async_0100 fail');
               done();
               return;
           }
           expect(data === GETMAIN_CALLID_ERRO).assertTrue();
           console.log('Telephony_CallManager_getMainCallId_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_getMainCallId_Promise_0100
    * @tc.name    If mainCallId is 999, call getMainCallId() to get the call and return -1
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_getMainCallId_Promise_0100', 0, async function (done) {
       try {
           var data = await call.getMainCallId(ERROR_CALLID_999);
           expect(data === GETMAIN_CALLID_ERRO).assertTrue();
           console.log('Telephony_CallManager_getMainCallId_Promise_0100 finish data = ' + data);
       } catch (err) {
           expect().assertFail();
           console.log('Telephony_CallManager_getMainCallId_Promise_0100 fail');
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_answer_Async_0100
    * @tc.name    CallId is 999. Call answer() in callback mode to answer the call and capture ERR
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_answer_Async_0100', 0, async function (done) {
       call.answer(ERROR_CALLID_999, (err) => {
           if (err) {
               console.log('Telephony_CallManager_answer_Async_0100 finish = ' + err.message);
               done();
               return;
           }
           console.log('Telephony_CallManager_answer_Async_0100 fail');
           expect().assertFail();
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_answer_Promise_0100
    * @tc.name    CallId is 999. Call answer() in callback mode to answer the call and capture ERR
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_answer_Promise_0100', 0, async function (done) {
       try {
           await call.answer(ERROR_CALLID_999);
           expect().assertFail();
           console.log('Telephony_CallManager_answer_Promise_0100 fail');
           done();
       } catch (err) {
           console.log('Telephony_CallManager_answer_Promise_0100 finish err = ' + err.message);
           done();
           return;
       }
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0100
    * @tc.name    PhoneNumber is 100000000000. Call formatPhoneNumber() to format the number.
    *             The return value is 10 000 000 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0100', 0, async function (done) {
       call.formatPhoneNumber('100000000000', (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0100 err = ' + err.message);
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0100 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '10 000 000 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0200
    * @tc.name    If phoneNumber is 10 000 000 0000, options: CN, call formatPhoneNumber() to format the number,
    *             and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0200', 0, async function (done) {
       call.formatPhoneNumber('10 000 000 0000', { countryCode: 'CN' }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0200 finish = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0200 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0300
    * @tc.name    If phoneNumber is (010)00000000, options: CN, call formatPhoneNumber() to format the number,
    *             return the value 010 0000 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0300', 0, async function (done) {
       call.formatPhoneNumber('(010)00000000', { countryCode: 'CN' }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0300 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '010 0000 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0300 finish data : ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0400
    * @tc.name    PhoneNumber is 010-0000-0000, options: CN, call formatPhoneNumber() to format the number,
    *             return 010 0000 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0400', 0, async function (done) {
       call.formatPhoneNumber('010-0000-0000', { countryCode: 'CN' }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0400 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '010 0000 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0400 finish data : ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0500
    * @tc.name    PhoneNumber 666666999999 is not supported in the current country. Options: CN. Call
    *             formatPhoneNumber() to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0500', 0, async function (done) {
       call.formatPhoneNumber('666666999999', { countryCode: 'CN' }, (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0500 finish err = ' + err.message);
               done();
               return;
           }
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0500 fail');
           expect().assertFail();
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0600
    * @tc.name    If phoneNumber is 2000000000, type non-existent options: abCDFG. Call
    *             formatPhoneNumber() to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0600', 0, async function (done) {
       call.formatPhoneNumber('2000000000', { countryCode: 'abcdefg' }, (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0600 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0600 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Async_0700
    * @tc.name    If phoneNumber is 2000000000, options: ', call formatPhoneNumber() to
    *             format the number and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Async_0700', 0, async function (done) {
       call.formatPhoneNumber('2000000000', { countryCode: '' }, (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumber_Async_0700 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Async_0700 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0100
    * @tc.name    PhoneNumber is 2000000. Call formatPhoneNumber() to format the number.
    *             The return value is 200 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0100', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumber('2000000');
           expect(data === '200 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0100 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0200
    * @tc.name    PhoneNumber is 010-100-0000, options: CN, call formatPhoneNumber() to format the number, err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0200', 0, async function (done) {
       try {
           await call.formatPhoneNumber('010-100-0000', { countryCode: 'CN' });
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0200 fail');
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0200 finish err = ' + err);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0300
    * @tc.name    PhoneNumber: (010)00000000, options: CN, call formatPhoneNumber() to format the number,
    *             return the value 010 0000 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0300', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumber('(010)00000000', { countryCode: 'CN' });
           expect(data === '010 0000 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0300 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0300 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })
   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0400
    * @tc.name    If phoneNumber is 200-0000, options: CN, call formatPhoneNumber() to format the
    *             number and return 200 0000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0400', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumber('200-0000', { countryCode: 'CN' });
           expect(data === '200 0000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0400 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0400 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0500
    * @tc.name    PhoneNumber 666666999999 is not supported in the current country. Options: CN. Call
    *             formatPhoneNumber() to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0500', 0, async function (done) {
       try {
           await call.formatPhoneNumber('666666999999', { countryCode: 'CN' });
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0500 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0500 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0600
    * @tc.name    If phoneNumber is 20000000, enter non-existent options: abCDFG and call
    *             formatPhoneNumber() to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0600', 0, async function (done) {
       try {
           await call.formatPhoneNumber('20000000', { countryCode: 'abcdefg' });
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0600 fail');
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0600 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumber_Promise_0700
    * @tc.name    If phoneNumber is 20000000, options: , call formatPhoneNumber() to format the number and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumber_Promise_0700', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumber('20000000', { countryCode: '' });
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0700 fail');
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumber_Promise_0700 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0100
    * @tc.name    PhoneNumber is 010-0000-0000, options: CN, call formatPhoneNumberToE164() to format the number,
    *             and return +861000000000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0100', 0, async function (done) {
       call.formatPhoneNumberToE164('010-0000-0000', 'CN', (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0100 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '+861000000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0200
    * @tc.name    If phoneNumber is (010)00000000, options: CN, call formatPhoneNumberToE164() to format the number,
    *             return +861000000000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0200', 0, async function (done) {
       call.formatPhoneNumberToE164('(010)00000000', 'CN', (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0200 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '+861000000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0200 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0300
    * @tc.name    If phoneNumber is 01000000000, options: CN, call formatPhoneNumberToE164() to format the number,
    *             and return +861000000000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0300', 0, async function (done) {
       call.formatPhoneNumberToE164('01000000000', 'CN', (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0300 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === '+861000000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0300 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0400
    * @tc.name    PhoneNumber 666666999999 is not supported in the current country. Options: CN. Call
    *             formatPhoneNumberToE164() to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0400', 0, async function (done) {
       call.formatPhoneNumberToE164('666666999999', 'CN', (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0400 finish err = ' + err.message);
               done();
               return;
           }
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0400 fail');
           expect().assertFail();
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0500
    * @tc.name    If phoneNumber is 01000000000, type non-existent options: abCDFG. Call formatPhoneNumberToE164()
    *             to format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0500', 0, async function (done) {
       call.formatPhoneNumberToE164('01000000000', 'abcdfg', (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0500 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0500 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Async_0600
    * @tc.name    If phoneNumber is 01000000000, options: ', call formatPhoneNumberToE164() to
    *             format the number and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Async_0600', 0, async function (done) {
       call.formatPhoneNumberToE164('01000000000', '', (err) => {
           if (err) {
               console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0600 finish err = ' + err.message);
               done();
               return;
           }
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Async_0600 fail');
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0100
    * @tc.name    PhoneNumber is 52300000000, options: CN, call formatPhoneNumberToE164() to format the number,
    *             return +8652300000000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0100', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumberToE164('52300000000', 'CN');
           expect(data === '+8652300000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0100 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0200
    * @tc.name    If phoneNumber is (523)00000000, options: CN, call formatPhoneNumberToE164() to format the number,
    *             return +8652300000000
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0200', 0, async function (done) {
       console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0200 running');
       try {
           var data = await call.formatPhoneNumberToE164('(523)00000000', 'CN');
           expect(data === '+8652300000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0200 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0200 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0300
    * @tc.name    PhoneNumber is 523-0000-0000, options: CN. Call formatPhoneNumberToE164() to format the number.
    *             +8652300000000 is returned
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0300', 0, async function (done) {
       try {
           var data = await call.formatPhoneNumberToE164('523-0000-0000', 'CN');
           expect(data === '+8652300000000').assertTrue();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0300 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0300 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0400
    * @tc.name    Currently, phoneNumber is 999999, options: CN. Call formatPhoneNumberToE164() to
    *             format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0400', 0, async function (done) {
       try {
           await call.formatPhoneNumberToE164('999999', 'CN');
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0400 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0400 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0500
    * @tc.name    PhoneNumber is 999999. Type non-existent options: abCDFG. Call formatPhoneNumberToE164() to
    *             format the number and capture err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0500', 0, async function (done) {
       try {
           await call.formatPhoneNumberToE164('999999', 'abcdefg');
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0500 fail');
           expect().assertFail();
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0500 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_formatPhoneNumberToE164_Promise_0600
    * @tc.name    If phoneNumber is 52300000000, options: ', call formatPhoneNumberToE164()
    *             to format the number and catch err
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_formatPhoneNumberToE164_Promise_0600', 0, async function (done) {
       try {
           await call.formatPhoneNumberToE164('52300000000', '');
           expect().assertFail();
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0600 fail');
       } catch (err) {
           console.log('Telephony_CallManager_formatPhoneNumberToE164_Promise_0600 finish err = ' + err.message);
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0100
    * @tc.name    PhoneNumber: 0+0+0, options is 1. Call isEmergencyPhoneNumber() to check whether it is an
    *             emergency number. The return value is false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0100', 0, async function (done) {
       call.isEmergencyPhoneNumber('0+0+0', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0100 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0100 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0200
    * @tc.name    PhoneNumber: 13766669999, options 1. Call isEmergencyPhoneNumber() to check whether it is an
    *             emergency number. The return value is false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0200', 0, async function (done) {
       call.isEmergencyPhoneNumber('13766669999', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0200 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0200 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0300
    * @tc.name    PhoneNumber: 000, options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency number
    *             The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0300', 0, async function (done) {
       call.isEmergencyPhoneNumber('000', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0300 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0300 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0400
    * @tc.name    PhoneNumber: 112 with options 1. Call isEmergencyPhoneNumber() to verify whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0400', 0, async function (done) {
       call.isEmergencyPhoneNumber('112', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0400 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0400 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0500
    * @tc.name    PhoneNumber: 911, options are 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    * number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0500', 0, async function (done) {
       call.isEmergencyPhoneNumber('911', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0500 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0500 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0600
    * @tc.name    PhoneNumber: 08 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0600', 0, async function (done) {
       call.isEmergencyPhoneNumber('08', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0600 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0600 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0700
    * @tc.name    PhoneNumber: 118, options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0700', 0, async function (done) {
       call.isEmergencyPhoneNumber('118', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0700 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0700 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0800
    * @tc.name    PhoneNumber: 999 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0800', 0, async function (done) {
       call.isEmergencyPhoneNumber('999', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0800 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0800 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_0900
    * @tc.name    PhoneNumber: 119. Call isEmergencyPhoneNumber() to determine whether it is an emergency number.
    *             The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_0900', 0, async function (done) {
       call.isEmergencyPhoneNumber('119', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0900 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_0900 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_1000
    * @tc.name    PhoneNumber: 110, isEmergencyPhoneNumber() is called back to determine whether it is an emergency
    *             number, returning true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Async_1000', 0, async function (done) {
       call.isEmergencyPhoneNumber('110', { slotId: SLOT_0 }, (err, data) => {
           if (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_1000 fail');
               expect().assertFail();
               done();
               return;
           }
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_1000 finish data = ' + data);
           done();
       })
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Async_1300
    * @tc.name    PhoneNumber: 110, options -1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is false
    * @tc.desc    Function test
    */
       it('Telephony_CallManager_isEmergencyPhoneNumber_Async_1300', 0, async function (done) {
           call.isEmergencyPhoneNumber('110', { slotId: ERR_SLOT_ID }, (err) => {
               if (err) {
                   console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_1300 finish err = ' + err);
                   done();
                   return;
               }
               expect().assertFail();
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Async_1300 fail ');
               done();
           })
       })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0100
    * @tc.name    PhoneNumber: 0+0+0, options is 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0100', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('0+0+0', { slotId: SLOT_0 });
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0100 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0200
    * @tc.name    PhoneNumber: 13766669999, options 1. Call isEmergencyPhoneNumber() to check whether it is an
    *             emergency number. The return value is false
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0200', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('13766669999', { slotId: SLOT_0 });
           expect(data === false).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0200 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0200 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0300
    * @tc.name    PhoneNumber: 000 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0300', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('000', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0300 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0300 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0400
    * @tc.name    PhoneNumber: 112 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0400', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('112', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0400 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0400 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0500
    * @tc.name    PhoneNumber: 911 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0500', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('911', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0500 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0500 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0600
    * @tc.name    PhoneNumber: 08. If options are 1, call isEmergencyPhoneNumber() to check whether it is an
    *             emergency number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0600', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('08', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0600 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0600 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0700
    * @tc.name    PhoneNumber: 118 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0700', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('118', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0700 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0700 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_0800
    * @tc.name    PhoneNumber: 999 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0800', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('999', { slotId: SLOT_0 });
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0800 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_0800 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_1100
    * @tc.name    PhoneNumber: 119 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1100', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('119');
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1100 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1100 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_1200
    * @tc.name    PhoneNumber: 110 with options 1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is true
    * @tc.desc    Function test
    */
   it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1200', 0, async function (done) {
       try {
           var data = await call.isEmergencyPhoneNumber('110');
           expect(data).assertTrue();
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1200 finish data = ' + data);
       } catch (err) {
           console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1200 fail');
           expect().assertFail();
           done();
           return;
       }
       done();
   })

   /*
    * @tc.number  Telephony_CallManager_isEmergencyPhoneNumber_Promise_1300
    * @tc.name    PhoneNumber: 120 with options -1. Call isEmergencyPhoneNumber() to check whether it is an emergency
    *             number. The return value is err
    * @tc.desc    Function test
    */
       it('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1300', 0, async function (done) {
           try {
               await call.isEmergencyPhoneNumber('120', { slotId: ERR_SLOT_ID });
               expect.assertFail();
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1300 fail');
           } catch (err) {
               console.log('Telephony_CallManager_isEmergencyPhoneNumber_Promise_1300 finish err =' + err);
               done();
               return;
           }
           done();
       })

    /*
     * @tc.number  Telephony_CallManager_getCallWaitingStatus_Async_0200
     * @tc.name    Call waiting is enabled. SlotId is -1. GetCallWaiting ()
     *             is called to obtain call waiting and err is captured
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallWaitingStatus_Async_0200', 0, async function (done) {
        call.getCallWaitingStatus(ERR_SLOT_ID, (err) => {
            if (err) {
                console.log('Telephony_CallManager_getCallWaitingStatus_Async_0200 finish : ' + err.message);
                done();
                return;
            }
            expect().assertFail();
            console.log('Telephony_CallManager_getCallWaitingStatus_Async_0200 fail');
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getCallWaitingStatus_Promise_0200
     * @tc.name    Call waiting is enabled and slotId is -1. The callback method calls getCallWaiting()
     *             to get the call waiting and catch ERR
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallWaitingStatus_Promise_0200', 0, async function (done) {
        try {
            await call.getCallWaitingStatus(ERR_SLOT_ID);
            expect().assertFail();
            done();
            console.log('Telephony_CallManager_getCallWaitingStatus_Promise_0200 fail');
        } catch (err) {
            console.log('Telephony_CallManager_getCallWaitingStatus_Promise_0200 finish err =' + err.message);
            done();
        }
    })

    /*
     * @tc.number  Telephony_CallManager_getCallWaitingStatus_Async_0100
     * @tc.name    Call waiting is enabled, slotId is 1, getCallWaiting() is called to getCallWaiting, and status is 1
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallWaitingStatus_Async_0100', 0, async function (done) {
        call.getCallWaitingStatus(SLOT_0, (err, data) => {
            if (err) {
                console.log('Telephony_CallManager_getCallWaitingStatus_Async_0100 fail err = ' + err);
                expect().assertFail();
                done();
                return;
            }
            console.log('Telephony_CallManager_getCallWaitingStatus_Async_0100 data = ' + data);
            expect(data === STATUS_ON).assertTrue();
            console.log('Telephony_CallManager_getCallWaitingStatus_Async_0100 finish');
            done();
        })
    })

    /*
     * @tc.number  Telephony_CallManager_getCallWaitingStatus_Promise_0100
     * @tc.name    When the call wait sate is on test call getCallWaiting() in callback is on
     * @tc.desc    Function test
     */
    it('Telephony_CallManager_getCallWaitingStatus_Promise_0100', 0, async function (done) {
        try {
            let data = await call.getCallWaitingStatus(SLOT_0);
            expect(data === STATUS_ON).assertTrue();
            console.log('Telephony_CallManager_getCallWaitingStatus_Promise_0100 data = ' + data);
            console.log('Telephony_CallManager_getCallWaitingStatus_Promise_0100 finish');
            done();
        } catch (err) {
            expect().assertFail();
            console.log('Telephony_CallManager_getCallWaitingStatus_Promise_0100 fail err =' + err);
            done();
        }
    })
})
