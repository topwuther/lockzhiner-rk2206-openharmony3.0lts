/**
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
import ConfigData from '../../common/baseUtil/ConfigData.ets';
import BaseModel from '../BaseModel.ets';
import LogUtil from '../../common/baseUtil/LogUtil.ets';
import Bundle from '@ohos.bundle';
import ResMgr from '@ohos.resourceManager';


const INDEX = 0;
const IS_INCLUDE_ABILITY_INFO = 0;
/**
 * app management service class
 */
export class AppManagementModel extends BaseModel {
  private mBundleInfoList: any[]= [];

  setAppManagementListener() {
    LogUtil.info('settings AppManagementModel getIconItem ResMgr:' + JSON.stringify(ResMgr));
    Bundle.getAllBundleInfo(IS_INCLUDE_ABILITY_INFO)
      .then((data) => {
      LogUtil.info('settings AppManagementModel setAppManagementListener getBundleInfos() start ');
      LogUtil.info('settings AppManagementModel data.length: ' + data.length + ' data: ' + JSON.stringify(data));
      this.getIconItem(INDEX, data.length, data);
    });
    LogUtil.info('settings appManagement init AppManagementModel setAppManagementListener end');
  }

  getIconItem(index, count, data) {
    LogUtil.info('settings AppManagementModel getIconItem start data.length' + data.length);
    let imageValue = '';
    let label = '';
    let that = this;
    LogUtil.info('settings AppManagementModel data[index].name :' + data[index].name);
    try {
      ResMgr.getResourceManager(data[index].name, (error, item) => {
        LogUtil.info('settings AppManagementModel getResourceManager item' + JSON.stringify(item) + "|error:" + error);
        let appInfo = data[index].appInfo;
        if (appInfo.labelId > 0) {
          item.getString(appInfo.labelId, (error, value) => {
            LogUtil.info('settings AppManagementModel getResourceManager  value.length:' + value.length);
            if (value != null) {
              LogUtil.info('settings AppManagementModel getResourceManager getString() value:' + value);
              label = value;
              LogUtil.info('settings AppManagementModel getResourceManager getString() label:' + label);
            } else {
              LogUtil.info('settings AppManagementModel getResourceManager getString() error:' + error);
            }
          });
        } else {
          LogUtil.info('settings AppManagementModel getResourceManager getString() label:' + appInfo.label);
          label = appInfo.label;
        }
        LogUtil.info('settings AppManagementModel getResourceManager getString() finish label:' + label);
        item.getMediaBase64(appInfo.iconId, (error, value) => {
          LogUtil.info('settings AppManagementModel getResourceManager getMediaBase64() :' + value.length);
          if (value.length > 0) {
            imageValue = value;
            LogUtil.info('settings AppManagementModel getResourceManager getMediaBase64() imageValue:' + imageValue);
          }
          LogUtil.info('settings AppManagementModel getResourceManager getMediaBase64() end');
          this.mBundleInfoList.push({
            settingIcon: imageValue,
            settingTitle: label,
            settingValue: '',
            settingArrow: $r('app.media.ic_settings_arrow'),
            settingSummary: data[index].versionName,
            settingBundleName: data[index].name,
            settingIconId: appInfo.iconId,
            settingUri: 'pages/applicationInfo'
          });
          if (count - 1 > index) {
            LogUtil.info('settings AppManagementModel getMediaBase64() if index:' + index + ' | count:' + count);
            index = index + 1;
            that.getIconItem(index, count, data);
          } else {
            LogUtil.info('settings AppManagementModel getMediaBase64() else index:' + index + ' | count:' + count);
            LogUtil.info('settings AppManagementModel mBundleInfoList[i]: ' + JSON.stringify(this.mBundleInfoList));
            AppStorage.SetOrCreate('appManagementList', this.mBundleInfoList);
          }
        });
      });
    } catch (error) {
      LogUtil.info('settings AppManagementModel catch error:' + error);
    }
    LogUtil.info('settings appManagement AppManagementModel getIconItem end');
  }
}

let appManagementModel = new AppManagementModel();
export default appManagementModel as AppManagementModel
;