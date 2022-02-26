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

import sim from '@ohos.telephony.sim';
import {simSlotId, OPERATOR_NUM_LENGTH, operatorCode, ICC_LENGTH, IMSI_LENGTH} from '../default/utils/Constant.test.js';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('SimFileManagerFunction', function () {

  /**
    * @tc.number  Telephony_Sim_getIMSI_Async_0100
    * @tc.name    Test getIMSI interface, input parameter 0, get return; value is not null.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getIMSI_Async_0100', 0, async function (done) {
    sim.getIMSI(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getIMSI_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getIMSI_Async_0100 data = ${data}`);
      expect(data !== '' || data !== undefined).assertTrue();
      expect(data.length === IMSI_LENGTH).assertTrue();
      console.log('Telephony_Sim_getIMSI_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getIMSI_Async_0200
    * @tc.name    To test the getIMSI interface, enter slotid as 4 and expect the result to enter err.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getIMSI_Async_0200', 0, async function (done) {
    sim.getIMSI(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Enter the exception ID to enter err.
        console.log('Telephony_Sim_getIMSI_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_Sim_getIMSI_Async_0200 fail');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getIMSI_Promise_0100
    * @tc.name    Test getIMSI interface, input parameter 0, get return; value is not null.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getIMSI_Promise_0100', 0, async function (done) {
    try {
      var data = await sim.getIMSI(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getIMSI_Promise_0100 data = ${data}`);
      expect(data !== '' || data !== undefined).assertTrue();
      expect(data.length === IMSI_LENGTH).assertTrue();
    } catch (err) {
      console.log(`Telephony_Sim_getIMSI_Promise_0100 fail, err : ${err.message}`);
      done();
      return;
    }
    console.log('Telephony_Sim_getIMSI_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getIMSI_Promise_0200
    * @tc.name    To test the getIMSI interface, enter slotid as 4 and expect the result to enter err.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getIMSI_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getIMSI(simSlotId.SLOT_ID_4);
    } catch (err) {
      console.log('Telephony_Sim_getIMSI_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log('Telephony_Sim_getIMSI_Promise_0200 fail');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimIccId_Async_0100
    * @tc.name    Test getSimIccId interface, input parameter 0, get return; value is not null.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimIccId_Async_0100', 0, async function (done) {
    sim.getSimIccId(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getSimIccId_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getSimIccId_Async_0100, data = ${data}`);
      expect(data !== '' || data !== undefined).assertTrue();
      expect(data.length === ICC_LENGTH).assertTrue();
      console.log('Telephony_Sim_getSimIccId_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimIccId_Async_0200
    * @tc.name    To test the getSimIccId interface, enter slotid as 4 and expect the result to enter err.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimIccId_Async_0200', 0, async function (done) {
    sim.getSimIccId(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Enter the exception ID to enter err.
        console.log('Telephony_Sim_getSimIccId_Async_0200 finish');
        done();
        return;
      }
      console.log('Telephony_Sim_getSimIccId_Async_0200 fail');
      expect().assertFail();
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimIccId_Promise_0100
    * @tc.name    Test getSimIccId interface, input parameter 0, get return; value is not null.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimIccId_Promise_0100', 0, async function (done) {
    try {
      var data = await sim.getSimIccId(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getSimIccId_Promise_0100 data${data}`);
      expect(data !== '' || data !== undefined).assertTrue();
      expect(data.length === ICC_LENGTH).assertTrue();
    } catch (err) {
      console.log(`Telephony_Sim_getSimIccId_Promise_0100 fail err${err.message}`);
      done();
      return;
    }
    console.log('Telephony_Sim_getSimIccId_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimIccId_Promise_0200
    * @tc.name    To test the getSimIccId interface, enter slotid as 4 and expect the result to enter err.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimIccId_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getSimIccId(simSlotId.SLOT_ID_4);
    } catch (err) {
      console.log('Telephony_Sim_getSimIccId_Promise_0200 finish');
      done();
      return;
    }
    console.log('Telephony_Sim_getSimIccId_Promise_0200 fail');
    expect().assertFail();
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getISOCountryCodeForSim_Async_0100
    * @tc.name    Enter normal parameters to test whether the getISOCountryCodeForSim interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getISOCountryCodeForSim_Async_0100', 0, async function (done) {
    var countryCode = 'cn';
    sim.getISOCountryCodeForSim(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getISOCountryCodeForSim_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getISOCountryCodeForSim_Async_0100, data = ${data}`);
      // The country code for mainland China is cn.
      expect(data === countryCode).assertTrue();
      console.log('Telephony_Sim_getISOCountryCodeForSim_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getISOCountryCodeForSim_Async_0200
    * @tc.name    Enter exception parameters to test whether the getISOCountryCodeForSim interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getISOCountryCodeForSim_Async_0200', 0, async function (done) {
    sim.getISOCountryCodeForSim(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Expect slot id 4 return err here.
        console.log('Telephony_Sim_getISOCountryCodeForSim_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log(`Telephony_Sim_getISOCountryCodeForSim_Async_0200 fail, data = ${data}`);
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getISOCountryCodeForSim_Promise_0100
    * @tc.name    Enter normal parameters to test whether the getISOCountryCodeForSim interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getISOCountryCodeForSim_Promise_0100', 0, async function (done) {
    var countryCode = 'cn';
    try {
      var data = await sim.getISOCountryCodeForSim(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getISOCountryCodeForSim_Promise_0100, data = ${data}`);
      // The country code for mainland China is cn.
      expect(data === countryCode).assertTrue();
    } catch (err) {
      console.log(`Telephony_Sim_getISOCountryCodeForSim_Promise_0100 fail, err: ${err.message}`);
      expect().assertFail();
      done();
      return;
    }
    console.log('Telephony_Sim_getISOCountryCodeForSim_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getISOCountryCodeForSim_Promise_0200
    * @tc.name    Enter exception parameters to test whether the getISOCountryCodeForSim interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getISOCountryCodeForSim_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getISOCountryCodeForSim(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Expect slotid 4 return err here.
      console.log('Telephony_Sim_getISOCountryCodeForSim_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log(`Telephony_Sim_getISOCountryCodeForSim_Promise_0200 fail, data = ${data}`);
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimOperatorNumeric_Async_0100
    * @tc.name    Enter normal parameters to test whether the getSimOperatorNumeric interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimOperatorNumeric_Async_0100', 0, async function (done) {
    var operatorNumTopThree = '460';
    sim.getSimOperatorNumeric(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getSimOperatorNumeric_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getSimOperatorNumeric_Async_0100 , data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      // The length of operator numeric in mainland China is 5.
      expect(data.length === OPERATOR_NUM_LENGTH).assertTrue();
      // The first three operator numeric in mainland China is '460'.
      expect(data.substring(0, 3) === operatorNumTopThree).assertTrue();
      console.log('Telephony_Sim_getSimOperatorNumeric_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimOperatorNumeric_Async_0200
    * @tc.name    Enter exception parameters to test whether the getSimOperatorNumeric interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimOperatorNumeric_Async_0200', 0, async function (done) {
    sim.getSimOperatorNumeric(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Expect slotid 4 return err here.
        console.log('Telephony_Sim_getSimOperatorNumeric_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log(`Telephony_Sim_getSimOperatorNumeric_Async_0200 fail, data = ${data}`);
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimOperatorNumeric_Promise_0100
    * @tc.name    Enter normal parameters to test whether the getSimOperatorNumeric interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimOperatorNumeric_Promise_0100', 0, async function (done) {
    var operatorNumTopThree = '460';
    try {
      var data = await sim.getSimOperatorNumeric(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getSimOperatorNumeric_Promise_0100, data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      // The length of operator numeric in mainland China is 5.
      expect(data.length === OPERATOR_NUM_LENGTH).assertTrue();
      // The first three operator numeric in mainland China is '460'.
      expect(data.substring(0, 3) === operatorNumTopThree).assertTrue();
    } catch (err) {
      console.log(`Telephony_Sim_getSimOperatorNumeric_Promise_0100 fail, err: ${err.message}`);
      expect().assertFail();
      done();
      return;
    }
    console.log('Telephony_Sim_getSimOperatorNumeric_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimOperatorNumeric_Promise_0200
    * @tc.name    Enter exception parameters to test whether the getSimOperatorNumeric interface function
    *             can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimOperatorNumeric_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getSimOperatorNumeric(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Expect slotid 4 return err here.
      console.log('Telephony_Sim_getSimOperatorNumeric_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log(`Telephony_Sim_getSimOperatorNumeric_Promise_0200 fail, data = ${data}`);
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimSpn_Async_0100
    * @tc.name    Enter normal parameters to test whether the getSimSpn interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimSpn_Async_0100', 0, async function (done) {
    var mobileSpn = '00434D4343FFFFFFFFFFFFFFFFFFFFFFFF';
    var unicomSpn = '00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF';
    var telecomSpn = '01804E2D56FD75354FE1FFFFFFFFFFFFFF';
    sim.getSimSpn(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getSimSpn_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getSimSpn_Async_0100 , data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      sim.getSimOperatorNumeric(simSlotId.SLOT_ID_0, (err, value) => {
        if (err) {
          console.log(`Telephony_Sim_getSimSpn_Async_0100 getSimOperatorNumeric fail, err: ${err.message}`);
          expect().assertFail();
          done();
          return;
        }
        console.log(`Telephony_Sim_getSimSpn_Async_0100 getSimOperatorNumeric , value : ${value}`);
        if ((value === operatorCode.MOBILE_CODE_1) || (value === operatorCode.MOBILE_CODE_2) ||
                (value === operatorCode.MOBILE_CODE_3) || (value === operatorCode.MOBILE_CODE_4)) {
          expect(data === mobileSpn).assertTrue();
        } else if ((value === operatorCode.UNICOM_CODE_1) || (value === operatorCode.UNICOM_CODE_2) ||
                (value === operatorCode.UNICOM_CODE_3)) {
          expect(data === unicomSpn).assertTrue();
        } else if ((value === operatorCode.TELECOM_CODE_1) || (value === operatorCode.TELECOM_CODE_2) ||
                (value === operatorCode.TELECOM_CODE_3)) {
          expect(data === telecomSpn).assertTrue();
        }
      });
      console.log('Telephony_Sim_getSimSpn_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimSpn_Async_0200
    * @tc.name    Enter exception parameters to test whether the getSimSpn interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimSpn_Async_0200', 0, async function (done) {
    sim.getSimSpn(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Expect slotid 4 return err here.
        console.log('Telephony_Sim_getSimSpn_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log(`Telephony_Sim_getSimSpn_Async_0200 fail, data = ${data}`);
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimSpn_Promise_0100
    * @tc.name    Enter normal parameters to test whether the getSimSpn interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimSpn_Promise_0100', 0, async function (done) {
    var mobileSpn = '00434D4343FFFFFFFFFFFFFFFFFFFFFFFF';
    var unicomSpn = '00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF';
    var telecomSpn = '01804E2D56FD75354FE1FFFFFFFFFFFFFF';
    try {
      var data = await sim.getSimSpn(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getSimSpn_Promise_0100, data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      try {
        var value = await sim.getSimOperatorNumeric(simSlotId.SLOT_ID_0);
        console.log(`Telephony_Sim_getSimSpn_Promise_0100, value : ${value}`);
        if ((value === operatorCode.MOBILE_CODE_1) || (value === operatorCode.MOBILE_CODE_2) ||
                (value === operatorCode.MOBILE_CODE_3) || (value === operatorCode.MOBILE_CODE_4)) {
          expect(data === mobileSpn).assertTrue();
        } else if ((value === operatorCode.UNICOM_CODE_1) || (value === operatorCode.UNICOM_CODE_2) ||
                (value === operatorCode.UNICOM_CODE_3)) {
          expect(data === unicomSpn).assertTrue();
        } else if ((value === operatorCode.TELECOM_CODE_1) || (value === operatorCode.TELECOM_CODE_2) ||
                (value === operatorCode.TELECOM_CODE_3)) {
          expect(data === telecomSpn).assertTrue();
        }
      } catch (err) {
        console.log(`Telephony_Sim_getSimSpn_Promise_0100 getSimOperatorNumeric fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
    } catch (err) {
      console.log(`Telephony_Sim_getSimSpn_Promise_0100 fail, err: ${err.message}`);
      expect().assertFail();
      done();
      return;
    }
    console.log('Telephony_Sim_getSimSpn_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimSpn_Promise_0200
    * @tc.name    Enter exception parameters to test whether the getSimSpn interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimSpn_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getSimSpn(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Expect slotid 4 return err here.
      console.log('Telephony_Sim_getSimSpn_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log(`Telephony_Sim_getSimSpn_Promise_0200 fail, data = ${data}`);
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimGid1_Async_0100
    * @tc.name    Enter normal parameters to test whether the getSimGid1 interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimGid1_Async_0100', 0, async function (done) {
    var minLength = 1;
    sim.getSimGid1(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getSimGid1_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      console.log(`Telephony_Sim_getSimGid1_Async_0100, data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      expect((data.length >= minLength)).assertTrue();
      console.log('Telephony_Sim_getSimGid1_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimGid1_Async_0200
    * @tc.name    Enter exception parameters to test whether the getSimGid1 interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimGid1_Async_0200', 0, async function (done) {
    sim.getSimGid1(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Expect slotid 4 return err here.
        console.log('Telephony_Sim_getSimGid1_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log(`Telephony_Sim_getSimGid1_Async_0200 fail, data = ${data}`);
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimGid1_Promise_0100
    * @tc.name    Enter normal parameters to test whether the getSimGid1 interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimGid1_Promise_0100', 0, async function (done) {
    var minLength = 1;
    try {
      var data = await sim.getSimGid1(simSlotId.SLOT_ID_0);
      console.log(`Telephony_Sim_getSimGid1_Promise_0100, data = ${data}`);
      expect(data !== '' && data !== undefined).assertTrue();
      expect((data.length >= minLength)).assertTrue();
    } catch (err) {
      console.log(`Telephony_Sim_getSimGid1_Promise_0100 fail, err: ${err.message}`);
      expect().assertFail();
      done();
      return;
    }
    console.log('Telephony_Sim_getSimGid1_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimGid1_Promise_0200
    * @tc.name    Enter exception parameters to test whether the getSimGid1 interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimGid1_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getSimGid1(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Expect slotid 4 return err here.
      console.log('Telephony_Sim_getSimGid1_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log(`Telephony_Sim_getSimGid1_Promise_0200 fail, data = ${data}`);
    done();
  });
});