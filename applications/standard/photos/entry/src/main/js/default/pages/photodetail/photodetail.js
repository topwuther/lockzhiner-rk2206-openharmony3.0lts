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
 * @file: 图片详情界面
 */

import prompt from '@system.prompt';
import router from '@system.router';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let media = mediaLibrary.getMediaLibraryHelper();

// 移动id
const MOVE_ID = 1;

// 复制id
const COPY_ID = 2;

// 删除id
const DELETE_ID = 3;

// 更多id
const MORE_ID = -10;

// 重命名id
const RENAME_ID = 1;

// 旋转id
const ROTATE_ID = 2;

// 照片类型
const PHOTO_TYPE = 3;

// 视频类型
const VIDEO_TYPE = 4;

// 操作初始值
const INIT_REFRESH_TYPE = -1;

// 删除操作
const DELETE_REFRESH_TYPE = 1;

// 修改操作
const MODIFY_REFRESH_TYPE = 2;

// 详细信息尺寸取小数点后4位
const DETAIL_SIZE = 4;

// 图片尺寸转换单位
const DATA_SIZE = 1024;

// 加载数据延时
const LOAD_DATA_TIME = 200;

// 图片缩小倍数
const SCALE_MINUS = 0.25;

// 图片放大倍数
const SCALE_ADD = 0.5;

// 图片最大放大倍数
const SCALE_MAX = 2;

// 命名重复弹框停留时长
const REPEAT_NAME_TIME = 1000;

// 重命名随机数
const RANDOM_NAME = 100;

// 旋转90度
const ROTATE_DEG = 90;

// 旋转270度
const ROTATE_DEG_MIN = -270;

// 懒加载列表数
const PAGE_SIZE = 28;

// 操作弹框延时
const POP_TIME = 50;

