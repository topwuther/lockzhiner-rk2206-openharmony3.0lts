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
 * @file: 图片列表界面
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

// 点击图片缩小效果
const SCALE_MIN = 0.9;

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
        bottomBarSource: [
            {
                id: MOVE_ID,
                src: '',
                disabled: false,
                visible: true,
                name: ''
            },
            {
                id: DELETE_ID,
                src: '',
                disabled: false,
                visible: true,
                name: ''
            },
            {
                id: SELECT_ALL_ID,
                src: '',
                disabled: false,
                visible: true,
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
        commonStyle: {
            width: '176px',
            height: '180px'
        },
        isShowBottomBar: false,
        list: [],
        cacheSource: [],
        cacheList: [],

        // 是否开启选择模式
        selectMode: false,

        // 是否全选
        isAllChecked: false,

        // 弹窗详细信息
        detailData: {},
        deleteDialogTitle: '',
        popVisible: false,
        utils: null,
        videoType: VIDEO_TYPE,
        photoType: PHOTO_TYPE,
        showEmptyDiv: false
    },

    /**
    * 初始化数据
    */
    onInit() {
        this.utils = this.$app.$def.utils;
        this.utils.logDebug('photoList => onInit');
        this.initNational();
    },

    /**
    * 组件销魂重置数据
    */
    onDestroy() {
        this.utils.logDebug('photoList => onDestroy');
        this.$app.$def.dataManage.setPhotoList([]);
    },

    /**
    * 初始化菜单资源
    */
    initNational() {
        this.utils.logDebug('photoList => initNational');
        this.topBarSource.leftSrc = this.utils.getIcon('back');
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
    onReady() {
        this.utils.logDebug('photoList => onReady');
    },

    /**
    * 组件显示加载数据
    */
    onShow() {
        this.utils.logDebug('photoList => onShow');
        this.loadData();
        if (this.selectMode) {
            this.onCheckedChange();
        }
    },

    /**
    * 组件隐藏保存数据
    */
    onHide() {
        this.utils.logDebug('photoList => onHide');
        this.$app.$def.dataManage.setPhotoList(this.list);
    },

    /**
    * 回退按钮
    *
    * @return {boolean} Verify result
    */
    onBackPress() {
        this.utils.logDebug('photoList => onBackPress');
        if (this.selectMode) {
            this.selectMode = false;
            this.onCheckedChange();
            return true;
        } else {
            return false;
        }
    },

    /**
    * 加载数据
    */
    loadData() {
        let self = this;
        self.utils.logDebug('photoList => loadData');
        if (self.album) {
            self.topBarSource.title = self.album.name;
            if (self.album.name === self.$t('strings.allPhotos')) {
                self.getAllPhotos();
            } else {
                self.getAlbumPhotos();
            }
        }
    },

    /**
    * 获取相册图片
    */
    getAlbumPhotos() {
        let self = this;
        self.utils.logDebug('photoList => getAlbumPhotos => startTime');

        //  用来保存从详情的选中数据
        let detailList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList()) || [];
        let args = {
            selections: self.album.name,
            selectionArgs: ['imagealbum', 'videoalbum'],
        };
        if (detailList.length === 0) {
            media.getMediaAssets(args, (error, images) => {
                self.utils.logDebug('photoList => getAlbumPhotos => endTime');
                self.cacheList = images;
                let list = [];
                if (images) {
                    self.dealAlbumPhotos(list, images);
                    self.cacheSource = list;
                    if (list.length >= PAGE_SIZE) {
                        self.list = list.slice(0, PAGE_SIZE);
                    } else {
                        self.list = list;
                    }
                    self.showEmptyDiv = false;
                    self.onCheckedChange();
                } else {
                    self.showEmptyDiv = true;
                    self.selectMode = false;
                    self.onCheckedChange();
                }
            });
        } else if (detailList.length > 0) {
            self.list = detailList;
            self.showEmptyDiv = false;
            self.onCheckedChange();
        }
    },

    /**
    * 处理图片相册数据
    *
    * @param {Array} list - 存储list
    * @param {Array} images - 被处理list
    */
    dealAlbumPhotos(list, images) {
        let self = this;
        for (let i = 0; i < images.length; i++) {
            let item = images[i];
            let obj = {
                mediaType: item.mediaType,
                size: item.size,
                dateAdded: item.dateAdded,
                dateModified: item.dateModified,
                name: item.name,
                albumName: item.albumName,
                albumId: item.albumId,
                id: item.id,
                isPause: true,
                src: 'file://' + item.URI,
                URI: item.URI,
                icon: '',
                checked: false,
                rotate: 0,
                scale: 1,
                itemStyle: Object.assign({}, this.commonStyle),
                imageStyle: Object.assign({}, this.commonStyle)
            };
            if (self.selectMode) {
                obj.icon = self.utils.getIcon('unselected');
            }
            list.push(obj);
        }
    },

    /**
    * 获取所有图片
    */
    getAllPhotos() {
        let self = this;
        // 用来保存从详情的选中数据
        let detailList = self.utils.deepCopy(self.$app.$def.dataManage.getPhotoList()) || [];

        if (detailList.length === 0) {
            self.utils.logDebug('photoList => getAllPhotos => startTime');
            media.getMediaAssets((error, images) => {
                self.utils.logDebug('photoList => getAllPhotos => endTime');
                self.cacheList = images;
                if (images) {
                    let list = [];
                    self.dealAlbumPhotos(list, images);
                    self.cacheSource = list;
                    if (list.length >= PAGE_SIZE) {
                        self.list = list.slice(0, PAGE_SIZE);
                    } else {
                        self.list = list;
                    }
                    self.showEmptyDiv = false;
                    self.onCheckedChange();
                } else {
                    self.showEmptyDiv = true;
                    self.selectMode = false;
                    self.onCheckedChange();
                }
            });
        } else if (detailList.length > 0) {
            self.showEmptyDiv = false;
            self.list = detailList;
            self.onCheckedChange();
        }
    },

    /**
    * 列表滑动到底部
    *
    * @return {boolean} Verify result
    */
    scrollBottom() {
        let self = this;
        self.utils.logDebug('photoList => scrollBottom');
        let listLen = self.list.length;
        let cacheLen = self.cacheSource.length;
        let len = cacheLen - listLen;
        if (len === 0) {
            return false;
        }
        let concatList = [];
        if (len >= PAGE_SIZE) {
            concatList = self.cacheSource.slice(listLen, listLen + PAGE_SIZE);
        } else if (len > 0 && len < PAGE_SIZE) {
            concatList = self.cacheSource.slice(listLen, listLen + len);
        } else {
            self.utils.logDebug('photoList => scrollBottom => lenError');
        }
        self.list = self.list.concat(self.initConcatList(concatList));
        self.onCheckedChange();
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
    * 顶部左侧按钮
    */
    topBarLeftClick() {
        let self = this;
        self.utils.logDebug('photoList => topBarLeftClick');
        if (self.selectMode) {
            self.selectMode = false;
        } else {
            router.back();
        }
        self.onCheckedChange();
    },

    /**
    * 顶部右侧按钮
    */
    topBarRightClick() {
        let self = this;
        self.utils.logDebug('photoList => topBarRightClick');
        let checkedList = self.getCheckedData();

        //  添加方式弹出操作窗
        if (checkedList.length > 0) {
            self.$element('add_type_dialog').show();
        }
    },

    /**
    * 底部菜单
    *
    * @param {Object} item - 底部菜单当前点击项
    */
    bottomTabClick(item) {
        let self = this;
        self.utils.logDebug('photoList => bottomTabClick');
        if (item.detail.id === MOVE_ID) {
            self.movePhotos();
        } else if (item.detail.id === DELETE_ID) {
            self.deletePhotos();
        } else if (item.detail.id === SELECT_ALL_ID) {
            self.setListChecked();
        }
    },

    /**
    * 弹出提示框点击事件
    *
    * @param {Object} item - 弹窗层点击项
    */
    popupItemClick(item) {
        let self = this;
        self.utils.logDebug('photoList => popupItemClick');
        if (item.detail.id === DELETE_ID) {
            self.copyPhotos();
        } else if (item.detail.id === MOVE_ID) {
            let list = self.getCheckedData();
            let size = 0;
            for (let i = 0; i < list.length; i++) {
                size += Number(list[i].size);
            }
            size = size / DATA_SIZE / DATA_SIZE;
            self.detailData = {
                number: list.length,
                size: size.toFixed(DETAIL_SIZE)
            };
            self.$element('detail_dialog').show();
        }
    },

    /**
    * 选中发生改变
    *
    * @return {boolean} Verify result
    */
    onCheckedChange() {
        let self = this;
        self.utils.logDebug('photoList => onCheckedChange');
        if (!self.selectMode) {
            self.topBarSource.isShowLeft = true;
            self.topBarSource.title = self.album.name;
            self.topBarSource.leftSrc = self.utils.getIcon('back');
            self.isShowBottomBar = false;
            self.isAllChecked = false;
            self.initChecked();
            return false;
        }
        if (self.getCheckedData().length === 0) {
            self.topBarSource.title = self.$t('strings.unChoose');
            self.topBarSource.leftSrc = self.utils.getIcon('close');
            self.bottomBarSource[2].name = self.$t('strings.selectAll');
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
            self.topBarSource.title = self.$t('strings.selected')
            + self.getCheckedData().length + self.$t('strings.items');
            self.topBarSource.leftSrc = self.utils.getIcon('close');
            if (self.getCheckedData().length === self.list.length) {
                self.isAllChecked = true;
                self.bottomBarSource[2].name = self.$t('strings.unSelectAll');
                self.bottomBarSource[2].src = '/common/image/svg/unselect_all.svg';
            } else {
                self.isAllChecked = false;
                self.bottomBarSource[2].name = self.$t('strings.selectAll');
                self.bottomBarSource[2].src = '/common/image/svg/select_all.svg';
            }
        }
    },

    /**
    * 放大按钮点击事件
    *
    * @param {Object} item - 当前列表长按项
    * @param {number} index - 当前列表长按项下标
    */
    scaleImgClick(item, index) {
        this.resetItemStyle(item);
        this.goDetail(item, index);
    },

    /**
    * 列表点击事件回调
    *
    * @param {Object} item - 当前列表长按项
    * @param {number} index - 当前列表长按项下标
    */
    photoClick(item, index) {
        let self = this;
        self.utils.logDebug('photoList => photoClick');
        self.hideBottomPop();
        if (self.selectMode) {
            item.checked = !item.checked;
            if (item.checked) {
                item.icon = self.utils.getIcon('selected');
            } else {
                item.icon = self.utils.getIcon('unselected');
            }
            self.onCheckedChange();
        }
        self.resetItemStyle(item);
        if (!self.selectMode) {
            self.goDetail(item, index);
        }
    },

    /**
    * 重置当前项宽高
    *
    * @param {Object} item - 当前列表项
    */
    resetItemStyle(item) {
        item.itemStyle = Object.assign({}, this.commonStyle);
        item.imageStyle = Object.assign({}, this.commonStyle);
    },

    /**
    * 跳转详情页面
    *
    * @param {Object} item - 当前列表长按项
    * @param {number} index - 当前列表长按项下标
    */
    goDetail(item, index) {
        let self = this;
        self.utils.logDebug('photoList => goDetail');
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
    * 长按事件
    *
    * @param {Object} item - 当前列表长按项
    * @param {number} index - 当前列表长按项下标
    * @return {boolean} Verify result
    */
    parentLongPress(item, index) {
        let self = this;
        self.utils.logDebug('photoList => parentLongPress');
        if (self.selectMode) {
            return false;
        }
        let gridList = self.list;

        // 宫格数据
        for (let index = 0; index < gridList.length; index++) {
            let item = gridList[index];
            item.icon = self.utils.getIcon('unselected');
        }

        self.selectMode = true;
        self.topBarSource.leftSrc = self.utils.getIcon('close');

        self.photoClick(item, index);
    },

    /**
    * 根据是否开启选择模式，初始化选中效果
    */
    initChecked() {
        let self = this;
        self.utils.logDebug('photoList => initChecked');
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
    * 获取选中数据
    *
    * @return {Array} list - 当前选中列表
    */
    getCheckedData() {
        let self = this;
        self.utils.logDebug('photoList => getCheckedData');
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
        self.utils.logDebug('photoList => setListChecked');
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
        this.utils.logDebug('photoList => addTypeDialogClick');
        if (obj.detail === this.$t('strings.copy')) {
            this.copyPhotos();
        } else if (obj.detail === this.$t('strings.move')) {
            this.movePhotos();
        }
    },

    /**
    * 删除
    */
    deleteDialogCommit() {
        let self = this;
        self.utils.logDebug('photoList => deleteDialogCommit => startTime');
        let choose = this.getCheckedData();
        let list = self.cacheList;
        for (let i = 0; i < choose.length; i++) {
            let checkItem = choose[i];
            for (let j = 0; j < list.length; j++) {
                let item = list[j];
                self.dealDeleteItem(choose, item, checkItem);
            }
        }
    },

    /**
    * 处理删除项逻辑判断
    *
    * @param {Array} choose - 选中数据
    * @param {Object} item - 列表项
    * @param {Object} checkItem - 选中列表项
    */
    dealDeleteItem(choose, item, checkItem) {
        let self = this;
        // 由于查询所有返回的图片id 跟查询单个相册返回的图片id 对应不上，所以不能只以id判断
        if (item.name === checkItem.name && item.id === checkItem.id) {
            item.commitDelete((error, commitFlag) => {
                self.utils.logDebug('photoList => deleteDialogCommit => endTime');
                if (commitFlag) {
                    self.$app.$def.dataManage.isRefreshed(true);
                    self.$app.$def.dataManage.setPhotoList([]);
                    self.selectMode = false;
                    self.dealDeleteDelayed(choose);
                }
            });
        }
    },

    /**
    * 处理删除项逻辑判断
    *
    * @param {Array} choose - 选中数据
    */
    dealDeleteDelayed(choose) {
        let self = this;
        setTimeout(() => {
            if (choose.length === self.list.length) {
                self.list = [];
                self.showEmptyDiv = true;
                self.onCheckedChange();
            } else {
                self.loadData();
            }
        }, LOAD_DATA_TIME);
    },


    /**
    * 删除弹窗确定
    */
    deleteQuery() {
        this.deleteDialogCommit();
    },

    /**
    * 删除图片弹窗显示
    *
    * @return {boolean} Verify result
    */
    deletePhotos() {
        this.utils.logDebug('photoList => deletePhotos');
        let length = this.getCheckedData().length;
        if (length === 0) {
            return false;
        }
        let child = this.$child('delete_dialog');
        child.setTitle(this.$t('strings.deleteTitleInfo') + length + this.$t('strings.files'));
        child.show();
    },

    /**
    * 移动
    */
    movePhotos() {
        let self = this;
        self.utils.logDebug('photoList => movePhotos');
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
        let self = this;
        self.utils.logDebug('photoList => copyPhotos');
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
    * 触摸事件开始
    *
    * @param {Object} item - 当前触摸列表项
    */
    touchStart(item) {
        this.utils.logDebug('photoList => touchStart');
        let width = item.itemStyle.width.replace('px', '');
        let height = item.itemStyle.height.replace('px', '');
        item.imageStyle.width = SCALE_MIN * width;
        item.imageStyle.height = SCALE_MIN * height;
    },

    /**
    * 触摸事件取消
    *
    * @param {Object} item - 当前触摸列表项
    */
    touchCancel(item) {
        this.utils.logDebug('photoList => touchCancel');
        item.imageStyle.width = item.itemStyle.width;
        item.imageStyle.height = item.itemStyle.height;
    },

    /**
    * 触摸事件结束
    *
    * @param {Object} item - 当前触摸列表项
    */
    touchEnd(item) {
        this.utils.logDebug('photoList => touchEnd');
        item.imageStyle.width = item.itemStyle.width;
        item.imageStyle.height = item.itemStyle.height;
    },

    /**
    * 隐藏底部菜单pop
    */
    hideBottomPop() {
        this.utils.logDebug('photoList => hideBottomPop');
        this.popVisible = false;
    },

    /**
    * 改变底部菜单pop显示隐藏
    *
    * @param {Object} e - 当前点击弹出层event
    */
    changePopVisible(e) {
        this.utils.logDebug('photoList => changePopVisible');
        this.popVisible = e.detail;
    }
};
