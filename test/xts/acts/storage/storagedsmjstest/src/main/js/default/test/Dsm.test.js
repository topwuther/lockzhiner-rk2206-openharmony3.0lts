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

describe('dsmTest', function () {

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0000
   * @tc.name DSM_JS_Function_0000
   * @tc.desc Insert SD card to start up in shutdown state
   */
  it('DSM_JS_Function_0000', 0, function () {
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0000 call getVolumes success. data:' + JSON.stringify(data));
        expect(data.volumeInfos[2].mId !== null).assertTrue();
        expect(data.volumeInfos[2].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0000 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0100
   * @tc.name DSM_JS_Function_0100
   * @tc.desc Insert SD card in power on state
   */
  it('DSM_JS_Function_0100', 0, function () {
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0100 call getVolumes success. data:' + JSON.stringify(data));
        expect(data.volumeInfos[2].mId !== null).assertTrue();
        expect(data.volumeInfos[2].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0100 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0200
   * @tc.name DSM_JS_Function_0200
   * @tc.desc Unplug the SD card in the power on state
   */
  it('DSM_JS_Function_0200', 0, function () {
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0200 call getVolumes success. data:' + JSON.stringify(data));
        for (let i = 0; i < data.volumeInfos.length; i++) {
          expect(typeof(data.volumeInfos[i].mId) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mDiskId) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mPartGuid) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mFsUuid) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mType) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mMountFlags) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mMountUserId) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mState) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mPath) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mInternalPath) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mFsLabel) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].Description) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].IsVisibleForUser) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsEmulated) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsPrimaryEmulatedForUser) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsRemovable) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsPrimary) == 'boolean').assertTrue();
        }
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0200 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0300
   * @tc.name DSM_JS_Function_0300
   * @tc.desc Uninstall SD card.
   */
  it('DSM_JS_Function_0300', 0, function () {
    let mId = 0;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0300 call getVolumes success.' + JSON.stringify(data));
        mId = data.volumeInfos[2].mId;
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0300 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.unMount({
      volId: mId,
      success: function () {
        console.log('DSM_JS_Function_0300 call unMount success.');
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0300 call unMount fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      }
    })
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0300 call getVolumes success.' + JSON.stringify(data));
        for (let i = 0; i < data.getDisks.length; i++) {
          expect(data.volumeInfos[i].mId !== 'public:179,33').assertTrue();
          expect(data.volumeInfos[i].mState !== 2).assertTrue();
        }
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0300 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0400
   * @tc.name DSM_JS_Function_0400
   * @tc.desc Mount SD card.
   */
  it('DSM_JS_Function_0400', 0, function () {
    devicesmgr.mount({
      volId: 'public:179,33',
      success: function () {
        devicesmgr.getVolumes({
          success: function (data) {
            console.log('DSM_JS_Function_0400 call getVolumes success. data:' + JSON.stringify(data));
            expect(data.volumeInfos[2].mState == 2).assertTure()
            expect(data.volumeInfos[2].mInternalPath !== null).assertTure()
          },
          fail: function (data, code) {
            console.log('DSM_JS_Function_0400 call getVolumes fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        })
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0700
   * @tc.name DSM_JS_Function_0700
   * @tc.desc Obtain disk information and check whether it is successful.
   */
  it('DSM_JS_Function_0700', 0, function () {
    devicesmgr.getDisks({
      success: function (data) {
        console.log('DSM_JS_Function_0700 call getDisks success.' + JSON.stringify(data));
        expect(data.getDisks[2].Description == 'SD card').assertTrue()
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0700 call getDisks fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0800
   * @tc.name DSM_JS_Function_0800
   * @tc.desc Find the corresponding volume information through the volume ID
   */
  it('DSM_JS_Function_0800', 0, function () {
    let mId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0800 call getVolumes success' + JSON.stringify(data));
        mId = data.volumeInfos[2].mId;
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0800 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findVolumeById({
      volId: mId,
      success: function (vol) {
        console.log('DSM_JS_Function_0800 call findVolumeById success' + JSON.stringify(vol))
        expect(vol.volumeInfos[2].mId !== null).assertTrue();
        expect(vol.volumeInfos[2].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0800 call findVolumeById fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_0900
   * @tc.name DSM_JS_Function_0900
   * @tc.desc Find the corresponding volume information through the volume ID
   */
  it('DSM_JS_Function_0900', 0, function () {
    let mId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0900 call getVolumes success' + JSON.stringify(data));
        mId = data.volumeInfos[0].mId;
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0900 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findVolumeById({
      volId: mId,
      success: function (vol) {
        console.log('DSM_JS_Function_0900 call findVolumeById success' + JSON.stringify(vol))
        expect(vol.volumeInfos[0].mId == 'emulated;0').assertTrue();
        expect(vol.volumeInfos[0].mInternalPath !== null).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0900 call findVolumeById fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1000
   * @tc.name DSM_JS_Function_1000
   * @tc.desc Find the corresponding volume information through the volume ID
   */
  it('DSM_JS_Function_1000', 0, function () {
    let mId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_0900 call getVolumes success' + JSON.stringify(data));
        mId = data.volumeInfos[1].mId;
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_0900 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findVolumeById({
      volId: mId,
      success: function (vol) {
        console.log('DSM_JS_Function_1000 call findVolumeById success' + JSON.stringify(vol))
        expect(vol.volumeInfos[1].mId == 'private').assertTrue();
        expect(vol.volumeInfos[1].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1000 call findVolumeById fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1100
   * @tc.name DSM_JS_Function_1100
   * @tc.desc Obtain the corresponding volume information through the UUID of the volume
   */
  it('DSM_JS_Function_1100', 0, function () {
    let mFsUuid;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1100 call getVolumes success' + JSON.stringify(data));
        mFsUuid = data.volumeInfos[2].mFsUuid;
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1100 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findVolumeByUuid({
      volumeUuid: mFsUuid,
      success: function (vol) {
        console.log('DSM_JS_Function_1100 call findVolumeByUuid success' + JSON.stringify(vol))
        expect(vol.volumeInfos[2].mId !== null).assertTrue();
        expect(vol.volumeInfos[2].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1100 call findVolumeByUuid fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1200
   * @tc.name DSM_JS_Function_1200
   * @tc.desc Get information about writable volumes
   */
  it('DSM_JS_Function_1200', 0, function () {
    devicesmgr.getWritableVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1200 call getWritableVolumes success. data:' + JSON.stringify(data));
        for (let i = 0; i < data.volumeInfos.length; i++) {
          expect(typeof(data.volumeInfos[i].mId) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mDiskId) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mPartGuid) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mFsUuid) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mType) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mMountFlags) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mMountUserId) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mState) == 'number').assertTrue();
          expect(typeof(data.volumeInfos[i].mPath) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mInternalPath) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].mFsLabel) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].Description) == 'string').assertTrue();
          expect(typeof(data.volumeInfos[i].IsVisibleForUser) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsEmulated) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsPrimaryEmulatedForUser) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsRemovable) == 'boolean').assertTrue();
          expect(typeof(data.volumeInfos[i].IsPrimary) == 'boolean').assertTrue();
        }
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1200 call getWritableVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1300
   * @tc.name DSM_JS_Function_1300
   * @tc.desc Obtain the corresponding disk information through the disk ID
   */
  it('DSM_JS_Function_1300', 0, function () {
    let mDiskId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1300 call getVolumes success' + data);
        mDiskId = data.volumeInfos[2].mDiskId;
        console.log('DSM_JS_Function_1300---' + data.volumeInfos[2].mDiskId);
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1300 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findDiskById({
      diskId: mDiskId,
      success: function (disk) {
        console.log('DSM_JS_Function_1300 call findDiskById success' + JSON.stringify(disk))
        expect(disk.findDiskById[0].mId == mDiskId).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1300 call findDiskById fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1400
   * @tc.name DSM_JS_Function_1400
   * @tc.desc Get UUID of primary storage
   */
  it('DSM_JS_Function_1400', 0, function () {
    devicesmgr.getPrimaryStorageUuid({
      success: function (primaryUuid) {
        console.log('DSM_JS_Function_1400 call getPrimaryStorageUuid success.' + JSON.stringify(primaryUuid));
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1400 call getPrimaryStorageUuid fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1500
   * @tc.name DSM_JS_Function_1500
   * @tc.desc Find its corresponding private volume through simulate volume
   */
  it('DSM_JS_Function_1500', 0, function () {
    let emuVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1500 call getVolumes success' + data);
        emuVol = data.volumeInfos[0]
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1500 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findPrivateForEmulate({
      emuVol: emuVol,
      success: function (priVol) {
        console.log('DSM_JS_Function_1500 call findPrivateForEmulate success' + JSON.stringify(priVol))
        expect(priVol.volumeInfos[1].mId == 'private').assertTrue();
        expect(priVol.volumeInfos[1].mState == 2).assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1500 call findPrivateForEmulate fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1600
   * @tc.name DSM_JS_Function_1600
   * @tc.desc Find its corresponding emulated volume through private volume
   */
  it('DSM_JS_Function_1600', 0, function () {
    var priVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1600 call getVolumes success' + data);
        priVol = data.volumeInfos[1]
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1600 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.findEmulateForPrivate({
      priVol: priVol,
      success: function (emuVol) {
        console.log('DSM_JS_Function_1600 call findEmulateForPrivate success' + JSON.stringify(emuVol))
        expect(priVol.volumeInfos[0].mId == 'emulated;0').assertTrue();
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1600 call findEmulateForPrivate fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1700
   * @tc.name DSM_JS_Function_1700
   * @tc.desc Get the name of the volume (SD card)
   */
  it('DSM_JS_Function_1700', 0, function () {
    var vol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1700 call getVolumes success' + data);
        vol = data.volumeInfos[2]
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1700 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.getBestVolumeDescription({
      vol: vol,
      success: function (desCription) {
        console.log('DSM_JS_Function_1700 call getBestVolumeDescription success' +JSON.stringify(desCription))
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1700 call getBestVolumeDescription fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1800
   * @tc.name DSM_JS_Function_1800
   * @tc.desc Get the name of the volume (Internal storage public)
   */
  it('DSM_JS_Function_1800', 0, function () {
    var vol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1800 call getVolumes success' + data);
        vol = data.volumeInfos[0]
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1800 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.getBestVolumeDescription({
      vol: vol,
      success: function (desCription) {
        console.log('DSM_JS_Function_1800 call getBestVolumeDescription success' + JSON.stringify(desCription))
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1800 call getBestVolumeDescription fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_Function_1900
   * @tc.name DSM_JS_Function_1900
   * @tc.desc Get the name of the volume (Internal storage private)
   */
  it('DSM_JS_Function_1900', 0, function () {
    var vol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_Function_1900 call getVolumes success' + data);
        vol = data.volumeInfos[1]
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1900 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.getBestVolumeDescription({
      vol: vol,
      success: function (desCription) {
        console.log('DSM_JS_Function_1900 call getBestVolumeDescription success' + JSON.stringify(desCription))
      },
      fail: function (data, code) {
        console.log('DSM_JS_Function_1900 call getBestVolumeDescription fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

})
