/* eslint-disable */

import { def, Log } from '../../utils/index.ts';

const arrayProto = Array.prototype;
export const arrayMethods = Object.create(arrayProto);

/*
 * Intercept mutating methods and emit events
 */

;[
  'push',
  'pop',
  'shift',
  'unshift',
  'splice',
  'sort',
  'reverse'
]
.forEach(function (method) {
  // cache original method
  const original = arrayProto[method];
  def(arrayMethods, method, function mutator() {
    // avoid leaking arguments:
    // http://jsperf.com/closure-with-arguments
    let i = arguments.length;
    const args = new Array(i);
    while (i--) {
      args[i] = arguments[i];
    }
    const result = original.apply(this, args);
    const ob = this.__ob__;
    let inserted;
    switch (method) {
      case 'push':
        inserted = args;
        break;
      case 'unshift':
        inserted = args;
        break;
      case 'splice':
        inserted = args.slice(2);
        break;
    }
    if (inserted) {
      ob.observeArray(inserted);
    }
    // Notify change.
    ob.dep.notify();
    return result;
  })
})

/**
 * Swap the element at the given index with a new value and emits corresponding event.
 * @param {Number} index
 * @param {*} val
 * @return {*} - replaced element
 */

def(
  arrayProto,
  '$set',
  function $set (index, val) {
    Log.warn(`"Array.prototype.$set" is not a standard API, `
      + `it will be removed in the next version.`);
    if (index >= this.length) {
      this.length = index + 1;
    }
    return this.splice(index, 1, val)[0];
  }
)

/**
 * Convenience method to remove the element at given index.
 * @param {Number} index
 * @param {*} val
 */

def(
  arrayProto,
  '$remove',
  function $remove (index) {
    Log.warn(`"Array.prototype.$remove" is not a standard API,`
      + ` it will be removed in the next version.`);
    if (!this.length) {
      return;
    }
    if (typeof index !== 'number') {
      index = this.indexOf(index);
    }
    if (index > -1) {
      this.splice(index, 1);
    }
  }
)

/**
 * Support includes for panda.
 * @param {Number} index
 * @param {*} val
 * @return {Boolean}
 */

def(
  arrayProto,
  'includes',
  function includes(param, start = 0) {
    if (start >= this.length) return false
    if (start < 0) {
      start = start + this.length < 0 ? 0 : start + this.length;
    }
    if (Number.isNaN(param)) {
      for (let i = start; i < this.length; i++) {
        if (Number.isNaN(this[i])) return true;
      }
    } else {
      for (let i = start; i < this.length; i++) {
        if (this[i] === param) return true;
      }
    }
    return false
  }
)
