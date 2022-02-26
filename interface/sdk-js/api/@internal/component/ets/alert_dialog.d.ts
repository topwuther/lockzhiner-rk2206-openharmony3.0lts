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

import {CommonMethod, Resource} from "./common"

export declare enum DialogAlignment {
  Top,
  Center,
  Bottom,
  Default
}

interface AlertDialog {
  show(value: {
    title?: string | Resource;
    message: string | Resource;
    autoCancel?: boolean;
    confirm?: {
      value: string | Resource;
      action: () => void;
    };
    cancel?: () => void;
    alignment?: DialogAlignment;
    offset?: { dx: number | string | Resource, dy: number | string | Resource };
  } | {
    title?: string | Resource;
    message: string | Resource;
    autoCancel?: boolean;
    primaryButton: {
      value: string | Resource;
      action: () => void;
    };
    secondaryButton: {
      value: string | Resource;
      action: () => void;
    };
    cancel?: () => void;
    alignment?: DialogAlignment;
    offset?: { dx: number | string | Resource, dy: number | string | Resource };
  });
}

export declare const AlertDialogInterface: AlertDialog;
