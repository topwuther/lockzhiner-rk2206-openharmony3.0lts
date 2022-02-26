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

/******/ (() => { // webpackBootstrap
    var __webpack_exports__ = {};
    /*!********************************************************************************************!*\
  !*** d:\workspace\my_works\MyApplication\entry\src\main\ets\default\workers\WorkerDemo.js ***!
  \********************************************************************************************/
    function foo(x) {return x}

    var worker = globalThis.requireNapi('worker');

    const parentPort = worker.parentPort;

    console.info("worker:: new version")

    parentPort.onclose = function() {
        console.info("worker::worker.js onclose");
    }

    parentPort.onmessage = function(e) {
        var data = e.data;
        console.info("worker:: worker thread worker data is " + data.data);
        switch(data.type) {
            case "normal":
                console.info("worker:: worker thread receive data " + data.data);
                parentPort.postMessage(data);
                console.info("worker:: worker thread post back");
                break;
            case "error":
                throw new Error("123");
                break;
            case "buffer":
                console.info("worker:: worker.js receive buffer length is  " + data.data.byteLength);
                parentPort.postMessage(data, [data.data]);
                console.info("worker:: worker.js post buffer length is  " + data.data.byteLength);
                break;
            default:
                console.info("worker:: worker.js receive unknow type");
                break
        }
    }

    // 反序列错误
    parentPort.onmessageerror = function() {
        console.info("worker:: worker.js onmessageerror");
    }

    // js执行异常
    parentPort.onerror = function(data) {
        console.info("worker:: worker.js onerror " + data.lineno + ", msg = " + data.message + ", filename = " + data.filename + ", colno = " + data.colno);
    }
    /******/ })()
;