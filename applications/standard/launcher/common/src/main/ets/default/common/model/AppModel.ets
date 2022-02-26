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

import BundleMgr from '@ohos.bundle';
import FeatureAbility from '@ohos.ability.featureAbility';
import Subscriber from '@ohos.commonevent';
import SystemApplication from '../configs/SystemApplication.ets';
import CheckEmptyUtils from '../../utils/CheckEmptyUtils.ets';
import EventConstants from '../constants/EventConstants.ets';

const UNINSTALL_SUCCESS = "UNINSTALL_SUCCESS";
const UNINSTALL_FAILED = "UNINSTALL_FAILED";
const IF_GET_ABILITY = 1;

export default class AppModel {
  private static appModel = new AppModel();
  private mBundleInfoList = [];
  private systemApplicationName = SystemApplication.SystemApplicationName;
  private mCommonEventSubscriber = null;
  private mAppListInstallListener = [];
  private mAppListUninstallListener = [];
  private mAppListChangeListener = [];
  private mUninstallCallback;
  private mCommonEventSubscribeInfo = {
    events: [
      EventConstants.EVENT_PACKAGE_ADDED,
      EventConstants.EVENT_PACKAGE_CHANGED,
      EventConstants.EVENT_PACKAGE_REMOVED]
  };

  private constructor() {
  }

  public static getInstance(): AppModel{
    return this.appModel;
  }

  getAppList(callback) {
    this.mBundleInfoList = [];
    console.info('Launcher AppModel getAppIcon getAppList');
    BundleMgr.getAllBundleInfo(IF_GET_ABILITY)
      .then((data) => {
      if (CheckEmptyUtils.isEmpty(data)) {
        console.error("Launcher AppModel getAppList getAllBundleInfo ERROR");
      }
      console.info('Launcher AppModel getAllBundleInfo >' + JSON.stringify(data));
      for (let i = 0; i < data.length; i++) {
        if (this.systemApplicationName.indexOf(data[i].name) > -1) {
        } else {
          this.mBundleInfoList.push(
            {
              System: data[i].appInfo.systemApp,
              AppName: data[i].appInfo.label,
              AppId: data[i].appId,
              AppIcon: data[i].appInfo.iconId,
              bundleName: data[i].name,
              labelId: data[i].appInfo.labelId,
              abilityName: data[i].abilityInfos[0].name,
            }
          )
        }
      }
      let appArrayLength = this.mBundleInfoList.length;
      console.info('Launcher AppModel mBundleInfoList' + JSON.stringify(this.mBundleInfoList));
      callback(this.mBundleInfoList);
    });
  }

  uninstallApp(uninstallBundleName, callback) {
    console.info('Launcher AppModel uninstallApp appId' + uninstallBundleName);
    this.mUninstallCallback = callback;
    let result = BundleMgr.getBundleInstaller()
      .then((data) => {
      if (CheckEmptyUtils.isEmpty(data)) {
        console.error("Launcher AppModel uninstallApp getBundleInstaller ERROR");
      }
      data.uninstall(uninstallBundleName, {
        param: {
          userId: 0,
          installFlag: 0,
          isKeepData: false
        }
      }, this.uninstallCallback.bind(this));
    })
      .catch(error =>
    console.info("Launcher AppModel uninstall err " + error));
  }

  private uninstallCallback(err, data) {
    console.info('Launcher AppModel uninstallCallback ' + JSON.stringify(data));
    if (data.statusMessage == "SUCCESS") {
      this.mUninstallCallback(UNINSTALL_SUCCESS);
    } else {
      this.mUninstallCallback(UNINSTALL_FAILED);
    }
    console.info('Launcher AppModel uninstallCallback ');
  }

  registerAppListInstallListener(listener) {
    if (this.mAppListInstallListener.indexOf(listener) == -1) {
      this.mAppListInstallListener.push(listener);
    }
  }

  /**
   * Unregister install listener.
   *
   * @param {object} listener - install listener
   */
  unregisterAppListInstallListener(listener) {
    let index = this.mAppListInstallListener.indexOf(listener);
    if (index != -1) {
      this.mAppListInstallListener.splice(index, 1);
    }
  }

  /**
   * Register uninstall listener.
   *
   * @param {object} listener - uninstall listener
   */
  registerAppListUninstallListener(listener) {
    if (this.mAppListUninstallListener.indexOf(listener) == -1) {
      this.mAppListUninstallListener.push(listener);
    }
  }

