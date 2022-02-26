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
 * 2021.01.08 - Rewrite some functions and remove some redundant judgments to fit framework.
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 */

/**
 * @fileOverview
 * Directive Parser
 */

import {
  typof,
  camelize,
  Log
} from '../../utils/index';
import Watcher from '../reactivity/watcher';
import {
  setDescendantStyle
} from './selector';
import {
  getDefaultPropValue
} from '../util/props';
import {
  matchMediaQueryCondition
} from '../extend/mediaquery/mediaQuery';
import {
  TemplateInterface,
  FragBlockInterface,
  AttrInterface
} from './compiler';
import Vm from './index';
import Element from '../../vdom/Element';

const SETTERS = {
  attr: 'setAttr',
  style: 'setStyle',
  data: 'setData',
  event: 'addEvent',
  idStyle: 'setIdStyle',
  tagStyle: 'setTagStyle'
};

/**
 * Bind id, attr, classnames, style, events to an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - Element to be bind.
 * @param {TemplateInterface} template - Structure of the component.
 * @param {Element | FragBlockInterface} parentElement - Parent element of current element.
 */
export function bindElement(vm: Vm, el: Element, template: TemplateInterface, parentElement: Element | FragBlockInterface): void {
  // Set descendant style.
  setDescendantStyle(
    vm.selector,
    {
      id: template.id,
      class: template.classList,
      tag: template.type
    },
    parentElement,
    vm,
    function(style: {[key: string]: any}) {
      if (!style) {
        return;
      }
      const css = vm.css || {};
      setAnimation(style, css);
      setFontFace(style, css);
      setStyle(vm, el, style);
    }
  );

  // inherit 'show' attribute of custom component
  if (el.isCustomComponent) {
    const value = vm['show'];
    if (template.attr && value !== undefined) {
      if (typeof value === 'function') {
        // vm['show'] is assigned to this.show in initPropsToData()
        template.attr['show'] = function() {
          return this.show;
        };
      } else {
        template.attr['show'] = value;
      }
    }
  }

  setId(vm, el, template.id, vm);
  setAttr(vm, el, template.attr);
  setStyle(vm, el, template.style);
  setIdStyle(vm, el, template.id);
  setClass(vm, el, template.classList);
  setTagStyle(vm, el, template.type);
  applyStyle(vm, el);

  bindEvents(vm, el, template.events);
  bindEvents(vm, el, template.onBubbleEvents, '');
  bindEvents(vm, el, template.onCaptureEvents, 'capture');
  bindEvents(vm, el, template.catchBubbleEvents, 'catchbubble');
  bindEvents(vm, el, template.catchCaptureEvents, 'catchcapture');

  if (!vm.isHide && !vm.init) {
    el.addEvent('hide');
    vm.isHide = true;
  }
}

/**
 * <p>Bind all props to sub vm and bind all style, events to the root element</p>
 * <p>of the sub vm if it doesn't have a replaced multi-node fragment.</p>
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 * @param {TemplateInterface} template - Structure of the component.
 * @param {Object} repeatItem - Item object.
 */
export function bindSubVm(vm: Vm, rawSubVm: Vm, rawTemplate: TemplateInterface, repeatItem: object): void {
  const subVm: any = rawSubVm || {};
  const template: any = rawTemplate || {};
  const options: any = subVm.vmOptions || {};

  let props = options.props;
  if (isArray(props) || !props) {
    if (isArray(props)) {
      props = props.reduce((result, value) => {
        result[value] = true;
        return result;
      }, {});
    }
    mergeProps(repeatItem, props, vm, subVm);
    mergeProps(template.attr, props, vm, subVm);
  } else {
    const attrData = template.attr || {};
    const repeatData = repeatItem || {};
    Object.keys(props).forEach(key => {
      const prop = props[key];
      let value = attrData[key] || repeatData[key] || undefined;
      if (value === undefined) {
        value = getDefaultPropValue(vm, prop);
      }
      mergePropsObject(key, value, vm, subVm);
    });
  }
}

