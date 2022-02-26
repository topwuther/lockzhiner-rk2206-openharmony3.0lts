/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import prompt from '@system.prompt'
import app from '@system.app';
import client from '@ohos.update';
const HAS_NEW_VERSION = 0;
const NO_NEW_VERSION = 1;
const UPDATE_STATE_DOWNLOAD_FAIL = 23
const UPDATE_STATE_DOWNLOAD_SUCCESS = 24;
const UPDATE_STATE_VERIFY_FAIL = 31;
const UPDATE_STATE_VERIFY_SUCCESS = 32;
var temp=0;
const page= {
    data: {
        title: "当前版本:10.2.1",
        button: "查看更新",
        pageType: "currVersion",
        versionName: "10.2.0",
        size: "20MB",
        journal: "本次更新解决了一些BUG。",
        matter: "请保持50%以上电量",
        showLoad: "",
        width: "5%",
        showBanner: "",
        showButton: "download",
        upgradeInfo: "",
        updater: undefined,
        timer: undefined,
        dialog:"",
        showSimpledialog:""
    },
    onInit() {
        console.info("onInit ");
        try {
            page.data.updater = client.getUpdater('/data/updater/updater.zip', 'OTA');
            page.getCurrVersion();
        } catch(error) {
            console.error(" Fail to get updater error: " + error);
        }
        console.info(`onInit finish `);
    },

    onClick: function() {
        console.info("onClick " + this.pageType);
        console.info("processClick " + page.data.pageType);
        if (page.data.pageType == "currVersion") { // 检查更新版本
            console.info("start to check new version ");
            page.data.pageType = "checkVersion";
            page.data.button = "取消查看";
            page.data.showLoad = "load";
            page.checkNewVersion();
        }  else if (page.data.pageType == "newVersion") { // 当前需要下载最新的版本
            page.data.pageType = "downVersion";
            page.data.showLoad = "load";
            page.data.showBanner = 'banner';
            page.data.button = "取消下载";
            if (page.data.updater == undefined) {
                page.data.pageType = "errorPage";
                page.data.showButton = 'download';
                page.data.button = '退出';
                page.data.title = "初始化出现错误，退出app";
                return;
            }

            page.data.updater.on("downloadProgress", progress => {
                console.log(`downloadProgress status: ` + progress.status);
                console.log(`downloadProgress percent: ` + progress.percent);
                console.log(`downloadProgress endReason: ` + progress.endReason);
                console.log(`downloadProgress pageType: ` + page.data.pageType);
                if (page.data.pageType != "downVersion") {
                    return;
                }
                let percent = progress.percent;
                if (progress.percent > 5) {
                    percent = progress.percent - 5;
                }else if(progress.percent>90){
                    percent = 90;
                }
                page.data.width = percent + '%';
                if (progress.percent == 100) {
                    page.data.showLoad = "";
                    page.data.showBanner = '';
                    if(temp==1){
                        this.$element('simpledialog').close();
                    }

                }
                // 下载成功 UpdateState.UPDATE_STATE_DOWNLOAD_SUCCESS
                if (progress.status == UPDATE_STATE_DOWNLOAD_SUCCESS ||
                progress.status == UPDATE_STATE_VERIFY_SUCCESS) {
                    page.data.pageType = "downSuccess";
                    page.data.showButton = "upgrade";
                    page.data.upgradeInfo =  page.data.versionName + "安装包下载完成，是否安装？";
                    page.data.updater.off("downloadProgress");
                } else if (progress.status == UPDATE_STATE_DOWNLOAD_FAIL ||
                progress.status == UPDATE_STATE_VERIFY_FAIL) { // 失败
                    console.log("downloadProgress error" + progress.endReason);
                    page.data.pageType = "errorPage";
                    page.data.showButton = 'download';
                    page.data.button = '退出';
                    page.data.title = "下载失败";
                    if (progress.endReason) {
                        page.data.title = "下载失败，失败原因：" + progress.endReason;
                    }
                    page.data.updater.off("downloadProgress");
                }
            });
            page.data.updater.download();

        } else if ( this.pageType == "lastVersion") { // 已经是最新的版本了，单击后退出页面
            page.data.showLoad = "";
            app.terminate();
        } else if (page.data.pageType == "checkVersion") { // 检查中，取消检查
            page.data.showSimpledialog="simpledialog";
            this.$element('simpledialog').show();
            page.data.dialog="是否取消检查";
        } else if (page.data.pageType == "downVersion") { // 下载中，取消下载
            page.data.showSimpledialog="simpledialog";
            page.data.dialog="是否取消下载";
            this.$element('simpledialog').show();
            temp=1;
        } else if (page.data.pageType == "errorPage") { // 出错，退出
            app.terminate();
        }
    },



    clickInstall: function() {
        console.info("clickInstall " + page.data.pageType);
        if (page.data.pageType == "downSuccess") { // 下载成功，开始升级
            console.info(`upgrade starting`);
            page.upgrade();
        }
    },

    clickCancel: function() {
        console.info("clickCancel " + page.data.pageType);
        if (page.data.pageType == "downSuccess") { // 下载成功，取消升级
            page.data.showSimpledialog="simpledialog";
            page.data.dialog="是否稍后安装";
            this.$element('simpledialog').show();
        }
    },

    getCurrVersion() {
        if (page.data.updater == undefined) {
            console.error("Can not import client");
            page.data.pageType = "errorPage";
            page.data.showButton = 'download';
            page.data.button = '退出';
            page.data.title = "初始化出现错误，退出app";
            return;
        }
        console.info("getCurrVersion begin " + page.data.updater);
        try {
            // 获取版本信息
            page.data.updater.getNewVersionInfo(function(err, info) {
                console.log("getNewVersionInfo success  " + info.status);
                console.log(`info versionName = ` + info.checkResults[0].versionName);
                console.log(`info versionCode = ` + info.checkResults[0].versionCode);
                console.log(`info verifyInfo = ` + info.checkResults[0].verifyInfo);
                console.log(`info descriptionId = ` + info.checkResults[0].descriptionId);
                console.log(`info size = ` + info.checkResults[0].size);
                if (info.status == NO_NEW_VERSION) { // 已经最新
                    page.data.title = "当前已经是最新版本";
                    page.data.button = "确定";
                    page.data.pageType = "lastVersion";
                    page.data.versionName = info.checkResults[0].versionName;
                } else if (info.status == HAS_NEW_VERSION) {
                    page.data.button = "查看更新";
                    page.data.pageType = "currVersion";
                    page.data.versionName = info.checkResults[0].versionName;
                } else {
                    console.error(" getNewVersionInfo errMsg " + info.errMsg);
                    page.data.title = "获取新版本失败";
                }
            });
        } catch(error) {
            console.error(" getNewVersionInfo catch " + error);
            page.data.title = "获取新版本失败";
        }
    },

    checkNewVersion: function() {
        if (page.data.updater == undefined) {
            page.data.pageType = "errorPage";
            page.data.showButton = 'download';
            page.data.button = '退出';
            page.data.title = "初始化出现错误，退出app";
            return;
        }
        console.log('checkNewVersion begin ');
        page.data.updater.checkNewVersion(function(err, info) {
            page.data.showLoad = "";
            console.error('checkNewVersion info ' + info.status);
            if (info.status == NO_NEW_VERSION) { // 已经最新
                page.data.title = "当前已经是最新版本";
                page.data.button = "确定";
                page.data.pageType = "lastVersion";
            } else if (info.status == HAS_NEW_VERSION) { // 有新版本
                console.log(`info versionName = ` + info.checkResults[0].versionName);
                console.log(`info versionCode = ` + info.checkResults[0].versionCode);
                console.log(`info verifyInfo = ` + info.checkResults[0].verifyInfo);
                console.log(`info descriptionId = ` + info.checkResults[0].descriptionId);
                console.log(`info content = ` + info.descriptionInfo[0].content);
                console.log(`info size = ` + info.checkResults[0].size);
                let size = info.checkResults[0].size / 1024 / 1024;
                page.data.versionName = info.checkResults[0].versionName;
                page.data.size = String(size.toFixed(2)) + "MB";
                if (info.descriptionInfo[0].content != undefined) {
                    page.data.journal = info.descriptionInfo[0].content;
                };
                page.data.pageType = "newVersion";
                page.data.button = "下载更新包";
            } else { // 出错
                console.error(`CheckNewVersion errMsg = ` + info.errMsg);
                page.data.pageType = "errorPage";
                page.data.showButton = 'download';
                page.data.button = '退出';
                page.data.title = "检查新版本失败";
                if (info.errMsg) {
                    page.data.title = "检查新版本失败，失败原因：" + info.errMsg;
                }
            }
        });
    },


    upgrade() {
        if ( page.data.updater == undefined) {
            page.data.pageType = "errorPage";
            page.data.showButton = 'download';
            page.data.button = '退出';
            page.data.title = "初始化出现错误，退出app";
            return;
        }
        page.data.updater.on("upgradeProgress", progress => {
            console.log("upgradeProgress on" + progress);
            console.log(`upgradeProgress status: ` + progress.status);
            console.log(`upgradeProgress percent: ` + progress.percent);
            page.data.width = progress.percent + '%';
            if (progress.status == 3) { // 失败
                page.data.updater.off("upgradeProgress");
                page.data.showLoad = "";
                page.data.pageType = "errorPage";
                page.data.showButton = 'download';
                page.data.button = '退出';
                page.data.title = "升级失败，失败原因：" + progress.endReason ;
            }
        });
        page.data.updater.upgrade();
    },

    closeDialog() {
        this.$element('simpledialog').close();
    },

    cancelDialog() {
        prompt.showToast({
            message: '取消对话框'
        })
    },
    cancelSchedule(e) {
        this.$element('simpledialog').close()
        prompt.showToast({
            message: '取消成功'
        })
    },
    setSchedule(e) {
        this.$element('simpledialog').close()
        prompt.showToast({
            message: '确定成功'
        })
        if (page.data.pageType == "downVersion"){
            page.data.updater.cancel();
            page.data.showLoad = "";
            page.data.showBanner = '';
            page.data.pageType = "newVersion";
            page.data.button = "下载更新包";
        }else if(page.data.pageType =="downSuccess"){
            page.data.upgradeInfo = "";
            page.data.showButton = 'download';
            page.data.button = "查看更新";
            page.data.pageType = "currVersion";
            page.getCurrVersion();
        }else if(page.data.pageType =="checkVersion"){
            page.data.button = "检查更新";
            page.getCurrVersion();
        }

    }
}

export default page;
