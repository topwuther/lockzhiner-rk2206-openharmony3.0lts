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
 * @file: 主入口
 */

import router from '@system.router';
import prompt from '@system.prompt';
import mediaLibrary from '@ohos.multimedia.medialibrary';
import featureAbility from '@ohos.ability.featureAbility';

let media = mediaLibrary.getMediaLibraryHelper();

// 重命名id
const RENAME_ID = 1;

// 删除id
const DELETE_ID = 2;

// 所有照片id
const ALL_PHOTO_ID = -1;

// 调用getAlbumImage延时
const GET_ALBUM_IMAGE_TIME = 10;

// 视频相册id
const VIDEO_ALBUM_ID = -2;

// 命名重复弹框停留时长
const REPEAT_NAME_TIME = 1000;

// 获取重命名的随机数
const RANDOM_INPUT_NAME = 10;

// 删除相册延时
const DELETE_ALBUM_TIME = 200;

// 懒加载列表数
const PAGE_SIZE = 10;

// 视频类型
const VIDEO_TYPE = 4;

export default {
    data: {
        headTitle: '',
        showEmptyDiv: true,
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: false,
            isShowRight: true
        },
        bottomBarSource: [
            {
                id: RENAME_ID,
                src: '',
                name: '',
                disabled: false,
                visible: true,
            },
            {
                id: DELETE_ID,
                src: '',
                name: '',
                disabled: false,
                visible: true,
            },
        ],
        listData: [],
        gridData: [],
        headSource: null,
        selectMode: false,
        isShowBottomBar: false,
        gridItemStyle: {
            width: '336px',
            padding: '8px',
            height: '490px',
            radius: '24px'
        },
        gridItemCheckedStyle: {
            height: '38px',
            width: '38px',
            padding: '8px',
        },
        gridCopyData: [],
        cacheAlbums: [],
        cacheOtherList: [],
        inputName: '',
        utils: null
    },

    /**
    * 初始化数据
    */
    onInit() {
        const self = this;
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('main => onInit => startTime');
        this.initNational();

        // 判断是否从相机跳转过来
        featureAbility.getWant().then(r => {
            self.utils.logDebug('main => onInit => endTime');
            if (r && r.parameters && r.parameters.uri) {
                router.replace(
                    {
                        uri: 'pages/photoDetail/photoDetail',
                        params: {
                            album: {
                                name: 'camera',
                                list: []
                            }
                        },
                    }
                );
            }
        });
    },

    /**
    * 初始化数据源
    */
    initNational() {
        this.utils.logDebug('main => initNational');
        this.topBarSource.rightSrc = this.utils.getIcon('add');
        this.topBarSource.leftSrc = this.utils.getIcon('close');
        this.headTitle = this.$t('strings.albums');
        this.bottomBarSource[0].name = this.$t('strings.rename');
        this.bottomBarSource[0].src = this.utils.getIcon('rename');
        this.bottomBarSource[1].name = this.$t('strings.delete');
        this.bottomBarSource[1].src = this.utils.getIcon('delete');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.utils.logDebug('main => onShow');
        if (this.$app.$def.dataManage.getRefreshed()) {
            this.loadData();
        }
    },

    /**
    * 回退按钮事件
    *
    * @return {boolean} - 返回是否是选择模式
    */
    onBackPress() {
        this.utils.logDebug('main => onBackPress');
        if (this.selectMode) {
            this.topBarLeftClick();
            return true;
        } else {
            return false;
        }
    },

    /**
    * 根据是否开启选择模式，初始化选中效果
    */
    initChecked() {
        let self = this;
        this.utils.logDebug('main => initChecked');
        let gridList = self.list;
        // 宫格数据
        for (let index = 0; index < gridList.length; index++) {
            let item = gridList[index];
            if (self.selectMode) {
                item.icon = self.utils.getIcon('unselected');
            } else {
                item.icon = '';
            }
            item.checked = false;
        }
    },

    /**
    * 列表滚动到底部事件
    */
    scrollBottom() {
        let self = this;
        this.utils.logDebug('main => scrollBottom');
        let listLen = self.listData.length;
        let cacheLen = self.cacheOtherList.length;

        let len = cacheLen - listLen;
        if (len > PAGE_SIZE) {
            self.listData = self.listData.concat(self.cacheOtherList.slice(listLen, listLen + PAGE_SIZE));
        } else if (len > 0 && len < PAGE_SIZE) {
            self.listData = self.listData.concat(self.cacheOtherList.slice(listLen, listLen + len));
        } else {
            self.utils.logDebug('main => scrollBottom => lenError');
        }
    },

    /**
    * 加载数据
    */
    loadData() {
        this.utils.logDebug('main => loadData');
        let self = this;
        self.selectMode = false;
        self.getAllPhotos().then(allRes => {
            if (allRes.src) {
                self.showEmptyDiv = false;
                if (allRes.mediaType === VIDEO_TYPE) {
                    allRes.src = '/common/image/icon/video_poster.png';
                }
                self.headSource = allRes;
            }
        });
        self.getAlbums().then(albumRes => {
            if (albumRes.grid || albumRes.list) {
                self.gridData = albumRes.grid;
                self.cacheOtherList = albumRes.list;
                if (albumRes.list && albumRes.list.length > PAGE_SIZE) {
                    self.listData = albumRes.list.slice(0, PAGE_SIZE);
                } else {
                    self.listData = albumRes.list;
                }
                self.getVideoAlbum().then(videoRes => {
                    if (videoRes.src) {
                        self.cacheAlbums.push(videoRes);
                        self.gridData.push(videoRes);
                    }
                });
                self.utils.logDebug('main albumRes' + self.listData.length);
            }
        });
        self.$app.$def.dataManage.isRefreshed(false);
    },

    /**
    * 获取所有照片数据
    *
    * @return {Promise} - 返回获取照片promise
    */
    getAllPhotos() {
        this.utils.logDebug('main => getAllPhotos => startTime');
        let self = this;
        return new Promise(function (resolve, reject) {
            media.getMediaAssets((error, value) => {
                self.utils.logDebug('main => getAllPhotos => endTime');
                if (value && value.length > 0) {
                    let obj = {
                        name: self.$t('strings.allPhotos'),
                        id: ALL_PHOTO_ID,
                        icon: '',
                        mediaType: value[0].mediaType,
                        checked: false,
                        type: 'system',
                        src: 'file://' + value[0].URI,
                        list: value
                    };
                    resolve(obj);
                } else {
                    self.showEmptyDiv = true;
                    reject(new Error('get data error'));
                }
            });
        });
    },

    /**
    * 获取所有相册数据
    *
    * @return {Promise} - 返回获取所有相册promise
    */
    getAlbums() {
        this.utils.logDebug('main => getAlbums => startTime');
        let self = this;
        let args = {
            selections: '',
            selectionArgs: ['imagealbum'],
        };
        return new Promise(function (resolve, reject) {
            media.getImageAlbums(args, (error, albums) => {
                self.utils.logDebug('main => getAlbums => endTime');
                if (albums) {
                    self.cacheAlbums = albums;
                    self.dealImageAlbums(albums).then(res => {
                        resolve(res);
                    });
                } else {
                    reject(new Error('get data error'));
                }
            });
        });
    },

    /**
    * 处理图片相册数据
    *
    * @return {Object} - 返回对象数据
    */
    dealImageAlbums(albums) {
        let self = this;
        return new Promise(function (resolve, reject) {
            let list = [];
            let grid = [];
            for (let i = 0; i < albums.length; i++) {
                let album = albums[i];
                setTimeout(() => {
                    let gridObj = {
                        name: album.albumName,
                        id: album.albumId,
                        icon: '',
                        checked: false,
                        type: '',
                        kind: '',
                        src: '',
                        list: []
                    };
                    self.getAlbumImage(album).then((res) => {
                        if (res && res.length > 0) {
                            if (res[0].mediaType === VIDEO_TYPE) {
                                gridObj.src = '/common/image/icon/video_poster.png';
                            } else {
                                gridObj.src = 'file://' + res[0].URI;
                            }
                            gridObj.list = res;
                        }
                        if (album.albumName === 'camera') {
                            gridObj.type = 'system';
                            grid.push(gridObj);
                        } else {
                            list.push(gridObj);
                        }
                        if (i === albums.length - 1) {
                            resolve({
                                list: list,
                                grid: grid
                            });
                        }
                    });
                }, (i + 1) * GET_ALBUM_IMAGE_TIME);
            }
        });
    },

    /**
    * 获取相册图片数据
    *
    * @param {string} album - 指定相册对象
    * @return {Promise} - 返回获取相册图片promise
    */
    getAlbumImage(album) {
        this.utils.logDebug('main => getAlbumImage => startTime');
        let args = {
            selections: album.albumName,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };
        return new Promise(function (resolve, reject) {
            media.getMediaAssets(args, (error, images) => {
                if (images !== null || images !== undefined) {
                    resolve(images);
                } else {
                    reject(new Error('get data error'));
                }
            });
        });
    },

    /**
    * 获取视频相册数据
    *
    * @return {Promise} - 返回获取视频相册promise
    */
    getVideoAlbum() {
        this.utils.logDebug('main => getVideoAlbum => startTime');
        let self = this;
        let args = {
            selections: '',
            selectionArgs: ['videoalbum'],
        };
        return new Promise(function (resolve, reject) {
            media.getVideoAssets(args, (error, albums) => {
                self.utils.logDebug('main => getVideoAlbum => endTime');
                if (albums && albums.length > 0) {
                    let videoObj = {
                        name: self.$t('strings.video'),
                        id: VIDEO_ALBUM_ID,
                        icon: '',
                        checked: false,
                        type: 'system',
                        kind: 'video',
                        src: '/common/image/icon/video_poster.png',
                        list: albums
                    };
                    resolve(videoObj);
                } else {
                    reject(new Error('get data error'));
                }
            });
        });
    },

    /**
    * 新建相册弹窗
    *
    * @param {Object} value - 新建相册名
    */
    createDialogAlbum(value) {
        this.utils.logDebug('main => createDialogAlbum');
        let name = value.detail;
        if (!this.isRepeatName(name)) {
            this.createAlbum(value.detail);
        }
    },

    /**
    * 新建相册接口
    *
    * @param {string} albumName - 新建相册名
    */
    createAlbum(albumName) {
        this.utils.logDebug('main => createAlbum => startTime');
        let self = this;
        self.$element('create_dialog').close();
        media.createAlbum((err, album) => {
            this.utils.logDebug('main => createAlbum => err' + err);
            if (album) {
                album.albumName = albumName;
                album.commitCreate((err, fcommitFlag) => {
                    self.utils.logDebug('main => createAlbum => endTime');
                    if (fcommitFlag) {
                        self.$app.$def.dataManage.isRefreshed(true);
                        router.push({
                            uri: 'pages/selectAlbum/selectAlbum',
                            params: {
                                createParams: {
                                    name: albumName,
                                    type: 'createAlbum'
                                }
                            }
                        });
                    }
                });
            }
        });
    },

    /**
    * 修改相册名称弹窗
    *
    * @param {Object} value - 相册名
    */
    renameDialogAlbum(value) {
        this.utils.logDebug('main => renameDialogAlbum');
        let name = value.detail;
        if (!this.isRepeatName(name)) {
            this.modifyAlbumName(name);
        }
    },

    /**
    * 判断是否重复相册名
    *
    * @param {string} name - 相册名
    * @return {boolean} - 中断代码
    */
    isRepeatName(name) {
        this.utils.logDebug('main => isRepeatName');
        let list = this.cacheAlbums || [];
        let flag = false;
        for (let i = 0; i < list.length; i++) {
            let item = list[i];
            if (item.albumName === name) {
                prompt.showToast({
                    message: this.$t('strings.repeatName'),
                    duration: REPEAT_NAME_TIME
                });
                flag = true;
                break;
            }
        }
        return flag;
    },

    /**
    * 修改相册接口
    *
    * @param {string} albumName - 相册名
    */
    modifyAlbumName(albumName) {
        this.utils.logDebug('main => modifyAlbumName => startTime');
        let self = this;
        self.$element('rename_dialog').close();
        let args = {
            selections: '',
            selectionArgs: ['imagealbum'],
        };
        media.getImageAlbums(args, (error, albums) => {
            if (albums) {
                for (let j = 0; j < albums.length; j++) {
                    let item = albums[j];
                    self.dealCommitModifyAlbum(item, albumName);
                }
            }
        });
    },

    /**
    * 处理修改相册接口数据判断
    *
    * @param {Object} item - 被处理list对象
    * @param {string} albumName - 相册名
    */
    dealCommitModifyAlbum(item, albumName) {
        let self = this;
        let checkedItem = self.getCheckedData()[0];
        if (checkedItem.name === item.albumName) {
            item.albumName = albumName;
            item.commitModify((err, fcommitFlag) => {
                if (fcommitFlag) {
                    self.utils.logDebug('main => modifyAlbumName => endTime');
                    self.$app.$def.dataManage.isRefreshed(true);
                    self.loadData();
                    self.onCheckedChange();
                }
            });
        }
    },

    /**
    * 顶部左侧按钮
    */
    topBarLeftClick() {
        this.utils.logDebug('main => topBarLeftClick');
        let self = this;
        self.selectMode = false;
        self.listData.forEach(item => {
            item.checked = false;
            item.icon = '';
        });
        self.gridData.forEach(item => {
            item.icon = self.utils.getIcon(item.kind);
        });
        self.headSource.checked = false;
        self.headSource.icon = '';
        self.topBarSource.isShowLeft = false;
        self.topBarSource.isShowRight = true;
        self.isShowBottomBar = false;
    },

    /**
    * 顶部右侧按钮
    */
    topBarRightClick() {
        this.utils.logDebug('main => topBarRightClick');
        this.$element('create_dialog').show();
    },

    /**
    * 底部菜单点击事件
    *
    * @param {Object} item - 底部点击项
    * @return {boolean} - 中断代码
    */
    bottomTabClick(item) {
        this.utils.logDebug('main => bottomTabClick');
        let length = this.getCheckedData().length;
        if (item.detail.id === DELETE_ID) {
            if (length === 0) {
                return false;
            }
            let child = this.$child('delete_dialog');
            child.setTitle(this.$t('strings.deleteInfo') + length + this.$t('strings.items'));
            child.show();
        } else if (item.detail.id === RENAME_ID) {
            if (length !== 1) {
                return false;
            }
            this.inputName = this.getCheckedData()[0].name + Math.round(Math.random() * RANDOM_INPUT_NAME);
            this.$element('rename_dialog').show();
        }
    },

    /**
    * 列表点击事件回调
    *
    * @param {Object} obj - 列表点击项
    */
    itemClick(obj) {
        this.utils.logDebug('main => itemClick');
        let item = obj.detail.item;
        let uri = 'pages/photoList/photoList';
        if (item.kind === 'video') {
            uri = 'pages/videoList/videoList';
        }
        router.push(
            {
                uri: uri,
                params: {
                    album: item
                },
            }
        );
    },

    /**
    * 长按事件回调
    */
    longPress() {
        this.utils.logDebug('main => longPress');
        let self = this;
        self.selectMode = true;
        self.topBarSource.isShowLeft = true;
        self.topBarSource.isShowRight = false;
        self.topBarSource.title = self.$t('strings.unChoose');
        self.isShowBottomBar = true;
        self.onCheckedChange();
    },

    /**
    * 选中发生改变事件回调
    */
    onCheckedChange() {
        this.utils.logDebug('main => onCheckedChange');
        let self = this;
        let length = this.getCheckedData().length;
        if (self.selectMode) {
            if (length === 0) {
                self.topBarSource.title = self.$t('strings.unChoose');
                for (let i = 0; i < self.bottomBarSource.length; i++) {
                    let item = self.bottomBarSource[i];
                    item.disabled = true;
                }
            } else if (length !== 1) {
                self.bottomBarSource[0].disabled = true;
                self.bottomBarSource[1].disabled = false;
                self.topBarSource.title = self.$t('strings.selected') + length + self.$t('strings.items');
            } else {
                self.bottomBarSource[0].disabled = false;
                self.bottomBarSource[1].disabled = false;
                self.topBarSource.title = self.$t('strings.selected') + length + self.$t('strings.items');
            }
        } else {
            self.topBarSource.isShowLeft = false;
            self.isShowBottomBar = false;
            self.topBarSource.isShowRight = true;
        }
    },

    /**
    * 选中数据
    *
    * @return {Array} - 选中列表
    */
    getCheckedData() {
        this.utils.logDebug('main => getCheckedData');
        let self = this;
        let list = [];
        self.listData.forEach(item => {
            if (item.checked) {
                list.push(item);
            }
        });
        self.gridData.forEach(item => {
            if (item.checked) {
                list.push(item);
            }
        });
        return list;
    },

    /**
    * 删除回调
    */
    deleteQuery() {
        this.deletePhotos();
    },

    /**
    * 删除相册接口
    */
    deletePhotos() {
        this.utils.logDebug('main => deletePhotos');
        let self = this;
        let choose = self.getCheckedData();
        for (let j = 0; j < choose.length; j++) {
            let chooseItem = choose[j];
            for (let i = 0; i < self.cacheAlbums.length; i++) {
                let item = self.cacheAlbums[i];
                if (item.albumName === chooseItem.name) {
                    self.dealCommitDeleteAlbum(j, item, choose);
                }
            }
        }
    },

    /**
    * 处理删除相册接口数据判断
    *
    * @param {Object} item - 被处理list对象
    * @param {string} albumName - 相册名
    */
    dealCommitDeleteAlbum(j, item, choose) {
        let self = this;
        item.commitDelete((err, deleteFlag) => {
            self.utils.logDebug('main => deletePhotos => endTime');
            if (deleteFlag) {
                self.$app.$def.dataManage.isRefreshed(true);
                if (j === choose.length - 1 && choose.length === self.listData.length) {
                    self.listData = [];
                } else {
                    setTimeout(() => {
                        self.loadData();
                        self.onCheckedChange();
                    }, DELETE_ALBUM_TIME);
                }
            }
        });
    }
};
