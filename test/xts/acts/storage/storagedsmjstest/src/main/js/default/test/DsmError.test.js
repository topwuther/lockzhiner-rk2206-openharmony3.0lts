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

describe('dsmErrorTest', function () {

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0100
   * @tc.name DSM_JS_ERROR_0100
   * @tc.desc Verify that the mount volume ID is null successfully.
   */
  it('DSM_JS_ERROR_0100', 0, function () {
    devicesmgr.mount({
      volId: '',
      success: function () {
        console.log('DSM_JS_ERROR_0100 call mount success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0100 call mount pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0200
   * @tc.name DSM_JS_ERROR_0200
   * @tc.desc Verify that the mount does not exist and the volume ID is successful.
   */
  it('DSM_JS_ERROR_0200', 0, function () {
    devicesmgr.mount({
      volId: '123',
      success: function () {
        console.log('DSM_JS_ERROR_0200 call mount success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0200 call mount pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0300
   * @tc.name DSM_JS_ERROR_0300
   * @tc.desc Verify that the unmount does not exist and the volume ID is successful.
   */
  it('DSM_JS_ERROR_0300', 0, function () {
    devicesmgr.unMount({
      volId: '123',
      success: function () {
        console.log('DSM_JS_ERROR_0300 call unMount success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0300 call unMount pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0400
   * @tc.name DSM_JS_ERROR_0400
   * @tc.desc Verify that the unmount volume ID is null successfully.
   */
  it('DSM_JS_ERROR_0400', 0, function () {
    devicesmgr.unMount({
      volId: '',
      success: function () {
        console.log('DSM_JS_ERROR_0400 call unMount success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0400 call unMount pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0500
   * @tc.name DSM_JS_ERROR_0500
   * @tc.desc Verify that the format does not exist and whether the volume ID is successful.
   */
  it('DSM_JS_ERROR_0500', 0, function () {
    devicesmgr.format({
      volId: '123',
      success: function () {
        console.log('DSM_JS_ERROR_0500 call format success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0500 call format pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0600
   * @tc.name DSM_JS_ERROR_0600
   * @tc.desc Verify that the ID of the formatted volume is null successfully.
   */
  it('DSM_JS_ERROR_0600', 0, function () {
    devicesmgr.format({
      volId: '',
      success: function () {
        console.log('DSM_JS_ERROR_0600 call format success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0600 call format pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0700
   * @tc.name DSM_JS_ERROR_0700
   * @tc.desc Verify that the SD card is successfully copied and mounted..
   */
  it('DSM_JS_ERROR_0700', 0, function () {
    let mId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_JS_ERROR_0900 call getVolumes success. data:' + JSON.stringify(data));
        for (let i = 0; i < data.volumeInfos.length; i++) {
          if (data.volumeInfos[i].mState == 2 && data.volumeInfos[i].mId.indexOf('public:179') != -1) {
            mId = data.volumeInfos[i].mId;
            break;
          }
        }
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0700 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    devicesmgr.mount({
      volId: mId,
      success: function () {
        console.log('DSM_JS_ERROR_0700 call mount success');
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0700 call mount fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0800
   * @tc.name DSM_JS_ERROR_0800
   * @tc.desc Verify that volId is empty
   */
  it('DSM_JS_ERROR_0800', 0, function () {
    devicesmgr.findVolumeById({
      volId: '',
      success: function () {
        console.log('DSM_JS_ERROR_0800 call findVolumeById success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0800 call findVolumeById pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_0900
   * @tc.name DSM_JS_ERROR_0900
   * @tc.desc Invalid volId validation
   */
  it('DSM_JS_ERROR_0900', 0, function () {
    devicesmgr.findVolumeById({
      volId: '1234',
      success: function () {
        console.log('DSM_JS_ERROR_0900 call findVolumeById success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_0900 call findVolumeById pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1000
   * @tc.name DSM_JS_ERROR_1000
   * @tc.desc Verify that volumeUuid is null
   */
  it('DSM_JS_ERROR_1000', 0, function () {
    devicesmgr.findVolumeByUuid({
      volumeUuid: '',
      success: function () {
        console.log('DSM_JS_ERROR_1000 call findVolumeByUuid success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1000 call findVolumeByUuid pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1100
   * @tc.name DSM_JS_ERROR_1100
   * @tc.desc Invalid volumeUuid validation
   */
  it('DSM_JS_ERROR_1100', 0, function () {
    devicesmgr.findVolumeByUuid({
      volumeUuid: '321',
      success: function () {
        console.log('DSM_JS_ERROR_1100 call findVolumeByUuid success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1100 call findVolumeByUuid pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1200
   * @tc.name DSM_JS_ERROR_1200
   * @tc.desc Verify that diskId is empty
   */
  it('DSM_JS_ERROR_1200', 0, function () {
    devicesmgr.findDiskById({
      diskId: '',
      success: function () {
        console.log('DSM_JS_ERROR_1200 call findDiskById success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1200 call findDiskById pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1300
   * @tc.name DSM_JS_ERROR_1300
   * @tc.desc Invalid DiskID verified
   */
  it('DSM_JS_ERROR_1300', 0, function () {
    devicesmgr.findDiskById({
      diskId: '789',
      success: function () {
        console.log('DSM_JS_ERROR_1300 call findDiskById success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1300 call findDiskById pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1400
   * @tc.name DSM_JS_ERROR_1400
   * @tc.desc Verify that the value of the incoming simulate collection is null
   */
  it('DSM_JS_ERROR_1400', 0, function () {
    devicesmgr.findPrivateForEmulate({
      emuVol: {
        mId: '',
        mDiskId: '',
        mPartGuid: '',
        mFsUuid: '',
        mType: '',
        mMountFlags: '',
        mMountUserId: '',
        mState: '',
        mPath: '',
        mInternalPath: '',
        mFsLabel: '',
        Description: '',
        IsVisibleForUser: '',
        IsEmulated: '',
        IsPrimaryEmulatedForUser: '',
        IsRemovable: '',
        IsPrimary: ''
      },
      success: function () {
        console.log('DSM_JS_ERROR_1400 call findPrivateForEmulate success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1400 call findPrivateForEmulate pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1500
   * @tc.name DSM_JS_ERROR_1500
   * @tc.desc Verify that an invalid simulate set value was passed in
   */
  it('DSM_JS_ERROR_1500', 0, function () {
    devicesmgr.findPrivateForEmulate({
      emuVol: {
        mId: '1234',
        mDiskId: '321',
        mPartGuid: '1357',
        mFsUuid: '7890',
        mType: 2,
        mMountFlags: 3,
        mMountUserId: 0,
        mState: 6,
        mPath: '/storage/emulated',
        mInternalPath: '/data/media',
        mFsLabel: '123',
        Description: true,
        IsVisibleForUser: true,
        IsEmulated: true,
        IsPrimaryEmulatedForUser: true,
        IsRemovable: true,
        IsPrimary: true
      },
      success: function () {
        console.log('DSM_JS_ERROR_1500 call findPrivateForEmulate success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1500 call findPrivateForEmulate pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1600
   * @tc.name DSM_JS_ERROR_1600
   * @tc.desc Verify that the value of the incoming private collection is empty
   */
  it('DSM_JS_ERROR_1600', 0, function () {
    devicesmgr.findEmulateForPrivate({
      priVol: {
        mId: '',
        mDiskId: '',
        mPartGuid: '',
        mFsUuid: '',
        mType: '',
        mMountFlags: '',
        mMountUserId: '',
        mState: '',
        mPath: '',
        mInternalPath: '',
        mFsLabel: '',
        Description: '',
        IsVisibleForUser: '',
        IsEmulated: '',
        IsPrimaryEmulatedForUser: '',
        IsRemovable: '',
        IsPrimary: ''
      },
      success: function () {
        console.log('DSM_JS_ERROR_1600 call findEmulateForPrivate success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1600 call findEmulateForPrivate pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1700
   * @tc.name DSM_JS_ERROR_1700
   * @tc.desc Verify that an invalid private collection value was passed in
   */
  it('DSM_JS_ERROR_1700', 0, function () {
    devicesmgr.findEmulateForPrivate({
      priVol: {
        mId: '1234',
        mDiskId: '321',
        mPartGuid: '456',
        mFsUuid: '789',
        mType: 1,
        mMountFlags: 0,
        mMountUserId: 0,
        mState: 2,
        mPath: 'data',
        mInternalPath: '',
        mFsLabel: '',
        Description: true,
        IsVisibleForUser: true,
        IsEmulated: true,
        IsPrimaryEmulatedForUser: true,
        IsRemovable: true,
        IsPrimary: true
      },
      success: function () {
        console.log('DSM_JS_ERROR_1700 call findEmulateForPrivate success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1700 call findEmulateForPrivate pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1800
   * @tc.name DSM_JS_ERROR_1800
   * @tc.desc Verify that the vol set value is empty
   */
  it('DSM_JS_ERROR_1800', 0, function () {
    devicesmgr.getBestVolumeDescription({
      vol: {
        mId: '',
        mDiskId: '',
        mPartGuid: '',
        mFsUuid: '',
        mType: '',
        mMountFlags: '',
        mMountUserId: '',
        mState: '',
        mPath: '',
        mInternalPath: '',
        mFsLabel: '',
        Description: '',
        IsVisibleForUser: '',
        IsEmulated: '',
        IsPrimaryEmulatedForUser: '',
        IsRemovable: '',
        IsPrimary: ''
      },
      success: function () {
        console.log('DSM_JS_ERROR_1800 call getBestVolumeDescription success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1800 call getBestVolumeDescription pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_DSM_JS_ERROR_1900
   * @tc.name DSM_JS_ERROR_1900
   * @tc.desc Validate invalid vol set value
   */
  it('DSM_JS_ERROR_1900', 0, function () {
    devicesmgr.getBestVolumeDescription({
      vol: {
        mId: 'public:177,25',
        mDiskId: 'disk:178,24',
        mPartGuid: '',
        mFsUuid: 'd87ba2fe-828f-4237-8006-c6dc6c2910e7',
        mType: 0,
        mMountFlags: 2,
        mMountUserId: 0,
        mState: 2,
        mPath: '/storage/d87ba2fe-828f-4237-8006-c6dc6c2910e7',
        mInternalPath: '/data/ss/d87ba2fe-828f-4237-8006-c6dc6c2910e7',
        mFsLabel: '',
        Description: true,
        IsVisibleForUser: true,
        IsEmulated: true,
        IsPrimaryEmulatedForUser: true,
        IsRemovable: true,
        IsPrimary: true
      },
      success: function () {
        console.log('DSM_JS_ERROR_1900 call getBestVolumeDescription success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('DSM_JS_ERROR_1900 call getBestVolumeDescription pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    })
  })

})
