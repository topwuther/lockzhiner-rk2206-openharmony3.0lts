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

import Prompt from '@ohos.prompt';
import BaseAppPresenter from '../../../../../../../../common/src/main/ets/default/base/BaseAppPresenter.ets';

const PROHIBITED = '禁止卸载'
const UNINSTALL_SUCCEEDED = '卸载成功'
const UNINSTALL_DAILED = '卸载失败'
const KEY_APP_LIST = "appListInfo";
const BOTTOM_BAR_FLAG = -1;
const UNINSTALL_SUCCESS = "UNINSTALL_SUCCESS";
const UNINSTALL_FAILED = "UNINSTALL_FAILED";
const UNINSTALL_PROHIBITED = "UNINSTALL_PROHIBITED";
const PROPORTION = 0.85;
const KEY_ICON = "icon";
const KEY_NAME = "name";
const BOTTOM_BAR = 1;
const BOTTOM_BAR_ROW = -1;


export default class AppGridPresenter extends BaseAppPresenter {
  private static appGridPresenter: AppGridPresenter = new AppGridPresenter();
  private mBundleInfoList;
  private mGridConfig;
  private mScreenBottomBarTop = 0;
  private pageIndex = 0;
  private mGridAppsInfos;
  private mPageCoordinateData = {
    gridXAxis: [],
    gridYAxis: [],
    bottomXAxis: [],
    bottomYAxis: [],
    bottomSpacing: 0
  };

  private constructor() {
    super()
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    AppStorage.SetOrCreate('pageIndex', this.pageIndex);
  }

  public static getInstance(): AppGridPresenter{
    return this.appGridPresenter;
  }

  public getGridList() {
    this.mAppModel.getAppList(this.getGridListCallback.bind(this));
  }

  public getGridListCallback(list) {
    this.mBundleInfoList = list;
    this.mAppListInfoCacheManager.setCache(KEY_APP_LIST, list);
    this.pagingFiltering();
  }

  getGridConfig() {
    return this.mSettingsModel.getGridConfig();
  }

  private pagingFiltering() {
    let appInfo = {
      appGridInfo: [],
      appBottomBarInfo: []
    };
    let appBottomBarInfo = [];
    let appListInfo = [];
    let info = this.getLayoutInfo();
    let layoutInfo = info.layoutInfo;
    let bottomInfo = info.bottomBarInfo;

    for (let i = 0;i < layoutInfo.length; i++) {
      for (let j = 0; j < this.mBundleInfoList.length; j++) {
        if (layoutInfo[i].bundleName == this.mBundleInfoList[j].bundleName) {
          appListInfo.push(
            {
              System: this.mBundleInfoList[j].System,
              AppName: this.mBundleInfoList[j].AppName,
              AppId: this.mBundleInfoList[j].AppId,
              AppIcon: this.mBundleInfoList[j].AppIcon,
              bundleName: this.mBundleInfoList[j].bundleName,
              labelId: this.mBundleInfoList[j].labelId,
              abilityName: this.mBundleInfoList[j].abilityName,
              type: 0,
              area: layoutInfo[i].area,
              page: layoutInfo[i].page,
              row: layoutInfo[i].row,
              column: layoutInfo[i].column,
              x: 0,
            }
          );
        }
      }
    }
    for (let i = 0;i < bottomInfo.length; i++) {
      for (let j = 0; j < this.mBundleInfoList.length; j++) {
        if (bottomInfo[i].bundleName == this.mBundleInfoList[j].bundleName) {
          appBottomBarInfo.push(
            {
              System: this.mBundleInfoList[j].System,
              AppName: this.mBundleInfoList[j].AppName,
              AppId: this.mBundleInfoList[j].AppId,
              AppIcon: this.mBundleInfoList[j].AppIcon,
              bundleName: this.mBundleInfoList[j].bundleName,
              labelId: this.mBundleInfoList[j].labelId,
              abilityName: this.mBundleInfoList[j].abilityName,
              type: 0,
              area: bottomInfo[i].area,
              page: bottomInfo[i].page,
              row: bottomInfo[i].row,
              column: bottomInfo[i].column,
              x: 0,
            }
          );
        }
      }
    }
    appInfo.appGridInfo = this.integrateSwiper(appListInfo);
    appInfo.appBottomBarInfo = this.integrateBottomBar(appBottomBarInfo);
    AppStorage.SetOrCreate('appListInfo', appInfo);
  }

