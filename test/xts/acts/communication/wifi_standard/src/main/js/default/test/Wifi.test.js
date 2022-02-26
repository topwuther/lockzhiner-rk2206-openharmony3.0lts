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

import wifi_native_js from '@ohos.wifi_native_js'

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

// delay x ms
function sleep(delay) {
    var start = (new Date()).getTime();
    while ((new Date()).getTime() - start < delay) {
        continue;
    }
}

describe('appInfoTest', function () {
    beforeEach(function () {
    })

    afterEach(function () {
    })

    console.log("*************[wifi_test]start wifi js unit test[ver:18]*************");
    
    /**
    * @tc.number     wifi_native_js_unit_test_001
    * @tc.name       Test wifi.isWifiActive.
    * @tc.desc       Test wifi.isWifiActive API functionality.
    */
    it('wifi_native_js_unit_test_001', 0, function () {
        console.info("[wifi_test] wifi active  test.");
        expect(wifi_native_js.isWifiActive()).assertTrue();
    })

    /**
    * @tc.number     wifi_native_js_unit_test_002
    * @tc.name       Test wifi.scan.
    * @tc.desc       Test wifi.scan API functionality.
    */
    it('wifi_native_js_unit_test_002', 0, async function (done) {
        console.info("[wifi_test] wifi scan test.");
        expect(wifi_native_js.scan()).assertTrue();
        sleep(3000);

        wifi_native_js.getScanInfos(result => {
            console.info('[wifi_test] getScanInfos -> ' + JSON.stringify(result));
            expect(Object.keys(result).length).assertLarger(0);

            for (var i = 0; i < Object.keys(result).length; ++i) {
                console.info("[wifi_test] rssi: " + result[i].rssi + ", band: " + result[i].band + ", level: " + wifi_native_js.getSignalLevel(result[i].rssi, result[i].band));
            }
        })

        sleep(3000);
        expect(wifi_native_js.scan()).assertTrue();
        sleep(3000);

        wifi_native_js.getScanInfos().then(result => {
            console.info("[wifi_test] wifi scan result call promise.");
            expect(Object.keys(result).length).assertLarger(0);
        });
        done();
    })

    /**
    * @tc.number     wifi_native_js_unit_test_003
    * @tc.name       Test wifi.getSignalLevel.
    * @tc.desc       Test wifi.getSignalLevel API functionality.
    */
    it('wifi_native_js_unit_test_003', 0, function () {
        console.info("[wifi_test] wifi get signal level test.");
        expect(wifi_native_js.getSignalLevel(1, 1)).assertEqual(4);
        sleep(3000);
    })

    /**
    * @tc.number     wifi_native_js_unit_test_005
    * @tc.name       Test wifi.getScanInfos 0.
    * @tc.desc       Test wifi.getScanInfos API functionality.
    */
    it('wifi_native_js_unit_test_005', 0, async function (done) {
        console.info("[wifi_test] Wifi get scan infos callback test[1].");
        wifi_native_js.getScanInfos(result => {
            var clen = Object.keys(result).length;
            console.log("[wifi_test] wifi received scan info call back: " + clen);
            expect(result).assertLarger(0);
            console.info("[wifi_test] add device config callback: " + JSON.stringify(result));
            expect(JSON.stringify(result)).assertContain('ssid');
            sleep(5000);
            for (var j = 0; j < clen; ++j) {
                console.info("ssid: " + result[j].ssid);
                console.info("bssid: " + result[j].bssid);
                console.info("securityType: " + result[j].securityType);
                console.info("rssi: " + result[j].rssi);
                console.info("band: " + result[j].band);
                console.info("frequency: " + result[j].frequency);
                console.info("timestamp: " + result[j].timestamp);
            }
        });
        done();
    })
    sleep(20000);

    /**
    * @tc.number     wifi_native_js_unit_test_006
    * @tc.name       Test wifi.getScanInfos 1.
    * @tc.desc       Test wifi.getScanInfos API functionality.
    */
    it('wifi_native_js_unit_test_006', 0, async function (done) {
        console.info("[wifi_test] Wifi get scan infos callback test[2].");
        wifi_native_js.getScanInfos().then(
            result => {
                var clen = Object.keys(result).length;
                console.log("[wifi_test] wifi received scan promise result: " + clen);
                expect(result).assertLarger(0);
                console.info("[wifi_test] test_006 promise... " + JSON.stringify(result));
                expect(JSON.stringify(result)).assertContain('ssid');

                sleep(5000);
                for (var j = 0; j < clen; ++j) {
                    console.info("ssid: " + result[j].ssid);
                    console.info("bssid: " + result[j].bssid);
                    console.info("securityType: " + result[j].securityType);
                    console.info("rssi: " + result[j].rssi);
                    console.info("band: " + result[j].band);
                    console.info("frequency: " + result[j].frequency);
                    console.info("timestamp: " + result[j].timestamp);
                }
            });
        done();
    })

    console.log("*************[wifi_test] start wifi js unit test end*************");
})

