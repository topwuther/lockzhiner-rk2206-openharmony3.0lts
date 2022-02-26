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
 * @file: 新建相册选择相册里的图片
 */

import router from '@system.router';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let media = mediaLibrary.getMediaLibraryHelper();

// 照片类型
const PHOTO_TYPE = 3;

// 视频类型
const VIDEO_TYPE = 4;

// 懒加载列表数
const PAGE_SIZE = 28;

export default {
    data: {
        album: null,
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: true,
            isShowRight: false
        },
        utils: null,
        gridImageStyle: {
            width: '176px',
            height: '180px'
        },
        list: [],

        // 是否开启选择模式
        selectMode: true,

        // 是否全选
        isAllChecked: false,

        // 弹窗详细信息
        detailData: {},

        // 操作方式
        operationType: '',

        // 新建相册对象
        createParams: null,
        videoType: VIDEO_TYPE,
        photoType: PHOTO_TYPE,
        cacheOtherList: []
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.topBarSource.leftSrc = this.utils.getIcon('close');
        this.topBarSource.rightSrc = this.utils.getIcon('select');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.loadData();
    },

    /**
    * 组件隐藏保存数据
    */
    onHide() {
        this.utils.logDebug('selectAlbumPhoto => onHide');
        this.$app.$def.dataManage.setPhotoList(this.list);
    },

    /**
    * 组件销魂重置数据
    */
    onDestroy() {
        this.utils.logDebug('selectAlbumPhoto => onDestroy');
        this.$app.$def.dataManage.setPhotoList([]);
    },

    /**
    * 获取数据
    */
    loadData() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => loadData => startTime');
        if (self.album) {
            let args = {
                selections: self.album.name,
                selectionArgs: ['imagealbum', 'videoalbum'],
            };
            let shareList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList()) || [];
            if (shareList.length > 0) {
                self.list = shareList;
                self.onCheckedChange();
            }
            media.getMediaAssets(args, (error, images) => {
                self.utils.logDebug('selectAlbumPhoto => loadData => endTime');
                if (images && shareList.length === 0) {
                    for (let i = 0; i < images.length; i++) {
                        let item = images[i];
                        item.src = 'file://' + item.URI;
                        item.icon = self.utils.getIcon('unselected');
                        item.rotate = 0;
                        item.scale = 1;
                        item.checked = false;
                    }
                    self.cacheOtherList = images;
                    if (images.length > PAGE_SIZE) {
                        self.list = images.slice(0, PAGE_SIZE);
                    } else {
                        self.list = images;
                    }
                    self.onCheckedChange();
                }
            });
        }
    },

    /**
    * 顶部左侧按钮
    */
    topBarLeftClick() {
        this.utils.logDebug('selectAlbumPhoto => topBarLeftClick');
        router.back();
    },

    /**
    * 顶部右侧按钮
    */
    topBarRightClick() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => topBarRightClick');
        let checkedList = self.getCheckedData();

        // 添加方式弹出操作窗
        if (checkedList.length > 0) {
            self.$element('add_type_dialog').show();
        }
    },

    /**
    * 新建相册
    *
    * @param {Array} checkList - 新建弹窗选中数据
    */
    createAlbum(checkList) {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => createAlbum');
        router.push(
            {
                uri: 'pages/afterSelect/afterSelect',
                params: {
                    fromAlbum: self.album,
                    list: checkList,
                    operationType: 'create',
                    topBarSource: {
                        title: self.createParams.name,
                        leftSrc: '/common/image/svg/back.svg',
                        rightSrc: '/common/image/icon/selected.png',
                        isShowLeft: true,
                        isShowRight: false
                    },
                },
            }
        );
    },

    /**
    * 选中发生改变
    */
    onCheckedChange() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => onCheckedChange');
        let length = self.getCheckedData().length;
        if (length === 0) {
            self.topBarSource.title = self.$t('strings.unChoose');
            self.topBarSource.isShowRight = false;
            self.topBarSource.leftSrc = self.utils.getIcon('close');
            self.isAllChecked = false;
        } else {
            self.topBarSource.title = self.$t('strings.selected') + length + self.$t('strings.items');
            self.topBarSource.isShowRight = true;
            self.topBarSource.leftSrc = self.utils.getIcon('close');
            if (self.getCheckedData().length === self.list.length) {
                self.isAllChecked = true;
            } else {
                self.isAllChecked = false;
            }
        }
    },

    /**
    * 放大按钮点击事件
    *
    * @param {Object} item - 当前点击项
    * @param {number} index - 当前点击项下标
    */
    scaleImgClick(item, index) {
        this.goDetail(item, index);
    },

    /**
    * 列表点击事件
    *
    * @param {Object} item - 当前点击项
    */
    photoClick(item) {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => photoClick');
        item.checked = !item.checked;
        if (item.checked) {
            item.icon = self.utils.getIcon('selected');
        } else {
            item.icon = self.utils.getIcon('unselected');
        }
        self.onCheckedChange();
    },

    /**
    * 跳转详情页面
    *
    * @param {Object} item - 当前点击项
    * @param {number} index - 当前点击项下标
    */
    goDetail(item, index) {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => goDetail');
        router.push(
            {
                uri: 'pages/photoDetail/photoDetail',
                params: {
                    list: self.list,
                    album: self.album,
                    selectMode: self.selectMode,
                    shareIndex: index
                },
            }
        );
    },

    /**
    * 根据是否开启选择模式，初始化选中效果
    */
    initChecked() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => initChecked');
        let gridList = self.list;

        // 宫格数据
        for (let index = 0; index < gridList.length; index++) {
            let item = gridList[index];
            item.icon = self.utils.getIcon('unselected');
            item.checked = false;
        }
    },

    /**
    * 获取选中数据
    *
    * @return {Array} list - 选择数据
    */
    getCheckedData() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => getCheckedData');
        let list = [];
        self.list.forEach(item => {
            if (item.checked) {
                list.push(item);
            }
        });
        return list;
    },

    /**
    * 设置全选/全不选
    */
    setListChecked() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => setListChecked');
        let list = self.list;

        // 宫格数据
        for (let index = 0; index < list.length; index++) {
            let item = list[index];
            if (self.isAllChecked) {
                item.checked = false;
                item.icon = self.utils.getIcon('unselected');
            } else {
                item.checked = true;
                item.icon = self.utils.getIcon('selected');
            }

        }
        self.onCheckedChange();
    },

    /**
    * 设置全选/全不选
    *
    * @param {Object} obj - 选择类型弹窗点击项
    */
    addTypeDialogClick(obj) {
        this.utils.logDebug('selectAlbumPhoto => addTypeDialogClick');
        if (obj.detail === this.$t('strings.copy')) {
            this.copyPhotos();
        } else if (obj.detail === this.$t('strings.move')) {
            this.movePhotos();
        }
    },

    /**
    * 移动
    *
    * @return {boolean} Verify result
    */
    movePhotos() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => movePhotos');
        let choose = self.getCheckedData();
        if (choose.length === 0) {
            return false;
        }
        router.replace(
            {
                uri: 'pages/afterSelect/afterSelect',
                params: {
                    isOperationFrom: true,
                    operationType: 'move',
                    fromAlbum: self.album,
                    list: choose,
                    topBarSource: {
                        title: self.createParams.name,
                        leftSrc: '/common/image/svg/back.svg',
                        rightSrc: '/common/image/icon/selected.png',
                        isShowLeft: true,
                        isShowRight: false
                    },
                }
            }
        );
    },

    /**
    * 复制
    *
    * @return {boolean} Verify result
    */
    copyPhotos() {
        let self = this;
        self.utils.logDebug('selectAlbumPhoto => copyPhotos');
        let choose = self.getCheckedData();
        if (choose.length === 0) {
            return false;
        }
        router.replace(
            {
                uri: 'pages/afterSelect/afterSelect',
                params: {
                    isOperationFrom: true,
                    operationType: 'copy',
                    fromAlbum: self.album,
                    list: choose,
                    topBarSource: {
                        title: self.createParams.name,
                        leftSrc: '/common/image/svg/back.svg',
                        rightSrc: '/common/image/icon/selected.png',
                        isShowLeft: true,
                        isShowRight: false
                    },
                }
            }
        );
    },

    /**
    * 滚动到底部触发事件
    */
    scrollBottom() {
        this.utils.logDebug('selectAlbumPhoto => scrollBottom');
        let cacheLength = this.cacheOtherList.length;
        let listLength = this.list.length;
        let diffLength = cacheLength - listLength;
        if (diffLength >= PAGE_SIZE) {
            this.list = this.list.concat(this.cacheOtherList.slice(listLength, listLength + PAGE_SIZE));
        } else if (0 < diffLength < PAGE_SIZE) {
            this.list = this.list.concat(this.cacheOtherList.slice(listLength, listLength + diffLength));
        } else {
            this.utils.logDebug('selectAlbumPhoto => scrollBottom => lenError');
        }
    }
};
