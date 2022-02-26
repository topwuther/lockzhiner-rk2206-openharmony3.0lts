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

import fileio from '@system.fileio';
import fileShareAbility from '@ohos.fileshare';
import {
  describe,
  beforeAll,
  it,
  expect
}
from 'deccjsunit/index'
import {
  prepareFile,
  fmsFileName,
  fmsCacheName
}
from './Common'

describe('fileTest', function () {
  beforeAll(() => {
    prepareFile(fmsFileName('music.mp3'), 'content');
    prepareFile(fmsFileName('f0.txt'), 'content');
    prepareFile(fmsFileName('picture.jpg'), 'content');
    prepareFile(fmsFileName('picture2.jpg'), 'content');
    prepareFile(fmsFileName('文件.doc'), 'content');
    prepareFile(fmsFileName('cross Fire.apk'), 'content');
    prepareFile(fmsCacheName('video.mp4'), 'content');
    prepareFile(fmsFileName('file.xlsx'), 'content');
    prepareFile(fmsFileName('f2.ppt'), 'content');
    prepareFile(fmsFileName('f3.doc'), 'content');
    fileio.mkdirSync(fmsFileName('folder'));
    let text = '[{"type": "files-path", "name": "primary_leaf", "path": "/music.mp3"},' +
    '{"type": "files-path", "name": "primary_leaf2", "path": "/f0.txt"},' +
    '{"type": "files-path", "name": "primary_leaf3", "path": "/picture.jpg"},' +
    '{"type": "files-path", "name": "primary_leaf4", "path": "/picture2.jpg"},' +
    '{"type": "files-path", "name": "primary_leaf5", "path": "/文件.doc"},' +
    '{"type": "files-path", "name": "primary_leaf6", "path": "/cross Fire.apk"},' +
    '{"type": "cache-path", "name": "cacheprimary_leaf", "path": "/video.mp4"},' +
    '{"type": "files-path", "name": "primary_leaf7", "path": "/file.xlsx"},' +
    '{"type": "files-path", "name": "primary_leaf8", "path": "/f3.doc"},' +
    '{"type": "files-path", "name": "primary_leaf9", "path": "/folder"' +
    '}]';
    prepareFile(fmsFileName('test.json'), text);
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_100
   * @tc.name FMS_fuzzyFileToUri_100
   * @tc.desc Verify device ID,Call fuzzyfiletouri to
   * judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_100', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/f0.txt',
      displayName: 'f1.txt',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_100 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf2/f0.txt/leaf?displayName=f1.txt#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_100 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
      complete: function () {
        console.log('FMS_fuzzyFileToUri_100 call fuzzyFileToUri in complete');
      }
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_200
   * @tc.name FMS_fuzzyFileToUri_200
   * @tc.desc Verify app path,Call fuzzyfiletouri to
   * judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_200', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/picture.jpg',
      displayName: 'picture.png',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_200 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf3/picture.jpg/leaf?displayName=picture.png#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_200 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_300
   * @tc.name FMS_fuzzyFileToUri_300
   * @tc.desc Verify that the app path virtual path is an in package file,Call fuzzyfiletouri
   * to judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_300', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/../files/picture2.jpg',
      displayName: 'picture2.png',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_300 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf4/picture2.jpg/leaf?displayName=picture2.png#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_300 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_400
   * @tc.name FMS_fuzzyFileToUri_400
   * @tc.desc Verify the Chinese file name under the app path,Call fuzzyfiletouri
   * to judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_400', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/文件.doc',
      displayName: 'file',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_400 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf5/文件.doc/leaf?displayName=file#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_400 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_500
   * @tc.name FMS_fuzzyFileToUri_500
   * @tc.desc Verify the blurred Chinese file name,Call fuzzyfiletouri
   * to judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_500', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/cross Fire.apk',
      displayName: '你好，世界',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_500 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf6/cross Fire.apk/leaf?displayName=你好，世界#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_500 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_600
   * @tc.name FMS_fuzzyFileToUri_600
   * @tc.desc Verify cache path,Call fuzzyfiletouri
   * to judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_600', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://cache/video.mp4',
      displayName: 'v1.mp4',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_600 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/cache'
        +'/cacheprimary_leaf/video.mp4/leaf?displayName=v1.mp4#').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_600 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_700
   * @tc.name FMS_fuzzyFileToUri_700
   * @tc.desc Verify that displayName does not exist,Call fuzzyfiletouri
   * to judge whether the return value is correct.
   */
  it('FMS_fuzzyFileToUri_700', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/file.xlsx',
      success: function (uri) {
        console.log('FMS_fuzzyFileToUri_700 call fuzzyFileToUri success. uri:' + uri);
        expect(uri == 'dataability://440E-0906/ohos.acts.test.ability/primary/document/files'
        +'/primary_leaf7/file.xlsx/leaf').assertTrue();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_700 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
        expect(null).assertFail();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_800
   * @tc.name FMS_fuzzyFileToUri_800
   * @tc.desc Verify that authority = null,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_800', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: '',
      uri: 'internal://app/f2.ppt',
      success: function () {
        console.log('FMS_fuzzyFileToUri_800 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_800 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_900
   * @tc.name FMS_fuzzyFileToUri_900
   * @tc.desc Verify that authority is a different value,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_900', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'com.abc.test.ability',
      uri: 'internal://app/f3.doc',
      success: function () {
        console.log('FMS_fuzzyFileToUri_900 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_900 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1000
   * @tc.name FMS_fuzzyFileToUri_1000
   * @tc.desc Verify that uri = null,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_1000', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: '',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1000 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1000 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 302).assertTrue();
      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1100
   * @tc.name FMS_fuzzyFileToUri_1100
   * @tc.desc Verify that there is no file under the app path,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_1100', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/f4',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1100 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1100 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 301).assertTrue();

      },
    })
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1200
   * @tc.name FMS_fuzzyFileToUri_1200
   * @tc.desc Verify device ID=null,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_1200', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/music.mp3',
      displayName: 'helloworld',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1200 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1200 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 202).assertTrue();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1300
   * @tc.name FMS_fuzzyFileToUri_1300
   * @tc.desc Verify the virtual path to an out of package file,Call the interface fuzzyfiletouri
   * to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_1300', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/../../../applications/com.ohos.settings/com.ohos.settings/config.json',
      displayName: 'helloworld',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1300 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1300 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 302).assertTrue();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1400
   * @tc.name FMS_fuzzyFileToUri_1400
   * @tc.desc Verify that the folder exists under the app path,Call the interface
   * fuzzyfiletouri to check whether the interface call fails.
   */
  it('FMS_fuzzyFileToUri_1400', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/folder',
      displayName: 'helloworld',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1400 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1400 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_fuzzyFileToUri_1500
   * @tc.name FMS_fuzzyFileToUri_1500
   * @tc.desc The shared file is not configured in JSON
   */
  it('FMS_fuzzyFileToUri_1500', 0, function () {
    fileShareAbility.fuzzyFileToUri({
      deviceId: '440E-0906',
      authority: 'ohos.acts.test.ability',
      uri: 'internal://app/f2.ppt',
      success: function () {
        console.log('FMS_fuzzyFileToUri_1500 call fuzzyFileToUri success, fail!');
        expect(null).assertFail();
      },
      fail: function (data, code) {
        console.log('FMS_fuzzyFileToUri_1500 call fuzzyFileToUri pass, code: ' + code + ', data: ' + data);
        expect(code == 300).assertTrue();
      },
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Performance_0000
   * @tc.name FMS_Performance_0000
   * @tc.desc Performance test
   */
  it('FMS_Performance_0000', 0, function () {
    let start = new Date().getTime();
    for(let i = 0; i < 1000;i++){
      fileShareAbility.fuzzyFileToUri({
        deviceId: '440E-0906',
        authority: 'ohos.acts.test.ability',
        uri: 'internal://app/f0.txt',
        displayName: 'f1.txt',
        success: function () {
          let end = new Date().getTime();
          let time = end - start ;
          console.log('FMS_Performance_0000 call fuzzyFileToUri success. time:' +time+','+i);
        },
        fail: function (data, code) {
          console.log('FMS_Performance_0000 call fuzzyFileToUri fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
        complete: function () {
          console.log('FMS_Performance_0000 call fuzzyFileToUri in complete');
        }
      });
    }
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Performance_0000
   * @tc.name FMS_Performance_0000
   * @tc.desc stability test
   */
  it('FMS_stability_0000', 0, function () {
    for(let i = 0; i < 100000;i++){
      fileShareAbility.fuzzyFileToUri({
        deviceId: '440E-0906',
        authority: 'ohos.acts.test.ability',
        uri: 'internal://app/f0.txt',
        displayName: 'f1.txt',
        success: function () {
          console.log('FMS_stability_0000  success. '+i);
        },
        fail: function (data, code) {
          console.log('FMS_stability_0000 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
        complete: function () {
          console.log('FMS_stability_0000 call fuzzyFileToUri in complete');
        }
      });
    }
  })

})
