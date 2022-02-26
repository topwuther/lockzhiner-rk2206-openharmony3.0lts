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
import featureAbility from '@ohos.ability.featureAbility'
import ohos_data_ability from '@ohos.data.dataability'
import bundle from '@ohos.bundle'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('ActsDataAbilityHelperTest', function () {
    let bundleName = "com.ix.verify.act";
    //let abilityName = "VerifyActDataAbility";
    //let dataAbilityUri = ("dataability:///com.ohos.amsst.AppDataC.AmsStDataAbilityDataC1");
    let dataAbilityUri = ("dataability:///com.ix.VerifyActDataAbility");

    let DAHelper;
    let g_setTimeout = 100

    /*
    * @tc.number: bms_getJsAbility_0100
    * @tc.name: test the multi js ability
    * @tc.desc: 1.install a js hap which has an ability with full domain name
    *           2.check the ability name by the interface of getBundleInfo
    */
    it('bms_installAbility_0100', 0, async function (done) {
        console.info('bms_installAbility_0100====<begin');
        try{
            await install(['/system/vendor/amsStDataAbility.hap']);
            let data = await bundle.getBundleInfo(bundleName, 1);
            console.debug("=bms_installAbility_0100 getBundleInfo data====>"
            + (" json data【") + JSON.stringify(data)+ (" 】;"));
            //expect(data.abilityInfo.length).assertEqual(1);
            done();
        }catch(err) {
            console.error('=bms_installAbility_0100 install catch(err)====>:'+err);
            expect(false).assertTrue();
            done();
        }
        setTimeout(function () {
            console.info('=bms_installAbility_0100====> setTimeout');
        }, g_setTimeout)
        console.log('bms_installAbility_0100====<end')
    })
    async function install(bundlePath) {
        try{
            var installer = await bundle.getBundleInstaller();
            installer.install(bundlePath, {
                param: {
                    userId: 0,
                    installFlag: 1,
                    isKeepData: false
                }
            }, onReceiveInstallEvent);

            function onReceiveInstallEvent(err, data) {
                console.debug("=bms_installAbility_0100 onReceiveInstallEvent err,data====>"
                + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                expect(typeof err).assertEqual('object');
                expect(err.code).assertEqual(0);
                expect(typeof data).assertEqual('object');
                expect(data.status).assertEqual(0);
                expect(data.statusMessage).assertEqual('SUCCESS');
            }
        }catch(err) {
            console.error('=bms_installAbility_0100 getBundleInstaller catch(err)====>:'+err);
            expect(false).assertTrue();
        }
    }

/*
* @tc.number: ACTS_AcquireDataAbilityHelper_0100
* @tc.name: GetDataAbilityHelper : Connects an ability to a Service ability
* @tc.desc: Check the return value of the interface ()
*/
    it('ACTS_AcquireDataAbilityHelper_0100',0, async function (done) {
        console.log('ACTS_AcquireDataAbilityHelper_0100====<begin');
        console.debug("=ACTS_AcquireDataAbilityHelper_0100 dataAbilityUri====>"
        + (" json dataAbilityUri 【") + JSON.stringify(dataAbilityUri)+ (" 】; ====>"))
        let ret = false;
        try{
            var abilityHelper = featureAbility.acquireDataAbilityHelper(dataAbilityUri)
//                .then(function (data) {
//                console.debug("=ACTS_AcquireDataAbilityHelper_0100 then data====>"
//                + (" json data 【") + JSON.stringify(data)+ (" 】; ====>")+data);
//                expect(typeof(data)).assertEqual("object");
//                console.log('=ACTS_AcquireDataAbilityHelper_0100 promise JSON.stringify([object])====>:' + JSON.stringify(promise)+","+promise);
//                ret = true;
//                done()
//            }).catch(function (err){
//                console.debug("=ACTS_AcquireDataAbilityHelper_0100 catch err====>"
//                + ("json err 【") + JSON.stringify(err) + (" 】 ====>")+err);
//                console.log('=ACTS_AcquireDataAbilityHelper_0100 promise====>:' + JSON.stringify(promise))
//                ret = false;
//                done();
//            });
            DAHelper = abilityHelper;
            ret = true;
            done()
        }catch(err) {
            console.error('=ACTS_GetDataAbilityHelper_0100 acquireDataAbilityHelper catch(err)====>:'+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_AcquireDataAbilityHelper_0100====<end')
    })

/*
* @tc.number: ACTS_Insert_0100
* @tc.name: Insert : Indicates the path of the data to operate
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_Insert_0100', 0, async function (done) {
        console.log('ACTS_Insert_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('Insert_0100 DAHelper ====>: ' + JSON.stringify(DAHelper)+","+ DAHelper)
        let valueBucket
        try{
            DAHelper.insert(dataAbilityUri,valueBucket)
            .then(function (data){
                console.debug("=ACTS_Insert_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number");
                expect(data).assertEqual(1);
                ret = true;
                done();
            }).catch(function (err){
                console.debug("=ACTS_Insert_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false;
                done();
            });
        }catch(err) {
            console.error('=ACTS_Insert_0100 catch(err)====>:'+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Insert_0100====<end');

    })

/*
* @tc.number: ACTS_Insert_0200
* @tc.name: Insert : Indicates the path of the data to operate
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_Insert_0200', 0, async function (done) {
        console.log('ACTS_Insert_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getDataAbilityHelper ====>: ' + DAHelper)
        let valueBucket
        try{
            var asyncCallback = await DAHelper.insert(dataAbilityUri,valueBucket,
                (err,data)=>{
                    console.debug("=ACTS_Insert_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    expect(data).assertEqual(1);
                    console.log('=ACTS_Insert_0200 asyncCallback ====>:' + JSON.stringify(asyncCallback))
                    ret = true;
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_Insert_0200 catch(err)====>:'+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Insert_0200====<end');

    })

/*
* @tc.number: ACTS_Insert_0300
* @tc.name: Insert : Indicates the path of the data to operate
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_Insert_0300', 0, async function (done) {
        console.log('ACTS_Insert_0300====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('Insert_0300 DAHelper ====<: ' + JSON.stringify(DAHelper)+","+ DAHelper)
        const valueBucket = {
            "name": "ACTS_Insert_0300_rose",
            "age": 22,
            "salary": 200.5,
            "blobType": "u8",
        }
        try{
            var Promise = DAHelper.insert(dataAbilityUri,valueBucket)
                .then((data) => {
                console.debug("=ACTS_Insert_0300 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number");
                expect(data).assertEqual(1);
                ret = true;
                done();
            }).catch((err)=>{
                console.debug("=ACTS_Insert_0300 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                //debugAnsErrorCodePrint(err.code);
                ret = false;
                done();
            });
        }catch(err) {
            console.error('=ACTS_Insert_0300 catch(err)====>:'+err);
            ret = false;
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Insert_0300====<end');
    })

/*
* @tc.number: ACTS_Insert_0400
* @tc.name: Insert : Indicates the path of the data to operate
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_Insert_0400', 0, async function (done) {
        console.log('ACTS_Insert_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getDataAbilityHelper ====>: ' + DAHelper)
        const valueBucket = {
            "name": "ACTS_Insert_0400_rose1",
            "age": 221,
            "salary": 20.5,
            "blobType": "u8",
        }
        try{
            var asyncCallback = await DAHelper.insert(dataAbilityUri,valueBucket,
                (err,data)=>{
                    console.debug("=ACTS_Insert_0400 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    expect(data).assertEqual(1);
                    ret = true;
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_Insert_0400 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Insert_0400====<end');
    })

/*
* @tc.number: ACTS_BatchInsert_0100
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_BatchInsert_0100', 0, async function (done) {
        console.log('ACTS_BatchInsert_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        var valueBucket;
        try{
            DAHelper.batchInsert(
                dataAbilityUri,
                valueBucket
            ).then((data) => {
                console.debug("=ACTS_BatchInsert_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】; ====>"));
                expect(typeof(data)).assertEqual("number");
                ret = true;
                done();
            }).catch((err)=>{
                console.debug("=ACTS_BatchInsert_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_BatchInsert_0100  catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0100====<end');
    })

/*
* @tc.number: ACTS_BatchInsert_0200
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_BatchInsert_0200', 0, async function (done) {
        console.log('ACTS_BatchInsert_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        var valueBucket;
        try{
            await DAHelper.batchInsert(
                dataAbilityUri,
                valueBucket,
                (err,data) => {
                    console.debug("=ACTS_BatchInsert_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true;
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_BatchInsert_0200 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0200====<end');
    })

/*
* @tc.number: ACTS_BatchInsert_0300
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_BatchInsert_0300', 0, async function (done) {
        console.log('ACTS_BatchInsert_0300====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        var valueBucketArray = new Array({},{},{})
        try{
            DAHelper.batchInsert(
                dataAbilityUri,
                valueBucketArray,
            ).then((data) => {
                console.debug("=ACTS_BatchInsert_0300 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】; ====>"));
                expect(typeof(data)).assertEqual("number");
                ret = true;
                done();
            }).catch((err)=>{
                console.debug("=ACTS_BatchInsert_0300 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_BatchInsert_0300 batchInsert AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0300====<end');
    })

/*
* @tc.number: ACTS_BatchInsert_0400
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_BatchInsert_0400', 0, async function (done) {
        console.log('ACTS_BatchInsert_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        var valueBucketArray = new Array({},{},{})
        try{
            await DAHelper.batchInsert(
                dataAbilityUri,
                valueBucketArray,
                (err,data) => {
                    console.debug("=ACTS_BatchInsert_0400 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_BatchInsert_0400 catch(err)====>:'+err);
            ret = false
            done();
        }

        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0400====<end');
    })

    /*
* @tc.number: ACTS_BatchInsert_0500
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_BatchInsert_0500', 0, async function (done) {
        console.log('ACTS_BatchInsert_0500====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        try{
            const valueBucket = {
                "name": "ACTS_BatchInsert_0500_roe1",
                "age": 21,
                "salary": 20.5,
            }
            var valueBucketArray = new Array({"name": "BatchInsert_0500_roe11", "age": 21, "salary": 20.5, },
                {"name": "BatchInsert_0500_roe12", "age": 21, "salary": 20.5, },
                {"name": "BatchInsert_0500_roe13", "age": 21, "salary": 20.5, })

            DAHelper.batchInsert(
                dataAbilityUri,
                valueBucketArray,
            ).then((data) => {
                console.debug("=ACTS_BatchInsert_0500 BatchInsert Promise then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number");
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_BatchInsert_0500 BatchInsert Promise catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_BatchInsert_0500 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0500====<end');
    })

/*
* @tc.number: ACTS_BatchInsert_0600
* @tc.name: Inserts multiple data records into the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_BatchInsert_0600', 0, async function (done) {
        console.log('ACTS_BatchInsert_0600====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility BatchInsert getDataAbilityHelper ====>: ' + DAHelper)
        const valueBucket = {
            "name": "BatchInsert_0600_roe1",
            "age": 21,
            "salary": 20.5,
        }
        var valueBucketArray = new Array({"name": "BatchInsert_0600_roe11", "age": 21, "salary": 20.5, },
            {"name": "BatchInsert_0600_roe12", "age": 21, "salary": 20.5, },
            {"name": "BatchInsert_0600_roe13", "age": 21, "salary": 20.5, })
        try{
            await DAHelper.batchInsert(
                dataAbilityUri,
                valueBucketArray,
                (err,data) => {
                    console.debug("=ACTS_BatchInsert_0600 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_BatchInsert_0600 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_BatchInsert_0600====<end');
    })

///*
//* @tc.number: ACTS_Query_0100
//* @tc.name: Queries one or more data records in the database
//* @tc.desc: Check the return value of the interface (by Promise)
//*/
//    it('ACTS_Query_0100', 0, async function (done) {
//        console.log('ACTS_Query_0100====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        //var columnsArray = new Array({},{},{})
//        //var columnsArray = new Array({"name": "ACTS_Query_0100"})
//        var columnsArray = new Array({})
//        console.error('=ACTS_Query_0100 ====>:'
//        + ("json columnsArray 【") + JSON.stringify(columnsArray) + (" 】 ")+","+columnsArray);
////        const valueBucket = {
////            "name": "ACTS_Query_0100_roe1",
////        }
//        try{
//            //let predicates = new ohos_data_ability.DataAbilityPredicates()
//            let predicates = null
//            console.error('=ACTS_Query_0100 ====>:'
//            + ("json predicates 【") + JSON.stringify(predicates) + (" 】 ")+","+predicates);
//                    DAHelper.query(dataAbilityUri,columnsArray,predicates
//                    ).then((data) => {
//                        console.debug("=ACTS_Query_0100 then data====>"
//                        + ("json data 【") + JSON.stringify(data)+ (" 】"));
//                        expect(typeof(data)).assertEqual("number");
//                        ret = true
//                        expect(ret).assertTrue();
//                        done();
//                    }).catch((err)=>{
//                        console.debug("=ACTS_Query_0100 catch err ====>"
//                        + ("json err 【") + JSON.stringify(err) + (" 】 "));
//                        ret = false
//                        expect(ret).assertTrue();
//                        done();
//                    });
//        }catch(err) {
//            console.error('=ACTS_Query_0100 insert query AsyncCallback catch(err)====>:'+err);
//            ret = false
//            expect(ret).assertTrue();
//            done();
//        }
//        setTimeout(function(){
//            console.error('=ACTS_Query_0100 setTimeout ret====>:'+ret);
//        }, g_setTimeout);
//        console.log('ACTS_Query_0100====<end');
//    })
//
///*
//* @tc.number: ACTS_Query_0200
//* @tc.name: Queries one or more data records in the database
//* @tc.desc: Check the return value of the interface (by AsyncCallback)
//*/
//    it('ACTS_Query_0200', 0, async function (done) {
//        console.log('ACTS_Query_0200====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        var columnsArray = new Array({},{},{})
//        let predicates = null
//        console.log('featureAbility Query getDataAbilityHelper ====>: ' + DAHelper)
//        try{
//            await DAHelper.query(
//                dataAbilityUri,
//                columnsArray,
//                predicates,
//                (err,data) => {
//                    console.debug("=ACTS_Query_0200 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                    expect(typeof(data)).assertEqual("object");
//                    ret = true
//                    done();
//                },
//            );
//        }catch(err) {
//            console.error('=ACTS_Query_0200 catch(err)====>:'+err);
//            ret = false
//            done();
//        }
//        setTimeout(function(){
//            expect(ret).assertTrue();
//        }, g_setTimeout);
//        console.log('ACTS_Query_0200====<end');
//    })

/*
* @tc.number: ACTS_Query_0300
* @tc.name: Queries one or more data records in the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_Query_0300', 0, async function (done) {
        console.log('ACTS_Query_0300====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        var columnsArray = new Array("value1", "value2", "value3", "value4")
        let predicates = new ohos_data_ability.DataAbilityPredicates()
        try{
            let queryPromise = DAHelper.query(
                dataAbilityUri,
                columnsArray,
                predicates
            ).then((data) => {
                console.debug("=ACTS_Query_0300 Query then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】") +" , " + data);
                expect(typeof(data)).assertEqual("object");
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_Query_0300 Query catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
            console.debug("=ACTS_Query_0300 queryPromise ====>"
            + ("json queryPromise 【") + JSON.stringify(queryPromise) + (" 】 "));
        }catch(err) {
            console.error('=ACTS_Query_0300 query catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Query_0300====<end');
    })
//
/*
* @tc.number: ACTS_Query_0400
* @tc.name: Queries one or more data records in the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_Query_0400', 0, async function (done) {
        console.log('ACTS_Query_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        var columnsArray = new Array("ACTS_Query_0400_roe1", "value2", "value3", "value4")
        let predicates = new ohos_data_ability.DataAbilityPredicates()
        try{
            await DAHelper.query(
                dataAbilityUri,
                columnsArray,
                predicates,
                (err,data) => {
                    console.debug("=ACTS_Query_0400 query err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;") + " , " +data);
                    expect(typeof(data)).assertEqual("object");
                    ret = true
                    done();
                },);
        }catch(err) {
            console.error('=ACTS_Query_0400 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Query_0400====<end');
    })
//
////    /*
////* @tc.number: ACTS_Update_0100
////* @tc.name: Updates one or more data records in the database.
////* @tc.desc: Check the return value of the interface (by Promise)
////*/
////    it('ACTS_Update_0100', 0, async function (done) {
////        console.log('ACTS_Update_0100====<begin');
////        let ret = false;
////        expect(typeof(DAHelper)).assertEqual("object");
////        console.log('featureAbility Update getDataAbilityHelper ====>: ' + DAHelper)
////        const valueBucket = {
////            "name": "ACTS_Update_0100_roe1",
////        }
////        try{
////            //let predicates = new ohos_data_ability.DataAbilityPredicates()
////            let valueBucketNull = null
////            let predicates = null
////            DAHelper.update(
////                dataAbilityUri,
////                valueBucketNull,
////                predicates
////            ).then((data) => {
////                console.debug("=ACTS_Update_0100 then data====>"
////                + ("json data 【") + JSON.stringify(data)+ (" 】"));
////                expect(typeof(data)).assertEqual("number");
////                ret = true
////                done();
////            }).catch((err)=>{
////                console.debug("=ACTS_Update_0100 catch err ====>"
////                + ("json err 【") + JSON.stringify(err) + (" 】 "));
////                ret = false
////                done();
////            });
////        }catch(err) {
////            console.error('=ACTS_Update_0100 catch(err)====>:'+err);
////            ret = false
////            done();
////        }
////        setTimeout(function(){
////            expect(ret).assertTrue();
////        }, g_setTimeout);
////        console.log('ACTS_Update_0100====<end');
////    })
////
/////*
////* @tc.number: ACTS_Update_0200
////* @tc.name: Updates one or more data records in the database.
////* @tc.desc: Check the return value of the interface (by AsyncCallback)
////*/
////    it('ACTS_Update_0200', 0, async function (done) {
////        console.log('ACTS_Update_0200====<begin');
////        let ret = false;
////        expect(typeof(DAHelper)).assertEqual("object");
////        console.log('featureAbility Update getDataAbilityHelper ====>: ' + DAHelper)
////        const valueBucket = {
////            "name": "ACTS_Update_0200_roe1",
////            "age": 21,
////            "salary": 20.5,
////
////        }
////        try{
////            //let predicates = new ohos_data_ability.DataAbilityPredicates()
////            let valueBucketNull = null
////            let predicates = null
////            await DAHelper.update(
////                dataAbilityUri,
////                valueBucketNull,
////                predicates,
////                (err,data) => {
////                    console.debug("=ACTS_Update_0200 err,data=======>"
////                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
////                    expect(typeof(data)).assertEqual("number");
////                    ret = true
////                    done();
////                },
////            );
////        }catch(err) {
////            console.error('=ACTS_Update_0200 catch(err)====>:'+err);
////            ret = false
////            done();
////        }
////        setTimeout(function(){
////            expect(ret).assertTrue();
////        }, g_setTimeout);
////        console.log('ACTS_Update_0200====<end');
////    })
//
//    /*
//* @tc.number: ACTS_Update_0300
//* @tc.name: Updates one or more data records in the database.
//* @tc.desc: Check the return value of the interface (by Promise)
//*/
//    it('ACTS_Update_0300', 0, async function (done) {
//        console.log('ACTS_Update_0300====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        console.log('featureAbility Update getDataAbilityHelper ====>: ' + DAHelper)
//        const valueBucket = {"name": "ACTS_Update_0300_roe1","age": 21,"salary": 20.5,}
//        try{
//            let predicates = new ohos_data_ability.DataAbilityPredicates()
//            DAHelper.update(
//                dataAbilityUri,
//                valueBucket,
//                predicates
//            ).then((data) => {
//                console.debug("=ACTS_Update_0300 then data====>"
//                + ("json data 【") + JSON.stringify(data)+ (" 】"));
//                expect(typeof(data)).assertEqual("number");
//                ret = true
//                done();
//            }).catch((err)=>{
//                console.debug("=ACTS_Update_0300 catch err ====>"
//                + ("json err 【") + JSON.stringify(err) + (" 】 "));
//                ret = false
//                done();
//            });
//        }catch(err) {
//            console.error('=ACTS_Update_0300 update catch(err)====>:'+err);
//            ret = false
//            done();
//        }
//        setTimeout(function(){
//            expect(ret).assertTrue();
//        }, g_setTimeout);
//        console.log('ACTS_Update_0300====<end');
//    })

//    /*
//* @tc.number: ACTS_Update_0400
//* @tc.name: Updates one or more data records in the database.
//* @tc.desc: Check the return value of the interface (by AsyncCallback)
//*/
//    it('ACTS_Update_0400', 0, async function (done) {
//        console.log('ACTS_Update_0400====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        console.log('featureAbility Update getDataAbilityHelper ====>: ' + DAHelper)
//        const valueBucket = {"name": "ACTS_Update_0400_roe1","age": 21,"salary": 20.5,}
//        try{
//            let predicates = new ohos_data_ability.DataAbilityPredicates()
//            await DAHelper.update(
//                dataAbilityUri,
//                valueBucket,
//                predicates,
//                (err,data) => {
//                    console.debug("=ACTS_Update_0400 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                    expect(typeof(data)).assertEqual("number");
//                    ret = true
//                    done();
//                },
//            );
//        }catch(err) {
//            console.error('=ACTS_Update_0400 catch(err)====>:'+err);
//            ret = false
//            done();
//        }
//        setTimeout(function(){
//            expect(ret).assertTrue();
//        }, g_setTimeout);
//        console.log('ACTS_Update_0400====<end');
//    })

///*
//* @tc.number: ACTS_Delete_0100
//* @tc.name: Deletes one or more data records. This method should be implemented by a Data ability.
//* @tc.desc: Check the return value of the interface (by Promise)
//*/
//    it('ACTS_Delete_0100', 0, async function (done) {
//        console.log('ACTS_Delete_0100====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        console.log('featureAbility getDataAbilityHelper ====>: ' + DAHelper)
//        try{
//            let predicates = null
//            DAHelper.delete(
//                dataAbilityUri,
//                predicates
//            ).then((data) => {
//                console.debug("=ACTS_Delete_0100 then data====>"
//                + ("json data 【") + JSON.stringify(data)+ (" 】"));
//                expect(typeof(data)).assertEqual("number");
//                ret = true
//                done();
//            }).catch((err)=>{
//                console.debug("=ACTS_Delete_0100 catch err ====>"
//                + ("json err 【") + JSON.stringify(err) + (" 】 "));
//                ret = false
//                done();
//            });
//        }catch(err) {
//            console.error('=ACTS_Delete_0100 catch(err)====>:'+err);
//            ret = false
//            done();
//        }
//        setTimeout(function(){
//            expect(ret).assertTrue();
//        }, g_setTimeout);
//        console.log('ACTS_Delete_0100====<end');
//    })

///*
//* @tc.number: ACTS_Delete_0200
//* @tc.name: Deletes one or more data records. This method should be implemented by a Data ability.
//* @tc.desc: Check the return value of the interface (by AsyncCallback)
//*/
//    it('ACTS_Delete_0200', 0, async function (done) {
//        console.log('ACTS_Delete_0200====<begin');
//        let ret = false;
//        expect(typeof(DAHelper)).assertEqual("object");
//        console.log('featureAbility getDataAbilityHelper ====>: ' + DAHelper)
//        try{
//            let predicates = null
//            await DAHelper.delete(
//                dataAbilityUri,
//                predicates,
//                (err,data) => {
//                    console.debug("=ACTS_Delete_0200 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                    expect(typeof(data)).assertEqual("number");
//                    expect(data).assertEqual(0);
//                    ret = true
//                    done();
//                },
//            );
//        }catch(err) {
//            console.error('=ACTS_Delete_0200 catch(err)====>:'+err);
//            ret = false
//            done();
//        }
//        setTimeout(function(){
//            expect(ret).assertTrue();
//        }, g_setTimeout);
//        console.log('ACTS_Delete_0200====<end');
//    })

/*
* @tc.number: ACTS_Delete_0300
* @tc.name: Deletes one or more data records. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_Delete_0300', 0, async function (done) {
        console.log('ACTS_Delete_0300====<begin');
        let ret = false;
        try{
            let predicates = new ohos_data_ability.DataAbilityPredicates()
            var datadelete = DAHelper.delete(
                dataAbilityUri,
                predicates
            ).then((data) => {
                console.debug("=ACTS_Delete_0300 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number");
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_Delete_0300 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
            console.debug("=ACTS_Delete_0300 datadelete====>"
            + ("json datadelete 【") + JSON.stringify(datadelete)+ (" 】")+" , " + datadelete);
        }catch(err) {
            console.error('=ACTS_Delete_0300 catch(err)====>:'+err);
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Delete_0300====<end');
    })

/*
* @tc.number: ACTS_Delete_0400
* @tc.name: Deletes one or more data records. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_Delete_0400', 0, async function (done) {
        console.log('ACTS_Delete_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getDataAbilityHelper ====>: ' + DAHelper)
        try{
            let predicates = new ohos_data_ability.DataAbilityPredicates()
            await DAHelper.delete(
                dataAbilityUri,
                predicates,
                (err,data) => {
                    console.debug("=ACTS_Delete_0400 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true
                    done();
                });
        }catch(err) {
            console.error('=ACTS_Delete_0400 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Delete_0400====<end');
    })

/*
* @tc.number: ACTS_GetFileTypes_0100
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_GetFileTypes_0100', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0100====<begin');
        let ret = false;
        let mimeTypeFilter='*/*'
        try{
            var promise = DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
            ).then((data) =>{
                console.debug("=ACTS_GetFileTypes_0100 getFileTypes then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                console.log('DataAbilityHelper getFileTypes data.length ====>: ' + data.length);
                for(var i = 0; i < data.length; i++) {
                    expect(typeof(data[i])).assertEqual("string");
                    console.log('=ACTS_GetFileTypes_0100 for data[' + i + '] ====>: ' + data[i])
                    expect(data[i]).assertEqual("");
                }
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_GetFileTypes_0100 getFileTypes catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
            console.log('featureAbility getFileTypes promise ====>: ' + promise)
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0100 getFileTypes AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0100====<end');
    })

/*
* @tc.number: ACTS_GetFileTypes_0200
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_GetFileTypes_0200', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getFileTypes getDataAbilityHelper ====>: ' + DAHelper)
        let mimeTypeFilter='*/*'
        try{
            await DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
                (err,data) => {
                    console.debug("=ACTS_GetFileTypes_0200 getFileTypes err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    console.log('=ACTS_GetFileTypes_0200 data.length ====>: ' + data.length);
                    for(var i = 0; i < data.length; i++) {
                        expect(typeof(data[i])).assertEqual("string");
                        console.log('=ACTS_GetFileTypes_0200 for data ====>: ' + err.code +" data[" + i + "]: " + data[i]);
                        expect(data[i]).assertEqual("");
                    }
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0200 getFileTypes AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0200====<end');
    })

    /*
* @tc.number: ACTS_GetFileTypes_0300
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_GetFileTypes_0300', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0300====<begin');
        let ret = false;
        let mimeTypeFilter='image/*'
        try{
            var promise = DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
            ).then((data) =>{
                console.debug("=ACTS_GetFileTypes_0300  then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                console.log('DataAbilityHelper getFileTypes data.length ====>: ' + data.length);
                for(var i = 0; i < data.length; i++) {
                    expect(typeof(data[i])).assertEqual("string");
                    console.log('= =ACTS_GetFileTypes_0300 for data[' + i + '] ====>: ' + data[i])
                    expect(data[i]).assertEqual("");
                }
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_GetFileTypes_0300 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                console.log('DataAbilityHelper getFileTypes error ====>: ' + err)
                ret = false
                done();
            });
            console.log('featureAbility getFileTypes promise ====>: ' + promise)
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0300 getFileTypes AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0300====<end');
    })

    /*
* @tc.number: ACTS_GetFileTypes_0400
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_GetFileTypes_0400', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getFileTypes getDataAbilityHelper ====>: ' + DAHelper)
        let mimeTypeFilter='image/*'
        try{
            await DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
                (err,data) => {
                    console.debug("=ACTS_GetFileTypes_0400 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    console.log('DataAbilityHelper  getFileTypes data.length ====>: ' + data.length);
                    for(var i = 0; i < data.length; i++) {
                        expect(typeof(data[i])).assertEqual("string");
                        console.log('=ACTS_GetFileTypes_0400 for ====>: ' + err.code +" data[" + i + "]: " + data[i]);
                        expect(data[i]).assertEqual("");
                    }
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0400 getFileTypes AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0400====<end');
    })
    /*
* @tc.number: ACTS_GetFileTypes_0500
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_GetFileTypes_0500', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0500====<begin');
        let ret = false;
        let mimeTypeFilter='*/jpg'
        try{
            var promise = DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
            ).then((data) =>{
                console.debug("=ACTS_GetFileTypes_0500 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                console.log('DataAbilityHelper getFileTypes data.length ====>: ' + data.length);
                for(var i = 0; i < data.length; i++) {
                    expect(typeof(data[i])).assertEqual("string");
                    console.log('=ACTS_GetFileTypes_0500 for data [' + i + '] ====>: ' + data[i])
                    expect(data[i]).assertEqual("");
                }
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_GetFileTypes_0500  catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                console.log('DataAbilityHelper getFileTypes error ====>: ' + err)
                ret = false
                done();
            });
            console.log('=ACTS_GetFileTypes_0500 promise ====>: ' + promise)
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0500 getFileTypes AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0500====<end');
    })

    /*
* @tc.number: ACTS_GetFileTypes_0600
* @tc.name: Obtains the MIME type of files.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_GetFileTypes_0600', 0, async function (done) {
        console.log('ACTS_GetFileTypes_0600====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility getFileTypes getDataAbilityHelper ====>: ' + DAHelper)
        let mimeTypeFilter='*/jpg'
        try{
            await DAHelper.getFileTypes(
                dataAbilityUri,
                mimeTypeFilter,
                (err,data) => {
                    console.debug("=ACTS_GetFileTypes_0600 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    console.log('=ACTS_GetFileTypes_0600 data.length ====>: ' + data.length);
                    for(var i = 0; i < data.length; i++) {
                        expect(typeof(data[i])).assertEqual("string");
                        console.log('=ACTS_GetFileTypes_0600 for errCode ====>: ' + err.code +" data[" + i + "]: " + data[i]);
                        expect(data[i]).assertEqual("");
                    }
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_GetFileTypes_0600 catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetFileTypes_0600====<end');
    })

/*
* @tc.number: ACTS_GetType_0100
* @tc.name: Obtains the MIME type matching the data specified by the URI of the Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_GetType_0100', 0, async function (done) {
        console.log('ACTS_GetType_0100====<begin');
        let ret = false;
        try{
            var promise = DAHelper.getType(
                dataAbilityUri,
            ).then(data =>{
                console.debug("=ACTS_GetType_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("string")
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_GetFileTypes_0500 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
            console.log('featureAbility getType promise ====>: ' + promise)
        }catch(err) {
            console.error('=ACTS_GetType_0100 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetType_0100====<end');
    })

/*
* @tc.number: ACTS_GetType_0200
* @tc.name: Obtains the MIME type matching the data specified by the URI of the Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_GetType_0200', 0, async function (done) {
        console.log('ACTS_GetType_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility GetType getDataAbilityHelper ====>: ' + DAHelper)
        try{
            await DAHelper.getType(
                dataAbilityUri,
                (err,data) => {
                    console.debug("=ACTS_GetType_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("string");
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_GetType_0200 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_GetType_0200====<end');
    })

/*
* @tc.number: ACTS_OpenFile_0100
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0100', 0, async function (done) {
        console.log('ACTS_OpenFile_0100====<begin');
        let ret = false;
        var mode = "r";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0100 getType catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0100====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_0200
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0200', 0, async function (done) {
        console.log('ACTS_OpenFile_0200====<begin');
        let ret = false;
        var mode = "w";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0200 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                console.log('DataAbilityHelper ACTS_OpenFile_0200 OpenFile promise ====>: ' + data)
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0200 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0200 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0200====<end');
    })
    /*
* @tc.number: ACTS_OpenFile_0300
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0300', 0, async function (done) {
        console.log('ACTS_OpenFile_0300====<begin');
        let ret = false;
        var mode = "wt";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0300 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                console.log('DataAbilityHelper ACTS_OpenFile_0300 OpenFile promise ====>: ' + data)
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0300 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0300 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0300====<end');
    })
    /*
* @tc.number: ACTS_OpenFile_0400
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0400', 0, async function (done) {
        console.log('ACTS_OpenFile_0400====<begin');
        let ret = false;
        var mode = "wa";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0400 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                console.log('DataAbilityHelper ACTS_OpenFile_0400 OpenFile promise ====>: ' + data)
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0400 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0400 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0400====<end');
    })
    /*
* @tc.number: ACTS_OpenFile_0500
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0500', 0, async function (done) {
        console.log('ACTS_OpenFile_0500====<begin');
        let ret = false;
        var mode = "rw";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0500 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                console.log('DataAbilityHelper ACTS_OpenFile_0500 OpenFile promise ====>: ' + data)
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0500 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0500 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0500====<end');
    })
    /*
* @tc.number: ACTS_OpenFile_0600
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_OpenFile_0600', 0, async function (done) {
        console.log('ACTS_OpenFile_0600====<begin');
        let ret = false;
        var mode = "rwt";
        try{
            var promise = DAHelper.openFile(
                dataAbilityUri,
                mode,
            ).then((data) =>{
                console.debug("=ACTS_OpenFile_0600 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("number")
                console.log('DataAbilityHelper ACTS_OpenFile_0600 OpenFile promise ====>: ' + data)
                ret = true
                done();
            }).catch(err => {
                console.debug("=ACTS_OpenFile_0600 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_OpenFile_0600 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0600====<end');
    })
/*
* @tc.number: ACTS_OpenFile_0700
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_0700', 0, async function (done) {
        console.log('ACTS_OpenFile_0700====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_0700 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "r";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_0700 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    console.log('DataAbilityHelper ACTS_OpenFile_0700 OpenFile asyncCallback errCode ====>: ' + err.code +" data: " + data);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_0700 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0700====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_0800
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_0800', 0, async function (done) {
        console.log('ACTS_OpenFile_0800====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_0800 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "w";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_0800 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    console.log('DataAbilityHelper ACTS_OpenFile_0800 OpenFile asyncCallback errCode ====>: ' + err.code +" data: " + data);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_0800 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0800====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_0900
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_0900', 0, async function (done) {
        console.log('ACTS_OpenFile_0900====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_0900 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "wt";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_0900 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    console.log('DataAbilityHelper ACTS_OpenFile_0900 OpenFile asyncCallback errCode ====>: ' + err.code +" data: " + data);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_0900 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_0900====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_1000
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_1000', 0, async function (done) {
        console.log('ACTS_OpenFile_1000====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_1000 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "wa";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_1000 err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    console.log('DataAbilityHelper ACTS_OpenFile_1000 OpenFile asyncCallback errCode ====>: ' + err.code +" data: " + data);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_1000 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_1000====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_1100
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_1100', 0, async function (done) {
        console.log('ACTS_OpenFile_1100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_1100 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "rw";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_1100 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_1100 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_1100====<end');
    })

    /*
* @tc.number: ACTS_OpenFile_1200
* @tc.name: Opens a file. This method should be implemented by a Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_OpenFile_1200', 0, async function (done) {
        console.log('ACTS_OpenFile_1200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_OpenFile_1200 OpenFile getDataAbilityHelper ====>: ' + DAHelper)
        var mode = "rwt";
        try{
            DAHelper.openFile(
                dataAbilityUri,
                mode,
                (err,data) => {
                    console.debug("=ACTS_OpenFile_1200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("number");
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_OpenFile_1200 getType AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_OpenFile_1200====<end');
    })

/*
* @tc.number: ACTS_Release_0100
* @tc.name: Releases the client resource of the Data ability.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_Release_0100', 0, async function (done) {
        console.log('ACTS_Release_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_Release_0100 getDataAbilityHelper ====>: ' + DAHelper)
        try{
            var promise = DAHelper.release(
                dataAbilityUri,
            ).then((data) =>{
                console.debug("=ACTS_Release_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("boolean")
                ret = true
                done();
            }).catch(err =>{
                console.debug("=ACTS_Release_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_Release_0100 release promise catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Release_0100====<end');
    })

/*
* @tc.number: ACTS_Release_0200
* @tc.name: Releases the client resource of the Data ability.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_Release_0200', 0, async function (done) {
        console.log('ACTS_Release_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility ACTS_Release_0200 getDataAbilityHelper ====>: ' + DAHelper)
        try{
            DAHelper.release(
                dataAbilityUri,
                (err,data) => {
                    console.debug("=ACTS_Release_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    console.log('featureAbility  getDataAbilityHelper ACTS_Release_0100  data: ' + data)
                    expect(typeof(data)).assertEqual("boolean");
                    console.log('DataAbilityHelper ACTS_Release_0200 asyncCallback errCode ====>: ' + err.code +" data: " + data);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_Release_0200 release AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_Release_0200====<end');
    })

/*
* @tc.number: ACTS_NormalizeUri_0100
* @tc.name: Converts the given uri that refer to the Data ability into a normalized URI.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_NormalizeUri_0100', 0, async function (done) {
        console.log('ACTS_NormalizeUri_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility normalizeUri getDataAbilityHelper ====>: ' + DAHelper)
        try{
            var promise = DAHelper.normalizeUri(
                dataAbilityUri,
            ).then((data) =>{
                console.debug("=ACTS_NormalizeUri_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("string")
                //expect(data).assertEqual("dataability:///com.ix.VerifyActDataAbility");
                ret = true
                done();
            }).catch(err =>{
                console.debug("=ACTS_NormalizeUri_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_NormalizeUri_0100 normalizeUri promise catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_NormalizeUri_0100====<end');
    })


 /*
* @tc.number: ACTS_NormalizeUri_0200
* @tc.name: Converts the given uri that refer to the Data ability into a normalized URI.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_NormalizeUri_0200', 0, async function (done) {
        console.log('ACTS_NormalizeUri_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility normalizeUri getDataAbilityHelper ====>: ' + DAHelper)
        try{
            DAHelper.normalizeUri(
                dataAbilityUri,
                (err,data) => {
                    console.debug("=ACTS_NormalizeUri_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("string");
                    expect(data).assertEqual(dataAbilityUri);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_NormalizeUri_0200 normalizeUri AsyncCallback catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_NormalizeUri_0200====<end');
    })

/*
* @tc.number: ACTS_DenormalizeUri_0100
* @tc.name: Converts the given normalized uri generated by normalizeUri into a denormalized one.
* @tc.desc: Check the return value of the interface (by promise)
*/
    it('ACTS_DenormalizeUri_0100', 0, async function (done) {
        console.log('ACTS_DenormalizeUri_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility DenormalizeUri getDataAbilityHelper ====>: ' + DAHelper)
        try{
            var promise = DAHelper.denormalizeUri(
                dataAbilityUri,
            ).then((data) =>{
                console.debug("=ACTS_DenormalizeUri_0100 then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                expect(typeof(data)).assertEqual("string")
                expect(data).assertEqual(dataAbilityUri);
                ret = true
                done();
            }).catch(err =>{
                console.debug("=ACTS_DenormalizeUri_0100 catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_DenormalizeUri_0100 denormalizeUri promise catch(err)====>:'+err);
            ret = false
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_DenormalizeUri_0100====<end');
    })

/*
* @tc.number: ACTS_DenormalizeUri_0200
* @tc.name: Converts the given normalized uri generated by normalizeUri into a denormalized one.
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_DenormalizeUri_0200', 0, async function (done) {
        console.log('ACTS_DenormalizeUri_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        console.log('featureAbility DenormalizeUri getDataAbilityHelper ====>: ' + DAHelper)
        try{
            DAHelper.denormalizeUri(
                dataAbilityUri,
                (err,data) => {
                    console.debug("=ACTS_DenormalizeUri_0200 err,data=======>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
                    expect(typeof(data)).assertEqual("string");
                    expect(data).assertEqual(dataAbilityUri);
                    ret = true
                    done();
                },
            );
        }catch(err) {
            console.error('=ACTS_DenormalizeUri_0200 denormalizeUri AsyncCallback catch(err)====>:'+err);
            ret = true
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_DenormalizeUri_0200====<end');
    })

//    var isFlagCallback01 = 0
//    var isFlagCallback02 = 0
//    var isFlagCallback03 = 0
//    function asyncCallback01(err,data){
//        console.debug("=ACTS_OnOff_ asyncCallback01 err,data=======>"
//        + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//        isFlagCallback01++;
//        console.debug("=ACTS_OnOff_ asyncCallback01 isFlagCallback01=======>"
//        + ("json isFlagCallback01【") + JSON.stringify(isFlagCallback01) + (" 】")+" , "+isFlagCallback01);
//    }
//    function asyncCallback02(err,data){
//        console.debug("=ACTS_OnOff_ asyncCallback02 err,data=======>"
//        + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//        isFlagCallback02++;
//        console.debug("=ACTS_OnOff_ asyncCallback02 isFlagCallback02=======>"
//        + ("json isFlagCallback02【") + JSON.stringify(isFlagCallback02) + (" 】")+" , "+isFlagCallback02);
//    }
//    function asyncCallback03(err,data){
//        console.debug("=ACTS_OnOff_ asyncCallback03 err,data=======>"
//        + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//        isFlagCallback02++;
//        console.debug("=ACTS_OnOff_ asyncCallback03 isFlagCallback03=======>"
//        + ("json isFlagCallback03【") + JSON.stringify(isFlagCallback03) + (" 】")+" , "+isFlagCallback03);
//    }
///*
//* @tc.number: ACTS_OnOff_0100
//* @tc.name: On/Off : Registers an observer to observe data specified by the given Uri
//* @tc.desc: Check the return value of the interface ()
//*/
//    it('ACTS_OnOff_0100', 0, async function (done) {
//        console.log('ACTS_OnOff_0100====<begin');
//        let ret = false;
//        try{
//            expect(typeof(DAHelper)).assertEqual("object");
//            await DAHelper.on("dataChange",dataAbilityUri,asyncCallback01);
//            await DAHelper.notifyChange(
//                dataAbilityUri,
//                (err,data) => {
//                    console.debug("=ACTS_OnOff_0100 notifyChange 1 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                },);
//            setTimeout(function(){console.log('ACTS_OnOff_0100====<setTimeout notifyChange 1');}, 5000);
//            console.debug("=ACTS_OnOff_0100 isFlagCallback01=======>"
//            + ("json isFlagCallback01【") + JSON.stringify(isFlagCallback01) + (" 】")+" , "+isFlagCallback01);
//            expect(isFlagCallback01).assertEqual(1);
//            await DAHelper.off("dataChange",dataAbilityUri,asyncCallback01);
//            await DAHelper.notifyChange(
//                dataAbilityUri,
//                (err,data) => {
//                    console.debug("=ACTS_OnOff_0100 notifyChange 2 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                },);
//            setTimeout(function(){console.log('ACTS_OnOff_0100====<setTimeout notifyChange 2');}, 5000);
//            console.debug("=ACTS_OnOff_0100 isFlagCallback01=======>"
//            + ("json isFlagCallback01【") + JSON.stringify(isFlagCallback01) + (" 】")+" , "+isFlagCallback01);
//            expect(isFlagCallback01).assertEqual(1);
//        }catch(err) {console.error('=ACTS_OnOff_0100  catch(err)====>:'+err); }
//        setTimeout(function(){console.log('ACTS_OnOff_0100====<setTimeout');}, g_setTimeout);
//        console.log('ACTS_OnOff_0100====<end');
//        done();
//    })
//
//    /*
//* @tc.number: ACTS_OnOff_0200
//* @tc.name: On/Off : Registers an observer to observe data specified by the given Uri
//* @tc.desc: Check the return value of the interface (by AsyncCallback)
//*/
//    it('ACTS_OnOff_0200', 0, async function (done) {
//        console.log('ACTS_OnOff_0200====<begin');
//        try{
//            await DAHelper.on("dataChange",dataAbilityUri,asyncCallback01);
//            await DAHelper.on("dataChange",dataAbilityUri,asyncCallback02);
//            await DAHelper.on("dataChange",dataAbilityUri,asyncCallback03);
//            await DAHelper.notifyChange(dataAbilityUri,
//                (err,data) => {
//                    console.debug("=ACTS_OnOff_0200 notifyChange 1 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                });
//            setTimeout(function(){console.log('ACTS_OnOff_0100====<setTimeout notifyChange 1');}, 5000);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback01=======>"
//            + ("json isFlagCallback01【") + JSON.stringify(isFlagCallback01) + (" 】")+" , "+isFlagCallback01);
//            expect(isFlagCallback01).assertEqual(1);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback02=======>"
//            + ("json isFlagCallback02【") + JSON.stringify(isFlagCallback02) + (" 】")+" , "+isFlagCallback02);
//            expect(isFlagCallback02).assertEqual(1);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback03=======>"
//            + ("json isFlagCallback03【") + JSON.stringify(isFlagCallback03) + (" 】")+" , "+isFlagCallback03);
//            expect(isFlagCallback03).assertEqual(1);
//            await DAHelper.off("dataChange",dataAbilityUri,asyncCallback02);
//            await DAHelper.notifyChange(dataAbilityUri,
//                (err,data) => {
//                    console.debug("=ACTS_OnOff_0200 notifyChange 2 err,data=======>"
//                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;"));
//                },);
//            setTimeout(function(){console.log('ACTS_OnOff_0200====<setTimeout notifyChange 2');}, 5000);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback01=======>"
//            + ("json isFlagCallback01【") + JSON.stringify(isFlagCallback01) + (" 】")+" , "+isFlagCallback01);
//            expect(isFlagCallback01).assertEqual(2);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback02=======>"
//            + ("json isFlagCallback02【") + JSON.stringify(isFlagCallback02) + (" 】")+" , "+isFlagCallback02);
//            expect(isFlagCallback02).assertEqual(1);
//            console.debug("=ACTS_OnOff_0200 isFlagCallback03=======>"
//            + ("json isFlagCallback03【") + JSON.stringify(isFlagCallback03) + (" 】")+" , "+isFlagCallback03);
//            expect(isFlagCallback03).assertEqual(2);
//        }catch(err) {
//            console.error('=ACTS_OnOff_0200  AsyncCallback catch(err)====>:'+err);
//        }
//        setTimeout(function(){console.log('ACTS_OnOff_0100====<setTimeout');}, g_setTimeout);
//        console.log('ACTS_OnOff_0200====<end');
//        done();
//    })

    /*
* @tc.number: ACTS_ExecuteBatch_0100
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ExecuteBatch_0100', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0100====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0100_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_INSERT,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
            ).then((data) => {
                console.debug("=ACTS_ExecuteBatch_0100 executeBatch then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_ExecuteBatch_0100 executeBatch catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0100 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0100====<end');
    })

/*
* @tc.number: ACTS_ExecuteBatch_0200
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_ExecuteBatch_0200', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0200====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0200_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                    {
                        uri: dataAbilityUri,
                        type: featureAbility.DataAbilityOperationType.TYPE_INSERT,
                        valuesBucket: valuesBucket,
                        predicates: null,
                        expectedCount:1,
                        PredicatesBackReferences: {},
                        interrupted:true,
                    }
                ],
                (err,data)=>{
                    console.debug("=ACTS_ExecuteBatch_0200 executeBatch err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;")+ data.length);
                    ret = true
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0200 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0200====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0300
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ExecuteBatch_0300', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0300====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0300_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_UPDATE,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
            ).then((data) => {
                console.debug("=ACTS_ExecuteBatch_0300 executeBatch then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_ExecuteBatch_0300 executeBatch catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0300 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0300====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0400
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_ExecuteBatch_0400', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0400====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0400_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_UPDATE,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
                (err,data)=>{
                    console.debug("=ACTS_ExecuteBatch_0400 executeBatch err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;")+ data.length);
                    ret = true
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0400 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0400====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0500
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ExecuteBatch_0500', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0500====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0500_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_DELETE,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
            ).then((data) => {
                console.debug("=ACTS_ExecuteBatch_0500 executeBatch then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_ExecuteBatch_0500 executeBatch catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0500 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0500====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0600
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_ExecuteBatch_0600', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0600====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0600_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_DELETE,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
                (err,data)=>{
                    console.debug("=ACTS_ExecuteBatch_0600 executeBatch err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;")+ data.length);
                    ret = true
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0600 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0600====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0700
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by Promise)
*/
    it('ACTS_ExecuteBatch_0700', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0700====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0700_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_ASSERT,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
            ).then((data) => {
                console.debug("=ACTS_ExecuteBatch_0700 executeBatch then data====>"
                + ("json data 【") + JSON.stringify(data)+ (" 】"));
                ret = true
                done();
            }).catch((err)=>{
                console.debug("=ACTS_ExecuteBatch_0700 executeBatch catch err ====>"
                + ("json err 【") + JSON.stringify(err) + (" 】 "));
                ret = false
                done();
            });
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0700 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0700====<end');
    })

    /*
* @tc.number: ACTS_ExecuteBatch_0800
* @tc.name: ExecuteBatch : Preforms batch operations on the database
* @tc.desc: Check the return value of the interface (by AsyncCallback)
*/
    it('ACTS_ExecuteBatch_0800', 0, async function (done) {
        console.log('ACTS_ExecuteBatch_0800====<begin');
        let ret = false;
        expect(typeof(DAHelper)).assertEqual("object");
        try{
            const valuesBucket = {
                "name": "ACTS_ExecuteBatch_0800_rose",
                "age": 22,
                "salary": 200.5,
                "blobType": "u8",
            }
            DAHelper.executeBatch(dataAbilityUri,
                [
                        {
                            uri: dataAbilityUri,
                            type: featureAbility.DataAbilityOperationType.TYPE_ASSERT,
                            valuesBucket: valuesBucket,
                            predicates: null,
                            expectedCount:1,
                            PredicatesBackReferences: {},
                            interrupted:true,
                        }
                ],
                (err,data)=>{
                    console.debug("=ACTS_ExecuteBatch_0800 executeBatch err,data====>"
                    + ("json err【") + JSON.stringify(err) + (" 】json data【") + JSON.stringify(data)+ (" 】;")+ data.length);
                    ret = true
                    done();
                }
            );
        }catch(err) {
            console.error('=ACTS_ExecuteBatch_0800 catch(err)====>:'
            + ("json err 【") + JSON.stringify(err) + (" 】 ,")+err);
            ret = false;
            done();
        }
        setTimeout(function(){
            expect(ret).assertTrue();
        }, g_setTimeout);
        console.log('ACTS_ExecuteBatch_0800====<end');
    })

    async function uninstall(bundleName)
    {
        try{
            var installer = await bundle.getBundleInstaller();
            installer.uninstall(bundleName, {
                param: {
                    userId: 0,
                    installFlag: 1,
                    isKeepData: false
                }
            }, onReceiveUninstallEvent);

            function onReceiveUninstallEvent(err, data) {
                console.info('========uninstall Finish========');
                expect(typeof err).assertEqual('object');
                expect(err.code).assertEqual(0);
                expect(typeof data).assertEqual('object');
                expect(data.status).assertEqual(0);
                expect(data.statusMessage).assertEqual('SUCCESS');
            }
        }catch(err) {
            console.error('=bms_uninstallAbility_0100 onReceiveUninstallEvent catch(err)====>:'+err);
            expect(false).assertTrue();
        }
    }

    /*
    * @tc.number: bms_uninstallAbility_0100
    * @tc.name: test the multi js ability
    * @tc.desc: 1.install a js hap with two ability
    *           2.check the ability name by the interface of getBundleInfo
    */
    it('bms_uninstallAbility_0100', 0, async function (done) {
        console.info('bms_uninstallAbility_0100====< begin');
        try{
            await uninstall(bundleName);
            done();
        }catch(err) {
            console.error('=bms_uninstallAbility_0100 uninstall catch(err)====>:'+err);
            expect(false).assertTrue();
            done();
        }
        setTimeout(function () {
            console.info('bms_uninstallAbility_0100====< setTimeout');
        }, g_setTimeout)
        console.info('bms_uninstallAbility_0100====< end');
    })
})