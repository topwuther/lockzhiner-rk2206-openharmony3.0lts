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

const DefaultLayoutConfig = {
  DefaultAppPageStartConfig: 'Grid',
  DefaultLayoutOptions: [
    { name: 'List', value: 'List', checked: false },
    { name: 'Grid', value: 'Grid', checked: false }
  ],
  DefaultGridConfig: 0,
  DefaultRecentProcessLimit: 20,
  DefaultRecentProcessLimitArray: [
    { name: '5', value: 5, checked: false },
    { name: '10', value: 10, checked: false },
    { name: '15', value: 15, checked: false },
    { name: '20', value: 20, checked: false }
  ],
}

export default DefaultLayoutConfig;