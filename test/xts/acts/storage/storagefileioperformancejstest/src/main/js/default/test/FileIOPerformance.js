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

import fileio from '@system.fileio'
import {
  describe,
  it,
}
  from 'deccjsunit/index'
import {
  FILE_CONTENT,
  prepareFile,
  nextFileName,
  sleep,
  randomString
}
  from './Common'

describe('fileTest', function () {

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_Dir_Dirent
   * @tc.name fileioPerformance_Dir_Dirent
   * @tc.desc Dirent Perform 1000 performance tests
   */
  it('fileioPerformance_Dir_Dirent', 0, function () {
    for (let i = 0; i < 1000; i++) {
      let dpath = nextFileName('fileioPerformance_Dir') + 'd'
      let fpath = dpath + '/f0'
      fileio.mkdirSync(dpath);
      prepareFile(fpath, FILE_CONTENT);
      let start = new Date().getTime();
      let dd = fileio.opendirSync(dpath);
      let end = new Date().getTime();
      let time = end - start;
      console.log('fileioPerformance_Dir,opendirSync:' + dd + ', time:' + time + ',' + i);
      let start1 = new Date().getTime();
      let dirent = dd.readSync();
      let end1 = new Date().getTime();
      let time1 = end1 - start1;
      console.log('fileioPerformance_Dir,readSync:' + dirent + ', time1:' + time1 + ',' + i);
      let start5 = new Date().getTime();
      let isBlockDevice = dirent.isBlockDevice();
      let end5 = new Date().getTime();
      let time5 = end5 - start5;
      console.log('fileioPerformance_Dirent,isBlockDevice:' + isBlockDevice + ', time5:' + time5 + ',' + i);
      let start6 = new Date().getTime();
      let isCharacterDevice = dirent.isCharacterDevice();
      let end6 = new Date().getTime();
      let time6 = end6 - start6;
      console.log('fileioPerformance_Dirent,isCharacterDevice:' + isCharacterDevice + ', time6:' + time6 + ',' + i);
      let start7 = new Date().getTime();
      let isDirectory = dirent.isDirectory();
      let end7 = new Date().getTime();
      let time7 = end7 - start7;
      console.log('fileioPerformance_Dirent,isDirectory:' + isDirectory + ', time7:' + time7 + ',' + i);
      let start8 = new Date().getTime();
      let isFIFO = dirent.isFIFO();
      let end8 = new Date().getTime();
      let time8 = end8 - start8;
      console.log('fileioPerformance_Dirent,isFIFO:' + isFIFO + ', time8:' + time8 + ',' + i);
      let start9 = new Date().getTime();
      let isFile = dirent.isFile();
      let end9 = new Date().getTime();
      let time9 = end9 - start9;
      console.log('fileioPerformance_Dirent,isFile:' + isFile + ', time9:' + time9 + ',' + i);
      let start1000 = new Date().getTime();
      let isSocket = dirent.isSocket();
      let end1000 = new Date().getTime();
      let time1000 = end1000 - start1000;
      console.log('fileioPerformance_Dirent,isSocket:' + isSocket + ', time1000:' + time1000 + ',' + i);
      let start12 = new Date().getTime();
      let isSymbolicLink = dirent.isSymbolicLink();
      let end12 = new Date().getTime();
      let time12 = end12 - start12;
      console.log('fileioPerformance_Dirent,isSymbolicLink:' + isSymbolicLink + ', time12:' + time12 + ',' + i);
      let start2 = new Date().getTime();
      let result = dd.closeSync();
      let end2 = new Date().getTime();
      let time2 = end2 - start2;
      console.log('fileioPerformance_Dir,closeSync:' + result + ', time2:' + time2 + ',' + i);
      let start3 = new Date().getTime();
      let unlinkSync = fileio.unlinkSync(fpath);
      let end3 = new Date().getTime();
      let time3 = end3 - start3;
      console.log('fileioPerformance_prop,unlinkSync:' + unlinkSync + ', time3:' + time3 + ',' + i);
      let start4 = new Date().getTime();
      let rmdirSync = fileio.rmdirSync(dpath);
      let end4 = new Date().getTime();
      let time4 = end4 - start4
      console.log('fileioPerformance_prop,rmdirSync:' + rmdirSync + ', time4:' + time4 + ',' + i);
    }
    sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_stat
   * @tc.name fileioPerformance_stat
   * @tc.desc Stat Perform 1000 performance tests
   */
  it('fileioPerformance_stat', 0, function () {
    for (let i = 0; i < 1000; i++) {
      let fpath = nextFileName('fileioPerformance_stat-4k');
      let txt = randomString(4096);
      prepareFile(fpath, txt);
      let start0 = new Date().getTime();
      let stat = fileio.statSync(fpath);
      let end0 = new Date().getTime();
      let time0 = end0 - start0;
      console.log('stat,statSync:' + stat + ', time0:' + time0 + ',' + i);
      let start = new Date().getTime();
      let isBlockDevice = stat.isBlockDevice();
      let end = new Date().getTime();
      let time = end - start;
      console.log('stat,isBlockDevice:' + isBlockDevice + ', time:' + time + ',' + i);
      let start1 = new Date().getTime();
      let isCharacterDevice = stat.isCharacterDevice();
      let end1 = new Date().getTime();
      let time1 = end1 - start1;
      console.log('stat,isCharacterDevice:' + isCharacterDevice + ', time1:' + time1 + ',' + i);
      let start2 = new Date().getTime();
      let isDirectory = stat.isDirectory();
      let end2 = new Date().getTime();
      let time2 = end2 - start2;
      console.log('stat,isDirectory:' + isDirectory + ', time2:' + time2 + ',' + i);
      let start3 = new Date().getTime();
      let isFIFO = stat.isFIFO();
      let end3 = new Date().getTime();
      let time3 = end3 - start3;
      console.log('stat,isFIFO:' + isFIFO + ', time3:' + time3 + ',' + i);
      let start4 = new Date().getTime();
      let isFile = stat.isFile();
      let end4 = new Date().getTime();
      let time4 = end4 - start4;
      console.log('stat,isFile:' + isFile + ', time4:' + time4 + ',' + i);
      let start5 = new Date().getTime();
      let isSocket = stat.isSocket();
      let end5 = new Date().getTime();
      let time5 = end5 - start5;
      console.log('stat,isSocket:' + isSocket + ', time5:' + time5 + ',' + i);
      let start6 = new Date().getTime();
      let isSymbolicLink = stat.isSymbolicLink();
      let end6 = new Date().getTime();
      let time6 = end6 - start6;
      console.log('stat,isSymbolicLink:' + isSymbolicLink + ', time6:' + time6 + ',' + i);
    }
    sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_prop
   * @tc.name fileioPerformance_prop
   * @tc.desc Prop Perform 1000 performance tests
   */
  it('fileioPerformance_prop', 0, function () {
    let dpath = nextFileName('fileioPerformance_Dir') + 'd'
    for (let i = 0; i < 1000; i++) {
      let start0 = new Date().getTime();
      let mkdirSync = fileio.mkdirSync(dpath + i);
      let end0 = new Date().getTime();
      let time0 = end0 - start0;
      console.log('fileioPerformance_prop_mkdirSync,mkdirSync:' + mkdirSync + ', time0:' + time0 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      prepareFile(dpath + '999/f0' + i, randomString(4096));
    }
    let fd;
    for (let i = 0; i < 1000; i++) {
      let start = new Date().getTime();
      fd = fileio.openSync(dpath + '999/f0' + i, 0o102, 0o666);
      let end = new Date().getTime();
      let time = end - start
      console.log('fileioPerformance_prop_openSync,openSync:' + fd + ', time:' + time + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let accessSync;
      let start1 = new Date().getTime();
      accessSync = fileio.accessSync(dpath + '999/f0' + i);
      let end1 = new Date().getTime();
      let time1 = end1 - start1;
      console.log('fileioPerformance_prop_accessSync,accessSync:' + accessSync + ', time1:' + time1 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let chmodSync;
      let start5 = new Date().getTime();
      chmodSync = fileio.chmodSync(dpath + '999/f0' + i, 0o660);
      let end5 = new Date().getTime();
      let time5 = end5 - start5;
      console.log('fileioPerformance_prop_chmodSync,chmodSync:' + chmodSync + ', time5:' + time5 + ',' + i);
    }
    let stat = fileio.Stat.statSync(dpath + '999/f0999');
    for (let i = 0; i < 1000; i++) {
      let chownSync;
      let start6 = new Date().getTime();
      chownSync = fileio.chownSync(dpath + '999/f0' + i, stat.uid, stat.gid);
      let end6 = new Date().getTime();
      let time6 = end6 - start6;
      console.log('fileioPerformance_prop_chownSync,chownSync:' + chownSync + ', time6:' + time6 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let copyFileSync;
      let start7 = new Date().getTime();
      copyFileSync = fileio.copyFileSync(dpath + '999/f0' + i, dpath + '998/f0' + i);
      let end7 = new Date().getTime();
      let time7 = end7 - start7;
      console.log('fileioPerformance_prop_copyFileSync,copyFileSync:' + copyFileSync + ', time7:' + time7 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let fchmodSync;
      let start8 = new Date().getTime();
      fchmodSync = fileio.fchmodSync(fd, 0o660);
      let end8 = new Date().getTime();
      let time8 = end8 - start8;
      console.log('fileioPerformance_prop_fchmodSync,fchmodSync:' + fchmodSync + ', time8:' + time8 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let fchownSync;
      let start9 = new Date().getTime();
      fchownSync = fileio.fchownSync(fd, stat.uid, stat.gid);
      let end9 = new Date().getTime();
      let time9 = end9 - start9;
      console.log('fileioPerformance_prop_fchownSync,fchownSync:' + fchownSync + ', time9:' + time9 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let fstatSync;
      let start1000 = new Date().getTime();
      fstatSync = fileio.fstatSync(fd);
      let end1000 = new Date().getTime();
      let time1000 = end1000 - start1000;
      console.log('fileioPerformance_prop_fstatSync,fstatSync:' + fstatSync + ', time1000:' + time1000 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let ftruncateSync;
      let start12 = new Date().getTime();
      ftruncateSync = fileio.ftruncateSync(fd);
      let end12 = new Date().getTime();
      let time12 = end12 - start12;
      console.log('fileioPerformance_prop_ftruncateSync,:' + ftruncateSync + ', time12:' + time12 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let renameSync;
      let start13 = new Date().getTime();
      renameSync = fileio.renameSync(dpath + '998/f0' + i, dpath + '998/f0' + i + 'a');
      let end13 = new Date().getTime();
      let time13 = end13 - start13;
      console.log('fileioPerformance_prop_renameSync,renameSync:' + renameSync + ', time13:' + time13 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let truncateSync;
      let start14 = new Date().getTime();
      truncateSync = fileio.truncateSync(dpath + '998/f0' + i + 'a', 1000);
      let end14 = new Date().getTime();
      let time14 = end14 - start14;
      console.log('fileioPerformance_prop_truncateSync,truncateSync:' + truncateSync + ', time14:' + time14 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let fsyncSync;
      let start16 = new Date().getTime();
      fsyncSync = fileio.fsyncSync(fd);
      let end16 = new Date().getTime();
      let time16 = end16 - start16;
      console.log('fileioPerformance_prop_fsyncSync,fsyncSync:' + fsyncSync + ', time16:' + time16 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let closeSync;
      let start2 = new Date().getTime();
      let fd1 = fileio.openSync(dpath + '999/f0' + i, 0o102, 0o666);
      closeSync = fileio.closeSync(fd1);
      let end2 = new Date().getTime();
      let time2 = end2 - start2;
      console.log('fileioPerformance_prop_closeSync,closeSync:' + closeSync + ', time2:' + time2 + ',' + i);
    }
    sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_prop_write_read
   * @tc.name fileioPerformance_prop_write_read
   * @tc.desc Prop_write_read Perform 1000 performance tests
   */
  it('fileioPerformance_prop_write_read', 0, function () {
    for (let i = 0; i < 1000; i++) {
      let dpath = nextFileName('prop_write_read') + 'd';
      let fpath = dpath + '/prop_write_read';
      fileio.mkdirSync(dpath);
      prepareFile(fpath, 'a');
      let fd = fileio.openSync(fpath, 0o102, 0o666);
      let data = randomString(4096);
      let start17 = new Date().getTime();
      let writeSync = fileio.writeSync(fd, data);
      let end17 = new Date().getTime();
      let time17 = end17 - start17;
      console.log('fileioPerformance_prop_write,4K_prop_writeSync:' + writeSync + ', time17:' + time17 + ',' + i);
      let readSync;
      let start18 = new Date().getTime();
      writeSync = fileio.readSync(fd, new ArrayBuffer(4096));
      let end18 = new Date().getTime();
      let time18 = end18 - start18;
      console.log('fileioPerformance_prop_readSync,4K_prop_readSync:' + readSync + ', time18:' + time18 + ',' + i);
      fileio.closeSync(fd);
      fileio.unlinkSync(fpath);
      fileio.rmdirSync(dpath);
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_stream
   * @tc.name fileioPerformance_stream
   * @tc.desc Stream Perform 1000 performance tests
   */
  it('fileioPerformance_stream', 0, function () {
    let fpath = nextFileName('fileioPerformance_stream4K');
    let txt = randomString(4096);
    prepareFile(fpath, txt);
    for (let i = 0; i < 1000; i++) {
      let fd = fileio.openSync(fpath, 0o2);
      let start = new Date().getTime();
      let ss = fileio.fdopenStreamSync(fd, 'r+');
      let end = new Date().getTime();
      let time = end - start;
      console.log('fileioPerformance_stream,fdopenStreamSync(r+):' + ss + ', time:' + time + ',' + i);
      fileio.closeSync(fd);
      ss.closeSync();
    }
    for (let i = 0; i < 1000; i++) {
      let fd0 = fileio.openSync(fpath, 0o2);
      let start0 = new Date().getTime();
      let ss0 = fileio.fdopenStreamSync(fd0, 'rb+');
      let end0 = new Date().getTime();
      let time0 = end0 - start0;
      console.log('fileioPerformance_stream,fdopenStreamSync(rb+):' + ss0 + ', time0:' + time0 + ',' + i);
      fileio.closeSync(fd0);
      ss0.closeSync();
    }
    for (let i = 0; i < 1000; i++) {
      let start1 = new Date().getTime();
      let createFlag0 = fileio.createStreamSync(fpath, 'r+');
      let end1 = new Date().getTime();
      let time1 = end1 - start1;
      console.log('fileioPerformance_stream,createStreamSync(r+):' + createFlag0 + ', time1:' + time1 + ',' + i);
      createFlag0.closeSync();
    }
    for (let i = 0; i < 1000; i++) {
      let start2 = new Date().getTime();
      let createFlag = fileio.createStreamSync(fpath, 'rb+');
      let end2 = new Date().getTime();
      let time2 = end2 - start2;
      console.log('fileioPerformance_stream,createStreamSync(rb+):' + createFlag + ', time2:' + time2 + ',' + i);
    }
    for (let i = 0; i < 1000; i++) {
      let createFlag = fileio.createStreamSync(fpath, 'r+');
      let start3 = new Date().getTime();
      let readSync = createFlag.readSync(new ArrayBuffer(4096));
      let end3 = new Date().getTime();
      let time3 = end3 - start3
      console.log('fileioPerformance_stream,readSync:' + readSync + ', time3:' + time3 + ',' + i);
      createFlag.closeSync();
    }
    for (let i = 0; i < 1000; i++) {
      let createFlag = fileio.createStreamSync(fpath, 'r+');
      let start4 = new Date().getTime();
      let flushSync = createFlag.flushSync();
      let end4 = new Date().getTime();
      let time4 = end4 - start4
      console.log('fileioPerformance_stream,flushSync:' + flushSync + ', time4:' + time4 + ',' + i);
      createFlag.closeSync();
    }
    for (let i = 0; i < 1000; i++) {
      let createFlag = fileio.createStreamSync(fpath, 'r+');
      let start5 = new Date().getTime();
      let closeSync = createFlag.closeSync();
      let end5 = new Date().getTime();
      let time5 = end5 - start5
      console.log('fileioPerformance_stream,closeSync:' + closeSync + ', time5:' + time5 + ',' + i);
    }
    sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_stream_write_read
   * @tc.name fileioPerformance_stream_write_read
   * @tc.desc Stream_write_read Perform 1000 performance tests
   */
  it('fileioPerformance_stream_write_read_4K', 0, function () {
    let fpath = nextFileName('stream_write_read');
    for (let i = 0; i < 1000; i++) {
      let ws = fileio.createStreamSync(fpath, 'w');
      let data = randomString(4096);
      let start1 = new Date().getTime();
      let writeSync = ws.writeSync(data);
      let end1 = new Date().getTime();
      let time1 = end1 - start1
      console.log('fileioPerformance_stream_write,4K_stream_writeSync:' + writeSync + ', time1:' + time1 + ',' + i);
      ws.closeSync();
    }
    sleep(3000);
    for (let i = 0; i < 1000; i++) {
      let rs = fileio.createStreamSync(fpath, 'r');
      let start2 = new Date().getTime();
      let readSync = rs.readSync(new ArrayBuffer(4096));
      let end2 = new Date().getTime();
      let time2 = end2 - start2
      console.log('fileioPerformance_stream_readSync,4K_stream_readSync:' + readSync + ', time2:' + time2 + ',' + i);
      rs.closeSync();
    }
    sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_stream_write_read
   * @tc.name fileioPerformance_stream_write_read
   * @tc.desc Stream_write_read Perform ROM remaining less than 10% of the total
   */
  it('fileioPerformance_stream_write_read', 0, function () {
    let fpath = nextFileName('stream_write_read');
    let data;
    for (let i = 0; i < 1024; i++) {
      data = data + randomString(1024);
    }
    for (let i = 0; i < 1000; i++) {
      let ws = fileio.createStreamSync(fpath, 'w');
      let start1 = new Date().getTime();
      let writeSync = ws.writeSync(data);
      let end1 = new Date().getTime();
      let time1 = end1 - start1;
      console.log('fileioPerformance_stream_write,1M_stream_writeSync:' + writeSync + ', time1:' + time1 + ',' + i);
      let rs = fileio.createStreamSync(fpath, 'r');
      let start2 = new Date().getTime();
      let readSync = rs.readSync(new ArrayBuffer(1048576));
      let end2 = new Date().getTime();
      let time2 = end2 - start2
      console.log('fileioPerformance_stream_readSync,1M_stream_readSync:' + readSync + ', time2:' + time2 + ',' + i);
      ws.closeSync();
      fileio.unlinkSync(fpath);
    }
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_prop_copyFile
   * @tc.name fileioPerformance_prop_copyFile
   * @tc.desc 0.5G Synchronous opening performance test of test files
   * The path needs to be manually placed in advance
   * data/accounts/account_0/appdata/ohos.acts.distributeddatamgr.distributedfile/cache/p1'的0.5g文件）
   */
  it('fileioPerformance_prop_copyFile', 0, function () {
    console.log('---fileioPerformance_prop_copyFile 0.5G---start---');
    let fpath = nextFileName('p1');
      let fpathTarget = nextFileName('p2');
      for (let i = 0; i < 1000; i++) {
        let start2 = new Date().getTime();
        let copyFileSync = fileio.copyFileSync(fpath, fpathTarget);
        let end2 = new Date().getTime();
        let time2 = end2 - start2
        console.log('fileioPerformance_prop_copyFileSync,copyFileSync:' + copyFileSync + ', time2:' + time2 + ',' + i);
        fileio.unlinkSync(fpathTarget);
        sleep(300);
      }
      fileio.closeSync(fd);
      sleep(3000);
  });

  /**
   * @tc.number SUB_STORAGE_fileioPerformance_prop_openSync
   * @tc.name fileioPerformance_prop_openSync
   * @tc.desc 0.5G Synchronous opening performance test of test files
   * The path needs to be manually placed in advance
   * data/accounts/account_0/appdata/ohos.acts.distributeddatamgr.distributedfile/cache/p1'的0.5g文件）
   */
  it('fileioPerformance_prop_openSync', 0, function () {
    console.log('---fileioPerformance_prop_openSync 0.5G---start---');
    let fpath = nextFileName('p1');
    let fd
    for (let i = 0; i < 1000; i++) {
      let start = new Date().getTime();
      fd = fileio.openSync(fpath, 0o2002);
      let end = new Date().getTime();
      let time = end - start
      console.log('fileioPerformance_prop_openSync,openSync:' + fd + ', time:' + time + ',' + i);
    }
    fileio.closeSync(fd);
    sleep(3000);
  });
});
