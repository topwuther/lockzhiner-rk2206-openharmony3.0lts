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

import filepicker from '@ohos.filepicker';
import {
  describe,
  it,
  expect
}
from 'deccjsunit/index'
import {
  prepareFile,
  fmsPublic,
}
from './Common'

describe('FilepickerTest', function () {

  let deviceID = 0;

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0000
   * @tc.name File_Test_Error_0000
   * @tc.desc Error of API, Move files when disk space is full.
   */
  it('File_Test_Error_0000', 0, function (done) {
    try {
      filepicker.getDeviceInfo({
        success: function (data) {
          console.log('File_Test_Error_0000_start call mkdir success.' + data.deviceList[0].deviceID);
          deviceID = data.deviceList[2].deviceID;
          if(deviceID == 0){
            console.log('SD card not mounted')
            done()
          }else{
            expect(null).assertFail()
          }
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0000_start call fail callback fail, code: ' + code + ', data: ' + data);
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0000 has failed for ' + e);
      expect(null).assertFail();
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0100
   * @tc.name File_Test_Error_0100
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/udocument/files/leaf
   */
  it('File_Test_Error_0100', 0, function (done) {
    try {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/udocument/files/leaf',
        success: function () {
          console.log('File_Test_Error_0100 call mkdir success.');
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0100 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 202).assertTrue();
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0100 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0200
   * @tc.name File_Test_Error_0200
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/data123ability:///public.storage.ability/deviceID/document/files.txt/leaf
   */
  it('File_Test_Error_0200', 0, function (done) {
    try {
      filepicker.mkdir({
        uri: 'data123ability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        success: function () {
          console.log('File_Test_Error_0200 call mkdir success.');
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0200 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 302).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0200 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0300
   * @tc.name File_Test_Error_0300
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/lea
   */
  it('File_Test_Error_0300', 0, function (done) {
    try {
      filepicker.mkdir({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/lea',
        success: function () {
          console.log('File_Test_Error_0300 call mkdir success.');
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0300 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 202).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0300 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0400
   * @tc.name File_Test_Error_0400
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/‘’/document/files/leaf
   */
  it('File_Test_Error_0400', 0, function (done) {
    try {
      filepicker.mkdir({
        uri: '/data/ss/files'
      });
      let fpath = fmsPublic('File_Test_Error_0400');
      prepareFile(fpath, 'hello');
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/files/File_Test_Error_0400'],
        dstUri: 'dataability:///public.storage.ability/‘’/document/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_0400 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0400 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0400 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0500
   * @tc.name File_Test_Error_0500
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/leaf
   */
  it('File_Test_Error_0500', 0, function (done) {
    try {
      let fpath = fmsPublic('File_Test_Error_0500');
      prepareFile(fpath, 'hello');
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/file/File_Test_Error_0500'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_0500 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0500 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0500 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0600
   * @tc.name File_Test_Error_0600
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/leaf
   */
  it('File_Test_Error_0600', 0, function (done) {
    try {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/file/A.txt'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_0600 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0600 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0600 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0700
   * @tc.name File_Test_Error_0700
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/lea
   */
  it('File_Test_Error_0700', 0, function (done) {
    try {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/files/a.txt'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/lea',
        success: function (data) {
          console.log('File_Test_Error_0700 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0700 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 202).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0700 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0800
   * @tc.name File_Test_Error_0800
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/bb/aa/files/leaf
   */
  it('File_Test_Error_0800', 0, function (done) {
    try {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/files/a.txt'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/document/bb/aa/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_0800 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0800 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0800 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_0900
   * @tc.name File_Test_Error_0900
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/data123ability:///public.storage.ability/' + deviceID + '/document/files/leaf
   */
  it('File_Test_Error_0900', 0, function (done) {
    try {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/files/a.txt'],
        dstUri: 'data123ability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_0900 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_0900 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 302).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_0900 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1000
   * @tc.name File_Test_Error_1000
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/leaf
   */
  it('File_Test_Error_1000', 0, function (done) {
    try {
      filepicker.saveFile({
        srcUri: ['dataability:///public.storage.ability/data/ss/files/a.txt'],
        srcUri1: ['dataability:///public.storage.ability/data/ss/files/a.txt'],
        dstUri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_1000 call mkdir success.' + data.saveFileList[0].status);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1000 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1000 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1100
   * @tc.name File_Test_Error_1100
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/files/leaf
   */
  it('File_Test_Error_1100', 0, function (done) {
    try {
      filepicker.searchFile({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/files/leaf',
        name: 'test',
        success: function (data) {
          console.log('File_Test_Error_1100 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1100 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1100 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1200
   * @tc.name File_Test_Error_1200
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/a/b/files/leaf
   */
  it('File_Test_Error_1200', 0, function (done) {
    try {
      filepicker.searchFile({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/a/b/files/leaf',
        name: 'test',
        success: function (data) {
          console.log('File_Test_Error_1200 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1200 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1200 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1300
   * @tc.name File_Test_Error_1300
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:''
   */
  it('File_Test_Error_1300', 0, function (done) {
    try {
      filepicker.searchFile({
        uri: '',
        name: 'a',
        success: function (data) {
          console.log('File_Test_Error_1300 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1300 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 302).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1300 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1400
   * @tc.name File_Test_Error_1400
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:'dataability:///public.storage.ability/device_id/udocument/files/leaf'
   */
  it('File_Test_Error_1400', 0, function (done) {
    try {
      filepicker.list({
        uri: 'dataability:///public.storage.ability/device_id/udocument/files/leaf',
        success: function (data) {
          console.log('File_Test_Error_1400 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1400 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 202).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1400 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1400
   * @tc.name File_Test_Error_1400
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:''
   */
  it('File_Test_Error_1500', 0, function (done) {
    try {
      filepicker.list({
        uri: '',
        success: function (data) {
          console.log('File_Test_Error_1500 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1500 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 302).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1500 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1500
   * @tc.name File_Test_Error_1500
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/dataability:///public.storage.ability/deviceID/document/file/leaf
   */
  it('File_Test_Error_1600', 0, function (done) {
    try {
      filepicker.list({
        uri: 'dataability:///public.storage.ability/' + deviceID + '/document/file/leaf',
        success: function (data) {
          console.log('File_Test_Error_1600 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1600 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 300).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1600 has failed for ' + e);
    }
  });

  /**
   * @tc.number SUB_STORAGE_File_Test_Error_1700
   * @tc.name File_Test_Error_1700
   * @tc.desc Error of API, Move files when disk space is full.
   *  Create the file in advance and give 777 permission,
   *  path:/data123ability:///public.storage.ability/deviceID/document/file/leaf
   */
  it('File_Test_Error_1700', 0, function (done) {
    try {
      filepicker.list({
        uri: 'data123ability:///public.storage.ability/' + deviceID + '/document/file/leaf',
        success: function (data) {
          console.log('File_Test_Error_1700 call mkdir success.' + data.fileList[0].uri);
          expect(null).assertFail();
        },
        fail: function (data, code) {
          console.error('File_Test_Error_1700 call fail callback fail, code: ' + code + ', data: ' + data);
          expect(code == 302).assertTrue()
          done()
        },
      });
    } catch (e) {
      console.log('File_Test_Error_1700 has failed for ' + e);
    }
  });
});
