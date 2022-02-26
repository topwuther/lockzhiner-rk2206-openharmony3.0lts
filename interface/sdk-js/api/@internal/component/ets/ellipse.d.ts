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

import {CommonShapeMethod} from "./common";

export declare class EllipseExtend<T> extends EllipseAttribute<T> {
}

interface Ellipse extends EllipseAttribute<Ellipse> {
  new (value?: { width?: string | number, height?: string | number }): Ellipse;
  (value?: { width?: string | number, height?: string | number }): Ellipse;
}

declare class EllipseAttribute<T> extends CommonShapeMethod<T> {
}

export declare const EllipseInterface: Ellipse;
