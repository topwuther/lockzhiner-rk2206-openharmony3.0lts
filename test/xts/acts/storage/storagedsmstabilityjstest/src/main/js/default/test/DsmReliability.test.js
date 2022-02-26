/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import devicesmgr from '@system.devicesmgr';
import {
  describe,
  beforeAll,
  beforeEach,
  afterEach,
  afterAll,
  it,
  expect
}
from 'deccjsunit/index'
describe('dsmReliabilityTest', function () {

  /**
   * @tc.number SUB_STORAGE_DSM_Reliability_0000
   * @tc.name DSM_Reliability_0000
   * @tc.desc Reliability test
   */
  it('DSM_Reliability_0000', 0, function () {
    for (let i = 0; i < 100000; i++) {
      let mId;
      let mFsUuid;
      let mDiskId;
      devicesmgr.getVolumes({
        success: function (data) {
          console.log('DSM_Reliability_0000  getVolumes success' + JSON.stringify(data) + i);
          mId = data.volumeInfos[2].mId;
          mFsUuid = data.volumeInfos[2].mFsUuid;
          mDiskId = data.volumeInfos[2].mDiskId;
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0000  getVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        }
      });
      devicesmgr.getDisks({
        success: function () {
          console.log('DSM_Reliability_0300  getDisks success.' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0300  getVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      devicesmgr.findVolumeById({
        volId: mId,
        success: function () {
          console.log('DSM_Reliability_0400  findVolumeById success' + i)
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0400  findVolumeById fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
      devicesmgr.findVolumeByUuid({
        volumeUuid: mFsUuid,
        success: function () {
          console.log('DSM_Reliability_0500  findVolumeByUuid success' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0500  findVolumeByUuid fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
      devicesmgr.getWritableVolumes({
        success: function () {
          console.log('DSM_Reliability_0600  getWritableVolumes success. data:' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0600  getWritableVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      devicesmgr.findDiskById({
        diskId: mDiskId,
        success: function () {
          console.log('DSM_Reliability_0700  findDiskById success' + i)
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0700  findDiskById fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
      devicesmgr.getPrimaryStorageUuid({
        success: function () {
          console.log('DSM_Reliability_0800  getPrimaryStorageUuid success.' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0800  getPrimaryStorageUuid fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      devicesmgr.unMount({
        volId: mId,
        success: function () {
          console.log('DSM_Reliability_0100  unMount success.' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0100 unMount fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        }
      })
      devicesmgr.mount({
        volId: mId,
        success: function () {
          console.log('DSM_Reliability_0200  mount success.' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0200  mount fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Reliability_0900
   * @tc.name DSM_Reliability_0900
   * @tc.desc Reliability test
   */
  it('DSM_Reliability_0900', 0, function () {
    let emuVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Reliability_0900  getVolumes success');
        emuVol = data.volumeInfos[0]
      },
      fail: function (data, code) {
        console.log('DSM_Reliability_0900  getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 100000; i++) {
      devicesmgr.findPrivateForEmulate({
        emuVol: emuVol,
        success: function () {
          console.log('DSM_Reliability_0900  findPrivateForEmulate success' + i)
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_0900  findPrivateForEmulate fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Reliability_1000
   * @tc.name DSM_Reliability_1000
   * @tc.desc Reliability test
   */
  it('DSM_Reliability_1000', 0, function () {
    let emuVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Reliability_1000  getVolumes success');
        emuVol = data.volumeInfos[1]
      },
      fail: function (data, code) {
        console.log('DSM_Reliability_1000  getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 100000; i++) {
      devicesmgr.findEmulateForPrivate({
        emuVol: emuVol,
        success: function () {
          console.log('DSM_Reliability_1000  findPrivateForEmulate success' + i)
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_1000  findPrivateForEmulate fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Reliability_1100
   * @tc.name DSM_Reliability_1100
   * @tc.desc Reliability test
   */
  it('DSM_Reliability_1100', 0, function () {
    let vol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Reliability_1100  getVolumes success');
        vol = data.volumeInfos[0]
      },
      fail: function (data, code) {
        console.log('DSM_Reliability_1100  getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 100000; i++) {
      devicesmgr.getBestVolumeDescription({
        vol: vol,
        success: function (desCription) {
          console.log('DSM_Reliability_1100 call getBestVolumeDescription success' + desCription)
        },
        fail: function (data, code) {
          console.log('DSM_Reliability_1100 call getBestVolumeDescription fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

})
