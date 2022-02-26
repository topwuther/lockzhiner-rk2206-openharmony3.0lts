/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import factory from '@ohos.data.distributeddata'

const KEY_TEST_INT_ELEMENT = 'key_test_int';
const KEY_TEST_FLOAT_ELEMENT = 'key_test_float';
const KEY_TEST_BOOLEAN_ELEMENT = 'key_test_boolean';
const KEY_TEST_STRING_ELEMENT = 'key_test_string';

const VALUE_TEST_INT_ELEMENT = 123;
const VALUE_TEST_FLOAT_ELEMENT = 321.12;
const VALUE_TEST_BOOLEAN_ELEMENT = true;
const VALUE_TEST_STRING_ELEMENT = 'value-string-001';
const SINGLE_VERSION = 1;
const SECURITY_LEVEL = 3;

var kvManager = null;
var kvStore = null;

describe('KvStoreTest', function () {
    const config = {
        bundleName : 'ohos.acts.distributeddatamgr',
        userInfo : {
            userId : '0',
            userType : 0
        }
    }

    const options = {
        createIfMissing : true,
        encrypt : false,
        backup : false,
        autoSync : true,
        kvStoreType : SINGLE_VERSION,
        schema : '',
        securityLevel : SECURITY_LEVEL,
    }

    it('testCreateKVManager001', 0, async function (done) {
        try {
            const promise = factory.createKVManager(config);
            promise.then((manager) => {
                kvManager = manager;
                expect(false).assertEqual(manager == null);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetKVStore001', 0, async function (done) {
        try {
            const promise = kvManager.getKVStore(options);
            promise.then((store) => {
                kvStore = store;
                expect(false).assertEqual(store == null);
            }).catch((err) => {
                expect(null).assertFail();
            });
            expect(null).assertFail();
            await promise;
        } catch (e) {
        }
        done();
    })

    it('testGetKVStore001', 0, async function (done) {
        try {
            const promise = kvManager.getKVStore('storeId', options);
            promise.then((store) => {
                kvStore = store;
                expect(false).assertEqual(store == null);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString003', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT, null);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        } catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString004', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT, '');
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        } catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString0051', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT, VALUE_TEST_STRING_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        } catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetString001', 0, async function (done) {
        try {
            const promise = kvStore.get(KEY_TEST_STRING_ELEMENT);
            promise.then((data) => {
                expect(VALUE_TEST_STRING_ELEMENT).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch (e){
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString0021', 0, async function (done) {
        try {
            var str = '';
            for (var i = 0 ; i < 4095; i++) {
                str += 'x';
            }
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT+'002', str);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;

            const promise1 = kvStore.get(KEY_TEST_STRING_ELEMENT + '002');
            promise1.then((data) => {
                expect(str).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString005', 0, async function (done) {
        try {
            const promise1 = kvStore.get(KEY_TEST_STRING_ELEMENT + '005');
            promise1.then((data) => {
                expect(null).assertFail();
            }).catch((err) => {
            });
            await promise1;
        }catch(e) {
        }
        done();
    })

    it('testPutInt001', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT, VALUE_TEST_INT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetInt001', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT+ '001', VALUE_TEST_INT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.get(KEY_TEST_INT_ELEMENT+ '001');
            promise1.then((data) => {
                expect(VALUE_TEST_INT_ELEMENT).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutInt002', 0, async function (done) {
        try {
            var intValue = 987654321;
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT+'002', intValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;

            const promise1 = kvStore.get(KEY_TEST_INT_ELEMENT+'002');
            promise1.then((data) => {
                expect(intValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutInt003', 0, async function (done) {
        try{
            var intValue = Number.MAX_VALUE;
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT+'003', intValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;

            const promise1 = kvStore.get(KEY_TEST_INT_ELEMENT+'003');
            promise1.then((data) => {
                expect(intValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutInt004', 0, async function (done) {
        try{
            var intValue = Number.MIN_VALUE;
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT+'004', intValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;

            const promise1 = kvStore.get(KEY_TEST_INT_ELEMENT+'004');
            promise1.then((data) => {
                expect(intValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutBoolean001', 0, async function (done) {
        try{
            const promise = kvStore.put(KEY_TEST_BOOLEAN_ELEMENT, VALUE_TEST_BOOLEAN_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetBoolean002', 0, async function (done) {
        var boolValue = false;
        const promise = kvStore.put(KEY_TEST_BOOLEAN_ELEMENT+ '002', boolValue);
        promise.then((data) => {
        }).catch((err) => {
            expect(null).assertFail();
        });
        await promise;
        const promise1 = kvStore.get(KEY_TEST_BOOLEAN_ELEMENT+ '002');
        promise1.then((data) => {
            expect(boolValue).assertEqual(data);
        }).catch((err) => {
            expect(null).assertFail();
        });
        await promise1;
        done();
    })

    it('testPutFloat001', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT, VALUE_TEST_FLOAT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetFloat002', 0, async function (done) {
        try {
            var floatValue = 123456.654321;
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT+ '002', floatValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.get(KEY_TEST_FLOAT_ELEMENT+ '002');
            promise1.then((data) => {
                expect(floatValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetFloat003', 0, async function (done) {
        try {
            var floatValue = 123456.0;
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT+ '003', floatValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.get(KEY_TEST_FLOAT_ELEMENT+ '003');
            promise1.then((data) => {
                expect(floatValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetFloat004', 0, async function (done) {
        try {
            var floatValue = 123456.00;
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT+ '004', floatValue);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.get(KEY_TEST_FLOAT_ELEMENT+ '004');
            promise1.then((data) => {
                expect(floatValue).assertEqual(data);
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteString001', 0, async function (done) {
        try {
            var str = 'this is a test string';
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT+'003', str);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.delete(KEY_TEST_STRING_ELEMENT+'003');
            promise1.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteString002', 0, async function (done) {
        try {
            var str = '';
            for (var i = 0 ; i < 4096; i++) {
                str += 'x';
            }
            const promise = kvStore.put(KEY_TEST_STRING_ELEMENT+'003', str);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;

            const promise1 = kvStore.delete(KEY_TEST_STRING_ELEMENT+'003');
            promise1.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteString003', 0, async function (done) {
        try {
            const promise1 = kvStore.delete(KEY_TEST_STRING_ELEMENT+'unkonwn');
            promise1.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteInt001', 0, async function (done) {
        try {
            const promise = kvStore.delete(KEY_TEST_INT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteFloat001', 0, async function (done) {
        try {
            const promise = kvStore.delete(KEY_TEST_FLOAT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteFloat002', 0, async function (done) {
        try {
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT+'002', VALUE_TEST_FLOAT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            const promise1 = kvStore.delete(KEY_TEST_FLOAT_ELEMENT+'002');
            promise1.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise1;
            const promise2 = kvStore.delete(KEY_TEST_FLOAT_ELEMENT+'002');
            promise2.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise2;
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteBoolean001', 0, async function (done) {
        try {
            const promise = kvStore.delete(KEY_TEST_BOOLEAN_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange001', 0, async function (done) {
        try {
            kvStore.on('dataChange', 0, function (data) {
                console.log('testOnChange001 0' + data)
                expect(true).assertEqual(data !=null);
            });
            const promise = kvStore.put(KEY_TEST_FLOAT_ELEMENT, VALUE_TEST_FLOAT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange002', 0, async function (done) {
        try {
            kvStore.on('dataChange', 1, function (data) { //remote
                console.log('testOnChange001 1' + data)
                expect(true).assertEqual(data !=null);
            });
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT + '002', VALUE_TEST_INT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange003', 0, async function (done) {
        try {
            kvStore.on('dataChange', 2, function (data) {
                console.log('testOnChange001 2' + data)
                expect(true).assertEqual(data !=null);
            });
            const promise = kvStore.put(KEY_TEST_INT_ELEMENT + '003', VALUE_TEST_INT_ELEMENT);
            promise.then((data) => {
            }).catch((err) => {
                expect(null).assertFail();
            });
            await promise;
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })
})