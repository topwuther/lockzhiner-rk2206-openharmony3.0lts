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

import FeatureAbility from '@ohos.ability.featureAbility';
import GridLayoutConfigs from '../configs/GridLayoutConfigs.ets'
import LayoutConfigManager from '../../manager/LayoutConfigManager.ets'
import FileUtils from '../../utils/FileUtils.ets'
import ILayoutConfig from './ILayoutConfig.ets'

const defaultLayoutInfoFilePath = "/data/accounts/account_0/applications/com.ohos.launcher/com.ohos.launcher/assets/launcher/resources/rawfile/layoutInfo.json";

export default class SettingsModel {
  private mLayoutConfig: ILayoutConfig;
  private mGridConfig = 1;
  private mGridLayoutTable = GridLayoutConfigs.GridLayoutTable;

  constructor() {
    this.mLayoutConfig = LayoutConfigManager.getLayoutConfig();
    let Etype = this.mLayoutConfig.getEquipmentType();
    if (Etype == 'phone') {
      this.mGridLayoutTable = GridLayoutConfigs.GridLayoutTable;
    } else {
      this.mGridLayoutTable = GridLayoutConfigs.GridLayoutTableHorizontal;
    }
  }

  /**
   * Get the grid view presetting collection of layout config information table.
   *
   * @return {object} Grid view presetting collection object.
   */
  public getGridLayoutTable(): any {
    return this.mGridLayoutTable;
  }

  /**
   * Get default layout information of grid view.
   *
   * @return {object} Default layout information of grid view.
   */
  public getDefaultLayoutInfo(): any {
    return FileUtils.readJsonFile(defaultLayoutInfoFilePath);
  }

  /**
   * Get layout config of grid view.
   *
   * @return {object} Layout config of grid view.
   */
  public getGridConfig(): any {
    this.mGridConfig = this.mLayoutConfig.getGridConfig();
    let gridLayout = this.mGridLayoutTable[0];
    for (let i = 0; i < this.mGridLayoutTable.length; i++) {
      if (this.mGridLayoutTable[i].id == this.mGridConfig) {
        gridLayout = this.mGridLayoutTable[i];
        break;
      }
    }
    return gridLayout;
  }

  /**
   * Set layout config id of grid view.
   *
   * @param {string} id - Layout config id of grid view.
   */
  public setGridConfig(id) {
    this.mLayoutConfig.setGridConfig(id);
  }

  /**
   * Get recent process max limit.
   *
   * @return {number} recent process max limit.
   */
  public getRecentProcessLimit(): any {
    return this.mLayoutConfig.getRecentProcessLimit();
  }

  /**
   * Set recent process max limit.
   *
   * @param {number} num - Recent process max limit.
   */
  public setRecentProcessLimit(num) {
    this.mLayoutConfig.setRecentProcessLimit(num);
  }

  /**
   * Get the layout view type.
   *
   * @return {string} Layout view type, should one of 'Grid' or 'List' which is stored in LayoutConstants class.
   */
  public getAppPageStartConfig(): any {
    return this.mLayoutConfig.getAppPageStartConfig();
  }

  /**
   * Set the layout view type.
   *
   * @param {string} type - Layout view type, should one of 'Grid' or 'List' which is stored in LayoutConstants class.
   */
  public setAppPageStartConfig(type) {
    this.mLayoutConfig.setAppPageStartConfig(type);
  }

  /**
   * Get layout information of grid view.
   *
   * @return {object} layout information.
   */
  public getLayoutInfo(): any {
    return this.mLayoutConfig.getGridLayoutInfo();
  }

  /**
   * Set layout information of grid view.
   */
  public setLayoutInfo(layoutInfo) {
    this.mLayoutConfig.setGridLayoutInfo(layoutInfo);
  }

  /**
   * Remove layout information of grid view.
   */
  public deleteLayoutInfo() {
    this.mLayoutConfig.deleteGridLayoutInfo();
  }

  /**
   * Close settings.
   */
  closeSettings() {
    console.info("Launcher settings SettingsModel  closeSettings start")
    FeatureAbility.terminateAbility()
      .then(data => console.info("Launcher settings SettingsModel closeSettings promise::then : " + data))
      .catch(error => console.info("Launcher settings SettingsModel closeSettings promise::catch : " + error));
    console.info("Launcher settings SettingsModel closeSettings end ");
  }

  public setEquipment(EType) {
    if (EType == 'phone') {
      this.mGridLayoutTable = GridLayoutConfigs.GridLayoutTable;
    } else {
      this.mGridLayoutTable = GridLayoutConfigs.GridLayoutTableHorizontal;
    }
    this.mLayoutConfig.setEquipmentType(EType);
  }
}