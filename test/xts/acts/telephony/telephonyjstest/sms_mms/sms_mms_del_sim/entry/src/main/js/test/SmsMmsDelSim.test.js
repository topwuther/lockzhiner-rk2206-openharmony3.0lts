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

describe('SmsMmsDelTest', function () {
  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;
  const CORRECT_SMS_PDU = '01000F9168683106019196F400080A00680065006C006C006F';

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

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Async_0100
   * @tc.name     When SLOTID is the correct value,Deletes a text message from the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Async_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.MESSAGE_HAS_BEEN_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_delSimMessage_Async_0100 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_delSimMessage_Async_0100 addSimMessage finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_delSimMessage_Async_0100 get 1 fail');
          done();
          return;
        }
        sms.delSimMessage(TRUE_SLOT_ID, result[0].indexOnSim, (delerr) => {
          if (delerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_delSimMessage_Async_0100 del fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_delSimMessage_Async_0100 delSimMessage finish');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_delSimMessage_Async_0100 get 2 fail');
              done();
              return;
            }
            expect(getresult.length === 0).assertTrue();
            console.log('Telephony_SmsMms_delSimMessage_Async_0100 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Async_0200
   * @tc.name     When SLOTID is the wrong value,Deletes a text message from the SIM card fail
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Async_0200', 0, async function (done) {
    sms.delSimMessage(FALSE_SLOT_ID, 0, (err) => {
      if (err) {
        console.log('Telephony_SmsMms_delSimMessage_Async_0200 delSimMessage finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Async_0200 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Async_0300
   * @tc.name     Delete short messages with invalid values,Deletes a text message from the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Async_0300', 0, async function (done) {
    sms.delSimMessage(TRUE_SLOT_ID, 0, (delerr) => {
      if (delerr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_delSimMessage_Async_0300 del fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_delSimMessage_Async_0300 delSimMessage finish');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_delSimMessage_Async_0300 get 2 fail');
          done();
          return;
        }
        expect(getresult.length === 0).assertTrue();
        console.log('Telephony_SmsMms_delSimMessage_Async_0300 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Async_0400
   * @tc.name     Set index to an error value,Deletes a text message from the SIM card fail
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Async_0400', 0, async function (done) {
    let addIndex = 0xffffffff;
    sms.delSimMessage(TRUE_SLOT_ID, addIndex, (delerr) => {
      if (delerr) {
        console.log('Telephony_SmsMms_delSimMessage_Async_0400 delSimMessage finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Async_0400 del fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Promise_0100
   * @tc.name     When SLOTID is the correct value,Deletes a text message from the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Promise_0100', 0, async function (done) {
    let beforeSmsRecord = [];
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.MESSAGE_HAS_BEEN_SENT
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 addSimMessage finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 add fail');
      done();
      return;
    }
    let addIndex = 0;
    try {
      beforeSmsRecord = await sms.getAllSimMessages(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 getAllSimMessages before finish');
      addIndex = beforeSmsRecord[0].indexOnSim;
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 get 1 fail');
      done();
      return;
    }
    try {
      await sms.delSimMessage(TRUE_SLOT_ID, addIndex);
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 delAllSIMMessages cur finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 del  fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0100 get 3 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Promise_0200
   * @tc.name     When SLOTID is the wrong value,Deletes a text message from the SIM card fail
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Promise_0200', 0, async function (done) {
    try {
      await sms.delSimMessage(FALSE_SLOT_ID, 0);
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0200 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_delSimMessage_Promise_0200 finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Promise_0300
   * @tc.name     When msgIndex is the index value of invalid SMS messages,Deletes a text message from the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Promise_0300', 0, async function (done) {
    try {
      await sms.delSimMessage(TRUE_SLOT_ID, 0);
      console.log('Telephony_SmsMms_delSimMessage_Promise_0300 delAllSIMMessages cur finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0300 del fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0300 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0300 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_delSimMessage_Promise_0400
   * @tc.name     Set index to an error value,Deletes a text message from the SIM card fail
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_delSimMessage_Promise_0400', 0, async function (done) {
    let addIndex = 0xffffffff;
    try {
      await sms.delSimMessage(TRUE_SLOT_ID, addIndex);
      expect().assertFail();
      console.log('Telephony_SmsMms_delSimMessage_Promise_0400 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_delSimMessage_Promise_0400 finish');
      done();
    }
  });
});