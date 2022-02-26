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
 * @file: 缓存数据
 */

// 列表数据
let listData = [];

// 宫格数据
let gridData = [];

// 用来缓存相册列表与详情数据
let photoList = [];

// 用来缓存首页相册数据
let mainAlbumList = [];

// 是否刷新
let isRefreshed = true;

export default {

    /**
    * 获取listData
    *
    * @return {Array} - 返回listData
    */
    getListData() {
        return listData;
    },

    /**
    * 获取gridData
    *
    * @return {Array} - 返回gridData
    */
    getGridData() {
        return gridData;
    },

    /**
    * 赋值photoList
    *
    * @param {Array} list - 图片列表数据
    */
    setPhotoList(list) {
        photoList = list;
    },

    /**
    * 获取photoList
    *
    * @return {Array} - 返回photoList
    */
    getPhotoList() {
        return photoList;
    },

    /**
    * 赋值mainAlbumList
    *
    * @param {Array} list - 主页面相册数据
    */
    setMainAlbumList(list) {
        mainAlbumList = list;
    },

    /**
    * 获取mainAlbumList
    *
    * @return {Array} - 返回mainAlbumList
    */
    getMainAlbumList() {
        return mainAlbumList;
    },

    /**
    * 赋值isRefreshed
    *
    * @param {boolean} flag - 是否刷新
    */
    isRefreshed(flag) {
        isRefreshed = flag;
    },

    /**
    * 获取isRefreshed
    *
    * @return {boolean} - 返回isRefreshed
    */
    getRefreshed() {
        return isRefreshed;
    }
};