  private integrateSwiper(list) {
    let gridAppsInfos = [];
    let allPageCount = this.mSettingsModel.getLayoutInfo().layoutDescription.pageCount;
    let max = allPageCount;
    for (let i = 0;i < list.length; i++) {
      if (max < list[i].page) {
        max = list[i].page;
      }
    }

    for (let i = 0;i < max; i++) {
      gridAppsInfos.push([]);
    }

    for (let i = 0;i < list.length; i++) {
      gridAppsInfos[list[i].page].push(list[i]);
    }
    this.mGridAppsInfos = gridAppsInfos;
    return gridAppsInfos;
  }

  private integrateBottomBar(list) {
    if (list.length == 5) {
      AppStorage.SetOrCreate('BottomBarItemWidth', '19%');
    } else if (list.length == 3) {
      AppStorage.SetOrCreate('BottomBarItemWidth', '28%');
    } else {
      AppStorage.SetOrCreate('BottomBarItemWidth', '25%');
    }
    for (let i = 0;i < list.length; i++) {
      list[i].x = this.mPageCoordinateData.bottomXAxis[list.length - 1][i];
    }
    return list;
  }

  private getLayoutInfo() {
    let info = {
      layoutInfo: [],
      bottomBarInfo: []
    };
    info = this.mSettingsModel.getLayoutInfo();
    let bottomBarInfo = info.bottomBarInfo;
    let isBottomBarLegal = this.ifBottomBarRationality(bottomBarInfo);
    let isLegal = this.ifLayoutRationality(info);
    if (isLegal && isBottomBarLegal) {
      info = this.updateLayoutInfo(info);
    } else if (!isBottomBarLegal) {
      let defaultInfo = this.mSettingsModel.getDefaultLayoutInfo();
      let defaultBottomBarInfo = defaultInfo.bottomBarInfo;
      let isDefaultLayoutLegal = this.ifLayoutRationality(defaultInfo);
      let isDefaultBottomBarLegal = this.ifBottomBarRationality(defaultBottomBarInfo);
      if (isDefaultLayoutLegal && isDefaultBottomBarLegal) {
        info = this.updateLayoutInfo(this.mSettingsModel.getDefaultLayoutInfo());
      } else {
        info = this.updateLayoutInfo(this.createNewInfo());
      }
    } else if (isBottomBarLegal && !isLegal) {
      info = this.updateLayoutInfo(this.createNewLayoutInfo());
    }
    this.mSettingsModel.setLayoutInfo(info);
    return info;
  }

  private ifBottomBarRationality(bottomBarInfo) {
    if (bottomBarInfo == undefined || this.ifBottomBarIsBigger(bottomBarInfo)) {
      return false;
    }
    return true;
  }

  private ifBottomBarIsBigger(bottomBarInfo) {
    if (bottomBarInfo.length > 5) {
      return true;
    }
    return false;
  }

  ifLayoutRationality = (info) => {
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    //verify whether the info is null.
    if (this.ifInfoIsNull(info)) {
      return false;
    }
    let layoutDescription = info.layoutDescription;
    //verify whether the layoutDescription is different.
    if (this.ifDescriptionIsDiffrent(layoutDescription, row, column)) {
      return false;
    }
    let layoutInfo = info.layoutInfo;
    //verify whether the layoutInfo's row and column is more than standard.
    if (this.ifColumnOrRowAreBigger(layoutInfo, row, column)) {
      return false;
    }
    //verify whether the layoutInfo's position is duplicated.
    if (this.ifDuplicatePosition(layoutInfo)) {
      return false;
    }
    //verify whether the layoutInfo's bundleName is duplicated.
    if (this.ifDuplicateBundleName(layoutInfo)) {
      return false;
    }
    return true;
  }

  private ifInfoIsNull(info) {
    if (info == undefined || info == '' || info == {} || info == null) {
      return true;
    }
    return false;
  }

  private ifDescriptionIsDiffrent(layoutDescription, row, column) {
    if (row != layoutDescription.row || column != layoutDescription.column) {
      return true;
    }
    return false;
  }

