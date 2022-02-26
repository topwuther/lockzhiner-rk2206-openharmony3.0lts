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

import fileio from '@ohos.fileio';
import filepicker from '@ohos.filepicker'
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
import {
  FILE_CONTENT,
  prepareFile,
  fileName,
  filePickerName,
  randomString,
  cacheFileName,
  prepareEmptyFile,
  nextFileName
}
from './Common'
describe('FmsTest', function () {
  let deviceID = 0;

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0000
   * @tc.name FMS_Function_0000
   * @tc.desc Function of API,Get current device information
   */
  it('FMS_Function_0000', 0,  function (done) {
    let progetDeviceInfo = new Promise(function (resolve, reject) {
      filepicker.getDeviceInfo({
        success: function (data) {
          console.log('FMS_Function_0000 success.' + JSON.stringify(data));
          if (data.deviceList.length == 0) {
            console.log('FMS_Function_0000 fail');
            expect(null).assertFail();
          } else {
            deviceID = data.deviceList[0].deviceID;
            console.log("---FMS_Function_0000------" + deviceID);
            done();
          }
        },
        fail: function (data, code) {
          console.log('FMS_Function_0000 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    })
    progetDeviceInfo.then(function () {
      console.log('FMS_Function_0000 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0100
   * @tc.name FMS_Function_0100
   * @tc.desc Function of API, Creates the specified directory recursively
   */
  it('FMS_Function_0100', 0,  function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/leaf',
        success: function () {
          console.log('FMS_Function_0100 mkdir success.');
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0100 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    })
    proMkdir.catch(function () {
      console.log('FMS_Function_0100 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0200
   * @tc.name FMS_Function_0200
   * @tc.desc Function of API, Creates the specified directory recursively
   */
  it('FMS_Function_0200', 0,  function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/你好/leaf',
        success: function () {
          console.log('FMS_Function_0200 mkdir success.');
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0200 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0200 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0300
   * @tc.name FMS_Function_0300
   * @tc.desc Function of API, Creates the specified directory recursively
   */
  it('FMS_Function_0300', 0,  function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/a/b/c/leaf',
        success: function () {
          console.log('FMS_Function_0300 mkdir success.');
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0300 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0300 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0400
   * @tc.name FMS_Function_0400
   * @tc.desc Function of API,
   */
  it('FMS_Function_0400', 0,  function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/root/files',
        success: function () {
          console.log('FMS_Function_0400 success.');
          done()
        },
        fail: function (data, code) {
          console.log('call fail callback fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      })
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0400 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0500
   * @tc.name FMS_Function_0500
   * @tc.desc Function of API, save file.The test file is exist.
   */
  it('FMS_Function_0500', 0,  function (done) {
    let fpath = filePickerName("a.txt", deviceID);
    expect(prepareFile(fpath, FILE_CONTENT) !== null).assertTrue();
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/' + deviceID + '/document/fms_test/a.txt/leaf'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/root',
        success: function (data) {
          console.log('FMS_Function_0500 success.' + data.saveFileList[0].status);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0500 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0500 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0600
   * @tc.name FMS_Function_0600
   * @tc.desc Function of API, save file.The test file is exist.
   */
  it('FMS_Function_0600', 0, async function (done) {
    let fpath = filePickerName("b.txt", deviceID);
    expect(prepareFile(fpath, FILE_CONTENT) !== null).assertTrue();
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/' + deviceID + '/document/fms_test/a.txt/leaf',
        'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/b.txt/leaf'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/root',
        success: function (data) {
          console.log('FMS_Function_0600 success.' + data.saveFileList[0].status);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0600 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0600 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0700
   * @tc.name FMS_Function_0700
   * @tc.desc Function of API, save file.The test file is exist.
   */
  it('FMS_Function_0700', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/' + deviceID + '/document/fms_test/a.txt/leaf'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/root/files',
        success: function (data) {
          console.log('FMS_Function_0700 success.' + data.saveFileList[0].status);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0700 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0700 success');
    });

  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0800
   * @tc.name FMS_Function_0800
   * @tc.desc Function of API, save file.The test file is exist.
   */
  it('FMS_Function_0800', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/' + deviceID + '/document/fms_test/a.txt/leaf',
        'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/b.txt/leaf'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/root/files',
        success: function (data) {
          console.log('FMS_Function_0800 success.' + data.saveFileList[0].status);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0800 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0800 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_0900
   * @tc.name FMS_Function_0900
   * @tc.desc Function of API, search file.The test file is exist.
   */
  it('FMS_Function_0900', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.searchFile({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/leaf',
        name: 'a.txt',
        success: function (data) {
          console.log('FMS_Function_0900 success.' + data.fileList[0].uri);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_0900 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_0900 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_1000
   * @tc.name FMS_Function_1000
   * @tc.desc Function of API, search file.The test file is exist.
   */
  it('FMS_Function_1000', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.searchFile({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/leaf',
        name: 'b.txt',
        success: function (data) {
          console.log('FMS_Function_1000 success.' + data.fileList[0].uri);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_1000 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_1000 success');
    });

  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_1100
   * @tc.name FMS_Function_1100
   * @tc.desc Function of API, search file.The test file is exist.
   */
  it('FMS_Function_1100', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.searchFile({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/root/files',
        name: 'a.txt',
        success: function (data) {
          console.log('FMS_Function_1100 success.' + data.fileList[0].uri);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_1100 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_1100 success');
    });

  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_1200
   * @tc.name FMS_Function_1200
   * @tc.desc Function of API, Get all file information in the specified directory
   */
  it('FMS_Function_1200', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.list({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/fms_test/leaf',
        success: function (data) {
          console.log('FMS_Function_1200 success.' + data.fileList[0].uri);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_1200 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_1200 success');
    });
  })

  /**
   * @tc.number SUB_STORAGE_FMS_Function_1300
   * @tc.name FMS_Function_1300
   * @tc.desc Function of API, Get all file information in the specified directory
   */
  it('FMS_Function_1300', 0, async function (done) {
    let proMkdir = new Promise(function (resolve, reject) {
      filepicker.list({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/root',
        success: function (data) {
          console.log('FMS_Function_1300 success.' + data.fileList[0].uri);
          done()
        },
        fail: function (data, code) {
          console.log('FMS_Function_1300 fail, code: ' + code + ', data: ' + data);
          expect(null).assertFail();
        },
      });
    })
    proMkdir.then(function () {
      console.log('FMS_Function_1300 success');
    });
  })

})
