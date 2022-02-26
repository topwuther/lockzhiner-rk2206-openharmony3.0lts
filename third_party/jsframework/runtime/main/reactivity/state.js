/* eslint-disable */
import Watcher from './watcher';
import Dep from './dep';
import {
  observe,
  proxy
} from './observer';
import {
  isPlainObject,
  hasOwn
} from '../../utils/index.ts';

export function initState (vm) {
  vm._watchers = [];
  initData(vm);
  initComputed(vm);
  initMethods(vm);
}

export function initData (vm) {
  let data = vm._data;
  initDataSegment(vm, data);
  let shareData = vm._shareData;
  initDataSegment(vm, shareData);
}

export function initDataSegment (vm, data) {
  if (!isPlainObject(data)) {
    data = {};
  }

  // Proxy data on page.
  const keys = Object.keys(data);
  let i = keys.length;
  while (i--) {
    proxy(vm, keys[i], data);
  }
  // Observe data.
  observe(data, vm);
}

export function initBases(vm) {
  const options = vm.vmOptions
  // mixins exist?
  if(hasOwn(options, 'mixins')) {
    options['mixins'].forEach(mixin => {
      if(typeof mixin == 'object') {
        Object.keys(mixin).forEach(key => {
          vm[key] = mixin[key]
        })
      }
      else if (typeof mixin == 'function') {
        vm[mixin.name] = mixin.bind(vm)
      }
      else {
        aceConsole.error("[JS Framework] mixin must be plain object or function")
      }
    })
  }
}

function noop () {
}

export function initComputed (vm) {
  const computed = vm._computed;
  if (computed) {
    for (let key in computed) {
      const userDef = computed[key];
      const def = {
        enumerable: true,
        configurable: true
      };
      if (typeof userDef === 'function') {
        def.get = makeComputedGetter(userDef, vm);
        def.set = noop;
      } else {
        def.get = userDef.get
          ? userDef.cache !== false
            ? makeComputedGetter(userDef.get, vm)
            : userDef.get.bind(vm)
          : noop;
        def.set = userDef.set
          ? userDef.set.bind(vm)
          : noop;
      }
      Object.defineProperty(vm, key, def);
    }
  }
}

function makeComputedGetter (getter, owner) {
  const watcher = new Watcher(owner, getter, null, {
    lazy: true
  });
  return function computedGetter () {
    if (watcher.dirty) {
      watcher.evaluate();
    }
    if (Dep.target) {
      watcher.depend();
    }
    return watcher.value;
  }
}

export function initMethods (vm) {
  const options = vm._vmOptions;
  for (let key in options) {
    if (typeof options[key] === 'function' && key !== 'data') {
      vm._methods[key] = options[key].bind(vm);
      proxyMethods(vm, key);
    }
  }
}

function proxyMethods(vm, key) {
  Object.defineProperty(vm, key, {
    configurable: true,
    enumerable: true,
    get: function proxyGetter () {
      return vm._methods[key];
    },
    set: function proxySetter(newMethod) {
      vm._methods[key] = typeof newMethod === 'function' && key !== 'data' ?
        newMethod.bind(vm) : newMethod;
    }
  });
}