  private ifColumnOrRowAreBigger(layoutInfo, row, column) {
    for (let i = 0; i < layoutInfo.length; i++) {
      //column or row are bigger than legal num
      if (layoutInfo[i].column >= column || layoutInfo[i].row >= row) {
        return true;
      }
    }
    return false;
  }

  ifDuplicatePosition(layoutInfo) {
    for (let i = 0; i < layoutInfo.length; i++) {
      for (let j = layoutInfo.length - 1; j > 0 && j > i; j--) {
        if (layoutInfo[i].page == layoutInfo[j].page && layoutInfo[i].row == layoutInfo[j].row && layoutInfo[i].column == layoutInfo[j].column) {
          return true;
        }

      }
    }
    return false;
  }

  ifDuplicateBundleName(layoutInfo) {
    let count = {};
    for (let i = 0; i < layoutInfo.length; i++) {
      if (count[layoutInfo[i].bundleName] == undefined || count[layoutInfo[i].bundleName] == null || count[layoutInfo[i].bundleName] == '') {
        count[layoutInfo[i].bundleName] = 0;
      } else if (++ count[layoutInfo[i].bundleName] > 1) {
        return true;
      }
    }
    return false;
  }

  private updateLayoutInfo(info) {
    let layoutDescription = info.layoutDescription;
    let layoutInfo = info.layoutInfo;
    let bottomBarInfo = info.bottomBarInfo;
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    let newApp = [];
    layoutDescription.row = row;
    layoutDescription.column = column;
    //Detect newly installed apps
    for (let i in this.mBundleInfoList) {
      let sign = false;
      for (let j in layoutInfo) {
        if (this.mBundleInfoList[i].bundleName == layoutInfo[j].bundleName) {
          sign = true;
          break;
        }
      }
      if (!sign) {
        newApp.push(this.mBundleInfoList[i]);
      }
    }
    for (let i = newApp.length - 1; i >= 0; i--) {
      let sign = false;
      for (let j = 0; j < bottomBarInfo.length; j++) {
        if (newApp[i].bundleName == bottomBarInfo[j].bundleName) {
          sign = true;
          break;
        }
      }
      if (sign) {
        newApp.splice(i, 1);
      }
    }
    //Detect uninstalled apps
    for (let i in layoutInfo) {
      let sign = false;
      for (let j in this.mBundleInfoList) {
        if (layoutInfo[i].bundleName == this.mBundleInfoList[j].bundleName) {
          sign = true;
          break;
        }
      }
      if (!sign) {
        layoutInfo.splice(i, 1);
      }
    }
    for (let i in bottomBarInfo) {
      let sign = false;
      for (let j in this.mBundleInfoList) {
        if (bottomBarInfo[i].bundleName == this.mBundleInfoList[j].bundleName) {
          sign = true;
          break;
        }
      }
      if (!sign) {
        bottomBarInfo.splice(i, 1);
      }
    }

    //The latest info position in the page
    let existNumber = this.getExistNumber(layoutInfo);
    //Add new app
    for (let i = 0; i < newApp.length; i++) {
      layoutInfo.push({
        bundleName: newApp[i].bundleName,
        type: 0,
        area: [1, 1],
        page: Math.floor((i + existNumber) / (column * row)),
        row: Math.floor((i + existNumber) % (column * row) / column),
        column: Math.floor((i + existNumber) % (column * row) % column),
      });
    }
    let pageCount = Math.ceil((newApp.length + existNumber) / (column * row));
    if (pageCount < this.mSettingsModel.getLayoutInfo().layoutDescription.pageCount) {
      layoutDescription.pageCount = this.mSettingsModel.getLayoutInfo().layoutDescription.pageCount;
    } else {
      layoutDescription.pageCount = pageCount;
    }
    info.layoutDescription = layoutDescription;
    info.layoutInfo = layoutInfo;
    info.bottomBarInfo = bottomBarInfo;
    return info;
  }

