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
 * @file: 视频列表界面
 */

import router from '@system.router';
import mediaLibrary from '@ohos.multimedia.medialibrary';

let media = mediaLibrary.getMediaLibraryHelper();

// 加载数据延时
const LOAD_DATA_TIME = 200;

// 移动id
const MOVE_ID = 1;

// 删除id
const DELETE_ID = 2;

// 全选id
const SELECT_ALL_ID = 3;

// 更多id
const MORE_ID = -10;

// 详细信息id
const ABOUT_ID = 1;

// 复制id
const COPY_ID = 2;

// 详细信息尺寸取小数点后4位
const DETAIL_SIZE = 4;

// 图片尺寸转换单位
const DATA_SIZE = 1024;

// 懒加载列表数
const PAGE_SIZE = 6;

export default {
    data: {
        listData: [],
        topBarSource: {
            title: '',
            leftSrc: '',
            rightSrc: '',
            isShowLeft: true,
            isShowRight: false
        },
        isShowBottomBar: false,
        bottomBarSource: [
            {
                id: MOVE_ID,
                src: '',
                disabled: true,
                name: ''
            },
            {
                id: DELETE_ID,
                src: '',
                disabled: true,
                name: ''
            },
            {
                id: SELECT_ALL_ID,
                src: '',
                disabled: true,
                name: ''
            },
            {
                id: MORE_ID,
                src: '',
                disabled: false,
                visible: true,
                name: ''
            },
        ],
        bottomBarPopList: [
            {
                id: ABOUT_ID,
                src: '',
                disabled: false,
                visible: true,
                name: ''
            },
            {
                id: COPY_ID,
                src: '',
                disabled: false,
                visible: true,
                name: ''
            }
        ],
        popVisible: false,
        videoWidth: '360px',
        selectMode: false,
        detailData: [],
        isAllChecked: false,
        album: null,
        deleteDialogTitle: '',
        utils: null,
        cacheOtherList: []
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('videoList => onInit');
        this.initNational();
    },

    /**
    * 初始化菜单资源
    */
    initNational() {
        this.utils.logDebug('videoList => initNational');
        this.topBarSource.leftSrc = this.utils.getIcon('close');
        this.topBarSource.rightSrc = this.utils.getIcon('select');
        this.bottomBarSource[0].name = this.$t('strings.move');
        this.bottomBarSource[0].src = this.utils.getIcon('move');
        this.bottomBarSource[1].name = this.$t('strings.delete');
        this.bottomBarSource[1].src = this.utils.getIcon('delete');
        this.bottomBarSource[2].name = this.$t('strings.selectAll');
        this.bottomBarSource[2].src = this.utils.getIcon('select_all');
        this.bottomBarSource[3].name = this.$t('strings.more');
        this.bottomBarSource[3].src = this.utils.getIcon('more');
        this.bottomBarPopList[0].name = this.$t('strings.detailInfo');
        this.bottomBarPopList[1].name = this.$t('strings.copy');
        this.bottomBarPopList[1].src = this.utils.getIcon('copy');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.loadData();
    },

    /**
    * 组件销魂重置数据
    */
    onDestroy() {
        this.utils.logDebug('videoList => onDestroy');
        this.$app.$def.dataManage.setPhotoList([]);
    },

    /**
    * 组件隐藏保存数据
    */
    onHide() {
        this.utils.logDebug('videoList => onHide');
        this.$app.$def.dataManage.setPhotoList(this.listData);
    },

    /**
    * 回退按钮
    *
    * @return {boolean} Verify result
    */
    onBackPress() {
        this.utils.logDebug('videoList => onBackPress');
        if (this.selectMode) {
            this.topBarSource.leftSrc = this.utils.getIcon('back');
            this.topBarSource.title = this.$t('strings.video');
            this.isShowBottomBar = false;
            this.selectMode = false;
            this.initListChecked();
            return true;
        } else {
            return false;
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
        if (self.isAllChecked) {
            list.forEach(item => {
                item.icon = self.utils.getIcon('selected');
                item.checked = true;
            });
        }
        return list;
    },

    /**
    * 加载数据
    */
    loadData() {
        this.utils.logDebug('videoList => loadData => startTime');
        let self = this;
        let arg = {
            selections: '',
            selectionArgs: ['videoalbum'],
        };

        // 用来保存从详情的选中数据
        let shareList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList()) || [];
        if (shareList.length > 0) {
            self.listData = shareList;
            self.onCheckedChange();
        }
        media.getVideoAssets(arg, (error, videos) => {
            self.utils.logDebug('videoList => loadData => endTime');
            if (videos && shareList.length === 0) {
                videos.forEach((item, index) => {
                    item.src = 'file://' + item.URI;
                    item.isPause = true;
                    if (self.selectMode) {
                        item.icon = self.utils.getIcon('unselected');
                    } else {
                        item.icon = '';
                        item.checked = false;
                    }
                });
                self.cacheOtherList = videos;
                if (videos.length > PAGE_SIZE) {
                    self.listData = videos.slice(0, PAGE_SIZE);
                } else {
                    self.listData = videos;
                }

                self.onCheckedChange();
            }
        });
    },

    /**
    * 顶部左侧按钮
    *
    * @return {boolean} Verify result
    */
    topBarLeftClick() {
        this.utils.logDebug('videoList => topBarLeftClick');
        if (this.selectMode) {
            this.topBarSource.leftSrc = this.utils.getIcon('back');
            this.topBarSource.title = this.$t('strings.video');
            this.isShowBottomBar = false;
            this.selectMode = false;
            this.initListChecked();
            return false;
        }
        router.back();
    },

    /**
    * 放大按钮点击事件
    *
    * @param {Object} item - 当前点击视频项
    * @param {number} index - 当前点击视频项下标
    */
    scaleClick(item, index) {
        this.utils.logDebug('videoList => scaleClick');
        router.push(
            {
                uri: 'pages/photoDetail/photoDetail',
                params: {
                    list: this.listData,
                    album: this.album,
                    selectMode: this.selectMode,
                    shareIndex: index
                },
            }
        );
    },

    /**
    * 列表项点击事件
    *
    * @param {Object} item - 当前点击视频项
    * @param {number} index - 当前点击视频项下标
    * @return {boolean} Verify result
    */
    videoClick(item, index) {
        this.utils.logDebug('videoList => videoClick');
        this.hideBottomPop();
        if (this.selectMode) {
            item.checked = !item.checked;
            if (item.checked) {
                item.icon = this.utils.getIcon('selected');
            } else {
                item.icon = this.utils.getIcon('unselected');
            }
            this.onCheckedChange();
            return false;
        }
        router.push(
            {
                uri: 'pages/photoDetail/photoDetail',
                params: {
                    list: this.listData,
                    album: this.album,
                    selectMode: this.selectMode,
                    shareIndex: index
                },
            }
        );
    },

    /**
    * 长按事件
    *
    * @param {Object} item - 当前点击视频项
    * @param {number} index - 当前点击视频项下标
    */
    longPress(item, index) {
        this.utils.logDebug('videoList => longPress');
        this.selectMode = true;
        this.topBarSource.leftSrc = this.utils.getIcon('close');
        this.topBarSource.title = this.$t('strings.unChoose');
        this.isShowBottomBar = true;
        this.initListChecked();
        this.videoClick(item, index);
    },

    /**
    * 选中改变事件
    *
    * @return {boolean} Verify result
    */
    onCheckedChange() {
        this.utils.logDebug('videoList => onCheckedChange');
        let self = this;
        let checkList = self.getCheckedData();
        if (!self.selectMode) {
            self.topBarSource.isShowLeft = true;
            self.topBarSource.title = self.album.name;
            self.topBarSource.leftSrc = self.utils.getIcon('back');
            self.isShowBottomBar = false;
            self.isAllChecked = false;
            self.initListChecked();
            return false;
        }
        if (checkList.length === 0) {
            self.topBarSource.title = this.$t('strings.unChoose');
            self.bottomBarSource[2].name = this.$t('strings.selectAll');
            self.bottomBarSource[2].src = '/common/image/svg/select_all.svg';
            self.isAllChecked = false;
            for (let i = 0; i < self.bottomBarSource.length; i++) {
                const item = self.bottomBarSource[i];
                if (item.id !== SELECT_ALL_ID) {
                    item.disabled = true;
                }
            }
        } else {
            for (let i = 0; i < self.bottomBarSource.length; i++) {
                const item = self.bottomBarSource[i];
                item.disabled = false;
            }
            self.isShowBottomBar = true;
            self.topBarSource.title = self.$t('strings.selected') + checkList.length + self.$t('strings.items');
            if (self.getCheckedData().length === self.listData.length) {
                self.isAllChecked = true;
                self.bottomBarSource[2].name = this.$t('strings.unSelectAll');
                self.bottomBarSource[2].src = '/common/image/svg/unselect_all.svg';
            } else {
                self.isAllChecked = false;
                self.bottomBarSource[2].name = this.$t('strings.selectAll');
                self.bottomBarSource[2].src = '/common/image/svg/select_all.svg';
            }
        }
    },

    /**
    * 初始化列表选中状态
    */
    initListChecked() {
        this.utils.logDebug('videoList => initListChecked');
        this.listData.forEach(item => {
            item.checked = false;
            item.icon = this.utils.getIcon('unselected');
        });
    },

    /**
    * 获取选中数据
    *
    * @return {Array} list - 选中数据
    */
    getCheckedData() {
        this.utils.logDebug('videoList => getCheckedData');
        let self = this;
        let list = [];
        self.listData.forEach(item => {
            if (item.checked) {
                list.push(item);
            }
        });
        return list;
    },

    /**
    * 底部菜单点击事件
    *
    * @param {Object} item - 当前点击底部菜单项
    */
    bottomTabClick(item) {
        this.utils.logDebug('videoList => bottomTabClick');
        if (item.detail.id === MOVE_ID) {
            this.movePhotos();
        } else if (item.detail.id === DELETE_ID) {
            this.deletePhotos();
        } else if (item.detail.id === SELECT_ALL_ID) {
            this.setListChecked();
        }
    },

    /**
    * 设置全选
    */
    setListChecked() {
        this.utils.logDebug('videoList => setListChecked');
        let self = this;
        let list = self.listData;

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
    * 设置全选
    *
    * @param {Object} item - 当前点击Pop菜单项
    * @return {boolean} Verify result
    */
    popupItemClick(item) {
        this.utils.logDebug('videoList => popupItemClick');
        if (item.detail.id === COPY_ID) {
            this.copyPhotos();
        } else if (item.detail.id === ABOUT_ID) {
            let list = this.getCheckedData();
            if (list.length === 0) {
                return false;
            }
            let size = 0;
            for (let i = 0; i < list.length; i++) {
                size += Number(list[i].size);
            }
            size = size / DATA_SIZE / DATA_SIZE;
            this.detailData = {
                number: list.length,
                size: size.toFixed(DETAIL_SIZE)
            };
            this.$element('detail_dialog').show();
        }
    },

    /**
    * 删除弹窗确定回调
    */
    deleteQuery() {
        this.deleteDialogCommit();
    },

    /**
    * 删除显示图片弹窗
    *
    * @return {boolean} Verify result
    */
    deletePhotos() {
        this.utils.logDebug('videoList => deletePhotos');
        let length = this.getCheckedData().length;
        if (length === 0) {
            return false;
        }
        let child = this.$child('delete_dialog');
        child.setTitle(this.$t('strings.selected') + length + this.$t('strings.items'));
        child.show();
    },

    /**
    * 删除图片接口调用
    */
    deleteDialogCommit() {
        this.utils.logDebug('videoList => deleteDialogCommit => startTime');
        let self = this;
        let choose = self.getCheckedData();
        let arg = {
            selections: '',
            selectionArgs: ['videoalbum'],
        };
        media.getVideoAssets(arg, (error, videos) => {
            self.utils.logDebug('videoList => deleteDialogCommit => endTime');
            if (videos) {
                for (let j = 0; j < choose.length; j++) {
                    let shareItem = choose[j];
                    videos.forEach((item) => {
                        self.dealCommitDelete(shareItem, item);
                    });
                }
            }
        });
    },

    /**
    * 处理删除操作
    */
    dealCommitDelete(shareItem, item) {
        let self = this;
        let choose = self.getCheckedData();
        if (item.name === shareItem.name && item.id === shareItem.id) {
            item.commitDelete((error, commitFlag) => {
                if (commitFlag) {
                    self.$app.$def.dataManage.isRefreshed(true);
                    self.selectMode = false;
                    setTimeout(() => {
                        if (choose.length === self.listData.length) {
                            self.listData = [];
                            self.onCheckedChange();
                        } else {
                            self.loadData();
                        }
                    }, LOAD_DATA_TIME);
                }
            });
        }
    },

    /**
    * 移动
    */
    movePhotos() {
        this.utils.logDebug('videoList => movePhotos');
        let self = this;
        let choose = self.getCheckedData();
        router.push(
            {
                uri: 'pages/selectAlbum/selectAlbum',
                params: {
                    isOperationFrom: true,
                    operationType: 'move',
                    fromAlbum: self.album,
                    checkedList: choose
                }
            }
        );
    },

    /**
    * 复制
    */
    copyPhotos() {
        this.utils.logDebug('videoList => copyPhotos');
        let self = this;
        let choose = self.getCheckedData();
        router.push(
            {
                uri: 'pages/selectAlbum/selectAlbum',
                params: {
                    isOperationFrom: true,
                    operationType: 'copy',
                    fromAlbum: self.album,
                    checkedList: choose
                }
            }
        );
    },

    /**
    * 隐藏pop弹窗
    */
    hideBottomPop() {
        this.utils.logDebug('videoList => hideBottomPop');
        this.popVisible = false;
    },

    /**
    * 改变pop弹窗显示状态
    *
    * @param {Object} e - 改变pop弹出层回调参数
    */
    changePopVisible(e) {
        this.utils.logDebug('videoList => changePopVisible');
        this.popVisible = e.detail;
    },

    /**
    * 滚动到底部触发事件
    *
    * @return {boolean} Verify result
    */
    scrollBottom() {
        this.utils.logDebug('videoList => scrollBottom');
        let cacheLength = this.cacheOtherList.length;
        let listLength = this.listData.length;
        let diffLength = cacheLength - listLength;
        if (diffLength === 0) {
            return false;
        }
        let concatList = [];
        if (diffLength >= PAGE_SIZE) {
            concatList = this.cacheOtherList.slice(listLength, listLength + PAGE_SIZE);
        } else if (0 < diffLength < PAGE_SIZE) {
            concatList = this.cacheOtherList.slice(listLength, listLength + diffLength);
        } else {
            this.utils.logDebug('videoList => scrollBottom => lenError');
        }
        this.listData = this.listData.concat(this.initConcatList(concatList));
        this.onCheckedChange();
    }
};
