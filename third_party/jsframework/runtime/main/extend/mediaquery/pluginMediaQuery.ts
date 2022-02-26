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

import { matchMediaQueryCondition } from './mediaQuery';

interface MediaQueryModule {
  addListener: Function;
  getDeviceType: Function;
}

interface MatchData {
  matches: boolean;
}

class MediaQueryList {
  private _matches: boolean;
  private _condition: string;
  private _onchange: any;
  private _listeners: any[];

  constructor(condition: string) {
    this._condition = condition;
    this._onchange = null;
    this._listeners = [];
  }

  get matches() {
    return this._matches;
  }

  set matches(matches: boolean) {
    this._matches = matches;
  }

  get condition() {
    return this._condition;
  }

  get media() {
    return this._condition;
  }

  get onchange() {
    return this._onchange;
  }

  set onchange(onchange: any) {
    this._onchange = onchange;
  }

  get listeners() {
    return this._listeners;
  }

  public addListener(matchFunction: any) {
    this._listeners.push(matchFunction);
  }

  public removeListener(matchFunction: any) {
    const index: number = this._listeners.indexOf(matchFunction);
    if (index > -1) {
      this._listeners.splice(index, 1);
    }
  }
}

/**
 * MediaQuery api.
 */
export class MediaQueryApi {
  private _module: MediaQueryModule;
  private _mqlArray: MediaQueryList[];

  constructor(module: MediaQueryModule) {
    this._module = module;
    this._mqlArray = [];
    this._module.addListener((data) => {
      if (!this._mqlArray || this._mqlArray.length === 0) {
        return;
      }
      data['device-type'] = data.deviceType;
      data['aspect-ratio'] = data.aspectRatio;
      data['device-width'] = data.deviceWidth;
      data['device-height'] = data.deviceHeight;
      data['round-screen'] = data.roundScreen;
      data['dark-mode'] = data.darkMode;
      for (let i = 0; i < this._mqlArray.length; i++) {
        const mediaQueryList:MediaQueryList = this._mqlArray[i];
        const result: boolean = matchMediaQueryCondition(mediaQueryList.condition, data, true);

        mediaQueryList.matches = result;
        const matchData: MatchData = { matches: false };
        matchData.matches = result;

        if (mediaQueryList.listeners && mediaQueryList.listeners.length !== 0) {
          for (let i = 0; i < mediaQueryList.listeners.length; i++) {
            const matchFunc = mediaQueryList.listeners[i];
            matchFunc(matchData);
          }
        }

        if (typeof mediaQueryList.onchange === 'function') {
          mediaQueryList.onchange(matchData);
        }
      }
    });
  }

  /**
   * Creates a MediaQueryList object based on the query condition.
   * @param {string} condition - Media query condition.
   * @return {MediaQueryList}
   */
  public matchMedia(condition: string): MediaQueryList {
    const mediaquerylist: MediaQueryList = new MediaQueryList(condition);
    this._mqlArray.push(mediaquerylist);
    return mediaquerylist;
  }
}
