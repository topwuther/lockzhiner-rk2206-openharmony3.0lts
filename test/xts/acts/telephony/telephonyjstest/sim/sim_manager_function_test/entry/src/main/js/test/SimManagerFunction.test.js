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
import {simSlotId} from '../default/utils/Constant.test.js';
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

describe('SimManagerFunction', function () {

  /**
    * @tc.number  Telephony_Sim_getSimAccountInfo_Async_0100
    * @tc.name    Enter normal parameters to test whether the getSimAccountInfo
    *             interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimAccountInfo_Async_0100', 0, async function (done) {
    var defaultName = 'simDefaultDisplayName';
    var defaultNumber = 'simDefaultDisplayNumber';
    sim.getSimAccountInfo(simSlotId.SLOT_ID_0, (err, data) => {
      if (err) {
        console.log(`Telephony_Sim_getSimAccountInfo_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      expect(data.slotIndex === simSlotId.SLOT_ID_0).assertTrue();
      // return the default piling data as simDefaultDisplayName.
      expect(data.showName === defaultName).assertTrue();
      // return the default piling data as simDefaultDisplayNumber.
      expect(data.showNumber === defaultNumber).assertTrue();
      console.log('Telephony_Sim_getSimAccountInfo_Async_0100 finish');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimAccountInfo_Async_0200
    * @tc.name    Enter exception parameters to test whether the getSimAccountInfo
    *             interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimAccountInfo_Async_0200', 0, async function (done) {
    sim.getSimAccountInfo(simSlotId.SLOT_ID_4, (err, data) => {
      if (err) {
        // Enter the exception ID to enter err.
        console.log('Telephony_Sim_getSimAccountInfo_Async_0200 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_Sim_getSimAccountInfo_Async_0200 fail');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_getSimAccountInfo_Promise_0100
    * @tc.name    Enter normal parameters to test whether the getSimAccountInfo
    *             interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimAccountInfo_Promise_0100', 0, async function (done) {
    var defaultName = 'simDefaultDisplayName';
    var defaultNumber = 'simDefaultDisplayNumber';
    try {
      var data = await sim.getSimAccountInfo(simSlotId.SLOT_ID_0);
      expect(data.slotIndex === simSlotId.SLOT_ID_0).assertTrue();
      // return the default piling data as simDefaultDisplayName.
      expect(data.showName === defaultName).assertTrue();
      // return the default piling data as simDefaultDisplayNumber.
      expect(data.showNumber === defaultNumber).assertTrue();
      console.log('Telephony_Sim_getSimAccountInfo_Async_0100 finish');
    } catch (err) {
      console.log(`Telephony_Sim_getSimAccountInfo_Promise_0100 fail, err: ${err.message}`);
      expect().assertFail();
      done();
      return;
    }
    console.log('Telephony_Sim_getSimAccountInfo_Promise_0100 finish');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getSimAccountInfo_Promise_0200
    * @tc.name    Enter exception parameters to test whether the getSimAccountInfo
    *             interface function can execute normally.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getSimAccountInfo_Promise_0200', 0, async function (done) {
    try {
      var data = await sim.getSimAccountInfo(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Enter the exception ID to enter err.
      console.log('Telephony_Sim_getSimAccountInfo_Promise_0200 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log('Telephony_Sim_getSimAccountInfo_Promise_0200 fail');
    done();
  });

  /**
    * @tc.number  Telephony_Sim_getDefaultVoiceSlotId_Async_0100
    * @tc.name    The test first sets the default card number and then calls
    *             the getDefaultVoiceLotid interface to get the SIM ID.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getDefaultVoiceSlotId_Async_0100', 0, async function (done) {
    var defaultValue = 0;
    sim.getDefaultVoiceSlotId((err, result) => {
      if (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      defaultValue = result;
    });
    sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_0, (err) => {
      if (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0100 setDefaultVoiceSLOT_ID_err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      sim.getDefaultVoiceSlotId((err, data) => {
        if (err) {
          console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0100 fail, err: ${err.message}`);
          expect().assertFail();
          done();
          return;
        }
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0100, data = ${data}`);
        expect(data === simSlotId.SLOT_ID_0).assertTrue();
        // Restore Settings
        sim.setDefaultVoiceSlotId(defaultValue, (err) => {
          if (err) {
            console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0100 setDefaultVoiceSLOT_ID_err: ${
              err.message}`);
            expect().assertFail();
            done();
            return;
          }
          console.log('Telephony_Sim_getDefaultVoiceSlotId_Async_0100 finish');
          done();
        });
      });
    });
  });

  /**
    * @tc.number  Telephony_Sim_getDefaultVoiceSlotId_Async_0200
    * @tc.name    The test first sets the default card number and then calls
    *             the getDefaultVoiceLotid interface to get the SIM ID.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getDefaultVoiceSlotId_Async_0200', 0, async function (done) {
    var defaultValue = 0;
    sim.getDefaultVoiceSlotId((err, result) => {
      if (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0200 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      defaultValue = result;
    });
    sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_2, (err) => {
      if (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0200：setDefaultVoiceSLOT_ID_err: ${
          err.message}`);
        expect().assertFail();
        done();
        return;
      }
      sim.getDefaultVoiceSlotId((err, data) => {
        if (err) {
          console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0200 fail, err: ${err.message}`);
          expect().assertFail();
          done();
          return;
        }
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0200, data = ${data}`);
        expect(data === simSlotId.SLOT_ID_2).assertTrue();
        // Restore Settings
        sim.setDefaultVoiceSlotId(defaultValue, (err) => {
          if (err) {
            console.log(`Telephony_Sim_getDefaultVoiceSlotId_Async_0200 setDefaultVoiceSLOT_ID_err: ${
              err.message}`);
            expect().assertFail();
            done();
            return;
          }
          console.log('Telephony_Sim_getDefaultVoiceSlotId_Async_0200 finish');
          done();
        });
      });
    });
  });

  /**
    * @tc.number  Telephony_Sim_getDefaultVoiceSlotId_Promise_0100
    * @tc.name    The test first sets the default card number and then calls
    *             the getDefaultVoiceLotid interface to get the default SIM ID.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getDefaultVoiceSlotId_Promise_0100', 0, async function (done) {
    var defaultValue = 0;
    var result = await sim.getDefaultVoiceSlotId();
    defaultValue = result;
    try {
      await sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_2);
      try {
        var data = await sim.getDefaultVoiceSlotId();
        expect(data === simSlotId.SLOT_ID_2).assertTrue();
      } catch (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Promise_0100 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
      // Restore Settings
      await sim.setDefaultVoiceSlotId(defaultValue);
      console.log('Telephony_Sim_getDefaultVoiceSlotId_Promise_0100 finish');
      done();
    } catch (err) {
      console.log(`Telephony_Sim_getDefaultVoiceSlotId_Promise_0100:setDefaultVoiceSLOT_ID_err: ${err.message}`);
      expect().assertFail();
      done();
    }
  });

  /**
    * @tc.number  Telephony_Sim_getDefaultVoiceSlotId_Promise_0200
    * @tc.name    The test first sets the default card number and then calls
    *             the getDefaultVoiceLotid interface to get the default SIM ID.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_getDefaultVoiceSlotId_Promise_0200', 0, async function (done) {
    var defaultValue = 0;
    var result = await sim.getDefaultVoiceSlotId();
    defaultValue = result;
    try {
      await sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_0);
      try {
        var data = await sim.getDefaultVoiceSlotId();
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Promise_0200, data = ${data}`);
        expect(data === simSlotId.SLOT_ID_0).assertTrue();
        // Restore Settings
        await sim.setDefaultVoiceSlotId(defaultValue);
        console.log('Telephony_Sim_getDefaultVoiceSlotId_Promise_0200 finish');
        done();
      } catch (err) {
        console.log(`Telephony_Sim_getDefaultVoiceSlotId_Promise_0200 fail, err: ${err.message}`);
        expect().assertFail();
        done();
        return;
      }
    } catch (err) {
      console.log(`Telephony_Sim_getDefaultVoiceSlotId_Promise_0200:setDefaultVoiceSLOT_ID_err: ${err.message}`);
      expect().assertFail();
      done();
    }
  });

  /**
    * @tc.number  Telephony_Sim_setDefaultVoiceSlotId_Async_0100
    * @tc.name    Test the setDefaultVoiceLotid interface when the input SIM ID parameter is 4.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_setDefaultVoiceSlotId_Async_0100', 0, async function (done) {
    sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_4, (err) => {
      if (err) {
        // Expect slotid 4 return err here.
        sim.getDefaultVoiceSlotId((err, result) => {
          if (err) {
            console.log(`Telephony_Sim_setDefaultVoiceSlotId_Async_0100 getDefaultVoiceSLOT_ID_fail, err: ${
              err.message}`);
            expect().assertFail();
            done();
            return;
          }
          expect(result !== simSlotId.SLOT_ID_4).assertTrue();
        });
        console.log('Telephony_Sim_setDefaultVoiceSlotId_Async_0100 finish');
        done();
        return;
      }
      expect().assertFail();
      console.log('Telephony_Sim_setDefaultVoiceSlotId_Async_0100 fail');
      done();
    });
  });

  /**
    * @tc.number  Telephony_Sim_setDefaultVoiceSlotId_Promise_0100
    * @tc.name    Test the setDefaultVoiceLotid interface when the input SIM ID parameter is 4.
    * @tc.desc    Function test
    */
  it('Telephony_Sim_setDefaultVoiceSlotId_Promise_0100', 0, async function (done) {
    try {
      var data = await sim.setDefaultVoiceSlotId(simSlotId.SLOT_ID_4);
    } catch (err) {
      // Expect slotid 4 return err here.
      var result = await sim.getDefaultVoiceSlotId();
      expect(result !== simSlotId.SLOT_ID_4).assertTrue();
      console.log('Telephony_Sim_setDefaultVoiceSlotId_Promise_0100 finish');
      done();
      return;
    }
    expect().assertFail();
    console.log('Telephony_Sim_setDefaultVoiceSlotId_Promise_0100 fail');
    done();
  });
});