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

import Router from '@system.router';

/**
 * An util that encapsulates methods from @system.router.
 */
let RouterUtil = function () {
  return {

    /**
     * Push the page into the Router stack.
     *
     * @param {string} uri - uri of the page
     * @param {object} params - params while opening the page
     */
    push: function (uri, params) {
      Router.push({
        uri: uri,
        params: params
      })
    },

    /**
     * Replace original page to the current page.
     *
     * @param {string} uri - uri of the page
     * @param {object} params - params while opening the page
     */
    replace: function (uri, params) {
      Router.replace({
        uri: uri,
        params: params
      })
    },

    /**
     * Back to previous page and pop current page.
     */
    back: function () {
      Router.back();
    },

    /**
     * Clear router stack.
     */
    clear: function () {
      Router.clear();
    }
  }
}();

export default RouterUtil;