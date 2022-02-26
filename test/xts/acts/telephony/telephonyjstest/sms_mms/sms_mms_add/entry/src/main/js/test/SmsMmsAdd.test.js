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

describe('SmsMmsAddTest', function () {

  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;

  const CORRECT_SMS_PDU = '01000F9168683106019196F400080A00680065006C006C006F';
  const RECEIVE_SMS_PDU = '240D91689141468496F600001270721142432302B319';
  // The PDU corresponding to the length is  CORRECT_SMS_PDU,RECEIVE_SMS_PDU
  var pduLength = [50, 44];

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
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0100
   * @tc.name     When SLOTID is the correct value,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_0100 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_0100 finish');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_0100 get fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
        let isAdd = (addOfPdu === CORRECT_SMS_PDU && getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_0100 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0200
   * @tc.name     When SLOTID is the wrong value,Failed to save SMS to SIM
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0200', 0, async function (done) {
    let data = {
      slotId: FALSE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (err, result) => {
      if (err) {
        console.log('Telephony_SmsMms_addSimMessage_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0200 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0300
   * @tc.name     Set the length of SMS service address "SMSC" exceeding the limit,Failed to save SMS to SIM
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '13800755500012222222',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_0300 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_0300 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0300 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0400
   * @tc.name     Set the SMS center service address "SMSC" to Chinese,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '短信中心服务地址',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_0400 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_0400 finish');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_0400 get cur fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
        let isAdd = (addOfPdu === CORRECT_SMS_PDU && getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_0400 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0500
   * @tc.name     Set the SMS center service address "SMSC" to English,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: 'English',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_0500 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_0500 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_0500 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0500 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0600
   * @tc.name     Set the SMS center service address "SMSC" to Special characters,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '$%&**^?',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_0600 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_0600 finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_0600 get cur fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
        let isAdd = (addOfPdu === CORRECT_SMS_PDU && getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_0600 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0700
   * @tc.name     Set the SMS center service address "SMSC" to Mixed character,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: 'gsSG**^$%#@短信中心123',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_0700 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_0700 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_0700 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0700 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0800
   * @tc.name     Set the PDU to empty,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_0800 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_0800 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_0800 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0800 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_0900
   * @tc.name     Set PDU to Chinese,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_0900', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '中文',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_0900 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_0900 get cur fail');
            done();
            return;
          }
          expect(getresult[0].shortMessage.pdu.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_0900 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_0900 get add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1000
   * @tc.name     Set PDU to English,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: 'scSA',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1000 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1000 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1000 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1000 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1100
   * @tc.name     Set PDU to figure,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '1233',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1100 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1100 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1100 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1100 add fail');
      done();
    });
  });


  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1200
   * @tc.name     Set PDU to Special characters,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '*&^%$#@!',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1200 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1200 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1200 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1200 add  fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1300
   * @tc.name     Set PDU to Mixed character,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '3xsGJ张三*&^%$#@!',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1300 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1300 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1300 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1300 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1400
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_READ,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_1400 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_1400 finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_1400 get cur fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[1]);
        let isAdd = (addOfPdu === RECEIVE_SMS_PDU && getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_READ);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_1400 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1500
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNREAD,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_1500 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_1500 finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_1500 get cur fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[1]);
        let isAdd = (addOfPdu === RECEIVE_SMS_PDU &&
        getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNREAD);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_1500 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1600
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNSENT,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_addSimMessage_Async_1600 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_addSimMessage_Async_1600 finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_addSimMessage_Async_1600 get cur fail');
          done();
          return;
        }
        let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
        let isAdd = (addOfPdu === CORRECT_SMS_PDU &&
        getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNSENT);
        expect(isAdd).assertTrue();
        console.log('Telephony_SmsMms_addSimMessage_Async_1600 getAllSimMessages cur finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1700
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNSENT,Set the PDU read type
   *              Description Failed to add SMS messages to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1700 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1700 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1700 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1700 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Async_1800
   * @tc.name     When status is equal to the correct value of  SIM_MESSAGE_STATUS_UNREAD,Set the PDU sending type
   *              Description Failed to add SMS messages to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Async_1800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        console.log('Telephony_SmsMms_addSimMessage_Async_1800 finish ');
        sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_addSimMessage_Async_1800 get cur fail');
            done();
            return;
          }
          expect(getresult.length === 0).assertTrue();
          console.log('Telephony_SmsMms_addSimMessage_Async_1800 getAllSimMessages cur finish');
        });
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Async_1800 add fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0100
   * @tc.name     When SLOTID is the correct value,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_0100 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0100 add fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[0]);
      expect(addOfPdu === CORRECT_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0100 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0100 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0200
   * @tc.name     When SLOTID is the wrong value,Failed to save SMS to SIM
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0200', 0, async function (done) {
    let data = {
      slotId: FALSE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0200 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0200 finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0300
   * @tc.name     Set the length of SMS service address "SMSC" exceeding the limit,
   *              Failed to save the SMS message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0300', 0, async function (done) {
    let curAddr = '13800755500012222222';
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: curAddr,
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0300 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0300 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0300 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0300 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0400
   * @tc.name     Set the SMS center service address "SMSC" to Chinese,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '短信中心服务地址',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_0400 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0400 add fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[0]);
      expect(addOfPdu === CORRECT_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0400 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0400 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0500
   * @tc.name     Set the SMS center service address "SMSC" to English,Failed to save the SMS message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: 'asAS',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0500 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0500 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0500 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0500 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0600
   * @tc.name     Set the SMS center service address "SMSC" to Special characters,Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '%&^*#！',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_0600 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0600 add fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[0]);
      expect(addOfPdu === CORRECT_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0600 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0600 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0700
   * @tc.name     Set the SMS center service address "SMSC" to Mixed character,
   *              Failed to save the SMS message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: 'gsSG**^$%#@短信中心123',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0700 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0700 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0700 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0700 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0800
   * @tc.name     Set the PDU to empty,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0800 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0800 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0800 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0800 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_0900
   * @tc.name     Set PDU to Chinese,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_0900', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '中文',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0900 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_0900 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0900 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_0900 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1000
   * @tc.name     Set PDU to English,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: 'scSA',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1000 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1000 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1000 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1000 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1100
   * @tc.name     Set PDU to figure,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '1233',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1100 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1100 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1100 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1100 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1200
   * @tc.name     Set PDU to Special characters,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '*&^%$#@!',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1200 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1200 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1200 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1200 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1300
   * @tc.name     Set PDU to Mixed character,Failed to save SMS to SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: '23xsGJ张三*&^%$#@!',
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1300 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1300 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1300 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1300 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1400
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_READ,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_1400 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1400 add  fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[1]);
      expect(addOfPdu === RECEIVE_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_READ).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1400 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1400 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1500
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNREAD,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_1500 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1500 add fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[1]);
      expect(addOfPdu === RECEIVE_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNREAD).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1500 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1500 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1600
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNSENT,
   *              Save a text message to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    try {
      await sms.addSimMessage(data);
      console.log('Telephony_SmsMms_addSimMessage_Promise_1600 finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1600 add fail');
      done();
      return;
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      let addOfPdu = interceptionPdu(promise[0].shortMessage.pdu, pduLength[0]);
      expect(addOfPdu === CORRECT_SMS_PDU &&
      promise[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNSENT).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1600 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1600 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1700
   * @tc.name     When status is equal to the correct value of SIM_MESSAGE_STATUS_UNSENT,Set the PDU read type
   *              Description Failed to add SMS messages to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1700 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1700 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1700 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1700 get 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_addSimMessage_Promise_1800
   * @tc.name     When status is equal to the correct value of  SIM_MESSAGE_STATUS_UNREAD,Set the PDU sending type
   *              Description Failed to add SMS messages to the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_addSimMessage_Promise_1800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    try {
      await sms.addSimMessage(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1800 add fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_addSimMessage_Promise_1800 finish ');
    }
    try {
      let promise = await sms.getAllSimMessages(TRUE_SLOT_ID);
      expect(promise.length === 0).assertTrue();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1800 getAllSimMessages cur finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_addSimMessage_Promise_1800 get 2 fail');
      done();
    }
  });
});