/**
 * Merge class and styles from vm to sub vm.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 * @param {TemplateInterface} template - Structure of the component.
 * @param {Element | FragBlockInterface} target - The target of element.
 */
export function bindSubVmAfterInitialized(vm: Vm, subVm: Vm, template: TemplateInterface, target: Element | FragBlockInterface): void {
  mergeClassStyle(template.classList, vm, subVm);
  mergeStyle(template.style, vm, subVm);
  if (target.children) {
    target.children[target.children.length - 1]._vm = subVm;
  } else {
    target.vm = subVm;
  }
  bindSubEvent(vm, subVm, template);
}

/**
 * Bind custom event from vm to sub vm for calling parent method.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 * @param {TemplateInterface} template - Structure of the component.
 */
function bindSubEvent(vm: Vm, subVm: Vm, template: TemplateInterface): void {
  if (template.events) {
    for (const type in template.events) {
      subVm.$on(camelize(type), function() {
        const args = [];
        for (const i in arguments) {
          args[i] = arguments[i];
        }
        if (vm[template.events[type]]
            && typeof vm[template.events[type]] === 'function') {
          vm[template.events[type]].apply(vm, args);
        }
      });
    }
  }
}

/**
 * Merge props from vm to sub vm.
 * @param {string} key - Get vm object by key.
 * @param {*} value - Default Value.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 * @return {*} Sub vm object.
 */
function mergePropsObject(key: string, value: any, vm: Vm, subVm: Vm): any {
  subVm.props.push(key);
  if (typeof value === 'function') {
    const returnValue = watch(vm, value, function(v) {
      subVm[key] = v;
    });
    // 'show' attribute will be inherited by elements in custom component
    if (key === 'show') {
      subVm[key] = value;
    } else {
      subVm[key] = returnValue;
    }
  } else {
    const realValue =
        value && value.__hasDefault ? value.__isDefaultValue : value;
    subVm[key] = realValue;
  }
  return subVm[key];
}

/**
 * Bind props from vm to sub vm and watch their updates.
 * @param {Object} target - Target object.
 * @param {*} props - Vm props.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 */
function mergeProps(target: object, props: any, vm: Vm, subVm: Vm): void {
  if (!target) {
    return;
  }
  for (const key in target) {
    if (!props || props[key] || key === 'show') {
      subVm.props.push(key);
      const value = target[key];
      if (typeof value === 'function') {
        const returnValue = watch(vm, value, function(v) {
          subVm[key] = v;
        });
        // 'show' attribute will be inherited by elements in custom component
        if (key === 'show') {
          subVm[key] = value;
        } else {
          subVm[key] = returnValue;
        }
      } else {
        subVm[key] = value;
      }
    }
  }
}

/**
 * Bind style from vm to sub vm and watch their updates.
 * @param {Object} target - Target object.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 */
function mergeStyle(target: { [key: string]: any }, vm: Vm, subVm: Vm): void {
  for (const key in target) {
    const value = target[key];
    if (typeof value === 'function') {
      const returnValue = watch(vm, value, function(v) {
        if (subVm.rootEl) {
          subVm.rootEl.setStyle(key, v);
        }
      });
      subVm.rootEl.setStyle(key, returnValue);
    } else {
      if (subVm.rootEl) {
        subVm.rootEl.setStyle(key, value);
      }
    }
  }
}

/**
 * Bind class and style from vm to sub vm and watch their updates.
 * @param {Object} target - Target object.
 * @param {Vm} vm - Vm object.
 * @param {Vm} subVm - Sub vm.
 */
function mergeClassStyle(target: Function | string[], vm: Vm, subVm: Vm): void {
  const css = vm.css || {};
  if (!subVm.rootEl) {
    return;
  }

  /**
   * Class name.
   * @constant {string}
   */
  const CLASS_NAME = '@originalRootEl';
  css['.' + CLASS_NAME] = subVm.rootEl.classStyle;

  function addClassName(list, name) {
    if (typof(list) === 'array') {
      list.unshift(name);
    }
  }

  if (typeof target === 'function') {
    const value = watch(vm, target, v => {
      addClassName(v, CLASS_NAME);
      setClassStyle(subVm.rootEl, css, v);
    });
    addClassName(value, CLASS_NAME);
    setClassStyle(subVm.rootEl, css, value);
  } else if (target !== undefined) {
    addClassName(target, CLASS_NAME);
    setClassStyle(subVm.rootEl, css, target);
  }
}

