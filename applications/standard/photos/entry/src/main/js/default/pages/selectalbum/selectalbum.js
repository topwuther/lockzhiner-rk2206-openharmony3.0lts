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
 * @file: 相册选择界面
 */

import router from '@system.router';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let media = mediaLibrary.getMediaLibraryHelper();

// 加载数据延时
const LOAD_DATA_TIME = 50;

// 懒加载列表数
const PAGE_SIZE = 10;

// 视频类型
const VIDEO_TYPE = 4;

export default {
    data: {
        listData: [],
        otherList: [],
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: true,
            isShowRight: false
        },
        isOperationFrom: false,
        operationType: '',
        createParams: null,
        checkedList: [],
        fromAlbum: null,
        listItemStyle: {
            height: '160px'
        },
        utils: null,
        cacheList: []
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('selectAlbum => onInit');
        this.initNational();
    },

    /**
    * 初始化菜单资源
    */
    initNational() {
        this.utils.logDebug('selectAlbum => initNational');
        this.topBarSource.leftSrc = this.utils.getIcon('back');
        this.topBarSource.rightSrc = this.utils.getIcon('add');
        this.topBarSource.title = this.$t('strings.chooseAlbums');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.utils.logDebug('selectAlbum => onShow');
        if (this.operationType) {
            this.topBarSource.title = this.$t('strings.operateTitle');
        }
        this.getAlbums();
    },

    /**
    * 获取相册
    */
    getAlbums() {
        let self = this;
        let args = {
            selections: '',
            selectionArgs: ['imagealbum'],
        };
        self.utils.logDebug('selectAlbum => getImageAlbums => startTime');
        media.getImageAlbums(args, (error, albums) => {
            self.utils.logDebug('selectAlbum => getImageAlbums => endTime');
            if (albums) {
                let list = [];
                let others = [];
                for (let i = 0; i < albums.length; i++) {
                    let album = albums[i];
                    let gridObj = {
                        name: album.albumName,
                        id: album.albumId,
                        icon: '/common/image/svg/arrow-right.svg',
                        checked: false,
                        showNumber: true,
                        src: self.$app.$def.utils.getIcon('default'),
                        list: []
                    };
                    setTimeout(() => {
                        self.getMediaAssets(gridObj, album, list, others);
                    }, (i + 1) * LOAD_DATA_TIME);
                }
                setTimeout(() => {
                    if (others.length > PAGE_SIZE) {
                        self.otherList = others.slice(0, PAGE_SIZE);
                    } else {
                        self.otherList = others;
                    }
                    self.cacheList = others;
                    self.listData = list;
                }, (albums.length + 1) * LOAD_DATA_TIME);
            }
        });
    },

    /**
    * 获取相册
    */
    getMediaAssets(gridObj, album, list, others) {
        let self = this;
        let args = {
            selections: album.albumName,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };
        self.utils.logDebug('selectAlbum => getMediaAssets => startTime =>');
        media.getMediaAssets(args, (error, images) => {
            self.utils.logDebug('selectAlbum => getMediaAssets => endTime =>');
            if (images && images.length > 0) {
                gridObj.src = images[0].mediaType === VIDEO_TYPE
                    ? '/common/image/icon/video_poster.png' : 'file://' + images[0].URI;
                gridObj.list = images;
            }
            // 判断是新建选相册  还是操作选相册
            if (self.createParams && self.createParams.type === 'createAlbum'
            && images && images.length > 0) {
                if (album.albumName === 'camera') {
                    gridObj.type = 'system';
                    list.push(gridObj);
                } else {
                    others.push(gridObj);
                }
                // 剔除掉当前选中的相册， 由于查询出来是全部相册，避免移动/复制到当前选中相册
            } else if (self.isOperationFrom && self.fromAlbum.name !== album.albumName) {
                if (album.albumName === 'camera') {
                    gridObj.type = 'system';
                    list.push(gridObj);
                } else {
                    others.push(gridObj);
                }
            }
        });
    },

    /**
    * 顶部左侧按钮
    */
    topBarLeftClick() {
        this.utils.logDebug('selectAlbum => topBarLeftClick');
        router.back();
    },

    /**
    * 列表点击回调
    *
    * @param {Object} obj - 当前点击列表项
    */
    listClick(obj) {
        let self = this;
        self.utils.logDebug('selectAlbum => listClick');
        let detail = obj.detail.item;
        if (self.createParams) {
            self.createParams.album = detail;
        }

        let routerParams = {
            uri: 'pages/selectAlbumPhoto/selectAlbumPhoto',
            params: {
                album: detail,
                isOperationFrom: self.isOperationFrom,
                createParams: self.createParams
            }
        };
        if (self.isOperationFrom) {
            routerParams.params.operationType = self.operationType;
            if (self.fromAlbum.name === self.$t('strings.allPhotos') && self.isRepeatAlbum(detail)) {
                routerParams.params.operationType = '';
            }
            routerParams.uri = 'pages/afterSelect/afterSelect';
            routerParams.params.list = self.checkedList;
            routerParams.params.fromAlbum = self.fromAlbum;
            routerParams.params.toAlbum = detail;
            routerParams.params.topBarSource = {
                title: detail.name,
                leftSrc: '/common/image/svg/back.svg',
                rightSrc: '/common/image/icon/selected.png',
                isShowLeft: true,
                isShowRight: false
            };
        }

        router.replace(routerParams);
    },

    /**
    * 判断所有照片入口进来 是否操作了自身相册
    *
    * @param {Object} detail - 当前操作项
    * @return {boolean} Verify result
    */
    isRepeatAlbum(detail) {
        let self = this;
        self.utils.logDebug('selectAlbum => isRepeatAlbum');
        let list = self.checkedList;
        let flag = false;
        for (let i = 0; i < list.length; i++) {
            let item = list[i];
            let arrStr = item.URI.split('/');
            let albumName = arrStr[arrStr.length - 2];
            if (albumName === detail.name) {
                flag = true;
                break;
            }
        }
        return flag;
    },

    /**
    * 列表滑动到底部
    */
    scrollBottom() {
        let self = this;
        this.utils.logDebug('selectAlbum => scrollBottom');
        let listLen = self.otherList.length;
        let cacheLen = self.cacheList.length;
        let len = cacheLen - listLen;
        if (len > PAGE_SIZE) {
            self.otherList = self.otherList.concat(self.cacheList.slice(listLen, listLen + PAGE_SIZE));
        } else if (len > 0 && len < PAGE_SIZE) {
            self.otherList = self.otherList.concat(self.cacheList.slice(listLen, listLen + len));
        } else {
            self.utils.logDebug('selectAlbum => scrollBottom => lenError');
        }
    }
};