  /**
   * Unregister uninstall listener.
   *
   * @param {object} listener - uninstall listener
   */
  unregisterAppListUninstallListener(listener) {
    let index = this.mAppListUninstallListener.indexOf(listener);
    if (index != -1) {
      this.mAppListUninstallListener.splice(index, 1);
    }
  }

  /**
   * Register change listener.
   *
   * @param {object} listener - uninstall listener
   */
  registerAppListChangeListener(listener) {
    if (this.mAppListChangeListener.indexOf(listener) == -1) {
      this.mAppListChangeListener.push(listener);
    }
  }

  /**
   * Unregister change listener.
   *
   * @param {object} listener - change listener
   */
  unregisterAppListChangeListener(listener) {
    let index = this.mAppListChangeListener.indexOf(listener);
    if (index != -1) {
      this.mAppListChangeListener.splice(index, 1);
    }
  }

  registerAppListEvent() {
    Subscriber.createSubscriber(
      this.mCommonEventSubscribeInfo,
      this.createInstallationSubscriberCallBack.bind(this)
    );
  }

  unregisterAppListEvent() {
    Subscriber.unsubscribe(this.mCommonEventSubscriber, () => {
      console.info("Launcher AppModel unregisterAppListEvent");
    });
  }

  private createInstallationSubscriberCallBack(err, data) {
    console.info("Launcher AppModel createInstallationSubscriberCallBack");
    this.mCommonEventSubscriber = data;
    Subscriber.subscribe(this.mCommonEventSubscriber, this.installationSubscriberCallBack.bind(this));
  }

  private installationSubscriberCallBack(err, data) {
    if (err.code == 0) {
      if (CheckEmptyUtils.isEmpty(data)) {
        console.error("Launcher AppModel installationSubscriberCallBack ERROR! data is empty");
      }
      console.info("Launcher AppModel installationSubscriberCallBack data = " + JSON.stringify(data));
      let callbackData = data;
      if (callbackData.event == EventConstants.EVENT_PACKAGE_REMOVED) {
        this.reportAppInstallEvent(callbackData.event, callbackData);
        return;
      }
      BundleMgr.getBundleInfo(callbackData.bundleName, IF_GET_ABILITY)
        .then(data => {
        console.info('Launcher AppModel installation subscriber getBundleInfo ' + JSON.stringify(data));
        let bundleInfo = {
          System: data.appInfo.systemApp,
          AppName: data.appInfo.label,
          AppId: data.appId,
          AppIcon: data.appInfo.iconId,
          bundleName: data.name,
          labelId: data.appInfo.labelId,
          abilityName: data.abilityInfos[0].name,
        };
        this.reportAppInstallEvent(callbackData.event, bundleInfo);
      })
        .catch(e => {
        console.error("Launcher AppModel installation subscriber getBundleInfo error ");
      });
    } else {
      console.error("Launcher AppModel app list change failed --- err = " + JSON.stringify(err));
    }
  }

  private reportAppInstallEvent(event, bundleInfo) {
    console.info("Launcher AppModel reportAppInstallEvent + " + event);
    switch (event) {
      case EventConstants.EVENT_PACKAGE_ADDED:
        this.notifyEventListener(this.mAppListInstallListener, bundleInfo);
        break;
      case EventConstants.EVENT_PACKAGE_CHANGED:
        this.notifyEventListener(this.mAppListChangeListener, bundleInfo);
        break;
      case EventConstants.EVENT_PACKAGE_REMOVED:
        this.notifyEventListener(this.mAppListUninstallListener, bundleInfo);
        break;
      default:
        break;
    }
  }

  startApplication(abilityName, bundleName) {
    let paramBundleName = bundleName;
    let paramAbilityName = abilityName;
    // promise
    console.info('Launcher ==> AppModel startApplication abilityName ==> ' + abilityName + " bundleName ==> " + bundleName);
    let result = FeatureAbility.startAbility({
      want: {
        bundleName: paramBundleName,
        abilityName: paramAbilityName
      }
    })
      .then(data =>
    console.info("Launcher AppModel startApplication promise::then : " + JSON.stringify(data))
    )
      .catch(error =>
    console.info("Launcher AppModel startApplication promise::catch : " + JSON.stringify(error))
    );
    console.info("Launcher ==> AppModel startApplication  AceApplication : startAbility : " + result);
  }

  private notifyEventListener(eventListener, bundleInfo) {
    for (let i = 0; i < eventListener.length; i++) {
      let listener = eventListener[i];
      if (listener != undefined && listener != null) {
        console.info("Launcher AppModel notifyEventListener " + JSON.stringify(bundleInfo));
        listener(bundleInfo);
      }
    }
  }
}