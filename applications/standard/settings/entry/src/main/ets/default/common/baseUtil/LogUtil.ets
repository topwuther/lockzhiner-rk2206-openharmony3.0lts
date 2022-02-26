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

/**
 * Log level
 */
let LOG_LEVEL = {
  /**
     * debug
     */
  DEBUG: 'debug',

  /**
     * log
     */
  LOG: 'log',

  /**
     * info
     */
  INFO: 'info',

  /**
     * warn
     */
  WARN: 'warn',

  /**
     * error
     */
  ERROR: 'error'
};

/**
 *  log package tool class
 */
export class LogUtil extends BaseModel {
  debug(msg): void {
    console.info(msg);
  }

  log(msg): void {
    console.log(msg);

  }

  info(msg): void {
    console.info(msg);
  }

  warn(msg): void {
    console.warn(msg);
  }

  error(msg): void {
    console.error(msg);
  }
}

let mLogUtil = new LogUtil();
export default mLogUtil as LogUtil
;
