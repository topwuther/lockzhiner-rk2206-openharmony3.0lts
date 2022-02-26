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

describe('SmsMmsCreateTest', function () {
  var rawArrayNull = [];

  var rawArray = [
      0x08,0x91,0x68,0x31,0x08,0x70,0x55,0x05,0xF0,0x01,0x00,
      0x07,0x91,0x68,0x01,0x80,0xF6,0x00,0x08,0x04,0x00,0x41,0x00,0x61
  ];
  // rawArray PDU data
  const SC_TIMESTAMP = 12011;
  var MESSAGEBODY = 'Aa';
  var RAWADDRESS = '+8610086';
  var SC_ADDRESS = '+8613800755500';

  var pduArray = [
      0x00,0x01,0x00,0x07,0x91,0x68,0x01,0x80,0xF6,0x00,
      0x08,0x0A,0x00,0x68,0x00,0x65,0x00,0x6C,0x00,0x6C,0x00,0x6F
  ];
  // pduArray PDU data
  var PDU_MESSAGEBODY = 'hello';
  var PDU_RAWADDRESS = '+8610086';
  const PDU_SC_TIMESTAMP = 11950;

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Async_0100
   * @tc.name    Call interface CreateMessage,
   *             pass in the PDU(rawArray) in line with the coding specification, the specification is 3GPP,
   *             shortMessage Don't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Async_0100', 0, async function (done) {
    sms.createMessage(rawArray, '3gpp', (err, shortMessage) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_createMessage_Async_0100 fail');
        done();
        return;
      }
      expect(shortMessage.visibleMessageBody === MESSAGEBODY).assertTrue();
      expect(shortMessage.visibleRawAddress === RAWADDRESS).assertTrue();
      expect(shortMessage.messageClass === sms.FORWARD_MESSAGE).assertTrue();
      expect(shortMessage.protocolId === 0).assertTrue();
      expect(shortMessage.scAddress === SC_ADDRESS).assertTrue();
      expect(shortMessage.scTimestamp === SC_TIMESTAMP).assertTrue();
      expect(shortMessage.isReplaceMessage).assertFalse();
      expect(shortMessage.hasReplyPath).assertFalse();
      expect(shortMessage.pdu.length > 0).assertTrue();
      expect(shortMessage.status === 0).assertTrue();
      expect(shortMessage.isSmsStatusReportMessage).assertTrue();
      console.log('Telephony_SmsMms_createMessage_Async_0100 finish');
      done();
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Async_0200
   * @tc.name    Call interface CreateMessage,
   *             The incoming PDU is empty, the specification is 3GPP,
   *             shortMessage isn't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Async_0200', 0, async function (done) {
    sms.createMessage(rawArrayNull, '3gpp', (err, shortMessage) => {
      if (err) {
        console.log('Telephony_SmsMms_createMessage_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Async_0200 fail');
      done();
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Async_0500
   * @tc.name    Call interface CreateMessage,
   *             pass in the PDU(pduArray) in line with the coding specification, the specification is 3GPP,
   *             shortMessage Don't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Async_0500', 0, async function (done) {
    sms.createMessage(pduArray, '3gpp', (err, shortMessage) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_createMessage_Async_0700 fail');
        done();
        return;
      }
      expect(shortMessage.visibleMessageBody === PDU_MESSAGEBODY).assertTrue();
      expect(shortMessage.visibleRawAddress === PDU_RAWADDRESS).assertTrue();
      expect(shortMessage.messageClass === sms.FORWARD_MESSAGE).assertTrue();
      expect(shortMessage.protocolId === 0).assertTrue();
      expect(shortMessage.scAddress.length === 0).assertTrue();
      expect(shortMessage.scTimestamp === PDU_SC_TIMESTAMP).assertTrue();
      expect(shortMessage.isReplaceMessage).assertFalse();
      expect(shortMessage.hasReplyPath).assertFalse();
      expect(shortMessage.pdu.length > 0).assertTrue();
      expect(shortMessage.status === 0).assertTrue();
      expect(shortMessage.isSmsStatusReportMessage).assertTrue();
      console.log('Telephony_SmsMms_createMessage_Async_0500 finish');
      done();
    });
  });

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Promise_0100
   * @tc.name    Call interface CreateMessage,
   *             pass in the PDU(rawArray) in line with the coding specification, the specification is 3GPP,
   *             promise returns the result Don't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Promise_0100', 0, async function (done) {
    try {
      var promise = await sms.createMessage(rawArray, '3gpp');
      expect(promise.visibleMessageBody === MESSAGEBODY).assertTrue();
      expect(promise.visibleRawAddress === RAWADDRESS).assertTrue();
      expect(promise.messageClass === sms.FORWARD_MESSAGE).assertTrue();
      expect(promise.protocolId === 0).assertTrue();
      expect(promise.scAddress === SC_ADDRESS).assertTrue();
      expect(promise.scTimestamp === SC_TIMESTAMP).assertTrue();
      expect(promise.isReplaceMessage).assertFalse();
      expect(promise.hasReplyPath).assertFalse();
      expect(promise.pdu.length > 0).assertTrue();
      expect(promise.status === 0).assertTrue();
      expect(promise.isSmsStatusReportMessage).assertTrue();
      console.log('Telephony_SmsMms_createMessage_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0100 fail');
      done();
    }
  });

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Promise_0200
   * @tc.name    Call interface CreateMessage,
   *             The incoming PDU is empty, the specification is 3GPP,
   *             promise returns the result Don't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Promise_0200', 0, async function (done) {
    try {
      await sms.createMessage(rawArrayNull, '3gpp');
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0200 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_createMessage_Promise_0200 finish');
      done();
    }
  });

  /*
   * @tc.number  Telephony_SmsMms_createMessage_Promise_0500
   * @tc.name    Call interface CreateMessage,
   *             pass in the PDU(pduArray) in line with the coding specification, the specification is 3GPP,
   *             promise returns the result Don't empty
   * @tc.desc    Function test
   */
  it('Telephony_SmsMms_createMessage_Promise_0500', 0, async function (done) {
    try {
      var promise = await sms.createMessage(pduArray, '3gpp');
      expect(promise.visibleMessageBody === PDU_MESSAGEBODY).assertTrue();
      expect(promise.visibleRawAddress === PDU_RAWADDRESS).assertTrue();
      expect(promise.messageClass === sms.FORWARD_MESSAGE).assertTrue();
      expect(promise.protocolId === 0).assertTrue();
      expect(promise.scAddress.length === 0).assertTrue();
      expect(promise.scTimestamp === PDU_SC_TIMESTAMP).assertTrue();
      expect(promise.isReplaceMessage).assertFalse();
      expect(promise.hasReplyPath).assertFalse();
      expect(promise.pdu.length > 0).assertTrue();
      expect(promise.status === 0).assertTrue();
      expect(promise.isSmsStatusReportMessage).assertTrue();
      console.log('Telephony_SmsMms_createMessage_Promise_0500 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0500 fail');
      done();
    }
  });
});