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

/**
* Window manager.
* @devices tv, phone, tablet, wearable.
*/
declare namespace window {
  /**
   * Obtain the top window of the current application.
   * @devices tv, phone, tablet, wearable.
   */
  function getTopWindow(): Promise<Window>;

  /**
   * The type of a window.
   * @devices tv, phone, tablet, wearable.
   */
  enum WindowType {
    /**
     * App.
     */
    TYPE_APP,
    /**
     * System alert.
     */
    TYPE_SYSTEM_ALERT,
  }

  /**
   * The interface of window.
   */
  interface Window {
    /**
     * Set the position of a window.
     * @param x Indicate the X-coordinate of the window.
     * @param y Indicate the Y-coordinate of the window.
     * @devices tv, phone, tablet, wearable, liteWearable.
     */
    moveTo(x: number, y: number): Promise<void>;

    /**
     * Set the size of a window .
     * @param width Indicates the width of the window.
     * @param height Indicates the height of the window.
     * @devices tv, phone, tablet, wearable, liteWearable.
     */
    resetSize(width: number, height: number): Promise<void>;

    /**
     * Set the type of a window.
     * @param type Indicate the type of a window.
     * @devices tv, phone, tablet, wearable, liteWearable.
     */
    setWindowType(type: WindowType): Promise<void>;
  }
}

export default window;