  private getExistNumber(layoutInfo) {
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    let existNumber = 0;
    for (let i = 0; i < layoutInfo.length; i++) {
      let Page = layoutInfo[i].page;
      let Row = layoutInfo[i].row;
      let Column = layoutInfo[i].column;
      let result = (Page * column * row) + (Row * column) + (Column + 1);
      if (result > existNumber) {
        existNumber = result;
      }
    }
    return existNumber;
  }

  private createNewInfo() {
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    let layoutNum = this.mBundleInfoList.length;
    let maxPerPage = column * row;
    let pageNum = Math.ceil(layoutNum / maxPerPage);
    let newLayoutInfo = {
      layoutDescription: {},
      layoutInfo: [],
      bottomBarInfo: []
    };
    newLayoutInfo.layoutDescription = {
      "pageCount": pageNum,
      "row": row,
      "column": column,
    }
    newLayoutInfo.layoutInfo = [];
    newLayoutInfo.bottomBarInfo = [];
    return newLayoutInfo;
  }

  private createNewLayoutInfo() {
    let info = this.mSettingsModel.getLayoutInfo();
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    let layoutNum = info.layoutInfo.length;
    let maxPerPage = column * row;
    let pageNum = Math.ceil(layoutNum / maxPerPage);
    let newLayoutInfo = {
      layoutDescription: {},
      layoutInfo: [],
      bottomBarInfo: []
    };
    newLayoutInfo.layoutDescription = {
      "pageCount": pageNum,
      "row": row,
      "column": column
    }
    newLayoutInfo.layoutInfo = [];
    if (info.bottomBarInfo == undefined) {
      newLayoutInfo.bottomBarInfo = [];
    } else {
      newLayoutInfo.bottomBarInfo = info.bottomBarInfo;
    }
    return newLayoutInfo;
  }

  public regroupDataAfterInstall(callbackList) {
    this.mBundleInfoList = callbackList;
    this.mAppListInfoCacheManager.setCache(KEY_APP_LIST, callbackList);
    this.pagingFiltering();
  }

  public regroupDataAfterUnInstall(result) {
    console.info("Launcher AppGridView getUninstallApp callback = " + result);
    if (result === UNINSTALL_PROHIBITED) {
      Prompt.showToast({
        message: PROHIBITED
      });
    } else if (result === UNINSTALL_SUCCESS) {
      Prompt.showToast({
        message: UNINSTALL_SUCCEEDED
      });
    } else if (result === UNINSTALL_FAILED) {
      Prompt.showToast({
        message: UNINSTALL_DAILED
      });
    }
  }

  public openApplication(abilityName, bundleName) {
    this.jumpTo(abilityName, bundleName);
  }

  public intoSetting() {
    console.info("Launcher AppGridView intoSetting");
    this.jumpToSetting();
  }


  /**
   * Longpress event for launcher.
   */
  public onPageLongPress() {
    AppStorage.SetOrCreate('blankPageBtnText', this.getBlankPageBtnStr());
  }

  /**
   * Get strings for addBlankPageButton.
   *
   * @return {string} AddBlankPageButton Strings.
   */
  public getBlankPageBtnStr() {
    return this.isBlankPage() ? $r('app.string.deleteBlankPage') : $r('app.string.addBlankPage');
  }

  public isBlankPage() {
    if (this.mGridAppsInfos[this.pageIndex].length == undefined) {
      return true;
    }
    if (this.mGridAppsInfos[this.pageIndex].length === 0) {
      return true;
    }
    return false;
  }

  public changeIndex(idx) {
    this.pageIndex = idx;
    AppStorage.SetOrCreate('pageIndex', this.pageIndex);
  }

  public getIndex() {
    return this.pageIndex;
  }

  /**
   * Add or delete the choosen blank page.
   */
  public addOrDeleteBlankPage() {
    if (this.isBlankPage()) {
      this.deleteBlankPage();
    } else {
      this.addBlankPage();
    }
  }

  /**
   * Add a blank page.
   */
  public addBlankPage() {
    console.info("Launcher AppGridView addBlankPage");
    let allPageCount = this.getGridPageCount() + 1;
    this.setGridPageCount(allPageCount);
    this.pagingFiltering();
    this.pageIndex = allPageCount - 1;
    AppStorage.SetOrCreate('pageIndex', this.pageIndex);
  }

