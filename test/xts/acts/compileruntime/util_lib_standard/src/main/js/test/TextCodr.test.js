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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import util from '@ohos.util'
describe('textEncoderTest', function () {
    it('util_printf_test_001', 0, function () {
        var format = "%i,%s";
        var value1 = 1.5;
        var value2 = "qwer";
        var value3 = 15;
        var result = util.printf(format, value1, value2, value3);
        expect(result).assertEqual("1,qwer 15");
    })

    it('util_printf_test_002', 0, function () {
        var format = "%O";
        var value = { name: 'jack', age: 15 };
        var result = util.printf(format, value);
        expect(result).assertEqual("{ name: 'jack' ,age: 15 }\n");
    })

    it('util_printf_test_003', 0, function () {
        var format = "%o";
        var value = [1, 2, 3];
        var result = util.printf(format, value);
        var res = '[ 1, 2, 3, [length]: 3 ]'
        expect(result).assertEqual(' [ 1, 2, 3, [length]: 3 ]');
    })

    it('util_printf_test_004', 0, function () {
        var format = "%s,%s,%s";
        var value1 = "1.5";
        var value2 = "qwer";
        var result = util.printf(format, value1, value2);
        expect(result).assertEqual('1.5,qwer,%s');
    })

    it('util_getErrorString_test_001', 0, async function () {
        var errnum = 10;
        var result = util.getErrorString(errnum);
        expect(result).assertEqual('No child process');
    })

    it('util_getErrorString_test_002', 0, async function () {
        console.log('--SK-- util_getErrorString_test_002');
        var errnum = 0;
        var result = util.getErrorString(errnum);
        expect(result).assertEqual('No error information');
    })

    it('util_getErrorString_test_003', 0, async function () {
        var errnum = -1;
        var result = util.getErrorString(errnum);
        expect(result).assertEqual('No error information');
    })

    it('util_promisewrapper_test_002', 0, async function () {
        function aysnFun(str1, str2, callback) {
            if (typeof str1 === 'string' && typeof str1 === 'string') {
                callback(null, str1 + str2);
            } else {
                callback('type err');
            }
        }
        let newPromiseObj = util.promiseWrapper(aysnFun)("Hello", 'World');
        newPromiseObj.then(res => {
            expect(res).strictEqual('HelloWorld');
        })
    })

    it('util_promisewrapper_test_003', 0, async function () {
        function aysnFun(str1, str2, callback) {
            if (typeof str1 === 'string' && typeof str1 === 'string') {
                callback(null, str1 + str2);
            } else {
                callback('type err');
            }
        }
        let newPromiseObj = util.promiseWrapper(aysnFun)([1, 2], 'World');
        newPromiseObj.catch(err => {
            expect(err).strictEqual('type err');
        })
    })

    it('util_promisewrapper_test_004', 0, async function () {
        const a = util.promiseWrapper(function() {});
        const b = util.promiseWrapper(a);
        expect(a).strictEqual(b);
    })

    it('util_promisewrapper_test_005', 0, async function () {
        let errToThrow;
        const thrower = util.promiseWrapper(function(a, b, c, cb) {
            errToThrow = new Error();
            throw errToThrow;
        });
        thrower(1, 2, 3).catch(err => {
            expect(err).strictEqual(errToThrow);
        })
    })

    it('util_promisewrapper_test_006', 0, async function () {
        const err = new Error();
        const a = util.promiseWrapper((cb) => cb(err))();
        const b = util.promiseWrapper(() => {throw err;})();
        Promise.all([
            a.then(e => {
                expect(e).strictEqual(err);
            }),
            b.then(e => {
                expect(e).strictEqual(err);
            })
        ]);
    })

    it('util_promisewrapper_test_007', 0, async function () {
        const err = new Error('callback with the error.');
        const stack = err.stack;
        const fn = util.promiseWrapper(function(cb) {
            cb(null);
            cb(err);
        });
        (async () => {
            await fn();
            await Promise.resolve();
            return expect(stack).strictEqual(err.stack);
        })();
    })

    it('util_promisewrapper_test_008', 0, async function () {
        function fn(err, val, callback) {
            callback(err, val);
        }
        (async () => {
            const value = await util.promiseWrapper(fn)(null, 42);
            expect(value).strictEqual(42);
        })();
    })

    it('util_callbackWrapper_test_001', 0, async function () {
        const promiseFn = [1, 2];
        try {
            util.callbackWrapper(promiseFn);
        } catch(e) {
            expect(e.message).strictEqual('original is not function');
        }
    })

    it('util_callbackWrapper_test_002', 0, async function () {
        async function promiseFn() {
            return Promise.resolve('value');
        };
        var cb = util.callbackWrapper(promiseFn);
        cb((err, ret) => {
            expect(err).strictEqual(null);
            expect(ret).strictEqual('value');
        })
    })

    it('util_callbackWrapper_test_003', 0, async function () {
        async function promiseFn() {
            return 42;
        };
        var cb = util.callbackWrapper(promiseFn);
        cb((err, ret) => {
            expect(err).strictEqual(null);
            expect(ret).strictEqual(42);
        })
    })

    it('util_callbackWrapper_test_004', 0, async function () {
        async function promiseFn() {
            return Promise.reject('value');
        };
        var cb = util.callbackWrapper(promiseFn);
        cb((err, ret) => {
            expect(err).strictEqual('value');
            expect(ret).strictEqual(undefined);
        })
    })

    it('util_callbackWrapper_test_005', 0, async function () {
        async function promiseFn(a, b) {
            return a + b;
        };
        var cb = util.callbackWrapper(promiseFn);
        cb(1, 2, (err, ret) => {
            expect(err).strictEqual(null);
            expect(ret).strictEqual(3);
        })
    })

    it('util_callbackWrapper_test_006', 0, async function () {
        async function promiseFn(){
            return null;
        };
        var cb = util.callbackWrapper(promiseFn);
        try {
            cb([1, 2])
        } catch(err) {
            expect(err.message).strictEqual('maybe is not function');
        }
    })

    it('encoding_test_001', 0, function () {
        var  that = new util.TextDecoder('utf-8', { ignoreBOM : true })
        var retStr = that.encoding
        expect(retStr).assertEqual('utf-8')
    })

    // encoding test
    it('encoding_test_002', 0, function () {
        var that = new util.TextDecoder('utf-16le')
        var encodingStr = that.encoding
        expect(encodingStr).assertEqual('utf-16le')
    })

    it('encoding_test_003', 0, function () {
        var that = new util.TextDecoder('utf-16be')
        var encodingStr = that.encoding
        expect(encodingStr).assertEqual('utf-16be')
    })

    // fatal test
    it('fatal_test_001', 0, function () {
        var that = new util.TextDecoder('utf-16be', { fatal : true })
        var fatalStr = that.fatal
        expect(fatalStr).assertEqual(true)
    })

    it('fatal_test_002', 0, function () {
        var that = new util.TextDecoder('utf-16be', { fatal : false })
        var fatalStr = that.fatal
        expect(fatalStr).assertEqual(false)
    })

    it('fatal_test_003', 0, function () {
        var that = new util.TextDecoder('utf-16be')
        var fatalStr = that.fatal
        expect(fatalStr).assertEqual(false)
    })

    it('fatal_test_004', 0, function () {
        var that = new util.TextDecoder('utf-8')
        var fatalStr = that.fatal
        expect(fatalStr).assertEqual(false)
    })

    // ignoreBOM test
    it('ignoreBOM_test_001', 0, function () {
        var that = new util.TextDecoder('utf-16be', { ignoreBOM : true })
        var ignoreBOMStr = that.ignoreBOM
        expect(ignoreBOMStr).assertEqual(true)
    })

    it('ignoreBOM_test_002', 0, function () {
        var that = new util.TextDecoder('utf-16be', { ignoreBOM : false })
        var ignoreBOMStr = that.ignoreBOM
        expect(ignoreBOMStr).assertEqual(false)
    })

    it('ignoreBOM_test_003', 0, function () {
        var that = new util.TextDecoder('utf-16be')
        var ignoreBOMStr = that.ignoreBOM
        expect(ignoreBOMStr).assertEqual(false)
    })

    it('ignoreBOM_test_004', 0, function () {
        var that = new util.TextDecoder('utf-8')
        var ignoreBOMStr = that.ignoreBOM
        expect(ignoreBOMStr).assertEqual(false)
    })

    it('decode_test_001', 0, function () {
        var that = new util.TextDecoder('utf-8');
        var arr = new Uint8Array(3);
        for (var i = 0; i < 3; i++) {
            arr[i] = 0x61 + i;
        }
        var retStr = that.decode(arr);
        var rel = 'abc';
        expect(retStr).assertEqual(rel);
    })

    it('decode_test_002', 0, function () {
        var that = new util.TextDecoder('utf-16le')
        var arr = new Uint8Array(6)
        arr[0] = 0x61;
        arr[1] = 0x00;
        arr[2] = 0x62;
        arr[3] = 0x00;
        arr[4] = 0x63;
        arr[5] = 0x00;
        var retStr = that.decode(arr)
        var rel = 'abc'
        expect(retStr).assertEqual(rel)
    })

    it('decode_test_003', 0, function () {
        var that = new util.TextDecoder('utf-16be');
        var arr = new Uint8Array(6);
        arr[0] = 0x00;
        arr[1] = 0x61;
        arr[2] = 0x00;
        arr[3] = 0x62;
        arr[4] = 0x00;
        arr[5] = 0x63;
        var retStr = that.decode(arr);
        var rel = 'abc'
        expect(retStr).assertEqual(rel);
    })

    it('decode_test_004', 0, function () {
        var that = new  util.TextDecoder('utf-8', { ignoreBOM : true })
        var arr = new Uint8Array(6)
        arr[0] = 0xEF;
        arr[1] = 0xBB;
        arr[2] = 0xBF;
        arr[3] = 0x61;
        arr[4] = 0x62;
        arr[5] = 0x63;
        var retStr = that.decode(arr, {stream:true})
        var BOM = '\uFEFF'
        var rel = 'abc'
        var re = BOM + rel
        expect(retStr).assertEqual(re);
    })

    it('decode_test_005', 0, function () {
        var that = new util.TextDecoder('utf-16le', { ignoreBOM : false })
        var arr = new Uint8Array(8)
        arr[0] = 0xFF;
        arr[1] = 0xFE;
        arr[2] = 0x61;
        arr[3] = 0x00;
        arr[4] = 0x62;
        arr[5] = 0x00
        arr[6] = 0x63;
        arr[7] = 0x00;
        var retStr = that.decode(arr, { stream : false })
        var BOM = '\uFEFF'
        var rel = 'abc'
        var re = BOM + rel
        expect(retStr).assertEqual(re)
    })

    // encode test
    it('test_get_encoding', 0, function () {
        var that = new util.TextEncoder()
        var str = that.encoding
        expect(str).assertEqual('utf-8')
    })

    it('test_encode_01', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(20)
        var result = new Uint8Array(buffer)
        result = that.encode('\uD800楼楼')
        expect(result[5]).assertEqual(188)
    })

    it('test_encode_02', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(20)
        var result = new Uint8Array(buffer)
        result = that.encode('a\uD800楼楼')
        expect(result[0]).assertEqual(0x61)
    })

    it('test_encode_03', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(20)
        var result = new Uint8Array(buffer)
        result = that.encode('abc\uD800楼楼')
        expect(result[1]).assertEqual(0x62)
    })

    it('test_encode_04', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(20)
        var result = new Uint8Array(buffer)
        result = that.encode('123\uD800楼楼')
        expect(result[0]).assertEqual(49)
        expect(result[9]).assertEqual(230)
    })

    it('test_encodeInto_01', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(20)
        var dest = new Uint8Array(buffer, 0, 13)
        var result = new Uint32Array(20)
        result = that.encodeInto('\uD800A\uDF06A楼HiA', dest)
        expect(result.read).assertEqual(7)
        expect(result.written).assertEqual(13)
    })

    it('test_encodeInto_02', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(6)
        var dest = new Uint8Array(buffer)
        var result = new Object()
        result = that.encodeInto('abc\u2603d', dest)
        expect(result.read).assertEqual(4)
        expect(result.written).assertEqual(6)
    })

    it('test_encodeInto_03', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(4)
        var dest = new Uint8Array(buffer)
        var result = new Object()
        result = that.encodeInto('abcd', dest)
        expect(result.read).assertEqual(4)
        expect(result.written).assertEqual(4)
    })

    it('test_encodeInto_04', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(4)
        var dest = new Uint8Array(buffer)
        var result = new Object()
        result = that.encodeInto('12345', dest)
        expect(result.read).assertEqual(4)
        expect(result.written).assertEqual(4)
    })

    it('test_encodeInto_05', 0, function () {
        var that = new util.TextEncoder()
        var buffer = new ArrayBuffer(4)
        var dest = new Uint8Array(buffer)
        var result = new Object()
        result = that.encodeInto('123 4*!@#', dest)
        expect(result.read).assertEqual(4)
        expect(result.written).assertEqual(4)
    })
})