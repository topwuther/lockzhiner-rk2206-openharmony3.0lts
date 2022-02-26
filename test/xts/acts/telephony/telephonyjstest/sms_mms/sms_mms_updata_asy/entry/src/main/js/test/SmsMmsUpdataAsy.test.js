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

describe('SmsMmsUpdataTest', function () {
  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;
  const CORRECT_SMS_PDU = '01000F9168683106019196F400080A00680065006C006C006F';
  const RECEIVE_SMS_PDU = '240D91689141468496F600001270721142432302B319';
  const RECEIVE_OTHER_SMS_PDU = '240D91689141468496F600001270721174322302B91C';

  // The PDU corresponding to the length is  CORRECT_SMS_PDU,RECEIVE_SMS_PDU,RECEIVE_OTHER_SMS_PDU
  var pduLength = [50, 44, 44];

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
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0100
   * @tc.name     When SLOTID is the wrong value,Failed to update SIM card SMS record
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0100', 0, async function (done) {
    let upData = {
      slotId: FALSE_SLOT_ID,
      msgIndex: 0,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.updateSimMessage(upData, (err) => {
      if (err) {
        console.log('Telephony_SmsMms_updateSimMessage_Async_0100 update finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_updateSimMessage_Async_0100 update fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0200
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_READ to SIM_MESSAGE_STATUS_UNREAD,
   *              Update a SIM card SMS record.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNREAD,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0200 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0200 add finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0200 get 1 fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0200 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0200 update fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_updateSimMessage_Async_0200 update finish ');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0200 fail');
              done();
              return;
            }
            expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNREAD).assertTrue();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0200 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0300
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_READ to SIM_MESSAGE_STATUS_SENT,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0300 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0300 finish addresult ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0300 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0300 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_0300 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_0300 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_READ).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0300 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0300 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0400
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_READ to SIM_MESSAGE_STATUS_UNSENT,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNSENT,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0400 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0400 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0400 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0400 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_0400 update finish result');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_0400 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_READ).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0400 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0400 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0500
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNREAD to SIM_MESSAGE_STATUS_READ,
   *              Update a SIM card SMS record.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_READ,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0500 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0500 finish add result');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0500 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0500 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0500 update fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_updateSimMessage_Async_0500 update finish result');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0500 fail');
              done();
              return;
            }
            expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_READ).assertTrue();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0500 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0600
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNREAD to SIM_MESSAGE_STATUS_SENT,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0600 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0600 finish add result');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0600 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0600 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_0600 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNREAD).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0600 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0600 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0700
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNREAD to SIM_MESSAGE_STATUS_UNSENT,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNREAD
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNSENT,
      pdu: RECEIVE_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0700 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0700 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0700 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0700 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_0700 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_0700 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNREAD).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0700 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0700 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0800
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_SENT to SIM_MESSAGE_STATUS_UNSENT,
   *              Update a SIM card SMS record.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNSENT,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0800 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0800 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0800 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0800 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0800 update fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_updateSimMessage_Async_0800 update finish result');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0800 fail');
              done();
              return;
            }
            expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNSENT).assertTrue();
            console.log('Telephony_SmsMms_updateSimMessage_Async_0800 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_0900
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_SENT to SIM_MESSAGE_STATUS_READ,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_0900', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_READ,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_0900 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_0900 finish add result finish ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0900 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_0900 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_0900 update finish result');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_0900 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_0900 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_0900 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1000
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_SENT to SIM_MESSAGE_STATUS_UNREAD,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNREAD,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1000 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1000 finish add result');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1000 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1000 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1000 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1000 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1000 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1000 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1100
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNSENT to SIM_MESSAGE_STATUS_SENT,
   *              Update a SIM card SMS record.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1100 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1100 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1100 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1100 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_updateSimMessage_Async_1100 update fail');
            done();
            return;
          }
          console.log('Telephony_SmsMms_updateSimMessage_Async_1100 update finish result ');
          sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1100 fail');
              done();
              return;
            }
            expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_SENT).assertTrue();
            console.log('Telephony_SmsMms_updateSimMessage_Async_1100 getAllSimMessages cur finish');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1200
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNSENT to SIM_MESSAGE_STATUS_READ,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_READ,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1200 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1200 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1200 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1200 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1200 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1200 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNSENT).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1200 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1200 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1300
   * @tc.name     Example Change the SMS status from SIM_MESSAGE_STATUS_UNSENT to SIM_MESSAGE_STATUS_UNREAD,
   *              Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_UNSENT
    };
    let addIndex = 0;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_UNREAD,
      pdu: CORRECT_SMS_PDU,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1300 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1300 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1300 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1300 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1300 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1300 fail');
                done();
                return;
              }
              expect(getresult[0].simMessageStatus === sms.SIM_MESSAGE_STATUS_UNSENT).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1300 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1300 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1400
   * @tc.name     Set the PDU to empty,Failed to update SIM card SMS record
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = '';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1400 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1400 finish addresult');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1400 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1400 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1400 update finish');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1400 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1400 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1400 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1500
   * @tc.name     Set the PDU to chinese,Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = '中文';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1500 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1500 finish addresult ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1500 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1500 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1500 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1500 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1500 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1500 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1600
   * @tc.name     Set the PDU to English,Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1600', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = 'zsZS';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1600 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1600 finish add result');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1600 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1600 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1600 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1600 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1600 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1600 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1700
   * @tc.name     Set the PDU to figure,Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = '1233';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1700 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1700 finish result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1700 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1700 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1700 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1700 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1700 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1700 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1800
   * @tc.name     Set the PDU to Special characters,Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = '!@#$%^&*';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1800 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1800 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1800 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1800 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1800 update finish updataresult ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1800 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1800 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1800 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_1900
   * @tc.name     Set the PDU to Mixed character,Failed to update the SMS record of the SIM card
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_1900', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = '122zgGB张三!@#$%^&*';
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_1900 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_1900 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1900 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_1900 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_1900 update finish ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_1900 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_1900 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_1900 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_2000
   * @tc.name     The SMS status does not change,
   *              the PDU data type is changed from the sending type to the receiving type,Update failed
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_2000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: CORRECT_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_SENT
    };
    let addIndex = 0;
    let upDataPdu = RECEIVE_OTHER_SMS_PDU;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_SENT,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_2000 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_2000 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_2000 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_2000 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_2000 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_2000 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[0]);
              expect(addOfPdu === CORRECT_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_2000 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_2000 update fail');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_updateSimMessage_Async_2100
   * @tc.name     The SMS status does not change,
   *              Example Change the data type of the PDU from the received type to the sent type,Update failed
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_updateSimMessage_Async_2100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      smsc: '',
      pdu: RECEIVE_OTHER_SMS_PDU,
      status: sms.SIM_MESSAGE_STATUS_READ
    };
    let addIndex = 0;
    let upDataPdu = CORRECT_SMS_PDU;
    let upData = {
      slotId: TRUE_SLOT_ID,
      msgIndex: addIndex,
      newStatus: sms.SIM_MESSAGE_STATUS_READ,
      pdu: upDataPdu,
      smsc: ''
    };
    sms.addSimMessage(data, (adderr) => {
      if (adderr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_updateSimMessage_Async_2100 add fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_updateSimMessage_Async_2100 finish add result ');
      sms.getAllSimMessages(TRUE_SLOT_ID, (err, result) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_2100 get fail');
          done();
          return;
        }
        addIndex = result[0].indexOnSim;
        console.log('Telephony_SmsMms_updateSimMessage_Async_2100 getAllSimMessages finish');
        sms.updateSimMessage(upData, (updataerr) => {
          if (updataerr) {
            console.log('Telephony_SmsMms_updateSimMessage_Async_2100 update finish result ');
            sms.getAllSimMessages(TRUE_SLOT_ID, (geterr, getresult) => {
              if (geterr) {
                expect().assertFail();
                console.log('Telephony_SmsMms_updateSimMessage_Async_2100 fail');
                done();
                return;
              }
              let addOfPdu = interceptionPdu(getresult[0].shortMessage.pdu, pduLength[2]);
              expect(addOfPdu === RECEIVE_OTHER_SMS_PDU).assertTrue();
              console.log('Telephony_SmsMms_updateSimMessage_Async_2100 getAllSimMessages cur finish');
            });
            done();
            return;
          }
          expect().assertFail();
          console.log('Telephony_SmsMms_updateSimMessage_Async_2100 update fail');
          done();
        });
      });
    });
  });
});