/**
 * Bind id to an element. Note: Each id is unique in a whole vm.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - Element object.
 * @param {Function | string} id - Unique vm id.
 * @param {Vm} target - Target vm.
 */
export function setId(vm: Vm, el: Element, id: Function | string, target: Vm): void {
  const map = Object.create(null);
  Object.defineProperties(map, {
    vm: {
      value: target,
      writable: false,
      configurable: false
    },
    el: {
      get: () => el || target.rootEl,
      configurable: false
    }
  });
  if (typeof id === 'function') {
    const handler = id;
    const newId = handler.call(vm);
    if (newId || newId === 0) {
      setElementId(el, newId);
      vm.ids[newId] = map;
    }
    watch(vm, handler, (newId) => {
      if (newId) {
        setElementId(el, newId);
        vm.ids[newId] = map;
      }
    });
  } else if (id && typeof id === 'string') {
    setElementId(el, id);
    vm.ids[id] = map;
  }
}

/**
 * Set id to Element.
 * @param {Element} el - Element object.
 * @param {string} id - Element id.
 */
function setElementId(el: Element, id: string): void {
  if (el) {
    el.id = id;
  }
}

/**
 * Bind attr to an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - Element.
 * @param {AttrInterface} attr - Attr to bind.
 */
function setAttr(vm: Vm, el: Element, attr: Partial<AttrInterface>): void {
  if (attr && attr.data) {
    // address data independently
    bindDir(vm, el, 'data', attr.data);
  }
  bindDir(vm, el, 'attr', attr);
}

/**
 * Set font family and get font resource.
 * @param {Object} css - Css style.
 * @param {string | string[]} fontFamilyNames - Font family names.
 * @return {*} Font resource.
 */
function _getFontFamily(css: any, fontFamilyNames: string | string[]): any[] {
  let results = [];
  const familyMap = css['@FONT-FACE'];
  if (typeof fontFamilyNames === 'string') {
    fontFamilyNames.split(',').forEach(fontFamilyName => {
      fontFamilyName = fontFamilyName.trim();
      let find = false;
      if (familyMap && Array.isArray(familyMap)) {
        let len = familyMap.length;
        while (len) {
          if (
            familyMap[len - 1].fontFamily &&
              familyMap[len - 1].fontFamily === fontFamilyName
          ) {
            results.push(familyMap[len - 1]);
            find = true;
          }
          len--;
        }
      } else if (familyMap && typeof familyMap === 'object') {
        const definedFontFamily = familyMap[fontFamilyName];
        if (definedFontFamily && definedFontFamily.src) {
          if (Array.isArray(definedFontFamily.src)) {
            definedFontFamily.src = definedFontFamily.src.map(item => `url("${item}")`).join(',');
          }
          results.push(definedFontFamily);
          find = true;
        }
      }
      if (!find) {
        results.push({ 'fontFamily': fontFamilyName });
      }
    });
  } else if (Array.isArray(fontFamilyNames)) {
    results = fontFamilyNames;
  } else if (fontFamilyNames) {
    Log.warn(`GetFontFamily Array error, unexpected fontFamilyNames type [${typeof fontFamilyNames}].`);
  }
  return results;
}

/**
 * Select class style.
 * @param {Object} css - Css style.
 * @param {Function | string[]} classList - List of class label.
 * @param {number} index - Index of classList.
 * @return {*} Select style.
 */
function selectClassStyle(css: object, classList: Function | string[], index: number, vm: Vm): any {
  const key = '.' + classList[index];
  return selectStyle(css, key, vm);
}

/**
 * Select id style.
 * @param {Object} css - Css style.
 * @param {string} id - Id label.
 * @param {Vm} vm - Vm object.
 * @return {*} Select style.
 */
