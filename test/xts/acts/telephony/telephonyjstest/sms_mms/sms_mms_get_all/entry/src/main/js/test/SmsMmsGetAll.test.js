/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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
import sms from '@ohos.telephony.sms';
import {
  describe,
  beforeAll,
  beforeEach,
  afterEach,
  afterAll,
  it,
  expect
} from 'deccjsunit/index';

describe('SmsMmsGetAllTest', function () {
  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;
  const CORRECT_SMS_PDU = '01000F9168683106019196F400080A00680065006C006C006F';
  const PDU_LENGTH = 50;
  const INTERCEPT_POINT_PLUS = 20;

  beforeAll(async function () {
    //Delete all SMS messages from the SIM card
    sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
      if (geterr) {
        return;
      }
      if (getresult.length !== 0) {
        for (let index = 0; index < getresult.length; ++index) {
          sms.delSimMessage(TRUE_SLOT_ID, getresult[index].indexOnSim, (err) => {});
        }
      }
    });
  });

  afterEach(async function () {
    //Delete all SMS messages from the SIM card
    sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
      if (geterr) {
        return;
      }
      if (getresult.length !== 0) {
        for (let index = 0; index < getresult.length; ++index) {
          sms.delSimMessage(TRUE_SLOT_ID, getresult[index].indexOnSim, (err) => {});
        }
      }
    });
  });

  // Gets the PDU that is stored
  function interceptionPdu (parameter, pduLength) {
    let strPdu = Array.from(parameter, function (byte) {
      return (`0${(byte & 0xFF).toString(16)}`).slice(-2);
    }).join('');
    let newPdu = strPdu.toUpperCase();

    let pduBegin = INTERCEPT_POINT_PLUS;
    let intPoint = Number(newPdu.substring(0, 2));
    pduBegin = intPoint * 2 + 2;
    let pduEnd = pduLength + pduBegin;
    return newPdu.substring(pduBegin, pduEnd);
  }

  /**
   * @tc.number   Telephony_SmsMms_getAllSIMMessages_Async_0100
   * @tc.name     When SLOTID is the correct value,Query all SMS records of the SIM card.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getAllSIMMessages_Async_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 get fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, PDU_LENGTH);
        let isAdd = (addOfPdu === CORRECT_SMS_PDU &&
                        getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 getAllSimMessages cur finish');
        sms.delSimMessage(TRUE_SLOT_ID, getresult[0].indexOnSim, (delerr) => {
          if (delerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 del fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 delSimMessage finish');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 get 2 fail');
              done();
              return;
            }
            expect(getresult.length === 0).assertTrue();
            console.log('Telephony_SmsMms_getAllSIMMessages_Async_0100 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_getAllSIMMessages_Async_0200
   * @tc.name     When "SLOTID" is an error value,Failed to query all SMS records for SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getAllSIMMessages_Async_0200', 0, async function (done) {
    sms.getAllSimMessages(FALSE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_getAllSIMMessages_Async_0200 fail');
        done();
        return;
      }
      expect(result === undefined || result.length === 0).assertTrue();
      console.log('Telephony_SmsMms_getAllSIMMessages_Async_0200 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_getAllSIMMessages_Promise_0100
   * @tc.name     Delete the SMS, then get all the SMS
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getAllSIMMessages_Promise_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 addSimMessage finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 get add fail');
      done();
      return;
    }
    try {
      let promiseGet = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promiseGet[0].shortMessage.pdu, PDU_LENGTH);
      let isAdd = (addOfPdu === CORRECT_SMS_PDU &&
                promiseGet[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT);
      expect(isAdd).assertTrue();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 getAllSimMessages before finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 get befor fail');
      done();
      return;
    }
    try {
      await sms.delSimMessage(TRUE_SLOT_ID, addIndex);
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 delAllSIMMessages cur finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 get del fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0100 get cur fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_getAllSIMMessages_Promise_0200
   * @tc.name     When "SLOTID" is an error value,Failed to query all SMS records for SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getAllSIMMessages_Promise_0200', 0, async function (done) {
    try {
      let promise = await sms.getAllSimMessages(FALSE_SLOT_ID);
      expect(promise === null || promise === undefined || promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0200 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getAllSIMMessages_Promise_0200 fail');
      done();
    }
  });
});