  /**
   * Get pageCount.
   *
   * @return {number} PageCount.
   */
  public getGridPageCount() {
    return this.mSettingsModel.getLayoutInfo().layoutDescription.pageCount;
  }

  /**
   * Set pageCount.
   *
   * @param {number} pageCount - PageCount.
   */
  private setGridPageCount(pageCount) {
    let gridLayoutInfo = this.mSettingsModel.getLayoutInfo();
    gridLayoutInfo.layoutDescription.pageCount = pageCount;
    this.mSettingsModel.setLayoutInfo(gridLayoutInfo);
  }

  /**
   * Delete the choosen blank page.
   */
  private deleteBlankPage() {
    console.info("Launcher AppGridView deleteBlankPage");
    this.deleteGridPage(this.pageIndex);
    this.pageIndex = this.pageIndex - 1;
    AppStorage.SetOrCreate('pageIndex', this.pageIndex);
    this.setGridPageCount(this.mSettingsModel.getLayoutInfo().layoutDescription.pageCount - 1);
    this.pagingFiltering();
  }

  /**
   * Delete blank page.
   *
   * @param {number} pageIndex - Index of the page which is to be deleted.
   */
  private deleteGridPage(pageIndex) {
    let info = this.mSettingsModel.getLayoutInfo();
    let layoutInfo = info.layoutInfo;
    for (let element of layoutInfo) {
      if (element.page > pageIndex) {
        element.page = element.page - 1;
      }
    }
    info.layoutInfo = layoutInfo;
    this.mSettingsModel.setLayoutInfo(info);
  }

  public layoutAdjustment(startInfo, endInfo, bottomBar) {
    if (startInfo.row == BOTTOM_BAR_FLAG && endInfo.row != BOTTOM_BAR_FLAG) {
      this.bottomToTopAdjustment(startInfo, endInfo, bottomBar);
    } else if (startInfo.row == BOTTOM_BAR_FLAG && endInfo.row == BOTTOM_BAR_FLAG) {
      this.bottomToBottomAdjustment(startInfo, endInfo, bottomBar);
    } else if (startInfo.row != BOTTOM_BAR_FLAG && endInfo.row == BOTTOM_BAR_FLAG) {
      this.topToBottomAdjustment(startInfo, endInfo, bottomBar);
    } else {
      this.topToTopAdjustment(startInfo, endInfo);
    }
  }

  bottomToTopAdjustment(startInfo, endInfo, bottomBar) {
    let info = this.mSettingsModel.getLayoutInfo();
    let layoutInfo = info.layoutInfo;
    let bottomBarInfo = info.bottomBarInfo;
    let moveItem = {
      bundleName: "",
      type: 0,
      page: -1,
      row: -1,
      column: -1
    }
    for (let i = bottomBar.length - 1; i >= 0; i--) {
      if (startInfo.X >= bottomBar[i].x - this.mPageCoordinateData.bottomSpacing && startInfo.X < bottomBar[i].x + this.mPageCoordinateData.bottomSpacing) {
        moveItem.bundleName = bottomBarInfo[i].bundleName;
        moveItem.type = bottomBarInfo[i].type;
        bottomBarInfo.splice(i, 1);
        layoutInfo.push(moveItem);
        break;
      }
    }
    this.moveLayout(moveItem, endInfo, layoutInfo, moveItem);
    for (let i = layoutInfo.length - 1; i >= 0; i--) {
      if (layoutInfo[i].page == -1 && layoutInfo[i].column == -1 && layoutInfo[i].row == -1) {
        layoutInfo.splice(i, 1);
        break;
      }
    }
    info.layoutInfo = layoutInfo;
    info.bottomBarInfo = bottomBarInfo;
    this.mSettingsModel.setLayoutInfo(info);
    this.pagingFiltering();
  }