function selectIdStyle(css: object, id: string, vm: Vm): any {
  const key = '#' + id;
  return selectStyle(css, key, vm);
}

/**
 * Replace style.
 * @param {*} oStyle - Current style.
 * @param {*} rStyle - New style.
 */
function replaceStyle(oStyle: any, rStyle: any): void {
  if (!rStyle || rStyle.length <= 0) {
    return;
  }
  Object.keys(rStyle).forEach(function(key) {
    oStyle[key] = rStyle[key];
  });
}

/**
 * Select style for class label, id label.
 * @param {Object} css - Css style.
 * @param {string} key - Key index.
 * @param {Vm} vm - Vm object.
 * @return {*}
 */
function selectStyle(css: object, key: string, vm: Vm): any {
  const style = css[key];
  if (!vm) {
    return style;
  }
  const mediaStatus = vm.mediaStatus;
  if (!mediaStatus) {
    return style;
  }
  const mqArr = css['@MEDIA'];
  if (!mqArr) {
    vm.init = true;
    return style;
  }
  const classStyle = {};
  if (style) {
    Object.keys(style).forEach(function(key) {
      classStyle[key] = style[key];
    });
  }
  for (let i$1 = 0; i$1 < mqArr.length; i$1++) {
    if (matchMediaQueryCondition(mqArr[i$1].condition, mediaStatus, false)) {
      replaceStyle(classStyle, mqArr[i$1][key]);
    }
  }
  return classStyle;
}

/**
 * Set class style after SelectClassStyle.
 * @param {Element} el - Element object.
 * @param {Object} css - Css style.
 * @param {string[]} classList - List of class label.
 */
function setClassStyle(el: Element, css: object, classList: string[], vm?: Vm): void {
  const SPACE_REG: RegExp = /\s+/;
  const newClassList: string[] = [];
  if (Array.isArray(classList)) {
    classList.forEach(v => {
      if (typeof v === 'string' && SPACE_REG.test(v)) {
        newClassList.push(...v.trim().split(SPACE_REG));
      } else {
        newClassList.push(v);
      }
    });
  }
  classList = newClassList;
  const classStyle = {};
  const length = classList.length;
  if (length === 1) {
    const style = selectClassStyle(css, classList, 0, vm);
    if (style) {
      Object.keys(style).forEach((key) => {
        classStyle[key] = style[key];
      });
    }
  } else {
    const rets = [];
    const keys = Object.keys(css || {});
    for (let i = 0; i < length; i++) {
      const clsKey = '.' + classList[i];
      const style = selectStyle(css, clsKey, vm);
      if (style) {
        const order = clsKey === '.@originalRootEl' ? -1000 : keys.indexOf(clsKey);
        rets.push({style: style, order: order});
      }
    }
    if (rets.length === 1) {
      const style = rets[0].style;
      if (style) {
        Object.keys(style).forEach((key) => {
          classStyle[key] = style[key];
        });
      }
    } else if (rets.length > 1) {
      rets.sort(function(a, b) {
        if (!a) {
          return -1;
        } else if (!b) {
          return 1;
        } else {
          return a.order > b.order ? 1 : -1;
        }
      });
      const retStyle = {};
      rets.forEach(function(key) {
        if (key && key.style) {
          Object.assign(retStyle, key.style);
        }
      });
      Object.keys(retStyle).forEach((key) => {
        classStyle[key] = retStyle[key];
      });
    }
  }

  const keyframes = css['@KEYFRAMES'];
  if (keyframes) {
    /*
     * Assign @KEYFRAMES's value.
     */
    const animationName = classStyle['animationName'];
    if (animationName) {
      classStyle['animationName'] = keyframes[animationName];
      classStyle['animationName'].push({'animationName': animationName});
    }
    const transitionEnter = classStyle['transitionEnter'];
    if (transitionEnter) {
      classStyle['transitionEnter'] = keyframes[transitionEnter];
    }
    const transitionExit = classStyle['transitionExit'];
    if (transitionExit) {
      classStyle['transitionExit'] = keyframes[transitionExit];
    }
    const sharedTransitionName = classStyle['sharedTransitionName'];
    if (sharedTransitionName) {
      classStyle['sharedTransitionName'] = keyframes[sharedTransitionName];
    }
  }
  const fontFace = classStyle['fontFamily'];
  if (fontFace) {
    const fontCompileList = _getFontFamily(css, fontFace);
    classStyle['fontFamily'] = fontCompileList;
  }
  el.setClassStyle(classStyle);
  el.classList = classList;
}

