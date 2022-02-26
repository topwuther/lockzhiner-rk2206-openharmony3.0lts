/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { Want } from '../ability/want';

/**
 * the info when object of WantAgent trigger
 *
 * @name TriggerInfo
 * @since 7
 * @devices phone, tablet, tv, wearable, car
 * @permission N/A
 */
export interface TriggerInfo {
  code: number;
  want?: Want;
  permission?: string;
  extraInfo?: {[key: string]: any};
}