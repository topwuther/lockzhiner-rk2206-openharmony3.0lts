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

import DefaultLayoutConfig from '../../../../../../../../common/src/main/ets/default/common/configs/DefaultLayoutConfig.ets'
import GridLayoutConfigs from '../../../../../../../../common/src/main/ets/default/common/configs/GridLayoutConfigs.ets'
import SettingsModel from '../../../../../../../../common/src/main/ets/default/common/model/SettingsModel.ets'

let mSettingsModel: SettingsModel;

/**
 * Class SettingsPresenter.
 */
export default class SettingsPresenter {
  private static settingPresenter: SettingsPresenter = new SettingsPresenter();
  private callbackList = [];

  /**
   * Grid settingList.
   *
   */
  private gridSettingsList = [
    {
      ida: 0,
      settingName: $r('app.string.layoutStyle'),
      settingValue: '',
      valueList: DefaultLayoutConfig.DefaultLayoutOptions
    },
    {
      ida: 1,
      settingName: $r('app.string.launcherLayout'),
      settingValue: '',
      valueList: GridLayoutConfigs.GridLayoutTable
    },
    {
      ida: 2,
      settingName: $r('app.string.recentTasksSetting'),
      settingValue: '',
      valueList: DefaultLayoutConfig.DefaultRecentProcessLimitArray
    }
  ]

  /**
   * List settingList.
   *
   */
  private listSettingsList = [
    {
      ida: 0,
      settingName: $r('app.string.layoutStyle'),
      settingValue: '',
      valueList: DefaultLayoutConfig.DefaultLayoutOptions
    },
    {
      ida: 2,
      settingName: $r('app.string.recentTasksSetting'),
      settingValue: '',
      valueList: DefaultLayoutConfig.DefaultRecentProcessLimitArray
    }
  ]

  /**
   * Constructor.
   *
   * @param {object} settingsModel - model of setting.
   */
  constructor() {
    mSettingsModel = new SettingsModel();
    this.gridSettingsList[1].valueList = mSettingsModel.getGridLayoutTable();
  }

  /**
   * Get settingsPresenter instance.
   *
   * @return {settingPresenter} - settingPresenter.
   */
  public static getInstance(): SettingsPresenter{
    return this.settingPresenter;
  }

  /**
   * Get setting list.
   *
   * @return [settingList] - setting list.
   */
  public getSettingList() {
    let layout = mSettingsModel.getAppPageStartConfig();
    for (let i = 0; i < this.gridSettingsList[0].valueList.length; i++) {
      if (this.gridSettingsList[0].valueList[i].name == layout) {
        this.gridSettingsList[0].valueList[i].checked = true;
        this.listSettingsList[0].valueList[i].checked = true;
        break;
      }
    }
    let gridLayout = mSettingsModel.getGridConfig().layout;
    for (let i = 0; i < this.gridSettingsList[1].valueList.length; i++) {
      if (this.gridSettingsList[1].valueList[i].name == gridLayout) {
        this.gridSettingsList[1].valueList[i].checked = true;
        break;
      }
    }
    let Limit = mSettingsModel.getRecentProcessLimit().toString();
    for (let i = 0; i < this.gridSettingsList[2].valueList.length; i++) {
      if (this.gridSettingsList[2].valueList[i].name == Limit) {
        this.gridSettingsList[2].valueList[i].checked = true;
        this.listSettingsList[1].valueList[i].checked = true;
        break;
      }
    }
    if (layout == 'Grid') {
      this.gridSettingsList[0].settingValue = layout;
      this.gridSettingsList[1].settingValue = gridLayout;
      this.gridSettingsList[2].settingValue = Limit;
      return this.gridSettingsList;
    } else {
      this.listSettingsList[0].settingValue = layout;
      this.listSettingsList[1].settingValue = Limit;
      return this.listSettingsList;
    }
  }

  /**
   * Set system setting value.
   *
   * @param {string} settingsName - setting name.
   * @param {string} settingValue - setting value.
   */
  setSettingsValue(ida, settingValue) {
    if (ida == 0) {
      this.setAppPageStartConfig(settingValue);
    } else if (ida == 1) {
      this.setGridConfig(settingValue);
    } else {
      this.setRecentProcessLimit(settingValue);
    }
    this.settingUpdate();
  }

  /**
   * Set app start config.
   *
   * @param {string} type - the type of config.
   */
  setAppPageStartConfig(type) {
    mSettingsModel.setAppPageStartConfig(type);
  }

  /**
   * Update setting.
   *
   */
  settingUpdate() {
    mSettingsModel.closeSettings();
  }

  /**
   * Set grid config.
   *
   * @param {string} id - the id of grid config.
   */
  setGridConfig(id) {
    mSettingsModel.setGridConfig(id);
  }

  /**
   * Set recent process.
   *
   * @param {number} num - the num of recent process.
   */
  setRecentProcessLimit(num) {
    mSettingsModel.setRecentProcessLimit(num);
  }

  /**
   * Back to the desktop interface.
   *
   */
  backToTheDesktop() {
    this.settingUpdate();
  }

  /**
   * Register value callback.
   *
   * @param {string} settingsName - setting name.
   * @param {function()} settingValue - setting value.
   */
  registerValueCallback(ida, settingValue) {
    this.callbackList.push({
      id: ida,
      fun: settingValue
    });
  }

  /**
   * Change page setting value.
   *
   * @param {string} settingsName - setting name.
   * @param {string} settingValue - setting value.
   */
  changeSettingValue(ida, settingValue) {
    for (let i = 0;i < this.callbackList.length; i++) {
      if (this.callbackList[i].id == ida) {
        this.callbackList[i].fun(settingValue);
        break;
      }
    }
  }
}