/**
 * Bind classnames to an element
 * @param {Vm} vm - Vm object.
 * @param {Element} el - Element object.
 * @param {Function | string[]} classList - List of class label.
 */
export function setClass(vm: Vm, el: Element, classList: Function | string[]): void {
  if (typeof classList !== 'function' && !Array.isArray(classList)) {
    return;
  }
  if (Array.isArray(classList) && !classList.length) {
    el.setClassStyle({});
    return;
  }
  const style = vm.css || {};
  if (typeof classList === 'function') {
    const value = watch(vm, classList, v => {
      setClassStyle(el, style, v, vm);
    });
    setClassStyle(el, style, value, vm);
  } else {
    setClassStyle(el, style, classList, vm);
  }
}

/**
 * Support css selector by id and component.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {Function | string} id - Id label.
 */
export function setIdStyle(vm: Vm, el: Element, id: Function | string): void {
  if (id) {
    const css = vm.css || {};
    if (typeof id === 'function') {
      const value = watch(vm, id, v => {
        doSetStyle(vm, el, selectIdStyle(css, v, vm), css, 'idStyle');
      });
      doSetStyle(vm, el, selectIdStyle(css, value, vm), css, 'idStyle');
    } else if (typeof id === 'string') {
      doSetStyle(vm, el, selectIdStyle(css, id, vm), css, 'idStyle');
    }
  }
}

/**
 * Set style.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {*} style - Style to be Set.
 * @param {*} css - Css style.
 * @param {string} name - Bind by name.
 */
function doSetStyle(vm: Vm, el: Element, style: any, css: any, name: string): void {
  if (!style) {
    return;
  }
  const typeStyle = {};
  Object.assign(typeStyle, style);
  setAnimation(typeStyle, css);
  setFontFace(typeStyle, css);
  bindDir(vm, el, name, typeStyle);
}

/**
 * Set FontFace.
 * @param {*} style - Style.
 * @param {*} css - Css style.
 */
function setFontFace(style: any, css: any): void {
  const fontFace = style['fontFamily'];
  if (fontFace) {
    const fontCompileList = _getFontFamily(css, fontFace);
    style['fontFamily'] = fontCompileList;
  }
}

/**
 * Set Animation
 * @param {*} style - Style.
 * @param {*} css - Css style.
 */
function setAnimation(style: any, css: any): void {
  const animationName = style['animationName'];
  const keyframes = css['@KEYFRAMES'];
  if (animationName && keyframes) {
    style['animationName'] = keyframes[animationName];
    if (style['animationName']) {
      style['animationName'].push({'animationName': animationName});
    }
  }
}

/**
 * Set tag style.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {string} tag - Tag.
 */
export function setTagStyle(vm: Vm, el: Element, tag: string): void {
  const css = vm.css || {};
  if (tag && typeof tag === 'string') {
    doSetStyle(vm, el, selectStyle(css, tag, vm), css, 'tagStyle');
  }
}

/**
 * Bind style to an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {*} style - Style.
 */
function setStyle(vm: Vm, el: Element, style: any): void {
  bindDir(vm, el, 'style', style);
}

/**
 * Add an event type and handler to an element and generate a dom update.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {string} type - Type added to event.
 * @param {Function} handler - Handle added to event.
 */
function setEvent(vm: Vm, el: Element, type: string, handler: Function): void {
  el.addEvent(type, handler.bind(vm));
}

/**
 * Add all events of an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {Object} events - Events of an element.
 */
