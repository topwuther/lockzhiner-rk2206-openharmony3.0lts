/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * 2021.01.08 - Rewrite the function 'register' and make it simpler.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

import { Log } from '../utils/utils';

export interface OptionsInterface {
  create: (id: string) => any
  destroy: (id: string) => void
  refresh?: (id:string, data: object) => Error | void
}

export interface ServicesInterface {
  name: string
  options: OptionsInterface
}

export const services: ServicesInterface[] = [];

/**
 * Register a service.
 * @param {string} name - Service name.
 * @param {OptionsInterface} options - Could have { create, destroy, refresh } lifecycle methods.
 */
export function register(name: string, options: OptionsInterface): void {
  const hasName = services.map(
    service => service.name
  ).indexOf(name) >= 0;

  if (hasName) {
    Log.warn(`Service '${name}' has been registered already!`);
  } else {
    options = Object.assign({}, options);
    services.push({
      name,
      options
    });
  }
}

/**
 * Unregister a service by name.
 * @param {string} name - Service name.
 */
export function unregister(name: string): void {
  services.some((service: ServicesInterface, index: number) => {
    if (service.name === name) {
      services.splice(index, 1);
      return true;
    }
  });
}

export default {
  register,
  unregister
};
