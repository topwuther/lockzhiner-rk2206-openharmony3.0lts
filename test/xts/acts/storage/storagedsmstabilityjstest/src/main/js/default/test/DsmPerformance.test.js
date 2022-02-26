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

describe('dsmPerformanceTest', function () {

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0000
   * @tc.name DSM_Performance_0000
   * @tc.desc getVolumes Performance test
   */
  it('DSM_Performance_0000', 0, function () {
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.getVolumes({
        success: function (data) {
          var end = new Date().getTime();
          console.log('DSM_Performance_0000 call getVolumes success' + JSON.stringify(data));
          console.log('---DSM_Performance_0000---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0000 call getVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        }
      });
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0100
   * @tc.name DSM_Performance_0100
   * @tc.desc unMount mount Performance test
   */
  it('DSM_Performance_0100', 0, function () {
    let mId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_0100 call getVolumes success' + data);
        mId = data.volumeInfos[2].mId;
      },
      fail: function (data, code) {
        console.log('DSM_Performance_0100 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      if (mId != null) {
        devicesmgr.unMount({
          volId: mId,
          success: function () {
            console.log('DSM_Performance_0100 call unMount success.');
            var end = new Date().getTime();
            console.log('---DSM_Performance_0100---time:-' + (end - start) + '-----' + i);
          },
          fail: function (data, code) {
            console.log('DSM_Performance_0100 call unMount fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          }
        })
        var start1 = new Date().getTime();
        devicesmgr.mount({
          volId: mId,
          success: function () {
            var end1 = new Date().getTime();
            console.log('---DSM_Performance_0200---time:-' + (end1 - start1) + '-----' + i);
          },
          fail: function (data, code) {
            console.log('DSM_Performance_0200 call mount fail, code: ' + code + ', data: ' + data);
            expect(null).assertFail();
          },
        })
      } else {
        expect(null).assertFail();
      }
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0300
   * @tc.name DSM_Performance_0300
   * @tc.desc getDisks Performance test
   */
  it('DSM_Performance_0300', 0, function () {
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.getDisks({
        success: function (data) {
          console.log('DSM_Performance_0300 call getDisks success.' + JSON.stringify(data));
          var end = new Date().getTime();
          console.log('---DSM_Performance_0300---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0300 call getDisks fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0400
   * @tc.name DSM_Performance_0400
   * @tc.desc findVolumeById Performance test
   */
  it('DSM_Performance_0400', 0, function () {
    for (let i = 0; i < 1000; i++) {
      let mId;
      devicesmgr.getVolumes({
        success: function (data) {
          console.log('DSM_Performance_0400 call getVolumes success' + JSON.stringify(data));
          mId = data.volumeInfos[2].mId;
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0400 call getVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
      var start = new Date().getTime();
      devicesmgr.findVolumeById({
        volId: mId,
        success: function (data) {
          console.log('DSM_Performance_0400 call findVolumeById success' + JSON.stringify(data))
          var end = new Date().getTime();
          console.log('---DSM_Performance_0400---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0400 call findVolumeById fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0500
   * @tc.name DSM_Performance_0500
   * @tc.desc findVolumeByUuid Performance test
   */
  it('DSM_Performance_0500', 0, function () {
    let mFsUuid;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_0500 call getVolumes success' + JSON.stringify(data));
        mFsUuid = data.volumeInfos[2].mFsUuid;
      },
      fail: function (data, code) {
        console.log('DSM_Performance_0500 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.findVolumeByUuid({
        volumeUuid: mFsUuid,
        success: function (vol) {
          console.log('DSM_Performance_0500 call findVolumeByUuid success' + JSON.stringify(vol));
          var end = new Date().getTime();
          console.log('---DSM_Performance_0500---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0500 call findVolumeByUuid fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0600
   * @tc.name DSM_Performance_0600
   * @tc.desc getWritableVolumes Performance test
   */
  it('DSM_Performance_0600', 0, function () {
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.getWritableVolumes({
        success: function (data) {
          console.log('DSM_Performance_0600 call getWritableVolumes success. data:' + JSON.stringify(data));
          var end = new Date().getTime();
          console.log('---DSM_Performance_0600---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0600 call getWritableVolumes fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0700
   * @tc.name DSM_Performance_0700
   * @tc.desc findDiskById Performance test
   */
  it('DSM_Performance_0700', 0, function () {
    let mDiskId;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_0700 call getVolumes success' + JSON.stringify(data));
        mDiskId = data.volumeInfos[2].mDiskId;
      },
      fail: function (data, code) {
        console.log('DSM_Performance_0700 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.findDiskById({
        diskId: mDiskId,
        success: function (disk) {
          console.log('DSM_Performance_0700 call findDiskById success' + JSON.stringify(disk))
          var end = new Date().getTime();
          console.log('---DSM_Performance_0700---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0700 call findDiskById fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0800
   * @tc.name DSM_Performance_0800
   * @tc.desc getPrimaryStorageUuid Performance test
   */
  it('DSM_Performance_0800', 0, function () {
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.getPrimaryStorageUuid({
        success: function (primaryUuid) {
          console.log('DSM_Performance_0800 call getPrimaryStorageUuid success.' + JSON.stringify(primaryUuid));
          var end = new Date().getTime();
          console.log('---DSM_Performance_0800---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0800 call getPrimaryStorageUuid fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_0900
   * @tc.name DSM_Performance_0900
   * @tc.desc findPrivateForEmulate Performance test
   */
  it('DSM_Performance_0900', 0, function () {
    let emuVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_0900 call getVolumes success' + JSON.stringify(data));
        emuVol = data.volumeInfos[0]
      },
      fail: function (data, code) {
        console.log('DSM_Performance_0900 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.findPrivateForEmulate({
        emuVol: emuVol,
        success: function (priVol) {
          console.log('DSM_Performance_0900 call findPrivateForEmulate success' + JSON.stringify(priVol))
          var end = new Date().getTime();
          console.log('---DSM_Performance_0900---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_0900 call findPrivateForEmulate fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_1000
   * @tc.name DSM_Performance_1000
   * @tc.desc findPrivateForEmulate Performance test
   */
  it('DSM_Performance_1000', 0, function () {
    let emuVol ;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_1000 call getVolumes success' + JSON.stringify(data));
        emuVol = data.volumeInfos[1]
      },
      fail: function (data, code) {
        console.log('DSM_Performance_1000 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.findEmulateForPrivate({
        priVol: emuVol,
        success: function (emuVol) {
          console.log('DSM_Performance_1000 call findEmulateForPrivate success' + JSON.stringify(emuVol))
          var end = new Date().getTime();
          console.log('---DSM_Performance_1000---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_Performance_1000 call findEmulateForPrivate fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

  /**
   * @tc.number SUB_STORAGE_DSM_Performance_1100
   * @tc.name DSM_Performance_1100
   * @tc.desc findPrivateForEmulate Performance test
   */
  it('DSM_Performance_1100', 0, function () {
    let emuVol;
    devicesmgr.getVolumes({
      success: function (data) {
        console.log('DSM_Performance_1100 call getVolumes success' + JSON.stringify(data));
        emuVol = data.volumeInfos[2]
      },
      fail: function (data, code) {
        console.log('DSM_Performance_1100 call getVolumes fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    });
    for (let i = 0; i < 1000; i++) {
      var start = new Date().getTime();
      devicesmgr.getBestVolumeDescription({
        vol: emuVol,
        success: function (desCription) {
          console.log('DSM_JS_Function_1100 call getBestVolumeDescription success' + desCription)
          var end = new Date().getTime();
          console.log('---DSM_Performance_1100---time:-' + (end - start) + '-----' + i);
        },
        fail: function (data, code) {
          console.log('DSM_JS_Function_1100 call getBestVolumeDescription fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    }
  })

})
