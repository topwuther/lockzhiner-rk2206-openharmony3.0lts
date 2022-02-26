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

describe('SmsMmsSmscTest', function () {
  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;

  const OTHER_SMSC_NUMBER = '+8613800755500';
  const USABLE_SMSC_NUMBER = '+8613800512500';

  var DATA_SCADDR = '';
  beforeAll(async function () {
    sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
      if (geterr) {
        return;
      }
      DATA_SCADDR = getresult
    });
  });
  beforeEach(async function () {
    sms.setSmscAddr(TRUE_SLOT_ID, DATA_SCADDR, (err) => {});
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0100
   * @tc.name     When SLOTID is the correct value,Set up the SMS service address.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0100', 0, async function (done) {
    let beforeAddr = '';
    let curAddr = USABLE_SMSC_NUMBER;
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0100 get 1 fail');
        done();
        return;
      }
      beforeAddr = result;
      if (beforeAddr === curAddr) {
        curAddr = OTHER_SMSC_NUMBER;
      }
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_setSmscAddr_Async_0100 set fail');
          done();
          return;
        }
        console.log('setSmscAddr finish ');
        sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_setSmscAddr_Async_0100 get 2 fail');
            done();
            return;
          }
          expect(getresult === curAddr).assertTrue();
          console.log(`getSmscAddr result = ${getresult}`);
          console.log('Telephony_SmsMms_setSmscAddr_Async_0100 finish');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0200
   * @tc.name     When "SLOTID" is an error value,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0200', 0, async function (done) {
    sms.setSmscAddr(FALSE_SLOT_ID, '', (err) => {
      if (err) {
        console.log('Telephony_SmsMms_setSmscAddr_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Async_0200 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0300
   * @tc.name     Set the value of “scAddr” to null,Description Failed to set the SMS service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0300', 0, async function (done) {
    let curAddr = '';
    let beforeAddr = '';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0300 get 1 fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              console.log('Telephony_SmsMms_setSmscAddr_Async_0300 get 2 fail');
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log('Telephony_SmsMms_setSmscAddr_Async_0300 finish');
          });
          done();
          return;
        }
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0300 set fail');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0400
   * @tc.name     Sets the “scAddr” length to the maximum（20）,Set up the SMS service address.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0400', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '01234567890123456789';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0400 fail');
        done();
        return;
      }
      beforeAddr = result;
      if (beforeAddr === curAddr) {
        curAddr = '91234567890123456780';
      }
      console.log(`getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          expect().assertFail();
          done();
          return;
        }
        console.log('setSmscAddr finish ');
        sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            done();
            return;
          }
          expect(getresult === curAddr).assertTrue();
          console.log(`setSmscAddr result = ${getresult}`);
          console.log('Telephony_SmsMms_setSmscAddr_Async_0400 finish');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0500
   * @tc.name     Sets the “scAddr” length to exceed the maximum（20）,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0500', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '012345678901234567890';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0500 get fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log(`setSmscAddr result = ${getresult}`);
            console.log('Telephony_SmsMms_setSmscAddr_Async_0500 finish');
          });
          done();
          return;
        }
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0500 setSmscAddr fail');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0600
   * @tc.name     Set “scAddr” to Chinese ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0600', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '短信中心服务地址';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0600 fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`Telephony_SmsMms_setSmscAddr_Async_0600 getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log(`setSmscAddr result = ${getresult}`);
            console.log('Telephony_SmsMms_setSmscAddr_Async_0600 finish');
          });
          done();
          return;
        }
        console.log('Telephony_SmsMms_setSmscAddr_Async_0600 setSmscAddr fail ');
        expect().assertFail();
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0700
   * @tc.name     Set “scAddr” to English ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0700', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = 'zsAS';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0700 fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`Telephony_SmsMms_setSmscAddr_Async_0700 getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log(`setSmscAddr result = ${getresult}`);
            console.log('Telephony_SmsMms_setSmscAddr_Async_0700 finish');
          });
          done();
          return;
        }
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0700 setSmscAddr fail ');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0800
   * @tc.name     Set “scAddr” to  special character ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0800', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '@！#￥%……&*';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0800 fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`Telephony_SmsMms_setSmscAddr_Async_0800 getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log(`setSmscAddr result = ${getresult}`);
            console.log('Telephony_SmsMms_setSmscAddr_Async_0800 finish');
          });
          done();
          return;
        }
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0800 setSmscAddr fail ');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Async_0900
   * @tc.name     Set “scAddr” to  special Mixed character ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Async_0900', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '1Zz短信@！#￥%……&*+';
    sms.getSmscAddr(TRUE_SLOT_ID, (err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0900 fail');
        done();
        return;
      }
      beforeAddr = result;
      console.log(`Telephony_SmsMms_setSmscAddr_Async_0900 getSmscAddr beforeAddr = ${beforeAddr}`);
      sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (seterr) => {
        if (seterr) {
          sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
            if (geterr) {
              expect().assertFail();
              done();
              return;
            }
            expect(getresult === beforeAddr).assertTrue();
            console.log(`setSmscAddr result = ${getresult}`);
            console.log('Telephony_SmsMms_setSmscAddr_Async_0900 finish');
          });
          done();
          return;
        }
        expect().assertFail();
        console.log('Telephony_SmsMms_setSmscAddr_Async_0900 setSmscAddr fail ');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0100
   * @tc.name     When SLOTID is the correct value,Set up the SMS service address.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0100', 0, async function (done) {
    let beforeAddr = '';
    let curAddr = USABLE_SMSC_NUMBER;
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      if (beforeAddr === curAddr) {
        curAddr = OTHER_SMSC_NUMBER;
      }
      console.log(`getSmscAddr beforeAddr = ${beforeAddr}`);
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0100 fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      console.log('setSmscAddr  finish ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0100 fail');
      done();
      return;
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === curAddr).assertTrue();
      console.log(`getSmscAddr result = ${promise}`);
      console.log('Telephony_SmsMms_createMessage_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0100 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0200
   * @tc.name     When "SLOTID" is an error value,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0200', 0, async function (done) {
    let curAddr = USABLE_SMSC_NUMBER;
    try {
      await sms.setSmscAddr(FALSE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_createMessage_Promise_0200 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_createMessage_Promise_0200  finish ');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0300
   * @tc.name     Set the value of “scAddr” to null,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0300', 0, async function (done) {
    let beforeAddr = '';
    let curAddr = '';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 get finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 set finish');
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 get finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0300 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0400
   * @tc.name     Sets the “scAddr” length to the maximum（20）,set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0400', 0, async function (done) {
    let beforeAddr = '';
    let curAddr = '01234567890123456789';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      if (beforeAddr === curAddr) {
        curAddr = '91234567890123456780';
      }
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 getSmscAddr beforeAddr ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 set');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 fail');
      done();
      return;
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === curAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0400 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0500
   * @tc.name     Sets the “scAddr” length to exceed the maximum（20）,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0500', 0, async function (done) {
    let beforeAddr = '';
    let curAddr = '012345678901234567890';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 getSmscAddr beforeAddr ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 set fail');
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 set finish');
      done();
      return;
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0500 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0600
   * @tc.name     Set “scAddr” to Chinese ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0600', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '短信中心服务地址';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 getSmscAddr beforeAddr  ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 getSmscAddr fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 setSmscAddr fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 setSmscAddr finish  ');
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0600 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0700
   * @tc.name     Set “scAddr” to English ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0700', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = 'zsAS';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 getSmscAddr beforeAddr ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 getSmscAddr fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 setSmscAddr fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 setSmscAddr  finish');
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0700 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0800
   * @tc.name     Set “scAddr” to  special character ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0800', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '@！#￥%……&*';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 getSmscAddr beforeAddr  ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 getSmscAddr fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 setSmscAddr fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 setSmscAddr finish  ');
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0800 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setSmscAddr_Promise_0900
   * @tc.name     Set “scAddr” to  special Mixed character ,Failed to set short message service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setSmscAddr_Promise_0900', 0, async function (done) {
    var beforeAddr = '';
    let curAddr = '1Zz短信@！#￥%……&*+';
    try {
      beforeAddr = await sms.getSmscAddr(TRUE_SLOT_ID);
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 getSmscAddr beforeAddr ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 getSmscAddr fail');
      done();
      return;
    }
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 setSmscAddr fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 setSmscAddr finish ');
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === beforeAddr).assertTrue();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setSmscAddr_Promise_0900 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_getSmscAddr_Async_0100
   * @tc.name     When SLOTID is the correct value,Get the SMS center service address.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getSmscAddr_Async_0100', 0, async function (done) {
    let curAddr = USABLE_SMSC_NUMBER;
    sms.setSmscAddr(TRUE_SLOT_ID, curAddr, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_getSmscAddr_Async_0100 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_getSmscAddr_Async_0100 setSmscAddr finish  ');
      sms.getSmscAddr(TRUE_SLOT_ID, (geterr, getresult) => {
        if (geterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_getSmscAddr_Async_0100 fail');
          done();
          return;
        }
        expect(getresult === curAddr).assertTrue();
        console.log(`getSmscAddr result = ${getresult}`);
        console.log('Telephony_SmsMms_getSmscAddr_Async_0100 finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_getSmscAddr_Async_0200
   * @tc.name     When "SLOTID" is an error value,Failed to get SMS service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getSmscAddr_Async_0200', 0, async function (done) {
    sms.getSmscAddr(FALSE_SLOT_ID, (err, getresult) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_getSmscAddr_Async_0200 fail');
        done();
        return;
      }
      expect(getresult === '' || getresult === undefined);
      console.log('Telephony_SmsMms_getSmscAddr_Async_0200 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_getSmscAddr_Promise_0100
   * @tc.name     When SLOTID is the correct value,Get the SMS center service address.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getSmscAddr_Promise_0100', 0, async function (done) {
    let curAddr = USABLE_SMSC_NUMBER;
    try {
      await sms.setSmscAddr(TRUE_SLOT_ID, curAddr);
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0100 setSmscAddr beforeAddr ');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0100 setSmscAddr fail');
      done();
      return;
    }
    try {
      let promise = await sms.getSmscAddr(TRUE_SLOT_ID);
      expect(promise === curAddr).assertTrue();
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0100 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_getSmscAddr_Promise_0200
   * @tc.name     When "SLOTID" is an error value,Failed to get SMS service address
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getSmscAddr_Promise_0200', 0, async function (done) {
    try {
      let promise = await sms.getSmscAddr(FALSE_SLOT_ID);
      expect(promise === '' || promise === undefined);
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0200 finish');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_getSmscAddr_Promise_0200 fail');
      expect().assertFail();
      done();
    }
  });
});