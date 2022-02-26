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

import { Log } from '../../utils/index';
import { watch } from './directive';
import cssWhat from 'css-what';
import Element from '../../vdom/Element';
import Vm from './index';
import { cssType } from './vmOptions';
import {
  FragBlockInterface,
  isBlock
} from './compiler';

interface ContentInterface {
  type: string;
  name?: string;
  action?: string;
  value?: string;
  ignoreCase?: boolean;
}

interface ParamsInterface {
  id: string | Function;
  class: string[] | Function,
  tag: string
}

interface EvidenceInterface {
  id: string;
  ruleDef: ContentInterface[];
  style: Record<string, any>;
  score: number;
  order: number;
  priority?: number;
}

type ListType<T> = Partial<Record<'tagList' | 'idList' | 'classList', T[]>>

type ResStyle<T> = Partial<{resArray: T[], watchValue: T}>;

/**
 * Support css descendant selector.
 * @param {cssType } styleOri - Css style.
 * @return {ListType<EvidenceInterface>} object: {tagList, idList, classList}
 */
export function selector(styleOri: cssType): ListType<EvidenceInterface> {
  const style = addMetaToStyle(styleOri);
  const list: ListType<EvidenceInterface> = {};
  const keys = Object.keys(style || {});
  keys.forEach((key) => {
    if (style[key] && style[key]._meta) {
      const ruleDef = style[key]._meta.ruleDef;
      if (ruleDef) {
        const rule = ruleDef[ruleDef.length - 1];
        const object: EvidenceInterface = {
          id: rule.type === 'tag' ? rule.name : rule.value,
          ruleDef: ruleDef,
          style: style[key],
          score: getScore(ruleDef),
          order: keys.indexOf(key)
        };
        if (rule.type === 'tag') {
          list.tagList = list.tagList || [];
          list.tagList.push(object);
        } else if (rule.type === 'attribute' && rule.name === 'id') {
          list.idList = list.idList || [];
          list.idList.push(object);
        } else if (rule.type === 'attribute' && rule.name === 'class') {
          list.classList = list.classList || [];
          list.classList.push(object);
        } else {
          Log.error(`Invalid ruleDef, t = ${rule.type}, name = ${rule.name}.`);
        }
      }
    }
  });
  return list;
}

/**
 * Set descendant style.
 * @param {ListType<EvidenceInterface>} selector - Descendant selector.
 * @param {ParamsInterface} params - Param: {id, class, tag...}
 * @param {Element | FragBlockInterface} parentElement - Parent element object.
 * @param {Vm} vm - Vm object.
 * @param {Function} styleFunction - StyleFunction.
 */
export function setDescendantStyle(
  selector: ListType<EvidenceInterface>,
  params: Partial<ParamsInterface>,
  parentElement: Element | FragBlockInterface,
  vm: Vm,
  styleFunction: (style: object) => void
): void {
  const retStyle = {};
  let oldStyle;
  const applyStyleRes = applyStyle(selector, params, parentElement, vm, undefined);
  assignStyleByPriority(retStyle, applyStyleRes);
  styleFunction(retStyle);
  oldStyle = retStyle;
  const watchValue = applyStyleRes.watchValue;
  if (watchValue) {
    watch(vm, watchValue.value, v => {
      const applyStyleRes = applyStyle(selector, params, parentElement, vm, v);
      const retStyle = {};
      assignStyleByPriority(retStyle, applyStyleRes);
      if (oldStyle) {
        Object.keys(oldStyle).forEach(function(key) {
          if (!retStyle[key]) {
            retStyle[key] = '';
          }
        });
      }
      styleFunction(retStyle);
      oldStyle = retStyle;
    });
  }
}

/**
 * Apply style.
 * @param {ListType<EvidenceInterface>} selector - Descendant selector.
 * @param {ParamsInterface} params - Param: {id, class, tag...}
 * @param {Element | FragBlockInterface} parentElement - Parent element object.
 * @param {Vm} vm - Vm object.
 * @param {string[]} [classValue] - If has class value.
 * @return {ResStyle<any>} {resArray, watchValue}
 */
function applyStyle(
  selector: ListType<EvidenceInterface>,
  params: Partial<ParamsInterface>,
  parentElement: Element | FragBlockInterface,
  vm: Vm,
  classValue?: string[]
): ResStyle<any> {
  const applyStyleRes: ResStyle<any> = {};
  applyStyleRes.resArray = [];
  if (params.id) {
    const value = isIdFunction(params.id) ? params.id.call(vm, vm) : params.id;
    const rets = setDescendant(selector.idList, value, parentElement);
    if (rets) {
      applyStyleRes.resArray = applyStyleRes.resArray.concat(rets);
    }
  }
  if (!classValue) {
    if (params.class) {
      classValue = isClassFunction(params.class) ? params.class.call(vm, vm) : params.class;
      applyStyleRes.watchValue = isClassFunction(params.class) ? {list: selector.classList, value: params.class} : undefined;
    }
  }
  if (Array.isArray(classValue)) {
    classValue.forEach(function(value) {
      const rets = setDescendant(selector.classList, value, parentElement);
      if (rets) {
        applyStyleRes.resArray = applyStyleRes.resArray.concat(rets);
      }
    });
  } else {
    const rets = setDescendant(selector.classList, classValue, parentElement);
    if (rets) {
      applyStyleRes.resArray = applyStyleRes.resArray.concat(rets);
    }
  }

  if (params.tag) {
    const rets = setDescendant(selector.tagList, params.tag, parentElement);
    if (rets) {
      applyStyleRes.resArray = applyStyleRes.resArray.concat(rets);
    }
  }
  return applyStyleRes;
}