  bottomToBottomAdjustment(startInfo, endInfo, bottomBar) {
    let info = this.mSettingsModel.getLayoutInfo();
    let bottomBarInfo = info.bottomBarInfo;
    let moveItem = {
      bundleName: "",
      type: 0,
      page: 0,
      row: 0,
      column: 0,
    }
    for (let i = bottomBar.length - 1; i >= 0; i--) {
      if (startInfo.X >= bottomBar[i].x - this.mPageCoordinateData.bottomSpacing && startInfo.X < bottomBar[i].x + this.mPageCoordinateData.bottomSpacing) {
        moveItem.bundleName = bottomBarInfo[i].bundleName;
        moveItem.type = bottomBarInfo[i].type;
        moveItem.page = bottomBarInfo[i].page;
        moveItem.row = bottomBarInfo[i].row;
        moveItem.column = bottomBarInfo[i].column;
        bottomBarInfo.splice(i, 1);
      }
    }
    for (let i = 0; i < bottomBar.length; i++) {
      if (i == 0 && endInfo.X < bottomBar[i].x) {
        bottomBarInfo.splice(i, 0, moveItem);
        break;
      } else if (i != bottomBar.length - 1 && endInfo.X >= bottomBar[i].x && endInfo.X < bottomBar[i + 1].x) {
        bottomBarInfo.splice(i, 0, moveItem);
        break;
      } else if (i == bottomBar.length - 1 && endInfo.X >= bottomBar[i].x) {
        bottomBarInfo.push(moveItem);
        break;
      }
    }
    info.bottomBarInfo = bottomBarInfo;
    this.mSettingsModel.setLayoutInfo(info);
    this.pagingFiltering();
  }

  topToBottomAdjustment(startInfo, endInfo, bottomBar) {
    let info = this.mSettingsModel.getLayoutInfo();
    let layoutInfo = info.layoutInfo;
    let bottomBarInfo = info.bottomBarInfo;
    let moveItem = {
      bundleName: "",
      type: 0,
      page: 0,
      row: 0,
      column: 0
    };
    for (let i = layoutInfo.length - 1; i >= 0; i--) {
      if (layoutInfo[i].page == startInfo.page && layoutInfo[i].row == startInfo.row && layoutInfo[i].column == startInfo.column) {
        moveItem.bundleName = layoutInfo[i].bundleName;
        moveItem.type = layoutInfo[i].type;
        moveItem.page = layoutInfo[i].page;
        moveItem.row = layoutInfo[i].row;
        moveItem.column = layoutInfo[i].column;
        layoutInfo.splice(i, 1);
      }
    }
    if (bottomBar.length == 0) {
      bottomBarInfo.push(moveItem);
    } else if (bottomBar.length > 0 && bottomBar.length < 5) {
      for (let i = 0; i < bottomBar.length; i++) {
        if (i != bottomBar.length - 1 && bottomBar[i].x < endInfo.X && bottomBar[i + 1].x > endInfo.X) {
          bottomBarInfo.splice(i + 1, 0, moveItem);
          break;
        } else if (i == 0 && endInfo.X < bottomBar[i].x) {
          bottomBarInfo.splice(i, 0, moveItem);
          break;
        } else if (i == bottomBar.length - 1 && endInfo.X > bottomBar[i].x) {
          bottomBarInfo.push(moveItem);
          break;
        }
      }
    } else if (bottomBar.length >= 5) {
      this.swapAppIcon(moveItem, endInfo, bottomBar, bottomBarInfo);
      layoutInfo.push(moveItem);
    }
    info.layoutInfo = layoutInfo;
    info.bottomBarInfo = bottomBarInfo;
    this.mSettingsModel.setLayoutInfo(info);
    this.pagingFiltering();
  }

  topToTopAdjustment(startInfo, endInfo) {
    let info = this.mSettingsModel.getLayoutInfo();
    let layoutInfo = info.layoutInfo;
    this.moveLayout(startInfo, endInfo, layoutInfo, startInfo);
    info.layoutInfo = layoutInfo;
    this.mSettingsModel.setLayoutInfo(info);
    this.pagingFiltering();
  }