export default {
    data: {
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: true,
            isShowRight: true
        },
        shareIndex: 0,
        album: null,
        list: [],
        selectMode: false,
        isShowBottom: false,
        popVisible: false,
        bottomBarSource: [
            {
                id: MOVE_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            },
            {
                id: COPY_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            },
            {
                id: DELETE_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            },
            {
                id: MORE_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            },
        ],
        bottomBarPopList: [
            {
                id: RENAME_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            }, {
                id: ROTATE_ID,
                src: '',
                name: '',
                visible: true,
                disabled: false
            }
        ],
        commonStyle: {
            width: '176px',
            height: '180px'
        },
        currentItem: {},
        cacheList: [],
        cacheSource: [],
        inputName: '',
        refreshType: INIT_REFRESH_TYPE,
        detailData: {
            size: 0,
            name: ''
        },
        showScale: true,
        isScaleMinusDisable: false,
        isScaleAddDisable: false,
        utils: null,
        videoType: VIDEO_TYPE,
        photoType: PHOTO_TYPE
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('photoDetail => onInit');
        this.initNational();
        if (this.selectMode) {
            this.isShowBottom = false;
        } else {
            this.isShowBottom = true;
        }
    },

    /**
    * 回退按钮事件
    *
    * @return {boolean} Verify result
    */
    onBackPress() {
        this.utils.logDebug('photoDetail => onBackPress');
        this.topBarLeftClick();
        return true;
    },

    /**
    * 初始化菜单数据
    */
    initNational() {
        this.utils.logDebug('photoDetail => initNational');
        this.headTitle = this.$t('strings.albums');
        this.topBarSource.leftSrc = this.utils.getIcon('back');
        this.bottomBarSource[0].name = this.$t('strings.move');
        this.bottomBarSource[0].src = this.utils.getIcon('move');
        this.bottomBarSource[1].name = this.$t('strings.copy');
        this.bottomBarSource[1].src = this.utils.getIcon('copy');
        this.bottomBarSource[2].name = this.$t('strings.delete');
        this.bottomBarSource[2].src = this.utils.getIcon('delete');
        this.bottomBarSource[3].name = this.$t('strings.more');
        this.bottomBarSource[3].src = this.utils.getIcon('more');
        this.bottomBarPopList[0].name = this.$t('strings.rename');
        this.bottomBarPopList[1].name = this.$t('strings.rotate');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.utils.logDebug('photoDetail => onShow');
        this.loadData();
    },

    /**
    * 加载数据
    *
    * @return {boolean} Verify result
    */
    loadData() {
        let self = this;
        self.utils.logDebug('photoDetail => loadData');
        if (self.album) {
            if (self.album.name === self.$t('strings.video')) {
                self.getVideoAsset();
                return false;
            } else {
                self.getAlbumAsset();
            }
        }
    },

    /**
    * 获取视频数据
    */
    getVideoAsset() {
        let self = this;
        self.utils.logDebug('photoDetail => getVideoAsset => startTime');
        let photoList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList());
        if (photoList.length > 0) {
            self.list = photoList;
        }
        let args = {
            selections: '',
            selectionArgs: ['videoalbum'],
        };
        media.getVideoAssets(args, (error, videos) => {
            self.utils.logDebug('photoDetail => getVideoAsset => endTime');
            self.cacheList = videos || [];
            if (videos && videos.length > 0) {
                let list = self.dealListData(videos || []);
                if (self.refreshType === DELETE_REFRESH_TYPE) {
                    if (self.shareIndex > 0) {
                        self.shareIndex--;
                    } else {
                        self.shareIndex = 0;
                    }
                }

                // 如果长度为0 表示此从相机跳转过来或者是操作后刷新数据,实现分页
                if (photoList.length === 0) {
                    if (list.length >= PAGE_SIZE) {
                        self.list = list.slice(0, PAGE_SIZE);
                    } else {
                        self.list = list;
                    }
                    self.isRrefreshed = INIT_REFRESH_TYPE;
                }

                // 如果从列表点击的是最后一项进来, 并且后面还有数据的话， 要手动连接后面的数据 由于swiper滑动响应不到
                if (self.shareIndex === self.list.length - 1 && list.length > self.list.length) {
                    self.list = self.list.concat(list.slice(self.list.length, self.list.length + PAGE_SIZE));
                }
                self.initBar();
                self.cacheSource = list;
            }
        });
    },

    /**
    * 获取图片及所有照片数据
    */
    getAlbumAsset() {
        let self = this;
        self.utils.logDebug('photoDetail => getAlbumAsset => startTime');
        let args = {
            selections: self.album.name,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };

        //  表示直接从相机到主页面跳转过来
        if (self.list.length === 0) {
            self.topBarSource.title = 'camera';
            self.topBarSource.rightSrc = self.utils.getIcon('info');
            args.selections = 'camera';
            self.isShowBottom = true;
        }

        //  判断是否是所有照片， 获取当前图片所在相册
        if (self.album.name === self.$t('strings.allPhotos')) {
            args.selections = '';
        }
        self.getMedia(args);
    },

    /**
    * 获取图片及所有照片数据接口调用
    *
    * @param{Object} args - 接口参数
    */
    getMedia(args) {
        let self = this;
        let photoList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList());
        if (photoList.length > 0) {
            self.list = photoList;
        }
        media.getMediaAssets(args, (error, images) => {
            self.utils.logDebug('photoDetail => getAlbumAsset => endTime');
            self.cacheList = images || [];
            let list = self.dealListData(images || []);
            if (self.refreshType === DELETE_REFRESH_TYPE) {
                if (self.shareIndex > 0) {
                    self.shareIndex--;
                } else {
                    self.shareIndex = 0;
                }
            }
            if (photoList.length === 0) {
                if (list.length >= PAGE_SIZE) {
                    self.list = list.slice(0, PAGE_SIZE);
                } else {
                    self.list = list;
                }
                self.refreshType = INIT_REFRESH_TYPE;
            }

            // 如果从列表点击的是最后一项进来, 并且后面还有数据的话， 要手动连接后面的数据， 由于swiper滑动响应不到
            if (self.shareIndex === self.list.length - 1 && list.length > self.list.length) {
                self.list = self.list.concat(list.slice(self.list.length, self.list.length + PAGE_SIZE));
            }
            self.initBar();
            self.cacheSource = list;
        });
    },

    /**
    * 处理接口返回数据
    *
    * @param{Array} list - 接口返回参数
    * @return{Array} storeList - 处理后的数据
    */
    dealListData(list) {
        let self = this;
        let storeList = [];
        for (let i = 0; i < list.length; i++) {
            let item = list[i];
            item.icon = item.icon = self.selectMode ? self.utils.getIcon('unselected') : '';
            item.src = 'file://' + item.URI;
            item.checked = false;
            item.rotate = 0;
            item.scale = 1;
            item.isPause = true;
            item.itemStyle = Object.assign({}, self.commonStyle);
            item.imageStyle = Object.assign({}, self.commonStyle);
            storeList.push(item);
        }
        return storeList;
    },
    initBar() {
        let self = this;
        if (self.list.length > 0) {
            let item = self.list[self.shareIndex];
            self.currentItem = item;
            self.utils.logDebug(' ===  > ' + JSON.stringify(self.currentItem) + '      ');
            self.topBarSource.title = self.currentItem.name;
            if (self.currentItem.mediaType === VIDEO_TYPE) {
                self.showScale = false;
                self.bottomBarPopList[1].visible = false;
            } else {
                self.showScale = true;
                self.bottomBarPopList[1].visible = true;
            }
            if (self.selectMode) {
                self.isShowBottom = false;
                self.changeRightIcon(item);
            } else {
                self.topBarSource.rightSrc = self.utils.getIcon('info');
                self.isShowBottom = true;
            }
        }
    },

    /**
    * 选中发生改变
    */
    onCheckedChange() {
        let self = this;
        self.utils.logDebug('photoDetail => onCheckedChange');
        let length = self.getCheckedData().length;
        if (length === 0) {
            self.topBarSource.title = self.$t('strings.unChoose');
        } else {
            self.topBarSource.title = self.$t('strings.selected') + length + self.$t('strings.items');
        }
    },

    /**
    * 获取选中数据
    *
    * @return {Array} list - 当前选中数据
    */
    getCheckedData() {
        let self = this;
        self.utils.logDebug('photoDetail => getCheckedData');
        let list = [];
        self.list.forEach(item => {
            if (item.checked) {
                list.push(item);
            }
        });
        return list;
    },

    /**
    * 滑动回调
    *
    * @param {Object} item - 滑动后当前显示项
    * @return {boolean} Verify result
    */
    swiperChange(item) {
        this.utils.logDebug('photoDetail => swiperChange');
        let obj = this.list[item.index];
        this.currentItem.scale = 1;
        this.currentItem.rotate = 0;
        this.currentItem = obj;
        this.shareIndex = item.index;
        this.isScaleAddDisable = false;
        this.isScaleMinusDisable = false;
        if (this.currentItem.mediaType === VIDEO_TYPE) {
            this.showScale = false;
            this.bottomBarPopList[1].visible = false;
        } else {
            this.showScale = true;
            this.bottomBarPopList[1].visible = true;
        }
        if (this.selectMode) {
            this.changeRightIcon(obj);
        } else {
            this.topBarSource.title = this.currentItem.name;
        }
        let cacheLen = this.cacheSource.length;
        let listLen = this.list.length;
        if (this.shareIndex === listLen - 1) {
            let len = cacheLen - listLen;
            if (len === 0) {
                return false;
            }
            let concatList = [];
            if (len >= PAGE_SIZE) {
                concatList = this.cacheSource.slice(listLen, listLen + PAGE_SIZE);
            } else if (len > 0 && len < PAGE_SIZE) {
                concatList = this.cacheSource.slice(listLen, listLen + len);
            } else {
                this.utils.logDebug('photoDetail => swiperChange => lenError');
            }
            this.list = this.list.concat(this.initConcatList(concatList));
        }
    },

    /**
    * 初始化需要连接的数据
    *
    * @params {Array} list - 被初始化的数组
    * @return {Array} 返回处理后的数组
    */
    initConcatList(list) {
        let self = this;
        if (self.selectMode) {
            list.forEach(item => {
                item.icon = self.utils.getIcon('unselected');
                item.checked = false;
            });
        }
        return list;
    },

    /**
    * 滑动选中切换事件
    *
    * @param {Object} item - 滑动后当前项显示项
    */
    changeRightIcon(item) {
        let self = this;
        self.utils.logDebug('photoDetail => changeRightIcon');
        if (item.checked) {
            self.topBarSource.rightSrc = self.utils.getIcon('selected');
            item.icon = self.utils.getIcon('selected');
        } else {
            self.topBarSource.rightSrc = self.utils.getIcon('unselected_black');
            item.icon = self.utils.getIcon('unselected');
        }
        this.onCheckedChange();
    },

    /**
    * 顶部右侧按钮点击事件
    */
    topBarRightClick() {
        this.utils.logDebug('photoDetail => topBarRightClick');
        this.hideBottomPop();
        setTimeout(() => {
            if (this.selectMode) {
                this.currentItem.checked = !this.currentItem.checked;
                this.changeRightIcon(this.list[this.shareIndex]);
            } else {
                this.detailData.size = (Number(this.currentItem.size) / DATA_SIZE / DATA_SIZE).toFixed(DETAIL_SIZE);
                this.detailData.name = this.currentItem.name;
                this.$element('photo_detail_dialog').show();
            }
        }, POP_TIME);
    },

    /**
    * 详情弹窗取消
    */
    dialogCancel() {
        this.utils.logDebug('photoDetail => dialogCancel');
        this.$element('photo_detail_dialog').close();
    },

    /**
    * 顶部左侧按钮点击事件
    */
    topBarLeftClick() {
        this.utils.logDebug('photoDetail => topBarLeftClick');
        this.list.forEach(item => {
            item.rotate = 0;
            item.scale = 1;
        });
        this.$app.$def.dataManage.setPhotoList(this.list);
        router.back();
    },

    /**
    * 底部菜单按钮事件
    *
    * @param {Object} item - 底部菜单当前点击项
    */
    bottomTabClick(item) {
        this.utils.logDebug('photoDetail => bottomTabClick');
        if (item.detail.id === MOVE_ID) {
            this.movePhotos();
        } else if (item.detail.id === COPY_ID) {
            this.copyPhotos();
        } else if (item.detail.id === DELETE_ID) {
            this.deletePhotos();
        }
    },

    /**
    * 弹出提示框点击事件
    *
    * @param {Object} item - 弹窗pop层当前点击项
    */
    popupItemClick(item) {
        let self = this;
        self.utils.logDebug('photoDetail => popupItemClick');
        if (item.detail.id === MOVE_ID) {
            let arrData = self.currentItem.name.split('.');
            let firstName = arrData[0];
            let lastName = arrData[1];
            if (self.currentItem.mediaType === PHOTO_TYPE) {
                self.inputName = firstName + Math.round(Math.random() * RANDOM_NAME) + '.' + lastName;
            } else if (self.currentItem.mediaType === VIDEO_TYPE) {
                self.inputName = lastName + Math.round(Math.random() * RANDOM_NAME) + '.' + lastName;
            }
            self.$element('rename_dialog').show();
        } else if (item.detail.id === COPY_ID) {
            self.currentItem.scale = 1;
            self.isScaleAddDisable = false;
            self.isScaleMinusDisable = false;
            if (self.currentItem.rotate === ROTATE_DEG_MIN) {
                self.currentItem.rotate = 0;
            } else {
                self.currentItem.rotate -= ROTATE_DEG;
            }
        }
    },

    /**
    * 修改名称弹窗
    *
    * @param {Object} value - 弹窗修改相册对象
    */
    renameDialogAlbum(value) {
        this.utils.logDebug('photoDetail => renameDialogAlbum');
        let name = value.detail;
        if (!this.isRepeatName(name)) {
            this.modifyName(name);
        }
    },

    /**
    * 修改名称弹窗
    *
    * @param {string} albumName - 当前修改相册名
    */
    modifyName(albumName) {
        let self = this;
        self.utils.logDebug('photoDetail => modifyName');
        self.$element('rename_dialog').close();
        if (self.album.name === self.$t('strings.video')) {
            self.modifyVideoName(albumName);
        } else {
            self.modifyPhotoName(albumName);
        }
    },

    /**
    * 修改名称接口
    *
    * @param {string} albumName - 当前修改相册名
    */
    modifyPhotoName(albumName) {
        let self = this;
        self.utils.logDebug('photoDetail => modifyPhotoName => startTime');
        let checkItem = self.currentItem;
        let args = {
            selections: self.album.name,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };

        // 判断是否是所有照片， 获取当前图片所在相册
        if (self.album.name === self.$t('strings.allPhotos')) {
            args.selections = '';
        }
        media.getMediaAssets(args, (error, images) => {
            if (images) {
                for (let i = 0; i < images.length; i++) {
                    let item = images[i];
                    if (item.id === checkItem.id) {
                        self.dealStartModify(item, albumName);
                    }
                }
            }
        });
    },

    /**
    * 处理图片开始修改名称接口
    *
    * @param {Object} item - 当前修改项
    * @param {string} albumName - 当前修改相册名
    */
    dealStartModify(item, albumName) {
        let self = this;
        item.startModify((error, startFlag) => {
            if (startFlag) {
                item.name = albumName;
                item.commitModify((error, commitFlag) => {
                    if (commitFlag) {
                        self.utils.logDebug('photoDetail => modifyName => endTime');
                        self.refreshType = MODIFY_REFRESH_TYPE;
                        self.$app.$def.dataManage.setPhotoList([]);
                        setTimeout(() => {
                            self.loadData();
                        }, LOAD_DATA_TIME);
                    }
                });
            }
        });
    },

    /**
    * 修改视频名称
    *
    * @param {string} albumName - 当前修改视频名
    */
    modifyVideoName(albumName) {
        let self = this;
        self.utils.logDebug('photoDetail => modifyVideoName => startTime');
        let checkItem = self.currentItem;
        let args = {
            selections: '',
            selectionArgs: ['videoalbum'],
        };
        media.getVideoAssets(args, (error, videos) => {
            for (let i = 0; i < videos.length; i++) {
                let item = videos[i];
                if (item.id === checkItem.id) {
                    self.dealStartModify(item, albumName);
                }
            }
        });
    },

    /**
    * 判断是否重复相册名
    *
    * @param {string} name - 当前点击相册名
    * @return {boolean} Verify result
    */
    isRepeatName(name) {
        this.utils.logDebug('photoDetail => isRepeatName');
        let list = this.cacheList || [];
        let flag = false;
        for (let i = 0; i < list.length; i++) {
            let item = list[i];
            if (item.name === name) {
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
    * 删除图片
    */
    deleteQuery() {
        let self = this;
        self.utils.logDebug('photoDetail => deleteQuery => startTime');
        let currentItem = self.currentItem;
        let list = self.cacheList || [];
        for (let i = 0; i < list.length; i++) {
            let item = list[i];

            // 由于查询所有返回的图片id 跟查询单个相册返回的图片id 对应不上，所以不能只以id判断
            if (item.id === currentItem.id && item.name === currentItem.name) {
                item.commitDelete((error, commitFlag) => {
                    self.utils.logDebug('photoDetail => deleteQuery => endTime');
                    if (commitFlag) {
                        self.refreshType = DELETE_REFRESH_TYPE;
                        self.$app.$def.dataManage.setPhotoList([]);
                        setTimeout(() => {
                            self.loadData();
                        }, LOAD_DATA_TIME);
                        self.$app.$def.dataManage.isRefreshed(true);
                    }
                });
            }
        }
    },

    /**
    * 删除图片弹窗
    */
    deletePhotos() {
        this.utils.logDebug('photoDetail => deletePhotos');
        let child = this.$child('delete_dialog');
        if (this.currentItem.mediaType === VIDEO_TYPE) {
            child.setTitle(this.$t('strings.isDeleteChooseVideo'));
        } else if (this.currentItem.mediaType === PHOTO_TYPE) {
            child.setTitle(this.$t('strings.isDeleteChooseImage'));
        }
        child.show();
    },

    /**
    * 移动
    */
    movePhotos() {
        let self = this;
        self.utils.logDebug('photoDetail => movePhotos');
        if (self.album.name === self.$t('strings.allPhotos')) {
            let arrData = self.currentItem.URI.split('/');
            self.album.name = arrData[arrData.length - 2];
        }
        router.replace(
            {
                uri: 'pages/selectAlbum/selectAlbum',
                params: {
                    isOperationFrom: true,
                    operationType: 'move',
                    fromAlbum: self.album,
                    checkedList: [self.currentItem]
                }
            }
        );
    },

    /**
    * 复制
    */
    copyPhotos() {
        let self = this;
        self.utils.logDebug('photoDetail => copyPhotos');

        //  判断是否是所有照片， 获取当前图片所在相册
        if (self.album.name === self.$t('strings.allPhotos')) {
            let arrData = self.currentItem.URI.split('/');
            self.album.name = arrData[arrData.length - 2];
        }
        router.replace(
            {
                uri: 'pages/selectAlbum/selectAlbum',
                params: {
                    isOperationFrom: true,
                    operationType: 'copy',
                    fromAlbum: self.album,
                    checkedList: [self.currentItem]
                }
            }
        );
    },

    /**
    * 隐藏底部菜单弹窗Pop
    */
    hideBottomPop() {
        this.utils.logDebug('photoDetail => hideBottomPop');
        this.popVisible = false;
    },

    /**
    * 改变底部菜单弹窗显示隐藏
    *
    * @param {Object} e - 当前点击底部pop弹窗项event
    */
    changePopVisible(e) {
        this.utils.logDebug('photoDetail => changePopVisible');
        this.popVisible = e.detail;
    },

    /**
    * 放大按钮
    */
    leftScale() {
        this.utils.logDebug('photoDetail => leftScale');
        this.currentItem.scale += SCALE_ADD;
        this.isScaleMinusDisable = false;
        if (this.currentItem.scale > SCALE_MAX) {
            this.currentItem.scale = SCALE_MAX;
            this.isScaleAddDisable = true;
            this.isScaleMinusDisable = false;
        }
    },

    /**
    * 缩小按钮
    */
    rightScale() {
        this.utils.logDebug('photoDetail => rightScale');
        this.currentItem.scale -= SCALE_MINUS;
        this.isScaleAddDisable = false;
        if (this.currentItem.scale < SCALE_ADD) {
            this.currentItem.scale = SCALE_ADD;
            this.isScaleMinusDisable = true;
            this.isScaleAddDisable = false;
        }
    },

    /**
    * 视频加载完成后回调
    */
    videoPrepared() {
        this.utils.logDebug('videoPrepared');
    },

    /**
    * 视频开始后回调
    */
    videoStart() {
        this.utils.logDebug('videoStart');
    },

    /**
    * 视频暂停后回调
    * @param {Object} item - 列表点击当前项
    */
    videoPause(item) {
        item.isPause = true;
        this.utils.logDebug('videoPause');
    },

    /**
    * 视频播放完成后回调
    */
    videoFinish() {
        this.utils.logDebug('videoFinish');
    },

    /**
    * 视频加载出错后回调
    */
    videoError() {
        this.utils.logDebug('videoError');
    },

    /**
    * 视频手动停止后回调
    */
    videoStop() {
        this.utils.logDebug('videoStop');
    },

    /**
    * 播放按钮
    * @param {Object} item - 列表点击当前项
    * @param {number} index - 列表点击当前项下标
    */
    videoPlay(item, index) {
        if (item.isPause) {
            this.$element('video' + index).start();
        } else {
            this.$element('video' + index).pause();
        }
        item.isPause = !item.isPause;
    }
};
