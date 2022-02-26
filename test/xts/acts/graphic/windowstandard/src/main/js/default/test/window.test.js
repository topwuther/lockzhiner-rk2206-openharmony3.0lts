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
import app from '@system.app'
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import window from '@ohos.window'


describe('window_test', function(){
    var wnd;
    beforeAll(function(){
    })
    beforeEach(function(){
    })
    afterEach(function(){
    })
    afterAll(function(){
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0100
    * @tc.name       Test getTopWindowTest.
    * @tc.desc      Test window.getTopWindow API function test.
    */
    it('getTopWindowTest', 0, function(){
        console.log('jsunittest getTopWindowTest begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest getTopWindow wnd: ' + wnd)
            assertTrue(wnd != null)
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0200
    * @tc.name       Test movetest1.
    * @tc.desc      Test window.moveTo API function test1.
    */
    it('moveTest1', 0, function(){
        console.log('jsunittest movetest1 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest movetest1 getTopWindow wnd: ' + wnd)
            wnd.moveTo(200, 200).then(()=>{
                console.log('jsunittest movetest1 wnd.moveTo(200, 200) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0300
    * @tc.name       Test moveTest2.
    * @tc.desc      Test window.moveTo API function test2.
    */
    it('moveTest2', 0, function(){
        console.log('jsunittest movetest2 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest movetest2 getTopWindow wnd: ' + wnd)
            wnd.moveTo(100, 100).then(()=>{
                console.log('jsunittest movetest2 wnd.moveTo(100, 100) success')
            })
            wnd.moveTo(0, 0).then(()=>{
                console.log('jsunittest movetest2 wnd.moveTo(0, 0) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0400
    * @tc.name       Test moveTest3.
    * @tc.desc      Test window.moveTo API function test3.
    */
    it('moveTest3', 0, function(){
        console.log('jsunittest movetest3 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest movetest3 getTopWindow wnd: ' + wnd)
            wnd.moveTo(20000, 20000).then(()=>{
                console.log('jsunittest movetest3 wnd.moveTo(20000, 20000) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0500
    * @tc.name       Test moveTest4.
    * @tc.desc      Test window.moveTo API function test4.
    */
    it('moveTest4', 0, function(){
        console.log('jsunittest movetest4 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest movetest4 getTopWindow wnd: ' + wnd)
            wnd.moveTo(-200, -200).then(()=>{
                console.log('jsunittest movetest4 wnd.moveTo(-200, -200) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0600
    * @tc.name       Test moveTest5.
    * @tc.desc      Test window.moveTo API function test5.
    */
    it('moveTest5', 0, function(){
        console.log('jsunittest movetest5 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest movetest5 getTopWindow wnd: ' + wnd)
            for (var i = 1; i <= 5; i++) {
                wnd.moveTo(100, 100).then(()=>{
                    console.log('jsunittest movetest5 wnd.movetest5(100, 100) success, count:"%d\n"',i)
                })
            }
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0700
    * @tc.name       Test resetSizeTest1.
    * @tc.desc      Test window.resetSize API function test1.
    */
    it('resetSizeTest1', 0, function(){
        console.log('jsunittest resetSizeTest1 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest resetSizeTest1 getTopWindow wnd: ' + wnd)
            wnd.resetSize(200, 600).then(()=>{
                console.log('jsunittest resetSizeTest wnd.resetSize(200, 600) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0800
    * @tc.name       Test resetSizeTest2.
    * @tc.desc      Test window.resetSize API function test2.
    */
    it('resetSizeTest2', 0, function(){
        console.log('jsunittest resetSizeTest2 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest resetSizeTest2 getTopWindow wnd: ' + wnd)
            wnd.resetSize(20000, 20000).then(()=>{
                console.log('jsunittest resetSizeTest2 wnd.resetSize(20000, 20000) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_0900
    * @tc.name       Test resetSizeTest3.
    * @tc.desc      Test window.resetSize API function test3.
    */
    it('resetSizeTest3', 0, function(){
        console.log('jsunittest resetSizeTest3 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest resetSizeTest3 getTopWindow wnd: ' + wnd)
            wnd.resetSize(0, 0).then(()=>{
                console.log('jsunittest resetSizeTest3 wnd.resetSize(0, 0) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1000
    * @tc.name       Test resetSizeTest4.
    * @tc.desc      Test window.resetSize API function test4.
    */
    it('resetSizeTest4', 0, function(){
        console.log('jsunittest resetSizeTest4 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest resetSizeTest4 getTopWindow wnd: ' + wnd)
            wnd.resetSize(-1, -1).then(()=>{
                console.log('jsunittest resetSizeTest4 wnd.resetSize(-1, -1) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1100
    * @tc.name       Test resetSizeTest5.
    * @tc.desc      Test window.resetSize API function test5.
    */
    it('resetSizeTest5', 0, function(){
        console.log('jsunittest resetSizeTest5 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest resetSizeTest5 getTopWindow wnd: ' + wnd)
            for (var i = 1; i <= 5; i++) {
                wnd.resetSize(100, 100).then(()=>{
                    console.log('jsunittest resetSizeTest5 wnd.resetSize(100, 100) success, count:"%d\n"',i)
                })
            }
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1200
    * @tc.name       Test setWindowTypeTest1.
    * @tc.desc      Test window.setWindowType API function test1.
    */
    it('setWindowTypeTest1', 0, function(){
        console.log('jsunittest setWindowTypeTest1 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest1 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(0).then(()=>{
                console.log('jsunittest setWindowTypeTest1 wnd.setWindowType(0) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1300
    * @tc.name       Test setWindowTypeTest2.
    * @tc.desc      Test window.setWindowType API function test2.
    */
    it('setWindowTypeTest2', 0, function(){
        console.log('jsunittest setWindowTypeTest2 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest2 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(0).then(()=>{
                console.log('jsunittest setWindowTypeTest2 wnd.setWindowType(0) success')
            })
            wnd.moveTo(200, 200).then(()=>{
                console.log('jsunittest setWindowType1 wnd.moveTo(200, 200) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1400
    * @tc.name       Test setWindowTypeTest3.
    * @tc.desc      Test window.setWindowType API function test3.
    */
    it('setWindowTypeTest3', 0, function(){
        console.log('jsunittest setWindowTypeTest3 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest3 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(0).then(()=>{
                console.log('jsunittest setWindowTypeTest3 wnd.setWindowType(0) success')
            })
            wnd.resetSize(200, 400).then(()=>{
                console.log('jsunittest setWindowTypeTest3 wnd.resetSize(200, 400) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1500
    * @tc.name       Test setWindowTypeTest4.
    * @tc.desc      Test window.setWindowType API function test4.
    */
    it('setWindowTypeTest4', 0, function(){
        console.log('jsunittest setWindowTypeTest4 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest4 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(1).then(()=>{
                console.log('jsunittest setWindowTypeTest4 wnd.setWindowType(1) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1600
    * @tc.name       Test setWindowTypeTest5.
    * @tc.desc      Test window.setWindowType API function test5.
    */
    it('setWindowTypeTest5', 0, function(){
        console.log('jsunittest setWindowTypeTest5 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest5 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(1).then(()=>{
                console.log('jsunittest setWindowTypeTest5 wnd.setWindowType(1) success')
            })
            wnd.moveTo(100, 100).then(()=>{
                console.log('jsunittest setWindowTypeTest5 wnd.moveTo(100, 100) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1700
    * @tc.name       Test setWindowTypeTest6.
    * @tc.desc      Test window.setWindowType API function test6.
    */
    it('setWindowTypeTest6', 0, function(){
        console.log('jsunittest setWindowTypeTest6 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest6 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(1).then(()=>{
                console.log('jsunittest setWindowTypeTest6 wnd.setWindowType(1) success')
            })
            wnd.resetSize(100, 100).then(()=>{
                console.log('jsunittest setWindowTypeTest6 wnd.resetSize(100, 100) success')
            })
        })
    })
    /**
    * @tc.number     GRAPHIC_FUNCTION_WINDOW_TESTWINDOW_JSNAPI_1800
    * @tc.name       Test setWindowTypeTest7.
    * @tc.desc      Test window.setWindowType API function test7.
    */
    it('setWindowTypeTest7', 0, function(){
        console.log('jsunittest setWindowTypeTest7 begin')
        window.getTopWindow().then(wnd => {
            console.log('jsunittest setWindowTypeTest7 getTopWindow wnd: ' + wnd)
            wnd.setWindowType(10).then(()=>{
                console.log('jsunittest setWindowTypeTest7 wnd.setWindowType(10) success')
            })
            wnd.resetSize(400, 400).then(()=>{
                console.log('jsunittest setWindowTypeTest7 wnd.resetSize(400, 400) success')
            })
        })
    })
})