  private swapAppIcon = (moveItem, endInfo, bottomBar, bottomBarInfo) => {
    let tempInfo = {
      bundleName: moveItem.bundleName,
      type: moveItem.type,
      page: moveItem.page,
      row: moveItem.row,
      column: moveItem.column
    }
    for (let i = 0; i < bottomBar.length; i++) {
      if (endInfo.X < bottomBar[i].x && i == 0) {
        moveItem.bundleName = bottomBarInfo[i].bundleName;
        moveItem.type = bottomBarInfo[i].type;
        bottomBarInfo[i].bundleName = tempInfo.bundleName;
        bottomBarInfo[i].type = tempInfo.type;
        bottomBarInfo[i].page = tempInfo.page;
        bottomBarInfo[i].row = tempInfo.row;
        bottomBarInfo[i].column = tempInfo.column;
        break;
      } else if (i != bottomBar.length - 1 && endInfo.X >= bottomBar[i].x && endInfo.X < bottomBar[i + 1].x) {
        moveItem.bundleName = bottomBarInfo[i].bundleName;
        moveItem.type = bottomBarInfo[i].type;
        bottomBarInfo[i].bundleName = tempInfo.bundleName;
        bottomBarInfo[i].type = tempInfo.type;
        bottomBarInfo[i].page = tempInfo.page;
        bottomBarInfo[i].row = tempInfo.row;
        bottomBarInfo[i].column = tempInfo.column;
        break;
      } else if (i == bottomBar.length - 1 && endInfo.X >= bottomBar[i].x) {
        moveItem.bundleName = bottomBarInfo[i].bundleName;
        moveItem.type = bottomBarInfo[i].type;
        bottomBarInfo[i].bundleName = tempInfo.bundleName;
        bottomBarInfo[i].type = tempInfo.type;
        bottomBarInfo[i].page = tempInfo.page;
        bottomBarInfo[i].row = tempInfo.row;
        bottomBarInfo[i].column = tempInfo.column;
        break;
      }
    }
  }
  private moveLayout = (source, destination, layoutInfo, startInfo) => {
    let couldMoveForward = this.moveLayoutForward(source, destination, layoutInfo, startInfo);
    if (couldMoveForward) return;
    this.moveLayoutBackward(source, destination, layoutInfo, startInfo);
  }

  /**
   * Icons go forwards.
   *
   * @param {object} source - Start position information.
   * @param {object} destination - End position information.
   * @param {object} layoutInfo - Current layout information.
   * @param {object} startInfo - Recursion start position information.
   * @return {boolean} Move result.
   */
  private moveLayoutForward = (source, destination, layoutInfo, startInfo) => {
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let startLayoutInfo = layoutInfo.find(item => {
      return item.page == source.page && item.row == source.row && item.column == source.column;
    });
    let endLayoutInfo = layoutInfo.find(item => {
      return item.page == destination.page && item.row == destination.row && item.column == destination.column;
    });

    if (endLayoutInfo != undefined && !(endLayoutInfo.page == startInfo.page && endLayoutInfo.row == startInfo.row && endLayoutInfo.column == startInfo.column)) {
      if (endLayoutInfo.row == this.mGridConfig.row - 1 && endLayoutInfo.column == this.mGridConfig.column - 1) {
        return false;
      }

      let nextPosition = {
        page: destination.page,
        row: destination.column == this.mGridConfig.column - 1 ? destination.row + 1 : destination.row,
        column: destination.column == this.mGridConfig.column - 1 ? 0 : destination.column + 1
      }
      let couldMoveForward = this.moveLayoutForward(destination, nextPosition, layoutInfo, startInfo);
      if (!couldMoveForward) return false;
    }
    startLayoutInfo.page = destination.page;
    startLayoutInfo.row = destination.row;
    startLayoutInfo.column = destination.column;
    return true;
  }

  /**
   * Icons go backwards.
   *
   * @param {object} source - Start position information.
   * @param {object} destination - End position information.
   * @param {object} layoutInfo - Current layout information.
   * @param {object} startInfo - Recursion start position information.
   * @return {boolean} Move result.
   */
  private moveLayoutBackward = (source, destination, layoutInfo, startInfo) => {
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let startLayoutInfo = layoutInfo.find(item => {
      return item.page == source.page && item.row == source.row && item.column == source.column;
    });
    let endLayoutInfo = layoutInfo.find(item => {
      return item.page == destination.page && item.row == destination.row && item.column == destination.column;
    });

    if (endLayoutInfo != undefined && !(endLayoutInfo.page == startInfo.page && endLayoutInfo.row == startInfo.row && endLayoutInfo.column == startInfo.column)) {

      if (endLayoutInfo.row == 0 && endLayoutInfo.column == 0) {
        return false;
      }

      let nextPosition = {
        page: destination.page,
        row: (destination.column == 0 && destination.row > 0) ? destination.row - 1 : destination.row,
        column: destination.column == 0 ? this.mGridConfig.column - 1 : destination.column - 1
      }

      let couldMoveBackward = this.moveLayoutBackward(destination, nextPosition, layoutInfo, startInfo);
      if (!couldMoveBackward) return false;
    }
    startLayoutInfo.page = destination.page;
    startLayoutInfo.row = destination.row;
    startLayoutInfo.column = destination.column;
    return true;
  }

