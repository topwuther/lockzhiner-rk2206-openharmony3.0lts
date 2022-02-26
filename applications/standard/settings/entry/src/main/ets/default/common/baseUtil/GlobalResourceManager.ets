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
import BaseModel from '../../model/BaseModel.ets';
import Rsm from '@ohos.resourceManager';
import ConfigData  from './ConfigData.ets';

export class GlobalResourceManager extends BaseModel {

  public async getStringByResource(res: any): Promise<string>{
    let json = JSON.parse(JSON.stringify(res));
    let id = json.id;
    return await this.getStringById(id);
  }

  public getStringById(id: number): Promise<string>{
    let promise = new Promise<string>(resolve => {
      let resourceMgr = Rsm.getResourceManager(ConfigData.DEFAULT_BUNDLE_NAME);
        resourceMgr.then((result) => {
          result.getString(id)
          .then((resource) => {
            resolve(resource);
            console.info('getStringById resolve(resource) : ' + resolve(resource));
            console.info('getStringById resource : ' + resource);
            console.info('getStringById resource2 : ' + JSON.stringify(resource));
        })
          .catch((err) => {
            console.info('getStringById err : ' + JSON.stringify(err));
        });
      });
    });
    console.info('getStringById promise: ' + JSON.stringify(promise));
    return promise;
  }
}

let mGlobalResourceManager = new GlobalResourceManager();
export default mGlobalResourceManager as GlobalResourceManager
;
