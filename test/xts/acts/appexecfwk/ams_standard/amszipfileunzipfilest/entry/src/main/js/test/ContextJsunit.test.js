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
import zlib from '@ohos.zlib'
import fileio from '@ohos.fileio'


import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

console.log("-----------------888888888------------------");

describe('ZlibTest', function () {
   var FLUSH_TYPE_NO_FLUSH = 0;
   var FLUSH_TYPE_PARTIAL_FLUSH = 1;
   var FLUSH_TYPE_SYNC_FLUSH = 2;
   var FLUSH_TYPE_FULL_FLUSH = 3;
   var FLUSH_TYPE_FINISH = 4;
   var FLUSH_TYPE_BLOCK = 5;
   var FLUSH_TYPE_TREES = 6;
   var COMPRESS_LEVEL_NO_COMPRESSION = 0;
   var COMPRESS_LEVEL_BEST_SPEED = 1;
   var COMPRESS_LEVEL_BEST_COMPRESSION = 9;
   var COMPRESS_LEVEL_DEFAULT_COMPRESSION = -1;
   var COMPRESS_STRATEGY_DEFAULT_STRATEGY = 0;
   var COMPRESS_STRATEGY_FILTERED = 1;
   var COMPRESS_STRATEGY_HUFFMAN_ONLY = 2;
   var COMPRESS_STRATEGY_RLE = 3;
   var COMPRESS_STRATEGY_FIXED = 4;
   var MEM_LEVEL_MIN_MEMLEVEL = 1;
   var MEM_LEVEL_DEFAULT_MEMLEVEL = 8;
   var MEM_LEVEL_MAX_MEMLEVEL = 9;


/*
* @tc.number: ACTS_zipFile_0100
* @tc.name: zipFile :
* @tc.desc:
*/

  function zipFileCallBack(err, data){
       console.log("zipFileCallBack_0100 err: " + err.code);
       console.log("zipFileCallBack_0100 data: " + data);
       expect(err).assertEqual(0);
       expect(data).assertEqual(0);
   }
   it('ACTS_zipFile_0100', 0, async function (done) {
       console.log("==================ACTS_zipFile_0100 start==================");
       var src ="";
       var dest ="";

       zlib.zipFile(src,dest,
           (err, data) => {
               console.log("zipFileCallBack_0100 err: " + err.code);
               console.log("zipFileCallBack_0100 data: " + data);
               expect(err).assertEqual(0);
               expect(data).assertEqual(0);
           });

       function timeout() {
           expect().assertFail();
           console.debug('ACTS_zipFile_0100=====timeout======');
       }
       setTimeout(timeout, 5000);
       done();
   })
   

/*
* @tc.number: ACTS_zipFile_0200
* @tc.name: zipFile :
* @tc.desc:
*/

   it('ACTS_zipFile_0200', 0, async function (done) {
       console.log("==================ACTS_zipFile_0200 start==================");
       var src ="/data/test/";
       var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";

       zlib.zipFile(src,dest,
           (err, data) => {
               console.log("zipFileCallBack_0200 err: " + err.code);
               console.log("zipFileCallBack_0200 data: " + data);
               expect(err).assertEqual(0);
               expect(data).assertEqual(0);
           });

       function timeout() {
           expect().assertFail();
           console.debug('ACTS_zipFile_0100=====timeout======');
       }
       setTimeout(timeout, 5000);
       done();
   })

/*
* @tc.number: ACTS_zipFile_0300
* @tc.name: zipFile :
* @tc.desc:
*/

   it('ACTS_zipFile_0300', 0, async function (done) {
       console.log("==================ACTS_zipFile_0300 start==================");
       var src ="/data/test/amsZipfileUnzipfileST.hap";
       var dest ="";

       zlib.zipFile(src,dest,
           (err, data) => {
               console.log("zipFileCallBack_0300 err: " + err.code);
               console.log("zipFileCallBack_0300 data: " + data);
               expect(err).assertEqual(0);
               expect(data).assertEqual(0);
           });

       function timeout() {
           expect().assertFail();
           console.debug('ACTS_zipFile_0300=====timeout======');
       }
       setTimeout(timeout, 5000);
       done();
   })

/*
* @tc.number: ACTS_zipFile_0400
* @tc.name: zipFile :
* @tc.desc:
*/

   it('ACTS_zipFile_0400', 0, async function (done) {
       console.log("==================ACTS_zipFile_0400 start==================");
       var src ="/data/test/";
       var dest ="/data/testA/zip_amsZipfileUnzipfileST-signed.zip";

       zlib.zipFile(src,dest,
           (err, data) => {
               console.log("zipFileCallBack_0400 err: " + err.code);
               console.log("zipFileCallBack_0400 data: " + data);
               expect(err).assertEqual(0);
               expect(data).assertEqual(0);
           });

       function timeout() {
           expect().assertFail();
           console.debug('ACTS_zipFile_0400=====timeout======');
       }
       setTimeout(timeout, 5000);
       done();
   })
   

/*
* @tc.number: ACTS_zipFile_0500
* @tc.name: zipFile :
* @tc.desc:
*/

   it('ACTS_zipFile_0500', 0, async function (done) {
       console.log("==================ACTS_zipFile_0500 start==================");
       var src ="/data/test/amsZipfileUnzipfileST.hap";
       var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
           
       try{
        var Options= {
            flush:FLUSH_TYPE_NO_FLUSH,
        };
          await zlib.zipFile(src,dest,Options,
               () => {
                   console.log("ACTS_zipFile_0500 err:================================ ");
                    try{
                        var isfileio = fileio.accessSync(dest)
                        console.log("ACTS_zipFile_0500 isfileio:==============> "+JSON.stringify(isfileio));
                        var isDest = (fileio.accessSync(dest) !== null)
                        console.log("ACTS_zipFile_0500 isDest:==============> " +isDest);
                        expect(isDest).assertTrue();
                        var big = fileio.statSync(src).size;
                        var small = fileio.statSync(dest).size;
                        var isBigSmall = (big>=small)
                        expect(isBigSmall).assertTrue();
                        console.log("ACTS_zipFile_0500 big=" + big);
                        console.log("ACTS_zipFile_0500 small=" + small);
                        console.log("ACTS_zipFile_0500 big>=small=" + isBigSmall);
                    }catch(err) {
                        console.error('ACTS_zipFile_0500  assertTure  err:' + err);
                    }
                    console.debug('ACTS_zipFile_0500=====size======');
                   done();
               });
       }catch(err) {
           console.error('ACTS_zipFile_0500  size  err:' + err);

           done();
       }
 
    setTimeout(function(){
        console.debug('ACTS_zipFile_0500=====timeout======');
    }, '1000');
       
   })

/*
* @tc.number: ACTS_zipFile_0600
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_0600', 0, async function (done) {
    console.log("==================ACTS_zipFile_0600 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
         flush:FLUSH_TYPE_NO_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_0600 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_0600 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_0600 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_0600 big=" + big);
                     console.log("ACTS_zipFile_0600 small=" + small);
                     console.log("ACTS_zipFile_0600 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_0600  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_0600=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_0600  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_000=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_700
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_0700', 0, async function (done) {
    console.log("==================ACTS_zipFile_0700 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
         flush:FLUSH_TYPE_SYNC_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_0500 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_0700 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_0700 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_0700 big=" + big);
                     console.log("ACTS_zipFile_0700 small=" + small);
                     console.log("ACTS_zipFile_0700 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_0700  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_0700=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_0700  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_0700=====timeout======');
 }, '1000');
    
})
/*
* @tc.number: ACTS_zipFile_0800
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_0800', 0, async function (done) {
    console.log("==================ACTS_zipFile_0800 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
         flush:FLUSH_TYPE_FULL_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_0800 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_0800 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_0800 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_0800 big=" + big);
                     console.log("ACTS_zipFile_0800 small=" + small);
                     console.log("ACTS_zipFile_0800 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_0800  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_0800=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_0800  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_0800=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_0900
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_0900', 0, async function (done) {
    console.log("==================ACTS_zipFile_0900 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flush:FLUSH_TYPE_FINISH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_0900 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_0900 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_0900 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_0900 big=" + big);
                     console.log("ACTS_zipFile_0900 small=" + small);
                     console.log("ACTS_zipFile_0900 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_0900  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_0900=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_0900  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_0900=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_1000
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1000', 0, async function (done) {
    console.log("==================ACTS_zipFile_1000 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flush:FLUSH_TYPE_BLOCK,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1000 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1000 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1000 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1000 big=" + big);
                     console.log("ACTS_zipFile_1000 small=" + small);
                     console.log("ACTS_zipFile_1000 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1000  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1000=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1000  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1000=====timeout======');
 }, '1000');
    
})
/*
* @tc.number: ACTS_zipFile_1100
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1100', 0, async function (done) {
    console.log("==================ACTS_zipFile_1100 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flush:FLUSH_TYPE_TREES,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1100 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1100 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1100 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1100 big=" + big);
                     console.log("ACTS_zipFile_1100 small=" + small);
                     console.log("ACTS_zipFile_1100 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1100  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1100=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1100  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1100=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1200
* @tc.name: zipFile :
* @tc.desc:
*/

   it('ACTS_zipFile_1200', 0, async function (done) {
    console.log("==================ACTS_zipFile_0900 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        finishFlush:FLUSH_TYPE_NO_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1200 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1200 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1200 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1200 big=" + big);
                     console.log("ACTS_zipFile_1200 small=" + small);
                     console.log("ACTS_zipFile_1200 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1200  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1200=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1200  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1200=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1300
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1300', 0, async function (done) {
    console.log("==================ACTS_zipFile_1300 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_PARTIAL_FLUSH
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1300 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1300 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1300 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1300 big=" + big);
                     console.log("ACTS_zipFile_1300 small=" + small);
                     console.log("ACTS_zipFile_1300 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1300  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1300=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1300  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1300=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1400
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1400', 0, async function (done) {
    console.log("==================ACTS_zipFile_1400 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_SYNC_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1400 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1400 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1400 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1400 big=" + big);
                     console.log("ACTS_zipFile_1400 small=" + small);
                     console.log("ACTS_zipFile_1400 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1400  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1400=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1400  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1400=====timeout======');
 }, '1000');
    
})
/*
* @tc.number: ACTS_zipFile_1500
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1500', 0, async function (done) {
    console.log("==================ACTS_zipFile_1500 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_FULL_FLUSH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1500 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1500 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1500 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1500 big=" + big);
                     console.log("ACTS_zipFile_1500 small=" + small);
                     console.log("ACTS_zipFile_1500 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1500  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1500=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1500  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1500=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1600
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1600', 0, async function (done) {
    console.log("==================ACTS_zipFile_1600 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_FINISH,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1600 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1600 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1600 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1600 big=" + big);
                     console.log("ACTS_zipFile_1600 small=" + small);
                     console.log("ACTS_zipFile_1600 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1600  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1600=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1600  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1600=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1700
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1700', 0, async function (done) {
    console.log("==================ACTS_zipFile_1700 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_BLOCK,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1700 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1700 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1700 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1700 big=" + big);
                     console.log("ACTS_zipFile_1700 small=" + small);
                     console.log("ACTS_zipFile_1700 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1700  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1700=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1700  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1700=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1800
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1800', 0, async function (done) {
    console.log("==================ACTS_zipFile_1800 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_TREES,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1800 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1800 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1800 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1800 big=" + big);
                     console.log("ACTS_zipFile_1800 small=" + small);
                     console.log("ACTS_zipFile_1800 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1800  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1800=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1800  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1800=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_1900
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_1900', 0, async function (done) {
    console.log("==================ACTS_zipFile_1900 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        chunkSize:64,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_1900 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_1900 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_1900 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_1900 big=" + big);
                     console.log("ACTS_zipFile_1900 small=" + small);
                     console.log("ACTS_zipFile_1900 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_1900  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_1900=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_1900  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_1900=====timeout======');
 }, '1000');
    
})
/*
* @tc.number: ACTS_zipFile_2000
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2000', 0, async function (done) {
    console.log("==================ACTS_zipFile_2000 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        chunkSize:1024,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2000 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2000 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2000 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2000 big=" + big);
                     console.log("ACTS_zipFile_2000 small=" + small);
                     console.log("ACTS_zipFile_2000 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2000  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2000=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2000  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2000=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_2100
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2100', 0, async function (done) {
    console.log("==================ACTS_zipFile_2100 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        chunkSize:999,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2100 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2100 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2100 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2100 big=" + big);
                     console.log("ACTS_zipFile_2100 small=" + small);
                     console.log("ACTS_zipFile_2100 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2100  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2100=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2100  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2100=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2200
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2200', 0, async function (done) {
    console.log("==================ACTS_zipFile_2200 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        level:COMPRESS_LEVEL_NO_COMPRESSION ,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2200 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2200 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2200 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2200 big=" + big);
                     console.log("ACTS_zipFile_2200 small=" + small);
                     console.log("ACTS_zipFile_2200 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2200  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2200=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2200  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2200=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_2300
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2300', 0, async function (done) {
    console.log("==================ACTS_zipFile_2300 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        level:COMPRESS_LEVEL_BEST_SPEED,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2300 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2300 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2300 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2300 big=" + big);
                     console.log("ACTS_zipFile_2300 small=" + small);
                     console.log("ACTS_zipFile_2300 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2300  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2300=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2300  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2300=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2400
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2400', 0, async function (done) {
    console.log("==================ACTS_zipFile_2400 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        level:COMPRESS_LEVEL_BEST_COMPRESSION,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2400 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2400 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2400 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2400 big=" + big);
                     console.log("ACTS_zipFile_2400 small=" + small);
                     console.log("ACTS_zipFile_2400 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2400  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2400=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2400  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2400=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2500
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2500', 0, async function (done) {
    console.log("==================ACTS_zipFile_2500 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
       level:COMPRESS_LEVEL_DEFAULT_COMPRESSION,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2000 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2500 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2500 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2500 big=" + big);
                     console.log("ACTS_zipFile_2500 small=" + small);
                     console.log("ACTS_zipFile_2500 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2500  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2500=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2500  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2500=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2600
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2600', 0, async function (done) {
    console.log("==================ACTS_zipFile_2600 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        memlevel:MEM_LEVEL_MIN_MEMLEVEL,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2600 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2600 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2600 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2600 big=" + big);
                     console.log("ACTS_zipFile_2600 small=" + small);
                     console.log("ACTS_zipFile_2600 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2600  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2600=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2600  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2600=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2700
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2700', 0, async function (done) {
    console.log("==================ACTS_zipFile_2700 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        memlevel:MEM_LEVEL_MAX_MEMLEVEL,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2700 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2700 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2700 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2700 big=" + big);
                     console.log("ACTS_zipFile_2700 small=" + small);
                     console.log("ACTS_zipFile_2700 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2700  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2700=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2700  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2700=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2800
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2800', 0, async function (done) {
    console.log("==================ACTS_zipFile_2800 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        memlevel:MEM_LEVEL_DEFAULT_MEMLEVEL,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2800 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2800 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2800 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2800 big=" + big);
                     console.log("ACTS_zipFile_2800 small=" + small);
                     console.log("ACTS_zipFile_2800 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2800  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2800=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2800  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2800=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_2900
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_2900', 0, async function (done) {
    console.log("==================ACTS_zipFile_2900 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        strategy:COMPRESS_STRATEGY_DEFAULT_STRATEGY,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_2900 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_2900 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_2900 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_2900 big=" + big);
                     console.log("ACTS_zipFile_2900 small=" + small);
                     console.log("ACTS_zipFile_2900 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_2900  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_2900=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_2900  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_2900=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_3000
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_3000', 0, async function (done) {
    console.log("==================ACTS_zipFile_3000 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        strategy:COMPRESS_STRATEGY_FILTERED,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_3000 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_3000 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_3000 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_3000 big=" + big);
                     console.log("ACTS_zipFile_3000 small=" + small);
                     console.log("ACTS_zipFile_3000 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_3000  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_3000=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_3000  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_3000=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_zipFile_3100
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_3100', 0, async function (done) {
    console.log("==================ACTS_zipFile_3100 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        strategy:COMPRESS_STRATEGY_HUFFMAN_ONLY,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_3100 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_3100 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_3100 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_3100 big=" + big);
                     console.log("ACTS_zipFile_3100 small=" + small);
                     console.log("ACTS_zipFile_3100 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_3100  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_3100=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_3100  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_3100=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_3200
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_3200', 0, async function (done) {
    console.log("==================ACTS_zipFile_3200 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        strategy:COMPRESS_STRATEGY_RLE,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_3200 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_3200 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_3200 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_3200 big=" + big);
                     console.log("ACTS_zipFile_3200 small=" + small);
                     console.log("ACTS_zipFile_3200 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_3200  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_3200=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_3200  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_3200=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_zipFile_3300
* @tc.name: zipFile :
* @tc.desc:
*/

it('ACTS_zipFile_3300', 0, async function (done) {
    console.log("==================ACTS_zipFile_3300 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
        
    try{
     var Options= {
        strategy:COMPRESS_STRATEGY_HUFFMAN_ONLY,
     };
       await zlib.zipFile(src,dest,Options,
            () => {
                console.log("ACTS_zipFile_3300 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_zipFile_3300 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_zipFile_3300 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big>=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_zipFile_3300 big=" + big);
                     console.log("ACTS_zipFile_3300 small=" + small);
                     console.log("ACTS_zipFile_3300 big>=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_zipFile_3300  assertTure  err:' + err);
                 }
                 console.debug('ACTS_zipFile_3300=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_zipFile_3300  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_zipFile_3300=====timeout======');
 }, '1000');
    
})



/*
* @tc.number: ACTS_unzipFile_0100
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0100', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0100 start==================");
    var src ="";
    var dest ="";

    zlib.unzipFile(src,dest,
        (err, data) => {
            console.log("unzipFileCallBack_0100 err: " + err.code);
            console.log("unzipFileCallBack_0100 data: " + data);
            expect(err).assertEqual(0);
            expect(data).assertEqual(0);
        });

    function timeout() {
        expect().assertFail();
        console.debug('ACTS_unzipFile_0100=====timeout======');
    }
    setTimeout(timeout, 5000);
    done();
})


/*
* @tc.number: ACTS_unzipFile_0200
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0200', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0200 start==================");
    var src ="/data/test/";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";

    zlib.unzipFile(src,dest,
        (err, data) => {
            console.log("unzipFileCallBack_0200 err: " + err.code);
            console.log("unzipFileCallBack_0200 data: " + data);
            expect(err).assertEqual(0);
            expect(data).assertEqual(0);
        });

    function timeout() {
        expect().assertFail();
        console.debug('ACTS_unzipFile_0200=====timeout======');
    }
    setTimeout(timeout, 5000);
    done();
})


/*
* @tc.number: ACTS_unzipFile_0300
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0300', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0300 start==================");
    var src ="/data/test/amsZipfileUnzipfileST.hap";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";

    zlib.unzipFile(src,dest,
        (err, data) => {
            console.log("unzipFileCallBack_0300 err: " + err.code);
            console.log("unzipFileCallBack_0300 data: " + data);
            expect(err).assertEqual(0);
            expect(data).assertEqual(0);
        });

    function timeout() {
        expect().assertFail();
        console.debug('ACTS_unzipFile_0300=====timeout======');
    }
    setTimeout(timeout, 5000);
    done();
})


/*
* @tc.number: ACTS_unzipFile_0400
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0400', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0400 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/testA/";

    zlib.unzipFile(src,dest,
        (err, data) => {
            console.log("unzipFileCallBack_0400 err: " + err.code);
            console.log("unzipFileCallBack_0400 data: " + data);
            expect(err).assertEqual(0);
            expect(data).assertEqual(0);
        });

    function timeout() {
        expect().assertFail();
        console.debug('ACTS_unzipFile_0400=====timeout======');
    }
    setTimeout(timeout, 5000);
    done();
})

/*
* @tc.number: ACTS_unzipFile_0500
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0500', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0500 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
         flushFlush:FLUSH_TYPE_NO_FLUSH,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_0500 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_0500 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_0500 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_0500 big=" + big);
                     console.log("ACTS_unzipFile_0500 small=" + small);
                     console.log("ACTS_unzipFile_0500 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_0500  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_0500=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0500  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_0500=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_unzipFile_0600
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0600', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0600 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_PARTIAL_FLUSH,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_0600 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_0600 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_0600 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_0600 big=" + big);
                     console.log("ACTS_unzipFile_0600 small=" + small);
                     console.log("ACTS_unzipFile_0600 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_0600  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_0600=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0600  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_0600=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_unzipFile_0700
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0700', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0700 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_SYNC_FLUSH,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_0700 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_0700 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_0700 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_0700 big=" + big);
                     console.log("ACTS_unzipFile_0700 small=" + small);
                     console.log("ACTS_unzipFile_0700 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_0700  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_0700=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0700  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_0700=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_unzipFile_0800
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0800', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0800 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_FULL_FLUSH,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_0800 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_0800 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_0800 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_0800 big=" + big);
                     console.log("ACTS_unzipFile_0800 small=" + small);
                     console.log("ACTS_unzipFile_0800 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_0800  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_0800=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0800  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_0800=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_unzipFile_0900
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_0900', 0, async function (done) {
    console.log("==================ACTS_unzipFile_0900 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_FINISH,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_0900 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_0900 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_0900 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_0900 big=" + big);
                     console.log("ACTS_unzipFile_0900 small=" + small);
                     console.log("ACTS_unzipFile_0900 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_0900  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_0900=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0900  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_0900=====timeout======');
 }, '1000');
    
})


/*
* @tc.number: ACTS_unzipFile_1000
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_1000', 0, async function (done) {
    console.log("==================ACTS_unzipFile_1000 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_BLOCK,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_1000 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_1000 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_1000 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_1000 big=" + big);
                     console.log("ACTS_unzipFile_1000 small=" + small);
                     console.log("ACTS_unzipFile_1000 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_1000  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_1000=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_1000  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_1000=====timeout======');
 }, '1000');
    
})

/*
* @tc.number: ACTS_unzipFile_1100
* @tc.name: unzipFile :
* @tc.desc:
*/

it('ACTS_unzipFile_1100', 0, async function (done) {
    console.log("==================ACTS_unzipFile_1100 start==================");
    var src ="/data/test/zip_amsZipfileUnzipfileST-signed.zip";
    var dest ="/data/test/amsZipfileUnzipfileST.hap";
        
    try{
     var Options= {
        flushFlush:FLUSH_TYPE_TREES,
     };
       await zlib.unzipFile(src,dest,Options,
            () => {
                console.log("ACTS_unzipFile_1100 err:================================ ");
                 try{
                     var isfileio = fileio.accessSync(dest)
                     console.log("ACTS_unzipFile_1100 isfileio:==============> "+JSON.stringify(isfileio));
                     var isDest = (fileio.accessSync(dest) !== null)
                     console.log("ACTS_unzipFile_1100 isDest:==============> " +isDest);
                     expect(isDest).assertTrue();
                     var big = fileio.statSync(src).size;
                     var small = fileio.statSync(dest).size;
                     var isBigSmall = (big<=small)
                     expect(isBigSmall).assertTrue();
                     console.log("ACTS_unzipFile_1100 big=" + big);
                     console.log("ACTS_unipFile_1100 small=" + small);
                     console.log("ACTS_unzipFile_1100 big<=small=" + isBigSmall);
                 }catch(err) {
                     console.error('ACTS_unzipFile_1100  assertTure  err:' + err);
                 }
                 console.debug('ACTS_unzipFile_1100=====size======');
                done();
            });
    }catch(err) {
        console.error('ACTS_unzipFile_0500  size  err:' + err);

        done();
    }

 setTimeout(function(){
     console.debug('ACTS_unzipFile_1100=====timeout======');
 }, '1000');
    
})
})
