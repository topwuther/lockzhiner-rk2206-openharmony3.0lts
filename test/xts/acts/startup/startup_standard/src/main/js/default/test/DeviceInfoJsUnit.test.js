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

// @ts-nocheck
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import deviceinfo from '@ohos.deviceInfo'

describe('DeviceInfoTest', function () {

    const MAX_CHARACTERS_NUM_ONE = 33;
    const MAX_CHARACTERS_NUM_TWO = 65;
    const MAX_CHARACTERS_NUM = 0;

    beforeAll(function () {
        console.info('beforeAll: Prerequisites at the test suite level, which are executed before the test suite is executed.');

    })

    beforeEach(function () {
        console.info('beforeEach: Prerequisites at the test case level, which are executed before each test case is executed.');

    })
    afterEach(function () {
        console.info('afterEach: Test case-level clearance conditions, which are executed after each test case is executed.');

    })
    afterAll(function () {
        console.info('afterAll: Test suite-level cleanup condition, which is executed after the test suite is executed');

    })

    console.info('start################################start');
    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0100
     * @tc.name      testGetDeviceType01
     * @tc.desc      Get a string representing the device type.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_001', 0, function () {
        console.info('device_info_test_001 start');
		let ret = false;
        let deviceTypeInfo = deviceinfo.deviceType;
        console.info('the value of the deviceType is :' + deviceTypeInfo);

        expect(deviceTypeInfo).assertInstanceOf('String');
        if (deviceTypeInfo !="" && deviceTypeInfo != null && deviceTypeInfo != undefined) {
            ret = true;
        }
        expect(ret).assertTrue()        
        console.info('device_info_test_001 : end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0200
     * @tc.name     testGetManufacture01
     * @tc.desc      Get the manufacture name represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_002', 0, function () {
        console.info('device_info_test_002 start');
		let ret = false;
        let manufactureInfo = deviceinfo.manufacture;        
        console.info('the value of the manufactureInfo is :' + manufactureInfo);

        expect(manufactureInfo).assertInstanceOf('String');
        if (manufactureInfo !="" && manufactureInfo != null && manufactureInfo != undefined){
            ret = true;
        }
        expect(ret).assertTrue();		
        console.info('device_info_test_002 ：end');
    })
    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0300
     * @tc.name     testGetProductBrand01
     * @tc.desc      Get the product brand represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_003', 0, function () {
        console.info('testGetProductBrand01 start');
		let ret = false;
        let brandInfo = deviceinfo.brand;
        console.info('the value of the deviceinfo brand is :' + brandInfo);

        expect(brandInfo).assertInstanceOf('String');
        if (brandInfo !="" && brandInfo != null && brandInfo != undefined){
            ret = true;
        }
        expect(ret).assertTrue();        
        console.info('testGetProductBrand01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0400
     * @tc.name     testGetMarketName01
     * @tc.desc      Get the external product family name represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_004', 0, function () {
        console.info('testGetMarketName01 start')
		let ret = false;
        let marketNameInfo = deviceinfo.marketName;
        console.info('the value of the deviceinfo marketName is :' + marketNameInfo);

        expect(marketNameInfo).assertInstanceOf('String');
        if(marketNameInfo !="" && marketNameInfo != null && marketNameInfo != undefined){
            ret = true;
        }
        expect(ret).assertTrue();        
        console.info('testGetMarketName01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0500
     * @tc.name     testGetProductSeries01
     * @tc.desc      Get the product series represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_005', 0, function () {
        console.info('testGetProductSeries01 start');
        let ret = false;		
        let productSeriesInfo = deviceinfo.productSeries;
        console.info('the value of the deviceinfo productSeries is :' + productSeriesInfo);

        expect(productSeriesInfo).assertInstanceOf('String');
        if(productSeriesInfo !="" && productSeriesInfo != null && productSeriesInfo != undefined){
            ret = true;
        }
        expect(ret).assertTrue();		
        console.info('testGetProductSeries01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0600
     * @tc.name     testGetProductModel01
     * @tc.desc      Get the internal software sub-model represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_006', 0, function () {
        console.info('testGetProductModel01 start');
        let ret = false;		
        let productModelInfo = deviceinfo.productModel;
        console.info('the value of the deviceinfo productModel is :' + productModelInfo);

        expect(productModelInfo).assertInstanceOf('String');
        if(productModelInfo !="" && productModelInfo != null && productModelInfo != undefined){
            ret =true;
        }
		expect(ret).assertTrue();
        console.info('testGetProductModel01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0700
     * @tc.name     testGetSoftwareModel01
     * @tc.desc      Get the internal software sub-model represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_007', 0, function () {
        console.info('testGetSoftwareModel01 start');
        let ret = false;		
        let softwareModelInfo = deviceinfo.softwareModel;
        console.info('the value of the deviceinfo softwareModel is :' + softwareModelInfo);

        expect(softwareModelInfo).assertInstanceOf('String');
        if(softwareModelInfo !="" && softwareModelInfo != null && softwareModelInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetSoftwareModel01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0800
     * @tc.name     testGetHardWareModel01
     * @tc.desc      Get the hardware version represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_008', 0, function () {
        console.info('testGetHardWareModel01 start');
        let ret = false;		
        let hardwareModelInfo = deviceinfo.hardwareModel;
        console.info('the value of the deviceinfo hardwareModel is :' + hardwareModelInfo);

        expect(hardwareModelInfo).assertInstanceOf('String');
        if(hardwareModelInfo !="" && hardwareModelInfo != null && hardwareModelInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetHardWareModel01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0900
     * @tc.name     testGetHardWareProfile01
     * @tc.desc      Get the hardware profile represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_009', 0, function () {
        console.info('testGetHardWareProfile01 start');
        let ret = false;		
        let hardwareProfileInfo = deviceinfo.hardwareProfile;
        console.info('the value of the deviceinfo hardwareProfile is :' + hardwareProfileInfo);

        expect(hardwareProfileInfo).assertInstanceOf('String');
        if(hardwareProfileInfo !="" && hardwareProfileInfo != null && hardwareProfileInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetHardWareProfile01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0110
     * @tc.name     testGetSerial01
     * @tc.desc      Get the device serial number represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_010', 0, function () {
        console.info('testGetSerial01 start');
        let ret = false;        
        let serialInfo = deviceinfo.serial;
        console.info('the value of the deviceinfo serial is :' + serialInfo);

        expect(serialInfo).assertInstanceOf('String');
        if(serialInfo !="" && serialInfo != null && serialInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetSerial01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0120
     * @tc.name     testGetBootLoaderVersion01
     * @tc.desc      Get the bootloader version number represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_011', 0, function () {
        console.info('testGetBootLoaderVersion01 start');
        let ret = false;		
        let bootloaderVersionInfo = deviceinfo.bootloaderVersion;
        console.info('the value of the deviceinfo bootloaderVersion is :' + bootloaderVersionInfo);

        expect(bootloaderVersionInfo).assertInstanceOf('String');
        if(bootloaderVersionInfo !="" && bootloaderVersionInfo != null && bootloaderVersionInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBootLoaderVersion01 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0130
     * @tc.name     testGetabiList01
     * @tc.desc      Get the instruction set supported by the system.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_012', 0, function () {
        console.info('testGetabiList01 start');
        let ret = false;		
        let abiListInfo = deviceinfo.abiList;
        console.info('the value of the deviceinfo abiList is :' + abiListInfo);

        expect(abiListInfo).assertInstanceOf('String');
        if(abiListInfo !="" && abiListInfo != null && abiListInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetabiList01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0140
     * @tc.name     testGetabiList01
     * @tc.desc      Get the security patch level represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_013', 0, function () {
        console.info('testGetSecurityPatchTag01 start');
        let ret = false;		
        let securityPatchTagInfo = deviceinfo.securityPatchTag;
        console.info('the value of the deviceinfo securityPatchTag is :' + securityPatchTagInfo);

        expect(securityPatchTagInfo).assertInstanceOf('String');
        if(securityPatchTagInfo !="" && securityPatchTagInfo != null && securityPatchTagInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetSecurityPatchTag01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0150
     * @tc.name     testGetabiList01
     * @tc.desc      Get the version number visible to users represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_014', 0, function () {
        console.info('testGetDisplayVersion01 start');
        let ret = false;		
        let displayVersionInfo = deviceinfo.displayVersion;
        console.info('the value of the deviceinfo displayVersion is :' + displayVersionInfo);

        expect(displayVersionInfo).assertInstanceOf('String');
        if(displayVersionInfo !="" && displayVersionInfo != null && displayVersionInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetDisplayVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0160
     * @tc.name     testGetIncrementalVersion01
     * @tc.desc      Get the difference version number represented by a string.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_015', 0, function () {
        console.info('testGetIncrementalVersion01 start');
		let ret = false;
        let incrementalVersionInfo = deviceinfo.incrementalVersion;
        console.info('the value of the deviceinfo incrementalVersion is :' + incrementalVersionInfo);

        expect(incrementalVersionInfo).assertInstanceOf('String');
        if(incrementalVersionInfo !="" && incrementalVersionInfo != null && incrementalVersionInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetIncrementalVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0170
     * @tc.name     testGetOSReleaserType01
     * @tc.desc      Get the OS release type.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_016', 0, function () {
        console.info('testGetOSReleaserType01 start');
        let ret = false;		
        let osReleaseTypeInfo = deviceinfo.osReleaseType;
        console.info('the value of the deviceinfo osReleaseType is :' + osReleaseTypeInfo);

        expect(osReleaseTypeInfo).assertInstanceOf('String');
        if(osReleaseTypeInfo !="" && osReleaseTypeInfo != null && osReleaseTypeInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetOSReleaserType01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0180
     * @tc.name     testGetOSFullName01
     * @tc.desc      Get the operating system full name.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_017', 0, function () {
        console.info('testGetOSFullName01 start');
        let ret = false;
        let osFullNameInfo = deviceinfo.osFullName;
        console.info('the value of the deviceinfo osFullName is :' + osFullNameInfo);

        expect(osFullNameInfo).assertInstanceOf('String');
        if(osFullNameInfo !="" && osFullNameInfo != null && osFullNameInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetOSFullName01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0190
     * @tc.name     testGetMajorVersion01
     * @tc.desc      Get the major (M) version number, which increases with any updates to the overall architecture.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_018', 0, function () {
        console.info('testGetMajorVersion01 start');
		let ret = false;
        let majorVersionInfo = deviceinfo.majorVersion;
        console.info('the value of the deviceinfo majorVersion is :' + majorVersionInfo);

        expect(majorVersionInfo).assertInstanceOf('Number');
        if(majorVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetMajorVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0210
     * @tc.name     testGetSeniorVersion01
     * @tc.desc      Get the senior (S) version number, which increases with any updates to the partial.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_019', 0, function () {
        console.info('testGetSeniorVersion01 start');
		let ret = false;
        let seniorVersionInfo = deviceinfo.seniorVersion;
        console.info('the value of the deviceinfo seniorVersion is :' + seniorVersionInfo);

        expect(seniorVersionInfo).assertInstanceOf('Number');
        if(seniorVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetSeniorVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0220
     * @tc.name     testGetFeatureVersion01
     * @tc.desc      Get the feature (F) version number, which increases with any planned new features.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_020', 0, function () {
        console.info('testGetFeatureVersion01 start');
		let ret = false;
        let featureVersionInfo = deviceinfo.featureVersion;
        console.info('the value of the deviceinfo featureVersion is :' + featureVersionInfo);

        expect(featureVersionInfo).assertInstanceOf('Number');
        if(featureVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetFeatureVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0230
     * @tc.name     testGetBuildVersion01
     * @tc.desc      Get the build (B) version number, which increases with each new development build.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_021', 0, function () {
        console.info('testGetBuildVersion01 start');
		let ret = false;
        let buildVersionInfo = deviceinfo.buildVersion;
        console.info('the value of the deviceinfo buildVersion is :' + buildVersionInfo);

        expect(buildVersionInfo).assertInstanceOf('Number');
        if(buildVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0240
     * @tc.name     testGetSdkApiVersion01
     * @tc.desc      Get the API version number.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_022', 0, function () {
        console.info('testGetSdkApiVersion01 start');
		let ret = false;
        let sdkApiVersionInfo = deviceinfo.sdkApiVersion;
        console.info('the value of the deviceinfo sdkApiVersion is :' + sdkApiVersionInfo);

        expect(sdkApiVersionInfo).assertInstanceOf('Number');
        if(sdkApiVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetSdkApiVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0250
     * @tc.name     testGetFirstApiVersion01
     * @tc.desc      Get the first API version number.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_023', 0, function () {
        console.info('testGetFirstApiVersion01 start');
		let ret = true;
        let firstApiVersionInfo = deviceinfo.firstApiVersion;
        console.info('the value of the deviceinfo firstApiVersion is :' + firstApiVersionInfo);

        expect(firstApiVersionInfo).assertInstanceOf('Number');
        if(firstApiVersionInfo >= 0){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetFirstApiVersion01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0260
     * @tc.name     testGetVersionId01
     * @tc.desc      Get the version ID number.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_024', 0, function () {
        console.info('testGetVersionId01 start');
		let ret = false;
        let versionIdInfo = deviceinfo.versionId;
        console.info('the value of the deviceinfo versionId is :' + versionIdInfo);

        expect(versionIdInfo).assertInstanceOf('String');
        if(versionIdInfo !="" && versionIdInfo != null && versionIdInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetVersionId01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0270
     * @tc.name     testGetBuildType01
     * @tc.desc      Get the different build types of the same baseline code.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_025', 0, function () {
        console.info('testGetBuildType01 start');
		let ret = false;
        let buildTypeInfo = deviceinfo.buildType;
        console.info('the value of the deviceinfo buildType is :' + buildTypeInfo);

        expect(buildTypeInfo).assertInstanceOf('String');
        if(buildTypeInfo !="" && buildTypeInfo != null && buildTypeInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildType01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0280
     * @tc.name     testGetBuildUser01
     * @tc.desc      Get the different build user of the same baseline code.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_026', 0, function () {
        console.info('testGetBuildUser01 start');
		let ret = true;
        let buildUserInfo = deviceinfo.buildUser;
        console.info('the value of the deviceinfo buildUser is :' + buildUserInfo);

        expect(buildUserInfo).assertInstanceOf('String');
        if(buildUserInfo !="" && buildUserInfo != null && buildUserInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildUser01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0290
     * @tc.name     testGetBuildHost01
     * @tc.desc      Get the different build host of the same baseline code.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_027', 0, function () {
        console.info('testGetBuildHost01 start');
		let ret = false;
        let buildHostInfo = deviceinfo.buildHost;
        console.info('the value of the deviceinfo buildHost is :' + buildHostInfo);

        expect(buildHostInfo).assertInstanceOf('String');
        if(buildHostInfo !="" && buildHostInfo != null && buildHostInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildHost01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0310
     * @tc.name     testGetBuildTime01
     * @tc.desc      Get the the build time.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_028', 0, function () {
        console.info('testGetBuildTime01 start');
		let ret = false;
        let buildTimeInfo = deviceinfo.buildTime;
        console.info('the value of the deviceinfo buildTime is :' + buildTimeInfo);

        expect(buildTimeInfo).assertInstanceOf('String');
        if(buildTimeInfo !="" && buildTimeInfo != null && buildTimeInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildTime01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0320
     * @tc.name     testGetBuildRootHash01
     * @tc.desc      Get the version hash.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_029', 0, function () {
        console.info('testGetBuildRootHash01 start');
		let ret = false;
        let buildRootHashInfo = deviceinfo.buildRootHash;
        console.info('the value of the deviceinfo buildRootHash is :' + buildRootHashInfo);

        expect(buildRootHashInfo).assertInstanceOf('String');
        if(buildRootHashInfo !="" && buildRootHashInfo != null && buildRootHashInfo != undefined){
            ret = true;
        }
		expect(ret).assertTrue();
        console.info('testGetBuildRootHash01 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0330
     * @tc.name      testGetDeviceType02
     * @tc.desc      Get a string representing the device type which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_030', 0, function () {
        console.info('device_info_test_030 start');
        let deviceTypeInfo = deviceinfo.deviceType;
        console.info('the value of the deviceinfo deviceType is:' + deviceTypeInfo);

        let len = deviceTypeInfo.length
         
        console.info('the value of the device type characters:' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_030 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0340
     * @tc.name      testGetManufacture02
     * @tc.desc      Get a string representing the manufacture which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_031', 0, function () {
        console.info('device_info_test_031 start');
        let manufactureInfo = deviceinfo.manufacture;
        console.info('the value of the deviceinfo manufacture is :' + manufactureInfo);

        let len = manufactureInfo.length
        console.info('the value of the manufacture characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_031 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0350
     * @tc.name      testGetProductBrand02
     * @tc.desc      Get a string representing the external product family name which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_032', 0, function () {
        console.info('device_info_test_032 start');
        let brandInfo = deviceinfo.brand;
        console.info('the value of the deviceinfo brand is :' + brandInfo);

        let len = brandInfo.length
        console.info('the value of the external product family name characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_032 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0360
     * @tc.name      testGetMarketName02
     * @tc.desc      Get a string representing the product series which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_033', 0, function () {
        console.info('device_info_test_033 start');
        let marketNameInfo = deviceinfo.marketName;
        console.info('the value of the deviceinfo marketName is :' + marketNameInfo);

        let len = marketNameInfo.length
        console.info('the value of the product series characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_033 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0370
     * @tc.name      testGetProductSeries02
     * @tc.desc      Get a string representing the product series which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_034', 0, function () {
        console.info('device_info_test_034 start');
        let productSeriesInfo = deviceinfo.productSeries;
        console.info('the value of the deviceinfo  productSeries is :' + productSeriesInfo);

        let len = productSeriesInfo.length
        console.info('the value of the product series characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_034 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0380
     * @tc.name      testGetProductModel02
     * @tc.desc      Get a string representing the certified model which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_035', 0, function () {
        console.info('device_info_test_035 start');
        let productModelInfo = deviceinfo.productModel;
        console.info('the value of the deviceinfo productModel is :' + productModelInfo);

        let len = productModelInfo.length
        console.info('the value of the certified model characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_035 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0390
     * @tc.name      testGetSoftwareModel02
     * @tc.desc      Get a string representing the internal software sub-model which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_036', 0, function () {
        console.info('device_info_test_036 start');
        let softwareModelInfo = deviceinfo.softwareModel;
        console.info('the value of the deviceinfo softwareModel is :' + softwareModelInfo);

        let len = softwareModelInfo.length
        console.info('the value of the internal software sub-model characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_036 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0410
     * @tc.name      testGetHardwareModel02
     * @tc.desc      Get a string representing the hardware version which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_037', 0, function () {
        console.info('device_info_test_037 start');
        let hardwareModelInfo = deviceinfo.hardwareModel;
        console.info('the value of the deviceinfo hardwareModel is :' + hardwareModelInfo);

        let len = hardwareModelInfo.length;
        console.info('the value of the hardware version characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_037 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0420
     * @tc.name      testGetHardwareProfile02
     * @tc.desc      Get a string representing the hardware version which has a maximum of 1000 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_038', 0, function () {
        console.info('device_info_test_038 start');
        let hardwareProfileInfo = deviceinfo.hardwareProfile;
        console.info('the value of the deviceinfo hardwareProfile is :' + hardwareProfileInfo);

        let len = hardwareProfileInfo.length;
        console.info('the value of the hardware version characters is :' + len);
        expect(len).assertLess(1001);
        console.info('device_info_test_038 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0430
     * @tc.name      testGetSerial02
     * @tc.desc      Get a string representing the device serial number which has a maximum of 64 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_039', 0, function () {
        console.info('device_info_test_039 start');
        let serialInfo = deviceinfo.serial;
        console.info('the value of the deviceinfo serial is :' + serialInfo);

        let len = serialInfo.length;
        console.info('the value of the device serial number characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_TWO);
        console.info('device_info_test_039 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0440
     * @tc.name      testGetDisplayVersion02
     * @tc.desc      Get a string representing the version number visible to users which has a maximum of 64 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_040', 0, function () {
        console.info('device_info_test_040 start');
        let displayVersionInfo = deviceinfo.displayVersion;
        console.info('the value of the deviceinfo displayVersion is :' + displayVersionInfo);

        let len = displayVersionInfo.length;
        console.info('the value of the device serial number characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_TWO);
        console.info('device_info_test_040 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0450
     * @tc.name      testGetOsFullName02
     * @tc.desc      Get a string representing the operating system full name which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_041', 0, function () {
        console.info('device_info_test_041 start');
        let osFullNameInfo = deviceinfo.osFullName;
        console.info('the value of the deviceinfo osFullName is :' + osFullNameInfo);

        let len = osFullNameInfo.length;
        console.info('the value of the operating system full name characters is :' + len);
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE);
        console.info('device_info_test_041 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0460
     * @tc.name      testGetVersionId02
     * @tc.desc      Get a string representing the operating system full name which has a maximum of 127 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_042', 0, function () {
        console.info('device_info_test_042 start');
        let versionIdInfo = deviceinfo.versionId;
        console.info('the value of the deviceinfo versionId is :' + versionIdInfo);

        let len = versionIdInfo.length;
        console.info('the value of the operating system full name characters is :' + len)
        expect(len).assertLess(128);
        console.info('device_info_test_042 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0470
     * @tc.name      testGetBuildUser02
     * @tc.desc      Get a string representing the different build user of the same baseline code which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_043', 0, function () {
        console.info('device_info_test_043 start');
        let buildUserInfo = deviceinfo.buildUser;
        console.info('the value of the deviceinfo buildUser is :' + buildUserInfo);

        console.info('the value of the different build user of the same baseline code characters is :' + buildUserInfo.length);
        expect(buildUserInfo.length).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_043 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0480
     * @tc.name      testGetBuildHost02
     * @tc.desc      Get a string representing the different build host of the same baseline code which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_044', 0, function () {
        console.info('device_info_test_044 start');
        let buildHostInfo = deviceinfo.buildHost;
        console.info('the value of the deviceinfo buildHost is :' + buildHostInfo);

        let len = buildHostInfo.length
        console.info('the value of the different build host of the same baseline code characters is :' + len)
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_044 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0490
     * @tc.name      testGetDeviceType03
     * @tc.desc      Get a string representing the device type which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_045', 0, function () {
        console.info('device_info_test_045 start');
        let deviceTypeInfo = deviceinfo.deviceType;
        console.info('the value of the deviceinfo deviceType is :' + deviceTypeInfo);

        let len = deviceTypeInfo.length;
        console.info('the value of the device type characters:' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_045 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0510
     * @tc.name      testGetManufacture03
     * @tc.desc      Get a string representing the manufacture which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_046', 0, function () {
        console.info('device_info_test_046 start');
        let manufactureInfo = deviceinfo.manufacture;
        console.info('the value of the deviceinfo manufacture is :' + manufactureInfo);

        let len = manufactureInfo.length
        console.info('the value of the manufacture characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_046 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0520
     * @tc.name      testGetProductBrand03
     * @tc.desc      Get a string representing the external product family name which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_047', 0, function () {
        console.info('device_info_test_047 start');
        let brandInfo = deviceinfo.brand;
        console.info('the value of the deviceinfo brand is :' + brandInfo);

        let len = brandInfo.length;
        console.info('the value of the external product family name characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_047 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0530
     * @tc.name      testGetMarketName03
     * @tc.desc      Get a string representing the product series which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_048', 0, function () {
        console.info('device_info_test_048 start');
        let marketNameInfo = deviceinfo.marketName;
        console.info('the value of the deviceinfo marketName is :' + marketNameInfo);

        let len = marketNameInfo.length;
        console.info('the value of the product series characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_048 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0540
     * @tc.name      testGetProductSeries03
     * @tc.desc      Get a string representing the product series which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_049', 0, function () {
        console.info('device_info_test_049 start');
        let productSeriesInfo = deviceinfo.productSeries;
        console.info('the value of the deviceinfo productSeries is :' + productSeriesInfo);

        let len = productSeriesInfo.length;
        console.info('the value of the product series characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_049 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0550
     * @tc.name      testGetProductModel03
     * @tc.desc      Get a string representing the certified model which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_050', 0, function () {
        console.info('device_info_test_050 start');
        let productModelInfo = deviceinfo.productModel;
        console.info('the value of the deviceinfo productModel is :' + productModelInfo);

        let len = productModelInfo.length
        console.info('the value of the certified model characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_050 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0560
     * @tc.name      testGetSoftwareModel03
     * @tc.desc      Get a string representing the internal software sub-model which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_051', 0, function () {
        console.info('device_info_test_036 start');
        let softwareModelInfo = deviceinfo.softwareModel;
        console.info('the value of the deviceinfo softwareModel is :' + softwareModelInfo);

        let len = softwareModelInfo.length
        console.info('the value of the internal software sub-model characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_036 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0570
     * @tc.name      testGetHardwareModel03
     * @tc.desc      Get a string representing the hardware version which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_052', 0, function () {
        console.info('device_info_test_052 start');
        let hardwareModelInfo = deviceinfo.hardwareModel;
        console.info('the value of the deviceinfo hardwareModel is :' + hardwareModelInfo);

        let len = hardwareModelInfo.length;
        console.info('the value of the hardware version characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_052 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0580
     * @tc.name      testGetHardwareProfile03
     * @tc.desc      Get a string representing the hardware version which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_053', 0, function () {
        console.info('device_info_test_053 start');
        let hardwareProfileInfo = deviceinfo.hardwareProfile;
        console.info('the value of the deviceinfo hardwareProfile is :' + hardwareProfileInfo);

        let len = hardwareProfileInfo.length;
        console.info('the value of the hardware version characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_053 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0590
     * @tc.name      testGetSerial03
     * @tc.desc      Get a string representing the device serial number which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_054', 0, function () {
        console.info('device_info_test_054 start');
        let serialInfo = deviceinfo.serial;
        console.info('the value of the deviceinfo serial is :' + serialInfo);

        let len = serialInfo.length;
        console.info('the value of the device serial number characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_054 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0610
     * @tc.name      testGetDisplayVersion03
     * @tc.desc      Get a string representing the version number visible to users which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_055', 0, function () {
        console.info('device_info_test_055 start');
        let displayVersionInfo = deviceinfo.displayVersion;
        console.info('the value of the deviceinfo displayVersion is :' + displayVersionInfo);

        let len = displayVersionInfo.length;
        console.info('the value of the device serial number characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_055 ：end');
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0620
     * @tc.name      testGetIncrementalVersionInfo02
     * @tc.desc      Get a string representing the version number visible to users which has a maximum of 32 characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_056', 0, function () {
        console.info('device_info_test_056 start');
        let incrementalVersionInfo = deviceinfo.incrementalVersion;
        console.info('the value of the deviceinfo incrementalVersion is :' + incrementalVersionInfo);

        let len = incrementalVersionInfo.length
        console.info('the value of the device serial number characters is :' + len)
        expect(len).assertLess(MAX_CHARACTERS_NUM_ONE)
        console.info('device_info_test_056 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0630
     * @tc.name      testGetIncrementalVersionInfo03
     * @tc.desc      Get a string representing the version number visible to users which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_057', 0, function () {
        console.info('device_info_test_057 start');
        let incrementalVersionInfo = deviceinfo.incrementalVersion;
        console.info('the value of the deviceinfo incrementalVersion is :' + incrementalVersionInfo);

        let len = incrementalVersionInfo.length
        console.info('the value of the device serial number characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_057 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0640
     * @tc.name      testGetVersionId03
     * @tc.desc      Get a string representing the operating system full name which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_058', 0, function () {
        console.info('device_info_test_058 start');
        let versionIdInfo = deviceinfo.versionId;
        console.info('the value of the deviceinfo versionId is :' + versionIdInfo);

        let len = versionIdInfo.length
        console.info('the value of the operating system full name characters is :' + len)
        expect(len).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_058 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0650
     * @tc.name      testGetBuildUser03
     * @tc.desc      Get a string representing the different build user of the same baseline code which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_059', 0, function () {
        console.info('device_info_test_043 start');
        let buildUserInfo = deviceinfo.buildUser;
        console.info('the value of thebuildUser is :' + buildUserInfo);

        console.info('the value of the different build user of the same baseline code characters is :' + buildUserInfo.length);
        expect(buildUserInfo.length).assertLarger(MAX_CHARACTERS_NUM)
        console.info('device_info_test_059 ：end')
    })

    /**
     * @tc.number    SUB_STARTUP_JS_DEVCEINFO_0660
     * @tc.name      testGetBuildHost03
     * @tc.desc      Get a string representing the different build host of the same baseline code which has at least one characters.
     * @tc.size      : MEDIUM
     * @tc.type      : Function
     * @tc.level     : Level 0
     */
    it('device_info_test_060', 0, function () {
        console.info('device_info_test_060 start');
        let buildHostInfo = deviceinfo.buildHost;
        console.info('the value of the deviceinfo buildHost is :' + buildHostInfo);

        let len = buildHostInfo.length
        console.info('the value of the different build host of the same baseline code characters is :' + len);
        expect(len).assertLarger(MAX_CHARACTERS_NUM);
        console.info('device_info_test_060 ：end');
    })
})