/**
 * Assign style by priority.
 * @param {Object} retStyle - Returned style.
 * @param {ResStyle<any>} applyStyleRes - {resArray, watchValue}
 */
function assignStyleByPriority(retStyle: object, applyStyleRes: ResStyle<any>): void {
  const arr = applyStyleRes.resArray;
  arr.sort(function(a, b) {
    if (!a) {
      return -1;
    } else if (!b) {
      return 1;
    } else {
      return a.priority > b.priority ? 1 : -1;
    }
  });
  arr.forEach(function(key) {
    if (key && key.style) {
      Object.assign(retStyle, key.style);
    }
  });
}

/**
 * Set descendant for Node.
 * @param {EvidenceInterface} list - Node list to be set priority.
 * @param {string} value - Value.
 * @param {Element | FragBlockInterface} parentElement - Parent element.
 * @return {*} Node list after setting priority
 */
function setDescendant(list: EvidenceInterface[], value: string, parentElement: Element | FragBlockInterface) {
  if (!list) {
    return;
  }
  const rets = [];
  for (let i = 0; i < list.length; i++) {
    if (list[i].id === value) {
      const ruleDef = list[i].ruleDef;
      let parent = parentElement;
      let markElement;
      for (let j = ruleDef.length - 3; j >= 0; j = j - 2) {
        const rule = ruleDef[j];
        markElement = getElement(
          rule.type === 'tag' ? rule.type : rule.name,
          ruleDef[j + 1].type !== 'child',
          rule.type === 'tag' ? rule.name : rule.value,
          parent
        );
        if (!markElement) {
          break;
        }
        parent = markElement.parentNode;
        if (parent === undefined && markElement.ref !== '_documentElement' && global.treeModeParentNode) {
          parent = global.treeModeParentNode;
        }
      }
      if (markElement) {
        const ret = list[i];
        ret.priority = ret.score + ret.order * 0.01;
        rets.push(ret);
      }
    }
  }
  return rets;
}

/**
 * Get parent by condition
 * @param {string} type - Condition type: class id tag.
 * @param {string} iter - If Can get parent`s parent node.
 * @param {string} value - Condition value, class value/id value/tag value.
 * @param {Element | FragBlockInterface} element - ParentNode.
 * @return {*} ParentNode.
 */
function getElement(type: string, iter: boolean, value: string, element: Element | FragBlockInterface) {
  if (element === null) {
    return undefined;
  }
  if (isBlock(element)) {
    element = element.element;
  }
  let parentNode = element.parentNode as Element;
  if (parentNode === null && element.ref !== '_documentElement' && global.treeModeParentNode) {
    parentNode = global.treeModeParentNode;
  }
  // Type : class id tag per.
  const condition = type === 'class' ? element.classList && element.classList.indexOf(value) !== -1
    : type === 'id' ? element.id === value : element.type === value;
  return condition ? element : iter ? getElement(type, iter, value, parentNode) : undefined;
}

/**
 * Get score of Node.
 * @param {ContentInterface[]} ruleDef - Rule.
 * @return {number} - Score.
 */
function getScore(ruleDef: ContentInterface[]): number {
  let score = 0;
  for (let i = 0; i < ruleDef.length; i++) {
    const rule = ruleDef[i];
    if (rule.type === 'tag') {
      score += 1;
    } else if (rule.type === 'attribute' && rule.name === 'id') {
      score += 10000;
    } else if (rule.type === 'attribute' && rule.name === 'class') {
      score += 100;
    }
  }
  return score;
}

/**
 * Add meta to style.
 * @param {cssType} classObject - Class style Info.
 * @return {cssType } Class style Info List.
 */
function addMetaToStyle(classObject: cssType): cssType {
  const classList = classObject;
  const classListkey = Object.keys(classList || {});
  classListkey.forEach(key => {
    // Style is not empty and _meta exists.
    if (
      Object.keys(classList[key]).length > 0 &&
            !classList[key]['_meta'] &&
            key !== '.@originalRootEl' &&
            key !== '@KEYFRAMES' &&
            key !== '@MEDIA' &&
            key !== '@FONT-FACE' &&
            key !== '@TRANSITION'
    ) {
      const meta: { ruleDef?: string[] } = {};
      const keys = cssWhat(key);
      meta.ruleDef = keys[0];
      if (meta.ruleDef && meta.ruleDef.length >= 3) {
        classList[key]['_meta'] = meta;
      }
    }
  });
  return classList;
}

/**
 * Check if it is a Id function.
 * @param {Function | string} param - Any type.
 * @return {boolean} Param is Function or not.
 */
function isIdFunction(param: Function | string): param is Function {
  const newParam = <Function>param;
  return newParam.call !== undefined;
}

/**
 * Check if it is a class function.
 * @param {Function | string} param - Any type.
 * @return {boolean} Param is Function or not.
 */
function isClassFunction(param: Function | string[]): param is Function {
  const newParam = <Function>param;
  return newParam.call !== undefined;
}