function bindEvents(vm: Vm, el: Element, events: object, eventType?: string): void {
  if (!events) {
    return;
  }
  const keys = Object.keys(events);
  let i = keys.length;
  while (i--) {
    const key = keys[i];
    let handler = events[key];
    if (typeof handler === 'string') {
      handler = vm[handler];
      if (!handler || typeof handler !== 'function') {
        Log.warn(`The event handler '${events[key]}' is undefined or is not function.`);
        continue;
      }
    }
    const eventName: string = eventType ? eventType + key : key;
    setEvent(vm, el, eventName, handler);
  }
}

/**
 * <p>Set a series of members as a kind of an element.</p>
 * <p>for example: style, attr, ...</p>
 * <p>if the value is a function then bind the data changes.</p>
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {string} name - Method name.
 * @param {Object} data - Data that needed.
 */
function bindDir(vm: Vm, el: Element, name: string, data: object): void {
  if (!data) {
    return;
  }
  const keys = Object.keys(data);
  let i = keys.length;
  if (!i) {
    return;
  }
  const methodName = SETTERS[name];
  const method = el[methodName];
  const isSetStyle = methodName === 'setStyle';
  while (i--) {
    const key = keys[i];
    const value = data[key];
    if (key === 'ref') {
      vm.$refs[value] = el;
    }
    const isSetFont = isSetStyle && key === 'fontFamily';
    const setValue = function(value) {
      if (isSetFont) {
        value = filterFontFamily(vm, value);
      }
      method.call(el, key, value);
    };
    if (typeof value === 'function') {
      bindKey(vm, el, setValue, value);
    } else {
      setValue(value);
    }
  }
}

/**
 * Bind data changes to a certain key to a name series in an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - ELement component.
 * @param {Function} setValue - Set value.
 * @param {Function} calc - Watch the calc and return a value by calc.call().
 */
function bindKey(vm: Vm, el: Element, setValue: Function, calc: Function): void {
  // Watch the calc, and returns a value by calc.call().
  const watcher = newWatch(vm, calc, (value) => {
    function handler() {
      setValue(value);
    }
    const differ = vm && vm.app && vm.app.differ;
    if (differ) {
      differ.append('element', el.ref, handler);
    } else {
      handler();
    }
  });
  el.watchers.push(watcher);
  setValue(watcher.value);
}

/**
 * FontFamily Filter.
 * @param {Vm} vm - Vm object.
 * @param {string} fontFamilyName - FontFamily name.
 * @return {*} FontFamily Filter.
 */
export function filterFontFamily(vm: Vm, fontFamilyName: string): any[] {
  const css = vm.css || {};
  return _getFontFamily(css, fontFamilyName);
}

/**
 * Watch the calc.
 * @param {Vm} vm - Vm object.
 * @param {Function} calc - Watch the calc, and returns a value by calc.call().
 * @param {Function} callback - Callback callback Function.
 * @return {Watcher} New watcher for rhe calc value.
 */
export function newWatch(vm: Vm, calc: Function, callback: Function): Watcher {
  const watcher = new Watcher(vm, calc, function(value, oldValue) {
    if (typeof value !== 'object' && value === oldValue) {
      return;
    }
    callback(value);
  }, null);
  return watcher;
}

/**
 * Watch a calc function and callback if the calc value changes.
 * @param {Vm} vm - Vm object.
 * @param {Function} calc - Watch the calc, and returns a value by calc.call().
 * @param {Function} callback - Callback callback Function.
 * @return {*} Watcher value.
 */
export function watch(vm: Vm, calc: Function, callback: Function): any {
  const watcher = new Watcher(vm, calc, function(value, oldValue) {
    if (typeof value !== 'object' && value === oldValue) {
      return;
    }
    callback(value);
  }, null);
  return watcher.value;
}

/**
 * Apply style to an element.
 * @param {Vm} vm - Vm object.
 * @param {Element} el - Element object.
 */
function applyStyle(vm: Vm, el: Element): void {
  const css = vm.css || {};
  const allStyle = el.style;
  setAnimation(allStyle, css);
}

/**
 * Check if it is an Array.
 * @param {*} params - Any value.
 * @return {boolean} Return true if it is an array. Otherwise return false.
 */
function isArray(params: any): params is Array<string> {
  return Array.isArray(params);
}
