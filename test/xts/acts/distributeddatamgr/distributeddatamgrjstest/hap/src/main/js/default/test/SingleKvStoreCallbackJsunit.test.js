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

const KEY_TEST_INT_ELEMENT = 'key_test_int_2';
const KEY_TEST_FLOAT_ELEMENT = 'key_test_float_2';
const KEY_TEST_BOOLEAN_ELEMENT = 'key_test_boolean_2';
const KEY_TEST_STRING_ELEMENT = 'key_test_string_2';

const VALUE_TEST_INT_ELEMENT = 1234;
const VALUE_TEST_FLOAT_ELEMENT = 4321.12;
const VALUE_TEST_BOOLEAN_ELEMENT = true;
const VALUE_TEST_STRING_ELEMENT = 'value-string-002';

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

    it('testCreateKVManager102', 0, async function (done) {
        try {
            await factory.createKVManager(null, function (err, manager) {
                expect(null).assertFail();
            });
        }catch (e) {
            console.log("testCreateKVManager102 fail " + e);
        }
        done();
    })

    it('testCreateKVManager103', 0, async function (done) {
        try {
            await factory.createKVManager('', function (err, manager) {
                expect(null).assertFail();
            });
        }catch(e) {
            console.log("testCreateKVManager103 fail " + e);
        }
        done();
    })

    it('testCreateKVManager101', 0, async function (done) {
        try {
            await factory.createKVManager(config, function (err, manager) {
                kvManager = manager;
                expect(false).assertEqual(manager == null);
                done();
            });
        }catch(e) {
            console.log("testCreateKVManager101 fail " + e);
            expect(null).assertFail();
        }
    })

    it('testGetKVStore101', 0, async function (done) {
        try {
            await kvManager.getKVStore(options, function (err, store) {
                expect(null).assertFail();
            });
        }catch(e) {
            console.log("testGetKVStore101 fail " + e);
        }
        done();
    })

    it('testGetKVStore102', 0, async function (done) {
        try {
            await kvManager.getKVStore('storeId', options, function (err, store) {
                kvStore = store;
                expect(false).assertEqual(store == null);
                done();
            });
        }catch(e) {
            console.log("testGetKVStore102 fail " + e);
            expect(null).assertFail();
        }
    })

    it('testPutString101', 0, async function (done) {
        try {
            await kvStore.put(KEY_TEST_STRING_ELEMENT, VALUE_TEST_STRING_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetString101', 0, async function (done) {
        try{
            await kvStore.get(KEY_TEST_STRING_ELEMENT, function (err,data) {
                expect(VALUE_TEST_STRING_ELEMENT).assertEqual(data);
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutString102', 0, async function (done) {
        try {
            var str = '';
            for (var i = 0 ; i < 4095; i++) {
                str += 'x';
            }
            await kvStore.put(KEY_TEST_STRING_ELEMENT+'102', str, function (err,data) {
                kvStore.get(KEY_TEST_STRING_ELEMENT+'102', function (err,data) {
                    expect(str).assertEqual(data);
                });
            });
        }catch (e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutInt101', 0, async function (done) {
        try {
            await kvStore.put(KEY_TEST_INT_ELEMENT, VALUE_TEST_INT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetInt101', 0, async function (done) {
        try {
            await kvStore.get(KEY_TEST_INT_ELEMENT, function (err,data) {
                expect(VALUE_TEST_INT_ELEMENT).assertEqual(data);
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutBoolean101', 0, async function (done) {
        try {
            await kvStore.put(KEY_TEST_BOOLEAN_ELEMENT, VALUE_TEST_BOOLEAN_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetBoolean101', 0, async function (done) {
        try {
            await kvStore.get(KEY_TEST_BOOLEAN_ELEMENT, function (err,data) {
                expect(VALUE_TEST_BOOLEAN_ELEMENT).assertEqual(data);
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testPutFloat101', 0, async function (done) {
        try {
            await kvStore.put(KEY_TEST_FLOAT_ELEMENT, VALUE_TEST_FLOAT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testGetFloat101', 0, async function (done) {
        try {
            await kvStore.get(KEY_TEST_FLOAT_ELEMENT, function (err,data) {
                expect(VALUE_TEST_FLOAT_ELEMENT).assertEqual(data);
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteString101', 0, async function (done) {
        try {
            await kvStore.delete(KEY_TEST_STRING_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteInt101', 0, async function (done) {
        try{
            await kvStore.delete(KEY_TEST_INT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteFloat101', 0, async function (done) {
        try{
            await kvStore.delete(KEY_TEST_FLOAT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testDeleteBoolean101', 0, async function (done) {
        try{
            await kvStore.delete(KEY_TEST_BOOLEAN_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange100', 0, async function (done) {
        try {
            kvStore.on('dataChange', 0, function (data) {
                expect(true).assertEqual(data !=null);
            });
            await kvStore.put(KEY_TEST_FLOAT_ELEMENT, VALUE_TEST_FLOAT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange101', 0, async function (done) {
        try {
            kvStore.on('dataChange', 1, function (data) {
                expect(true).assertEqual(data !=null);
            });
            await kvStore.put(KEY_TEST_FLOAT_ELEMENT, VALUE_TEST_FLOAT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })

    it('testOnChange102', 0, async function (done) {
        try {
            kvStore.on('dataChange', 2, function (data) {
                expect(true).assertEqual(data !=null);
            });
            await kvStore.put(KEY_TEST_FLOAT_ELEMENT+ '102', VALUE_TEST_FLOAT_ELEMENT, function (err,data) {
                if (err != undefined) {
                    expect(null).assertFail();
                }
            });
            await(3000);
        }catch(e) {
            expect(null).assertFail();
        }
        done();
    })
})