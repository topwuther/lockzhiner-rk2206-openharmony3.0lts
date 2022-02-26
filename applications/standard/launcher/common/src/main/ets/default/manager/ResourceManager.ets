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

import Resmgr from '@ohos.resourceManager';
import AppResourceCacheManager from '../cache/AppResourceCacheManager.ets'
import CheckEmptyUtils from '../utils/CheckEmptyUtils.ets'

const KEY_ICON = "icon";
const KEY_NAME = "name";

export default class ResourceManager {
  private static resourceManager: ResourceManager = new ResourceManager();
  private mAppResourceCacheManager: AppResourceCacheManager = null;

  private constructor() {
  }

  public static getInstance(): ResourceManager{
    return this.resourceManager;
  }

  private getAppResourceCacheManager() {
    if (this.mAppResourceCacheManager == null) {
      this.mAppResourceCacheManager = new AppResourceCacheManager();
    }
    return this.mAppResourceCacheManager;
  }

  getAppIcon(path, bundleName, callback, defaultAppIcon) {
    if (path == null || path == undefined || path == "" || path <= 0) {
      console.info('Launcher ResourceManager getAppIcon iconId > ' + defaultAppIcon);
      callback(defaultAppIcon);
    } else {
      Resmgr.getResourceManager(bundleName)
        .then(item => {
        if (CheckEmptyUtils.isEmpty(item)) {
          console.error("Launcher ResourceManager getAppIcon getResourceManager ERROR! item is empty");
        }
        console.info('Launcher ResourceManager getAppIcon  data>' + item);
        item.getMediaBase64(path, (error, value) => {
          console.info('Launcher ResourceManager getAppIcon getMediaBase64 value>' + value);
          if (value != null) {
            callback(value);
          }
          else {
            callback(defaultAppIcon);
          }
        });
      })
        .catch(e => {
        console.error("Launcher ResourceManager getAppIcon error ");
        callback(defaultAppIcon);
      });
    }
  }

  getAppIconWithCache(path, bundleName, callback, defaultAppIcon) {
    if (path == null || path == undefined || path == "" || path <= 0) {
      console.info('Launcher ResourceManager getAppIcon iconId > ' + defaultAppIcon);
      callback(defaultAppIcon);
    } else {
      let iconBase64 = this.getAppResourceCacheManager().getCache(bundleName, KEY_ICON);
      if (iconBase64 == undefined || iconBase64 == null || iconBase64 == '') {
        Resmgr.getResourceManager(bundleName)
          .then(item => {
          if (CheckEmptyUtils.isEmpty(item)) {
            console.error("Launcher ResourceManager getAppIcon getResourceManager ERROR! item is empty");
          }
          console.info('Launcher ResourceManager getAppIcon  data>' + item);
          item.getMediaBase64(path, (error, value) => {
            console.info('Launcher ResourceManager getAppIcon getMediaBase64 value>' + value);
            if (value != null) {
              this.getAppResourceCacheManager().setCache(bundleName, KEY_ICON, value);
              callback(value);
            }
            else {
              callback(defaultAppIcon);
            }
          });
        })
          .catch(e => {
          console.error("Launcher ResourceManager getAppIcon error ");
          callback(defaultAppIcon);
        });
      } else {
        callback(iconBase64);
      }
    }
  }

  getAppName(labelId, bundleName, appName, callback) {
    if (labelId == null || labelId == undefined || labelId == "" || labelId <= 0) {
      console.info('Launcher ResourceManager getAppName callback ' + appName);
      callback(appName);
    } else {
      Resmgr.getResourceManager(bundleName)
        .then(item => {
        console.info('Launcher ResourceManager getAppName getResourceManager labelId' + labelId);
        item.getString(labelId, (error, value) => {
          if (CheckEmptyUtils.checkStrIsEmpty(value)) {
            console.error("Launcher AppModel getAppName getString ERROR! value is empty");
          }
          console.info('Launcher ResourceManager getAppName  getString value>' + value);
          if (value != null) {
            callback(value);
          } else {
            callback(appName);
          }
        });
      })
        .catch(e => {
        console.error("Launcher ResourceManager getAppName error ")
        callback(appName);
      });
    }
  }

  getAppNameWithCache(labelId, bundleName, appName, callback) {
    if (labelId == null || labelId == undefined || labelId == "" || labelId <= 0) {
      console.info('Launcher ResourceManager getAppName callback ' + appName);
      callback(appName);
    } else {
      let name = this.getAppResourceCacheManager().getCache(bundleName, KEY_NAME);
      if (name == undefined || name == null || name == '') {
        Resmgr.getResourceManager(bundleName)
          .then(item => {
          console.info('Launcher ResourceManager getAppName getResourceManager labelId' + labelId);
          item.getString(labelId, (error, value) => {
            if (CheckEmptyUtils.checkStrIsEmpty(value)) {
              console.error("Launcher AppModel getAppName getString ERROR! value is empty");
            }
            console.info('Launcher ResourceManager getAppName  getString value>' + value);
            if (value != null) {
              this.getAppResourceCacheManager().setCache(bundleName, KEY_NAME, value);
              callback(value);
            } else {
              callback(appName);
            }
          });
        })
          .catch(e => {
          console.error("Launcher ResourceManager getAppName error ")
          callback(appName);
        });
      } else {
        callback(name);
      }
    }
  }

  /**
     * Get app resource cache.
     *
     * @param {string} bundleName - bundleName of target file
     * @param {string} key - key of the cache
     */
  getAppResourceCache(bundleName, key) {
    return this.getAppResourceCacheManager().getCache(bundleName, key);
  }
}