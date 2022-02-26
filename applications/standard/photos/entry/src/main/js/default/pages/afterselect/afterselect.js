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

/**
 * @file: 图片、视频选择界面
 */

import router from '@system.router';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let media = mediaLibrary.getMediaLibraryHelper();

// 进度条
const PROGRESS = 100;

// 调用operations延时
const OPERATION_TIME = 50;

// 照片类型
const PHOTO_TYPE = 3;

// 视频类型
const VIDEO_TYPE = 4;

// 懒加载列表数
const PAGE_SIZE = 28;

// 懒加载列表数
const CLOSE_TIME = 200;

export default {
    data: {
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: true,
            isShowRight: false
        },
        isShowBottomBar: false,
        list: [],
        gridItemWidth: '176px',

        // 操作方式
        operationType: '',
        progress: 0,
        progressTitle: '',
        operateType: '',
        album: null,
        fromAlbum: null,
        toAlbum: null,
        utils: null,
        videoType: VIDEO_TYPE,
        photoType: PHOTO_TYPE,
        cacheOtherList: []
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('afterSelect => onInit');
        this.topBarSource.leftSrc = this.utils.getIcon('back');
        this.topBarSource.rightSrc = this.utils.getIcon('select');
        this.progressTitle = this.topBarSource.title;
    },

    /**
    * 初始化数据
    */
    onReady() {
        this.utils.logDebug('afterSelect => onReady');
        let self = this;
        setTimeout(() => {
            self.$element('progress_dialog').show();
            this.operations();
        }, OPERATION_TIME);
    },

    /**
    * 操作后调用方法
    */
    operations() {
        this.utils.logDebug('afterSelect => operations');
        let self = this;
        self.initData();
        if (self.operationType === 'move') {
            self.operateType = self.$t('strings.moving');
            if (self.fromAlbum.name === self.$t('strings.allPhotos')
            || self.fromAlbum.name === self.$t('strings.video')) {
                self.mediaAllAlbumMove();
            } else {
                self.mediaMove();
            }
        } else if (self.operationType === 'copy') {
            self.operateType = self.$t('strings.copying');
            if (self.fromAlbum.name === self.$t('strings.allPhotos')
            || self.fromAlbum.name === self.$t('strings.video')) {
                self.mediaAllAlbumCopy();
            } else {
                self.mediaCopy();
            }
        } else {
            self.loadData();
        }
        self.$app.$def.dataManage.isRefreshed(true);
    },

    /**
    * 所有相册入口复制
    */
    mediaAllAlbumCopy() {
        this.utils.logDebug('afterSelect => mediaAllAlbumCopy => startTime');
        let self = this;
        for (let i = 0; i < self.list.length; i++) {
            let item = self.list[i];
            let arrs = item.URI.split('/');
            let albumName = arrs[arrs.length - 2];
            let args = {
                selections: albumName,
                selectionArgs: ['imagealbum', 'videoalbum'],
            };
            media.getMediaAssets(args, (error, images) => {
                if (images) {
                    self.dealAllMediaCopyAsset(i, images, item);
                }
            });
        }
    },

    /**
    * 处理所有相册入口复制数据
    *
    * @param {Number} i - 所有操作数组下标
    * @param {Array} images - 查询出的数据
    * @param {Object} item - 当前项
    */
    dealAllMediaCopyAsset(i, images, item) {
        let self = this;
        for (let j = 0; j < images.length; j++) {
            let fromItem = images[j];
            if (fromItem.name === item.name) {
                self.mediaCreateAsset(
                    'copy',
                    fromItem,
                    self.toAlbum.name,
                    () => {
                        if (i === self.list.length - 1) {
                            self.$app.$def.dataManage.setPhotoList([]);
                            self.loadData();
                            self.utils.logDebug('afterSelect => mediaAllAlbumCopy => endTime');
                        }
                    }
                );
            }
        }
    },

    /**
    * 复制功能
    */
    mediaCopy() {
        this.utils.logDebug('afterSelect => mediaCopy => startTime');
        let self = this;
        let args = {
            selections: self.fromAlbum.name,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };
        media.getMediaAssets(args, (error, images) => {
            if (images) {
                self.dealMediaCopyAsset(images);
            }
        });
    },

    /**
    * 处理相册复制数据
    *
    * @param {Array} images - 查询出的数据
    */
    dealMediaCopyAsset(images) {
        let self = this;
        let num = 0;
        for (let j = 0; j < images.length; j++) {
            let fromItem = images[j];
            for (let i = 0; i < self.list.length; i++) {
                let item = self.list[i];
                if (fromItem.name === item.name) {
                    num++;
                    self.mediaCreateAsset(
                        'copy',
                        fromItem,
                        self.topBarSource.title,
                        () => {
                            if (i === self.list.length - 1 && num === self.list.length) {
                                self.$app.$def.dataManage.setPhotoList([]);
                                self.loadData();
                                self.utils.logDebug('afterSelect => mediaCopy => endTime');
                            }
                        }
                    );
                }
            }
        }
    },

    /**
    * 查询
    */
    loadData() {
        this.utils.logDebug('afterSelect => loadData => startTime');
        let self = this;

        let args = {
            selections: self.topBarSource.title,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };
        if (self.album && self.album.name === self.$t('strings.allPhotos')) {
            args.selections = '';
            self.topBarSource.title = self.$t('strings.allPhotos');
        }
        media.getMediaAssets(args, (error, images) => {
            self.utils.logDebug('afterSelect => loadData => endTime');
            if (images) {
                for (let i = 0; i < images.length; i++) {
                    let item = images[i];
                    item.src = 'file://' + item.URI;
                    item.icon = '';
                    item.checked = false;
                }
                self.cacheOtherList = images;
                if (images.length > PAGE_SIZE) {
                    self.list = images.slice(0, PAGE_SIZE);
                } else {
                    self.list = images;
                }
            }
        });

        setTimeout(() => {
            self.progress = PROGRESS;
            setTimeout(() => {
                self.$element('progress_dialog').close();
            }, CLOSE_TIME)
        }, self.list.length * PROGRESS)

    },

    /**
    * 所有相册入口移动功能
    */
    mediaAllAlbumMove() {
        this.utils.logDebug('afterSelect => mediaAllAlbumMove => startTime');
        let self = this;
        for (let i = 0; i < self.list.length; i++) {
            let item = self.list[i];
            let arrData = item.URI.split('/');
            let albumName = arrData[arrData.length - 2];
            let args = {
                selections: albumName,
                selectionArgs: ['imagealbum', 'videoalbum'],
            };
            media.getMediaAssets(args, (error, images) => {
                if (images) {
                    self.dealAllMediaMoveAsset(i, images, item);
                }
            });
        }
    },

    /**
    * 处理所有相册入口移动数据
    *
    * @param {Number} i - 所有操作数组下标
    * @param {Array} images - 查询出的数据
    * @param {Object} item - 当前项
    */
    dealAllMediaMoveAsset(i, images, item) {
        let self = this;
        for (let j = 0; j < images.length; j++) {
            let fromItem = images[j];
            if (fromItem.name === item.name) {
                self.mediaCreateAsset(
                    'move',
                    fromItem,
                    self.toAlbum.name,
                    () => {
                        if (i === self.list.length - 1) {
                            self.$app.$def.dataManage.setPhotoList([]);
                            self.loadData();
                            self.utils.logDebug(
                                'afterSelect => mediaAllAlbumMove => endTime');
                        }
                    }
                );
            }
        }
    },

    /**
    * 移动功能
    */
    mediaMove() {
        this.utils.logDebug('afterSelect => mediaMove => startTime');
        let self = this;
        let args = {
            selections: self.fromAlbum.name,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };

        media.getMediaAssets(args, (error, images) => {
            if (images) {
                self.dealMediaMoveAsset(images);
            }
        });
    },

    /**
    * 处理相册入口移动数据
    *
    * @param {Array} images - 查询出的数据
    */
    dealMediaMoveAsset(images) {
        let self = this;
        let num = 0;
        for (let j = 0; j < images.length; j++) {
            let fromItem = images[j];
            for (let i = 0; i < self.list.length; i++) {
                let item = self.list[i];
                if (fromItem.name === item.name) {
                    num++;
                    self.mediaCreateAsset(
                        'move',
                        fromItem,
                        self.topBarSource.title,
                        () => {
                            if (i === self.list.length - 1 && num === self.list.length) {
                                self.$app.$def.dataManage.setPhotoList([]);
                                self.loadData();
                                self.utils.logDebug('afterSelect => mediaMove => endTime');
                            }
                        }
                    );
                }
            }
        }
    },

    /**
    * 创建资源
    *
    * @param {string} type - 操作类型
    * @param {Object} fromItem - 被操作对象
    * @param {string} albumName - 相册名
    * @param {function} progressCallBack - 进度条回调
    * @param {function} lastCallBack - 操作完成后回调
    */
    mediaCreateAsset(type, fromItem, albumName, lastCallBack) {
        let self = this;
        media.createImageAsset((error, newAsset) => {
            if (newAsset) {
                newAsset.startCreate((error, startFlag) => {
                    if (startFlag) {
                        newAsset.albumName = albumName;
                        self.mediaCopyAsset(type, fromItem, newAsset, lastCallBack);
                    }
                });
            }
        });
    },

    /**
    * 复制资源
    *
    * @param {string} type - 操作类型
    * @param {Object} fromItem - 被操作对象
    * @param {string} albumName - 相册名
    * @param {function} progressCallBack - 进度条回调
    * @param {function} lastCallBack - 操作完成后回调
    */
    mediaCopyAsset(type, fromItem, newAsset, lastCallBack) {
        let self = this;
        fromItem.commitCopy(newAsset, (err, data) => {
            if (type === 'copy') {
                self.progressChange();
                lastCallBack();
                return;
            }
            fromItem.commitDelete((error, commitFlag) => {
                if (commitFlag) {
                    self.progressChange();
                    lastCallBack();
                }
            });
        });
    },

    /**
    * 初始化数据
    */
    initData() {
        this.utils.logDebug('afterSelect => initData');
        let list = this.list;
        for (let index = 0; index < list.length; index++) {
            let item = list[index];
            item.icon = '';
            item.checked = false;
        }
    },

    /**
    * 顶部左侧按钮
    */
    topBarLeftClick() {
        this.utils.logDebug('afterSelect => topBarLeftClick');
        router.back();
    },

    /**
    * 进度条改变事件
    */
    progressChange() {
        let self = this;
        if (self.progress < PROGRESS) {
            self.progress += Math.floor(PROGRESS / self.list.length);
            self.utils.logDebug('afterSelect => progressChange => ' + self.progress);
        } else {
            self.progress = PROGRESS;
        }
    },

    /**
    * 滚动到底部触发事件
    */
    scrollBottom() {
        this.utils.logDebug('afterSelect => scrollBottom');
        let cacheLength = this.cacheOtherList.length;
        let listLength = this.list.length;
        let diffLength = cacheLength - listLength;
        if (diffLength >= PAGE_SIZE) {
            this.list = this.list.concat(this.cacheOtherList.slice(listLength, listLength + PAGE_SIZE));
        } else if (0 < diffLength < PAGE_SIZE) {
            this.list = this.list.concat(this.cacheOtherList.slice(listLength, listLength + diffLength));
        } else {
            this.utils.logDebug('afterSelect => scrollBottom => lenError');
        }
    }
};
