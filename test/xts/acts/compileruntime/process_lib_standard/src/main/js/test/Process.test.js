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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/lite'
import  process  from '@ohos.process'

describe('ChildProcessTest', function () {

    // childprocess runCmd test
    it('test_runCmd_001', 0, async function () {
        var child = process.runCmd('echo abc')
        child.wait()
        var array = new Uint8Array([97, 98, 99, 10, 0])
        child.getOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    it('test_runCmd_002', 0, async function () {
        var child = process.runCmd('echo abc;', { maxBuffer : 2 })
        child.wait()
        var array = new Uint8Array([97, 98, 0])
        child.getOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    it('test_runCmd_003', 0, async function () {
        var child = process.runCmd('sleep 5; echo abc;', { timeout : 1, killSignal : 9 })
        child.wait()
        var array = new Uint8Array([0])
        child.getOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
        expect(child.exitCode).assertEqual(9)
    })

    it('test_runCmd_004', 0, async function () {
        var child = process.runCmd('sleep 2; echo abc;', { timeout : 9000, killSignal : 9 })
        child.wait()
        var array = new Uint8Array([97, 98, 99, 10, 0])
        child.getOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
        expect(child.exitCode).assertEqual(0)
    })

    // childprocess getOutput test
    it('test_getOutput_001', 0, async function () {
        var child = process.runCmd('echo bcd;')
        child.wait()
        var array = new Uint8Array([98, 99, 100, 0])
        child.getOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    // childprocess getErrorOutput test
    it('test_getErrorOutput_001', 0, async function () {
        var child = process.runCmd('makdir 1.txt')
        child.wait()
        var array = new Uint8Array([115, 104, 58, 32, 109, 97, 107, 100, 105, 114, 58, 32, 105, 110, 97, 99, 99,
            101, 115, 115, 105, 98, 108, 101, 32, 111, 114, 32, 110, 111, 116, 32, 102, 111, 117, 110, 100, 10, 0])
        child.getErrorOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    it('test_getErrorOutput_002', 0, async function () {
        var child = process.runCmd('echo "error" 1>&2')
        child.wait()
        var array = new Uint8Array([101, 114, 111, 114, 10, 0])
        child.getErrorOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    it('test_getErrorOutput_003', 0, async function () {
        var child = process.runCmd('1')
        child.wait()
        var array = new Uint8Array([115, 104, 58, 32, 49, 58, 32, 105, 110, 97, 99, 99, 101, 115, 115, 105, 98,
            108, 101, 32, 111, 114, 32, 110, 111, 116, 32, 102, 111, 117, 110, 100, 10, 0])
        child.getErrorOutput().then(val=>{
            for (var i = 0; i < array.length; i++) {
                expect(val[i]).assertEqual(array[i])
            }
        })
    })

    // childprocess wait test
    it('test_wait_001', 0, async function () {
        var child = process.runCmd('ls')
        var status = child.wait()
        status.then(val=>{
            expect(val).assertEqual(0)
        })
    })

    it('test_wait_002', 0, async function () {
        var child = process.runCmd('ls; sleep 5;')
        child.kill(9);
        var status = child.wait()
        status.then(val=>{
            expect(val).assertEqual(9)
        })
    })

    // childprocess pid test
    it('test_pid_001', 0, function () {
        var child = process.runCmd('ls; sleep 5;')
        child.wait()
        var pid_ = child.pid
        expect(pid_ > 0).assertEqual(true)
    })

    // childprocess ppid test
    it('test_ppid_001', 0, function () {
        var child = process.runCmd('ls; sleep 5;')
        var status = child.wait()
        var ppid_ = child.ppid
        expect(ppid_ > 0).assertEqual(true)
    })

    // childprocess kill test
    it('test_kill_001', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.kill(13)
        var temp = child.killed
        expect(temp).assertEqual(true)
    })

    it('test_kill_002', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.kill(1)
        var temp = child.killed
        expect(temp).assertEqual(true)
    })

    it('test_kill_003', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.kill('SIGHUP')
        var temp = child.killed
        expect(temp).assertEqual(true)
    })

    it('test_kill_004', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.kill('ABC')
        var temp = child.killed
        expect(temp).assertEqual(true)
    })

    it('test_kill_005', 0, function () {
        var child =  process.runCmd('ls')
        child.wait()
        var result = child.kill(9)
        var temp = child.killed
        expect(temp).assertEqual(false)
    })

    // childprocess close test
    it('test_close_001', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.close()
        expect(child.exitCode).assertEqual(9)
    })

    it('test_close_002', 0, function () {
        var child =  process.runCmd('ls')
        function sleep(ms, callback) {
            setTimeout(callback, ms)
        }
        sleep(1000, () => {} )
        var result = child.close()
        expect(child.exitCode===0).assertEqual(false)
    })

    it('test_close_003', 0, function () {
        var child =  process.runCmd('ls;')
        var status = child.wait()
        var result = child.close()
        expect(child.exitCode).assertEqual(0)
    })

    // childprocess killed test
    it('test_killed_001', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var result = child.kill(3)
        var temp = child.killed
        expect(temp).assertEqual(true)
    })

    // childprocess exitCode test
    it('test_exitCode_001', 0, function () {
        var child =  process.runCmd('ls; sleep 5s;')
        var temp = child.kill(9)
        child.wait()
        var result = child.exitCode
        expect(result).assertEqual(9)
    })

    // process GetUid test
    it('GetUid_test_001', 0, function () {
        var result = process.getUid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('GetUid_test_002', 0, function () {
        for(var i = 0; i < 3; i++){
            var result = process.getUid
            if(result != null) {
                if(result > 0) {
                    var flag = true
                }
                expect(flag).assertEqual(true)
            }
        }
    })

    // process GetGid test
    it('GetGid_test_001', 0, function () {
        var result = process.getGid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('GetGid_test_002', 0, function () {
        for(var i = 0; i < 3; i++){
            var result = process.getGid
            if(result != null) {
                if(result > 0) {
                    var flag = true
                }
                expect(flag).assertEqual(true)
            }
        }
    })

    // process GetEuid test
    it('GetEuid_test_001', 0, function () {
        var result = process.getEuid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('GetEuid_test_002', 0, function () {
        for(var i = 0; i < 3; i++){
            var result = process.getEuid
            if(result != null) {
                if(result > 0) {
                    var flag = true
                }
                expect(flag).assertEqual(true)
            }
        }
    })

    // process GetEgid test
    it('GetEgid_test_001', 0, function () {
        var result = process.getEgid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('GetEgid_test_002', 0, function () {
        for(var i = 0; i < 3; i++){
            var result = process.getEgid
            if(result != null) {
                if(result > 0) {
                    var flag = true
                }
                expect(flag).assertEqual(true)
            }
        }
    })

    // process GetGroups test
    it('GetGroups_test_001', 0, function () {
        var result = process.getGroups
        var len = result.length
        if(len > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('GetGroups_test_002', 0, function () {
        for(var i = 0; i < 3; i++){
            var result = process.getGroups
            if(result != null) {
                var len = result.length
                if(len > 0) {
                    var flag = true
                }
                expect(flag).assertEqual(true)
            }
        }
    })

    // process chdir test
    it('chdir_test_001', 0, function () {
        var result = process.chdir('/system')
        var result1 = process.cwd()
        expect(result1).assertEqual('/system')
    })

    it('chdir_test_002', 0, function () {
        var result = process.chdir('/system/lib')
        var result1 = process.cwd()
        expect(result1).assertEqual('/system/lib')
    })

    it('chdir_test_003', 0, function () {
        var result = process.chdir('/..')
        var result1 = process.cwd()
        expect(result1).assertEqual('/')
    })

    // process kill test
    it('kill_test_001', 0, function () {
        var result = process.kill(123, 3)
        expect(result).assertEqual(false)
    })

    it('kill_test_002', 0, function () {
        var pres = process.getPid
        var result = process.kill(pres, 23)
        expect(result).assertEqual(true)
    })

    it('kill_test_003', 0, function () {
        var pres = process.getPid
        var result = process.kill(pres, 28)
        expect(result).assertEqual(true)
    })

    it('kill_test_004', 0, function () {
        var pres = process.getPid
        var result = process.kill(pres, 17)
        expect(result).assertEqual(true)
    })

    // process uptime test
    it('uptime_test_001', 0, function () {
        var result1 = process.uptime()
        function sleep(d){
            while(process.uptime() - result1 <= d);
        }
        sleep(5);
        var result2 = process.uptime() - 6
        expect(result1).assertEqual(result2)
    })

    it('uptime_test_002', 0, function () {
        var result1 = process.uptime()
        function sleep(d){
            while(process.uptime() - result1 <= d);
        }
        sleep(8);
        var result2 = process.uptime() - 9
        expect(result1).assertEqual(result2)
    })

    it('uptime_test_003', 0, function () {
        var result1 = process.uptime()
        function sleep(d){
            while(process.uptime() - result1 <= d);
        }
        sleep(10);
        var result2 = process.uptime() - 11
        expect(result1).assertEqual(result2)
    })

    // process pid test
    it('pid_test_001', 0, function () {
        var result = process.getPid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('pid_test_002', 0, function () {
        for(var i = 0; i < 3; i++)
        {
            var result = process.getPid
            if(result > 0) {
                var flag = true
            }
            expect(flag).assertEqual(true)
        }
    })

    // process ppid test
    it('ppid_test_001', 0, function () {
        var result = process.getPpid
        if(result > 0) {
            var flag = true
        }
        expect(flag).assertEqual(true)
    })

    it('ppid_test_002', 0, function () {
        for(var i = 0; i < 3; i++)
        {
            var result = process.getPpid
            if(result > 0) {
                var flag = true
            }
            expect(flag).assertEqual(true)
        }
    })

    // process cwd test
    it('cwd_test_001', 0, function () {
        var cwd = process.cwd()
        expect(cwd).assertEqual('/')
    })

    it('cwd_test_002', 0, function () {
        for(var i = 0; i < 3; i++)
        {
            var cwd = process.cwd()
            expect(cwd).assertEqual('/')
        }
    })

    // process on test
    it('on_test_001', 0, function () {
        function add(){
            var value = 3 + 2
        }
        var ontest = process.on(1, add)
        expect(ontest).assertEqual(false)
    })

    it('on_test_002', 0, function () {
        function add1(num){
            var value = num + 3
        }
        var on = process.on("add123", add1)
    })

    it('on_test_003', 0, function () {
        function add2(num0, num1){
            var value = num0 + num1
        }
        var ontest = process.on(879, add2)
        expect(ontest).assertEqual(false)
    })
})