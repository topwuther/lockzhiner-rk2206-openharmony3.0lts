/* eslint-disable */
import Dep, { pushTarget, popTarget } from './dep';
import {
  remove,
  extend,
  isObject
} from '../../utils/index.ts';

let uid = 0;

/**
 * A watcher parses an expression, collects dependencies,
 * and fires callback when the expression value changes.
 * This is used for both the $watch() api and directives.
 * @param {Vue} vm
 * @param {String|Function} expOrFn
 * @param {Function} cb
 * @param {Object} options
 *                 - {Array} filters
 *                 - {Boolean} twoWay
 *                 - {Boolean} deep
 *                 - {Boolean} user
 *                 - {Boolean} sync
 *                 - {Boolean} lazy
 *                 - {Function} [preProcess]
 *                 - {Function} [postProcess]
 * @constructor
 */

export default function Watcher (vm, expOrFn, cb, options) {
  // Mix in options.
  if (options) {
    extend(this, options);
  }
  const isFn = typeof expOrFn === 'function';
  this.vm = vm;
  vm._watchers.push(this);
  this.expression = expOrFn;
  this.cb = cb;
  this.id = ++uid;
  this.active = true;
  this.dirty = this.lazy;
  this.deps = [];
  this.newDeps = [];
  this.depIds = new Set();
  this.newDepIds = new Set();

  if (isFn) {
    this.getter = expOrFn;
  }
  this.value = this.lazy
    ? undefined
    : this.get();

  // State for avoiding false triggers for deep and Array watchers during vm._digest().
  this.queued = this.shallow = false;
}

/**
 * Evaluate the getter, and re-collect dependencies.
 */
Watcher.prototype.get = function () {
  pushTarget(this);
  const value = this.getter.call(this.vm, this.vm);

  // "touch" every property so they are all tracked as dependencies for deep watching.
  if (this.deep) {
    traverse(value);
  }
  popTarget();
  this.cleanupDeps();
  return value;
}

/**
 * Add a dependency to this directive.
 * @param {Dep} dep
 */
Watcher.prototype.addDep = function (dep) {
  const id = dep.id;
  if (!this.newDepIds.has(id)) {
    this.newDepIds.add(id);
    this.newDeps.push(dep);
    if (!this.depIds.has(id)) {
      dep.addSub(this);
    }
  }
}

/**
 * Clean up for dependency collection.
 */
Watcher.prototype.cleanupDeps = function () {
  let i = this.deps.length;
  while (i--) {
    const dep = this.deps[i];
    if (!this.newDepIds.has(dep.id)) {
      dep.removeSub(this);
    }
  }
  let tmp = this.depIds
  this.depIds = this.newDepIds
  this.newDepIds = tmp
  this.newDepIds.clear()
  tmp = this.deps
  this.deps = this.newDeps
  this.newDeps = tmp
  this.newDeps.length = 0
}

/**
 * Subscriber interface. Will be called when a dependency changes.
 * @param {Boolean} shallow
 */
Watcher.prototype.update = function (shallow) {
  if (this.lazy) {
    this.dirty = true;
  } else {
    this.run();
  }
}

/**
 * Batcher job interface. Will be called by the batcher.
 */
Watcher.prototype.run = function () {
  if (this.active) {
    const value = this.get();
    if (
      value !== this.value ||
      ((isObject(value) || this.deep) && !this.shallow)
    ) {
      // Set new value.
      const oldValue = this.value;
      this.value = value;
      this.cb.call(this.vm, value, oldValue);
    }
    this.queued = this.shallow = false;
  }
}

/**
 * Evaluate the value of the watcher. This only gets called for lazy watchers.
 */
Watcher.prototype.evaluate = function () {
  this.value = this.get();
  this.dirty = false;
}

/**
 * Depend on all deps collected by this watcher.
 */
Watcher.prototype.depend = function () {
  let i = this.deps.length;
  while (i--) {
    this.deps[i].depend();
  }
}

/**
 * Remove self from all dependencies' subcriber list.
 */
Watcher.prototype.teardown = function () {
  if (this.active) {
    /* Remove self from vm's watcher list.
     * This is a somewhat expensive operation so we skip it
     * if the vm is being destroyed or is performing a v-for
     * re-render (the watcher list is then filtered by v-for).
     */
    if (!this.vm._isBeingDestroyed && !this.vm._vForRemoving) {
      remove(this.vm._watchers, this);
    }
    let i = this.deps.length;
    while (i--) {
      this.deps[i].removeSub(this);
    }
    this.active = false;
    this.vm = this.cb = this.value = null;
  }
}

/**
 * <p>Recursively traverse an object to evoke all converted<br>
 * getters, so that every nested property inside the object<br>
 * is collected as a "deep" dependency.</p>
 * @param {*} val
 * @param {Set} seen
 */
const seenObjects = new Set();

function traverse (val, seen) {
  let i, keys, isA, isO;
  if (!seen) {
    seen = seenObjects;
    seen.clear();
  }
  isA = Array.isArray(val);
  isO = isObject(val);
  if (isA || isO) {
    if (val.__ob__) {
      const depId = val.__ob__.dep.id;
      if (seen.has(depId)) {
        return;
      } else {
        seen.add(depId);
      }
    }
    if (isA) {
      i = val.length;
      while (i--) traverse(val[i], seen);
    } else if (isO) {
      keys = Object.keys(val);
      i = keys.length;
      while (i--) traverse(val[keys[i]], seen);
    }
  }
}
