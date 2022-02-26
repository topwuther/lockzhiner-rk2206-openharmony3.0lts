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

describe('SmsMmsTest', function () {
  const SMS_SEND_DST_NUMBER = '';
  const MAX_TEST = 160;

  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0100
   * @tc.name    Call the interface sendMessage, set the card slot parameter "slotId" to 1,
   *             and send SMS successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0100', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: '111111',
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0100 sendCallback fail');
          done();
          return;
        }
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0100 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0100 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0100 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0200
   * @tc.name    Call the interface sendMessage, set the card slot parameter "slotId" to FALSE_SLOT_ID,
   *             SMS failed to send
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0200', 0, async function (done) {
    sms.sendMessage({
      slotId: FALSE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: 'hello',
      sendCallback: (err, value) => {
        if (err) {
          console.log('Telephony_SmsMms_sendMessage_0200 fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_FAILURE_UNKNOWN).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0200 finish');
        done();
      },
      deliveryCallback: () => {}
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0300
   * @tc.name    Call interface sendMessage, set destinationHost "destinationHost" to empty,
   *             send SMS failed
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0300', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: '',
      content: '11111',
      sendCallback: (err, value) => {
        if (err) {
          console.log('Telephony_SmsMms_sendMessage_0300 fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_FAILURE_UNKNOWN).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0300 finish');
        done();
      },
      deliveryCallback: () => {}
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0400
   * @tc.name    Call interface sendMessage, set the content "content" to empty,
   *             send a message failed
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0400', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: '',
      sendCallback: (err, value) => {
        if (err) {
          console.log('Telephony_SmsMms_sendMessage_0400 fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_FAILURE_UNKNOWN).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0400 finish');
        done();
      },
      deliveryCallback: () => {}
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0500
   * @tc.name    Call the interface SendMessage and set the length of "Content" to the maximum(MAX_TEST),
   *             and send a short message successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0500', 0, async function (done) {
    const count = MAX_TEST;
    let str = '';
    for (let index = 0;index < count;index++) {
      str += 't';
    }
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: str,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0500 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0500 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0500 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0500 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0600
   * @tc.name    Call the SendMessage interface and set the length of "Content" to exceed the maximum value,
   *             and send a short message successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0600', 0, async function (done) {
    const count = MAX_TEST + 1;
    let str = '';
    for (let index = 0;index < count;index++) {
      str += 't';
    }
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: str,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0600 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0600 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0600 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0600 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0700
   * @tc.name    Call the interface SendMessage,
   *             set the content "Content" as the content of Chinese character type,
   *             and send a short message successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0700', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: '中文',
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0700 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0700 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0700 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0700 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0800
   * @tc.name    Call the interface sendMessage,
   *             set the content "content" to a single byte character (English alphabet or number) type of content,
   *             send a short message successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0800', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: 'hhhAAA112089',
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0800 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0800 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0800 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0800 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_0900
   * @tc.name    Call the interface sendMessage,
   *             set the content "content" to the content of special character type,
   *             and send the short message successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_0900', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: 'ㄅㄆ$￡á ǎ㊊↑◎┴%@&*^#',
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0900 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0900 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_0900 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_0900 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1000
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type, set the destinationPort "DestinationPort" as 0x00,
   *             and SMS is sent successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1000', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: [54, 2, 3, 6, 3, 1, 1],
      destinationPort: 0x00,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1000 sendCallback fail');
          done();
          return;
        }
        console.log(`rawdata msg sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1000 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          console.log('deliveryCallback err');
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1000 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1000 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1100
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type, set the destinationPort "DestinationPort" as 0xffff,
   *             and SMS is sent successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1100', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: [54, 2, 3, 6, 3, 1, 1],
      destinationPort: 0xffff,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1100 sendCallback fail');
          done();
          return;
        }
        console.log(`rawdata msg sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1100 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          console.log('deliveryCallback err');
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1100 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1100 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1200
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type, set the destinationPort "DestinationPort" as less than minimum,
   *             Failed to send SMS messages
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1200', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: [54, 2, 3, 6, 3, 1, 1],
      destinationPort: 0x00 - 1,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1200 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1200 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1200 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1200 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1300
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type, set the destinationPort "DestinationPort" as beyond the maximum,
   *             Failed to send SMS messages
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1300', 0, async function (done) {
    let contentArray = [];
    for (let i = 0;i < MAX_TEST;++i) {
      contentArray.push(1);
    }
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: contentArray,
      destinationPort: 0x00,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1300 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1300 sendCallback  finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1300 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1300 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1400
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type,Set "Content" length to maximum (160)，SMS sent successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1400', 0, async function (done) {
    let contentArray = [];
    for (let i = 0;i < MAX_TEST + 1;++i) {
      contentArray.push(1);
    }
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: contentArray,
      destinationPort: 0x00,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1400 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1400 sendCallback finish');
        done()
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1400 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1400 deliveryCallback finish');
        done();
      }
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_sendMessage_1500
   * @tc.name    Call the interface SendMessage.
   *             If the "content" is of data type,The content length exceeds the maximum length，SMS sent successfully
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_sendMessage_1500', 0, async function (done) {
    sms.sendMessage({
      slotId: TRUE_SLOT_ID,
      destinationHost: SMS_SEND_DST_NUMBER,
      content: [54, 2, 3, 6, 3, 1, 1],
      destinationPort: 0xffff + 1,
      sendCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1500 sendCallback fail');
          done();
          return;
        }
        console.log(`sendCallback success sendResult = ${value.result}`);
        expect(value.result === sms.SEND_SMS_SUCCESS).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1500 sendCallback finish');
      },
      deliveryCallback: (err, value) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_sendMessage_1500 deliveryCallback fail');
          done();
          return;
        }
        expect(value.pdu.length !== 0).assertTrue();
        console.log('Telephony_SmsMms_sendMessage_1500 deliveryCallback finish');
        done();
      }
    });
  });
});