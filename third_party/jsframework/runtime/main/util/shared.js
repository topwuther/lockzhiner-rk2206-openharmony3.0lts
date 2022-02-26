/**
 * Add properties to object.
 * @param {Object} obj - The object which need be added properties.
 * @param {*[]} prop - The properties which to be added.
 * @return {Object} The object which has been added properties.
 */
export function extend(obj, ...prop) {
  if (typeof Object.assign === 'function') {
    Object.assign(obj, ...prop);
  } else {
    const first = prop.shift();
    for (const key in first) {
      obj[key] = first[key];
    }
    if (prop.length) {
      extend(obj, ...prop);
    }
  }
  return obj;
}

/**
 * Add or modify a property.
 * @param {Object} obj - The object which need be added or modified a property.
 * @param {string} prop - The key of the property.
 * @param {*} val - The value of the the property.
 * @param {boolean} [enumerable] - If the property is enumerable.
 */
export function def(obj, prop, val, enumerable) {
  Object.defineProperty(obj, prop, {
    value: val,
    enumerable: !!enumerable,
    writable: true,
    configurable: true
  });
}


/**
 * Remove an item from an array.
 * @param {*[]} arr - The array from which removes an item.
 * @param {*} item - The item which to be removed.
 * @return {*} The item which has been removed.
 */
export function remove(arr, item) {
  if (arr.length) {
    const index = arr.indexOf(item);
    if (index >= 0) {
      return arr.splice(index, 1);
    }
  }
}

/**
 * Verify whether the property exists or not in object.
 * @param {Object} obj - The object which should be verified.
 * @param {string} prop - The property which should be verified.
 * @return {boolean} The result whether the property exists or not.
 */
export function hasOwn(obj, prop) {
  return Object.prototype.hasOwnProperty.call(obj, prop);
}

/**
 * Verify whether a variable is object.
 * @param {*} any - The variable which should be verified.
 * @return {boolean} The result whether a variable is an object.
 */
export function isObject(any) {
  return any !== null && typeof any === 'object';
}

/**
 * Verify whether a variable is an plain JavaScript object.
 * @param {*} any - The variable which should be verified.
 * @return {boolean} The result whether a variable is an plain JavaScript object.
 */
export function isPlainObject(any) {
  return Object.prototype.toString.call(any) === '[object Object]';
}
