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

import LruCache from './LruCache.ets';
import DiskLruCache from './DiskLruCache.ets';

/**
 * A Manager class that provides get/set/clear cache methods for app list data.
 */
export default class AppListInfoCacheManager {
  private lruCache;
  private diskLruCache;

  constructor() {
    this.lruCache = new LruCache();
    this.diskLruCache = new DiskLruCache();
  }

  /**
   * Get cache from disk or memory.
   *
   * @param {string} key - key of the cache map
   * @return {object} - cache get from the memory or disk
   */
  getCache(key) {
    console.info("Launcher AppListInfoCacheManager getCache key = " + key);
    let cache = this.lruCache.getCache(key);
    if (cache == undefined || cache == null || cache == '' || cache == -1) {
      return this.diskLruCache.getCache(key);
    } else {
      return cache;
    }
  }

  /**
   * Set cache to disk or memory.
   *
   * @param {string} key - key of the cache map
   * @param {object} value - value of the cache map
   */
  setCache(key, value) {
    console.info("Launcher AppListInfoCacheManager setCache key = " + key + " value = " + value);
    this.lruCache.putCache(key, value);
    this.diskLruCache.putCache(key, value);
  }

  /**
   * Clear cache of both disk and memory.
   */
  clearCache() {
    console.info("Launcher AppListInfoCacheManager clearCache");
    this.lruCache.clear();
    this.diskLruCache.clear();
  }
}