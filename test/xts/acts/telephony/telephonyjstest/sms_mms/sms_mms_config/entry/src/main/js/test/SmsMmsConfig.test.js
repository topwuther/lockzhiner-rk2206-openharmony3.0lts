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

describe('SmsMmsConfigTest', function () {
  const TRUE_SLOT_ID = 0;
  const FALSE_SLOT_ID = 9;
  const OTHER_SLOT_ID = 1;
  const SECOND_SLOT_ID = 2;
  const IDENTIFIER_MIN = 0;
  const IDENTIFIER_MAX = 0xFFFF;
  const RANTYPE_GSM = 1;
  const RANTYPE_CDMA = 2;
  const RANTYPE_ERROR = 3;

  var DEFAULT_SMS_SLOTID = 0;
  beforeAll(async function () {
    sms.getDefaultSmsSlotId((geterr, getresult) => {
      if (geterr) {
        return;
      }
      DEFAULT_SMS_SLOTID = getresult;
    });
  });
  beforeEach(async function () {
    sms.setDefaultSmsSlotId(DEFAULT_SMS_SLOTID, (err) => {});
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0100
   * @tc.name     Set "StartMessageld" to minimum(0x0000) and "EndMessageld" to minimum(0x0000) for cell broadcast,
   *              Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MIN,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0100 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0100 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0200
   * @tc.name     Set "StartMessageld" to minimum(0x0000) and "EndMessageld" to maxinum(0xFFFF) for cell broadcast,
   *              enable is true, Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0200 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0200 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0300
   * @tc.name     Set "StartMessageld" to minimum(0x0000) and "EndMessageld" to maxinum(0xFFFF) for cell broadcast,
   *              enable is false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0300 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0300 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0400
   * @tc.name     Set "StartMessageld" to minimum and "EndMessageld" to above maximum for cell broadcast,
   *              Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX + 1,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        console.log(`Telephony_SmsMms_setCBConfig_Async_0400  finish err : ${err.message}`);
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Async_0400 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0500
   * @tc.name     Set "StartMessageld" to below minimum(-1) and "EndMessageld" to  maximum(0xFFFF) for cell broadcast,
   *              Configuration failed
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN - 1,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        console.log(`Telephony_SmsMms_setCBConfig_Async_0500 finish err : ${err.message}`);
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Async_0500 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0600
   * @tc.name     When "SLOTID" is an error value,Failed to set up cell broadcast
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0600', 0, async function (done) {
    let data = {
      slotId: FALSE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        console.log(`Telephony_SmsMms_setCBConfig_Async_0600 finish err : ${err.message}`);
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Async_0600 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0700
   * @tc.name     Enable flag parameter "enable" to true, then set to true,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0700 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0700 finish');
    });
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0700 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0700 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0800
   * @tc.name     Enable flag parameter "enable" to false,
   *              then set to false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (initerr, initresult) => {
      if (initerr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0800 init fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0600 init finish');
      sms.setCBConfig(data, (err) => {
        if (err) {
          expect().assertFail();
          console.log('Telephony_SmsMms_setCBConfig_Async_0800 1 fail');
          done();
          return;
        }
        console.log('Telephony_SmsMms_setCBConfig_Async_0680 1 finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_0900
   * @tc.name     Enable flag parameter "enable" to true,
   *              then set to false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_0900', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    let secondData = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (err) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_0900 1 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_0900 1 finish');
      sms.setCBConfig(secondData, (seterr, setresult) => {
        if (seterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_setCBConfig_Async_0900 2 fail');
          done();
          return;
        }
        console.log('Telephony_SmsMms_setCBConfig_Async_0900 2 finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_1000
   * @tc.name     Enable flag parameter "enable" to false,
   *              then set to true,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_1000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    let secondData = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    sms.setCBConfig(data, (seterr, setresult) => {
      if (seterr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setCBConfig_Async_1000 1 fail');
        done();
        return;
      }
      console.log('Telephony_SmsMms_setCBConfig_Async_1000 1 finish ');
      sms.setCBConfig(secondData, (twoerr, tworesult) => {
        if (twoerr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_setCBConfig_Async_1000 2 fail');
          done();
          return;
        }
        console.log('Telephony_SmsMms_setCBConfig_Async_1000 2 finish');
        done();
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Async_1200
   * @tc.name     Set the network standard parameter "RANTYPE" to an error value,
   *              Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Async_1200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_ERROR
    };

    sms.setCBConfig(data, (err) => {
      if (err) {
        console.log(`Telephony_SmsMms_setCBConfig_Async_1200 finish err : ${err.message}`);
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Async_1200 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0100
   * @tc.name     Set "StartMessageld" to 0x0000 and "EndMessageld" to 0x0000 for cell broadcast,
   *              Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0100', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MIN,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0100 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0200
   * @tc.name     Set "StartMessageld" to 0x0000 and "EndMessageld" to 0xFFFF for cell broadcast,
   *              enable is true,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0200 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0200 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0300
   * @tc.name     Set "StartMessageld" to 0x0000 and "EndMessageld" to 0xFFFF for cell broadcast,
   *              enable is false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0300', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0300 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0300 fail');
      done();
    }
  });


  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0400
   * @tc.name     Set "StartMessageld" to minimum and "EndMessageld" to above maximum for cell broadcast,Setup failed
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0400', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX + 1,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0400 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_setCBConfig_Promise_0400 finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0500
   * @tc.name     Set "StartMessageld" to below minimum and "EndMessageld" to maximum for cell broadcast,Setup failed
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0500', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN - 1,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0500 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_setCBConfig_Promise_0500 finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0600
   * @tc.name     When "SLOTID" is an error value,Failed to set up cell broadcast
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0600', 0, async function (done) {
    let data = {
      slotId: FALSE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0600 fail');
      done();
    } catch (err) {
      console.log('Telephony_SmsMms_setCBConfig_Promise_0600 finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0700
   * @tc.name     Enable flag parameter "enable" to true,
   *              then set to true,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0700', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0700 1 finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0700 1 fail');
      done();
      return;
    }
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0700 2 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0700 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0800
   * @tc.name     Enable flag parameter "enable" to false,
   *              then set to false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0800', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0800 1 finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0800 1 fail');
      done();
      return;
    }
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0800 2 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0800 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_0900
   * @tc.name     Enable flag parameter "enable" to true,
   *              then set to false,Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_0900', 0, async function (done) {
    let dataInit = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(dataInit);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0900 init finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0900 init fail');
      done();
      return;
    }
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_0900  finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_0900  fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_1000
   * @tc.name     Enable flag parameter "enable" to false, then set to true,
   *              Configure a cell broadcast for an identity.
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_1000', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: false,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(data);
      console.log('Telephony_SmsMms_setCBConfig_Promise_1000 1 finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_1000 1 fail');
      done();
      return;
    }
    let secondData = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_GSM
    };
    try {
      await sms.setCBConfig(secondData);
      console.log('Telephony_SmsMms_setCBConfig_Promise_1000 2 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_1000 2 fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setCBConfig_Promise_1200
   * @tc.name     Set the network standard parameter "RANTYPE" to an error value,
   *              Failed to set up cell broadcast
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setCBConfig_Promise_1200', 0, async function (done) {
    let data = {
      slotId: TRUE_SLOT_ID,
      enable: true,
      startMessageId: IDENTIFIER_MIN,
      endMessageId: IDENTIFIER_MAX,
      ranType: RANTYPE_ERROR
    };
    try {
      await sms.setCBConfig(data);
      expect().assertFail();
      console.log('Telephony_SmsMms_setCBConfig_Promise_1200 fail');
      done();
    } catch (err) {
      console.log(`Telephony_SmsMms_setCBConfig_Promise_1200 finish err : ${err.message}`);
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setDefaultSmsSlotId_Async_0100
   * @tc.name     Set the default card slot ID for sending SMS
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100', 0, async function (done) {
    sms.getDefaultSmsSlotId((err, result) => {
      if (err) {
        expect().assertFail();
        console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 fail');
        done();
        return;
      }
      console.log(`Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 get result${result}`);
      let slotId = OTHER_SLOT_ID;
      if (slotId === result) {
        slotId = SECOND_SLOT_ID;
      }
      sms.setDefaultSmsSlotId(slotId, (seterr) => {
        if (seterr) {
          expect().assertFail();
          console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 set fail');
          done();
          return;
        }
        console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 set finish');
        sms.getDefaultSmsSlotId((geterr, getresult) => {
          if (geterr) {
            expect().assertFail();
            console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 fail');
            done();
            return;
          }
          expect(getresult === slotId).assertTrue();
          console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0100 finish');
          done();
        });
      });
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setDefaultSmsSlotId_Async_0200
   * @tc.name     The default card slot ID is set to an error value,Failed to set send SMS card slot ID
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setDefaultSmsSlotId_Async_0200', 0, async function (done) {
    let slotId = FALSE_SLOT_ID;
    sms.setDefaultSmsSlotId(slotId, (err) => {
      if (err) {
        console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Async_0200 fail');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100
   * @tc.name     Set the default card slot ID for sending SMS
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100', 0, async function (done) {
    let beforeSlotId = TRUE_SLOT_ID;
    try {
      beforeSlotId = await sms.getDefaultSmsSlotId();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100 get finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100 get fail');
      done();
      return;
    }
    let slotId = OTHER_SLOT_ID;
    if (slotId === beforeSlotId) {
      slotId = SECOND_SLOT_ID;
    }
    try {
      await sms.setDefaultSmsSlotId(slotId);
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100 set finish');
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100 set fail');
      done();
      return;
    }
    try {
      let promiseGet = await sms.getDefaultSmsSlotId();
      expect(promiseGet === slotId).assertTrue();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0100  fail');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_setDefaultSmsSlotId_Promise_0200
   * @tc.name     The default card slot ID is set to an error value,Failed to set send SMS card slot ID
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0200', 0, async function (done) {
    let slotId = FALSE_SLOT_ID;
    try {
      await sms.setDefaultSmsSlotId(slotId);
      expect().assertFail();
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0200  fail');
      done();
      return;
    } catch (err) {
      console.log('Telephony_SmsMms_setDefaultSmsSlotId_Promise_0200  finish');
      done();
    }
  });

  /**
   * @tc.number   Telephony_SmsMms_getDefaultSmsSlotId_Async_0100
   * @tc.name     Set the default card slot ID for sending SMS
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getDefaultSmsSlotId_Async_0100', 0, async function (done) {
    sms.getDefaultSmsSlotId((geterr, getresult) => {
      if (geterr) {
        expect().assertFail();
        console.log('Telephony_SmsMms_getDefaultSmsSlotId_Async_0100 fail');
        done();
        return;
      }
      expect(getresult === TRUE_SLOT_ID).assertTrue();
      console.log('Telephony_SmsMms_getDefaultSmsSlotId_Async_0100 finish');
      done();
    });
  });

  /**
   * @tc.number   Telephony_SmsMms_getDefaultSmsSlotId_Promise_0100
   * @tc.name     Set the default card slot ID for sending SMS
   * @tc.desc     Function test
   */
  it('Telephony_SmsMms_getDefaultSmsSlotId_Promise_0100', 0, async function (done) {
    try {
      let promise = await sms.getDefaultSmsSlotId();
      expect(promise === TRUE_SLOT_ID).assertTrue();
      console.log('Telephony_SmsMms_getDefaultSmsSlotId_Promise_0100 finish');
      done();
    } catch (err) {
      expect().assertFail();
      console.log('Telephony_SmsMms_getDefaultSmsSlotId_Promise_0100 fail');
      done();
    }
  });
});