/* eslint-disable */

import {
  Log,
  hasOwn
} from '../../utils/index.ts';
import * as _ from '../../utils/index.ts';

var objProto = Object.prototype;

/**
 * Add a new property to an observed object and emits corresponding event.
 * @param {String} key
 * @param {*} val
 * @public
 */
_.define(
  objProto,
  '$add',
  function $add (key, val) {
    Log.warn(`'Object.prototype.$add' is not a standard API,`
      + ` it will be removed in the next version.`);
    if (hasOwn(this, key)) {
      return;
    }
    var ob = this.__ob__;
    if (!ob || _.isReserved(key)) {
      this[key] = val;
      return;
    }
    ob.convert(key, val);
    ob.notify();
    if (ob.vms) {
      var i = ob.vms.length;
      while (i--) {
        var vm = ob.vms[i];
        vm._proxy(key);
        // vm._digest() // todo
      }
    }
  }
)

/**
 * Set a property on an observed object, calling add to ensure the property is observed.
 * @param {String} key
 * @param {*} val
 * @public
 */
_.define(
  objProto,
  '$set',
  function $set (key, val) {
    Log.warn(`"Object.prototype.$set" is not a standard API,`
      + ` it will be removed in the next version.`);
    this.$add(key, val);
    this[key] = val;
  }
)

/**
 * Deletes a property from an observed object and emits corresponding event.
 * @param {String} key
 * @public
 */
_.define(
  objProto,
  '$delete',
  function $delete (key) {
    Log.warn(`"Object.prototype.$delete" is not a standard API,`
      + ` it will be removed in the next version.`);
    if (!this.hasOwnProperty(key)) {
      return;
    }
    delete this[key];
    var ob = this.__ob__;
    if (!ob || _.isReserved(key)) {
      return;
    }
    ob.notify();
    if (ob.vms) {
      var i = ob.vms.length;
      while (i--) {
        var vm = ob.vms[i];
        vm._unproxy(key);
        // vm._digest() // todo
      }
    }
  }
)