  public calculateCoordinate(): void{
    this.mGridConfig = this.mSettingsModel.getGridConfig();
    let column = this.mGridConfig.column;
    let row = this.mGridConfig.row;
    this.mPageCoordinateData.gridXAxis = [];
    this.mPageCoordinateData.gridYAxis = [];
    this.mPageCoordinateData.bottomXAxis = [];
    this.mPageCoordinateData.bottomYAxis = [];
    for (let i = 1; i <= row; i++) {
      let touchPositioningY = (this.mScreenHeight * PROPORTION / row) * i;
      this.mPageCoordinateData.gridYAxis.push(touchPositioningY);
    }

    for (let i = 1; i <= column; i++) {
      let touchPositioningX = (this.mScreenWidth / column) * i;
      this.mPageCoordinateData.gridXAxis.push(touchPositioningX);
    }
    for (let i = 0;i < 5; i++) {
      this.mPageCoordinateData.bottomXAxis.push([]);
    }
    for (let i = 0;i < 5; i++) {
      let spacing = this.mScreenWidth / (i + 1);
      if (i == 0) {
        this.mPageCoordinateData.bottomXAxis[i].push(spacing * 0.5);
      } else if (i == 1) {
        spacing = this.mScreenWidth / 4;
        this.mPageCoordinateData.bottomXAxis[i].push(spacing * 1.5);
        this.mPageCoordinateData.bottomXAxis[i].push(spacing * 2.5);
      } else if (i == 2) {
        spacing = this.mScreenWidth * 0.08;
        let cellWidth = this.mScreenWidth * 0.28;
        for (let j = 0;j <= i; j++) {
          this.mPageCoordinateData.bottomXAxis[i].push(spacing + (cellWidth * (j + 0.5)));
        }
      } else if (i == 3) {
        for (let j = 0;j <= i; j++) {
          this.mPageCoordinateData.bottomXAxis[i].push(spacing * (j + 0.5));
        }
      } else {
        spacing = this.mScreenWidth * 0.025;
        let cellWidth = this.mScreenWidth * 0.19;
        for (let j = 0;j <= i; j++) {
          this.mPageCoordinateData.bottomXAxis[i].push(spacing + (cellWidth * (j + 0.5)));
        }
      }
    }

    this.mPageCoordinateData.bottomSpacing = this.mScreenWidth / 10;

    this.mScreenBottomBarTop = this.mScreenHeight * PROPORTION;

    for (let i = 0; i < column; i++) {
      this.mPageCoordinateData.bottomYAxis.push(this.mScreenBottomBarTop);
    }
  }

  public getTouchPosition(x, y): any {
    let position = {
      page: this.pageIndex,
      row: 0,
      column: 0,
      X: x,
      Y: y,
    };
    for (let i: number = 0;i < this.mPageCoordinateData.gridXAxis.length; i++) {
      if (x < this.mPageCoordinateData.gridXAxis[i]) {
        position.column = i;
        break;
      } else {
        position.column = this.mPageCoordinateData.gridXAxis.length - 1;
      }
    }
    for (let i: number = 0;i < this.mPageCoordinateData.gridYAxis.length; i++) {
      if (y < this.mPageCoordinateData.gridYAxis[i]) {
        position.row = i;
        break;
      } else {
        position.row = this.mPageCoordinateData.gridYAxis.length - 1;
      }
    }

    if (y > this.mScreenBottomBarTop) {
      position.row = BOTTOM_BAR_ROW;
    }
    return position
  }

  public setEquipment(EType) {
    this.mSettingsModel.setEquipment(EType);
    this.mGridConfig = this.mSettingsModel.getGridConfig();
  }
}