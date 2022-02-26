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

import * as framework from '../main';
import { services } from './service';
import { Log } from '../utils/utils';
import { Options } from '../main/app';
import { I18nInterface } from '../main/extend/i18n/I18n';
import { DPIInterface } from '../main/extend/dpi/Dpi';

export interface EnvInterface {
  config: Options;
  created: number;
  services: ServiceMapInterface;
}

export interface pageMapInterface {
  [key: string]: EnvInterface;
}

export interface ServicesInterface {
  I18n?: I18nInterface;
  dpi?: DPIInterface;
}

export interface ServiceMapInterface extends ServicesInterface {
  service?: any;
}

const pageMap: pageMapInterface = {};

/**
 * <p>Create instance by framework.</p>
 * <p>The framework is based on JS Bundle code.</p>
 * @param {string} id - Id of a page.
 * @param {string} code - JS Bundle code.
 * @param {Options} config - Page config
 * @param {Object} data - Data that needed.
 * @return {*}
 */
function createInstance(id: string, code: string, config: Options, data: object): any | Error {
  const page = pageMap[id];
  if (!page) {
    Log.debug(`Create a page.`);
    const env: EnvInterface = {
      config,
      created: Date.now(),
      services: createServices(id)
    };
    pageMap[id] = env;
    if (config && config.appCreate && config.appCode && config.packageName) {
      pageMap[config.appInstanceId] = env;
    }
    return framework.createInstance(id, code, config, data, env);
  }
  return new Error(`Invalid instance id '${id}'.`);
}

/**
 * Get root page by pageId.
 * @param {*} args - Args.
 * @return {*} Root page.
 */
function getRoot(...args: any[]): any | Error {
  const pageId = args[0];
  const page = getPage(pageId);
  if (page && framework) {
    return framework['getRoot'](pageId);
  }
  return new Error(`Invalid instance id '${pageId}'.`);
}

/**
 * Destroy a page.
 * @param {string} pageId - Id of a page.
 * @return {*}
 */
function destroyInstance(pageId: string): any | Error {
  const page = getPage(pageId);
  const result = framework.destroyInstance(pageId);
  if (page && framework) {
    services.forEach(service => {
      const destroy: (pageId: string) => void = service.options.destroy;
      if (destroy) {
        destroy(pageId);
      }
    });
    delete pageMap[pageId];
    return result;
  }
  return new Error(`Invalid page id '${pageId}'.`);
}

/**
 * <p>When native invokes this method,<br>
 * the receiveTasks method of the instance corresponding to the pageID is invoked.</p>
 * @param {string} pageId - Id of a page.
 * @param {*} tasks - Tasks from native.
 * @return {*}
 */
function callJS(pageId: string, tasks: any[]): any | Error {
  const page = getPage(pageId);
  if (page && framework) {
    return framework.receiveTasks(pageId, tasks);
  }
  return new Error(`Invalid page id '${pageId}'.`);
}

/**
 * Get page by id.
 * @param {string} id - Id of a page.
 * @return {EnvInterface} Page Env.
 */
function getPage(id: string): EnvInterface {
  return pageMap[id];
}

/**
 * Init JavaScript services for this instance.
 * @param {string} id - Create service by id.
 * @return {ServiceMapInterface} service map.
 */
function createServices(id: string): ServiceMapInterface {
  const serviceMap: ServiceMapInterface = {};
  services.forEach((service) => {
    Log.debug(`[JS Runtime] Create service ${service.name}.`);
    const create: (id: string) => any = service.options.create;
    if (create) {
      const result: any = create(id);
      Object.assign(serviceMap, result.instance);
    }
  });
  return serviceMap;
}

export default {
  createInstance: createInstance,
  getRoot: getRoot,
  callJS: callJS,
  destroyInstance: destroyInstance,
  appError: framework.appError,
  appDestroy: framework.appDestroy,
  appHide: framework.appHide,
  appShow: framework.appShow,
  registerModules: framework.registerModules
};
