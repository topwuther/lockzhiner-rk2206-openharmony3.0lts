/**
 * Verify whether a string starts with $ or _.
 * @param {string} str - The string which to be verified.
 * @return {boolean} The result whether the string starts with $ or _.
 */
export function isReserved(str) {
  const c = (str + '').charCodeAt(0);
  return c === 0x24 || c === 0x5F;
}

/**
 * Create a cached version of a function.
 * @param {Function} func - The function which to be created a cached version.
 * @return {Function} The cached version of the function.
 */
function cached(func) {
  const cache = Object.create(null);
  return function cachedFn(str) {
    const hit = cache[str];
    return hit || (cache[str] = func(str));
  };
}

/**
 * Camelize a hyphen-delmited string.
 * @param {string} str - A hyphen-delmited string.
 * @return {string} A camelized string.
 */
export const camelize = cached(str => {
  return str.replace(/-(\w)/g, (_, s) => s ? s.toUpperCase() : '');
});

/**
 * Check the type of a variable.
 * @param {*} any - The variable which to be checked.
 * @return {string} The type of the variable.
 */
export function typof(any) {
  const objType = Object.prototype.toString.call(any);
  return objType.substring(8, objType.length - 1).toLowerCase();
}

/**
 * Regular expression for component.
 * @constant {RegExp}
 */
const COMPONENT_REG = /^@app-component\//;

/**
 * Regular expression for module.
 * @constant {RegExp}
 */
const MODULE_REG = /^@app-module\//;

/**
 * Regular expression for application.
 * @constant {RegExp}
 */
const APPLICATION_REG = /^@app-application\//;

/**
 * Verify whether is a component.
 * @param {string} name - The name which need to be verified.
 * @return {boolean} The result whether is a component.
 */
export function isComponent(name) {
  return !!name.match(COMPONENT_REG);
}

/**
 * Verify whether is a module.
 * @param {string} name - The name which need to be verified.
 * @return {boolean} The result whether is a module.
 */
export function isModule(name) {
  return !!name.match(MODULE_REG);
}

/**
 * Verify whether is an application.
 * @param {string} name - The name which need to be verified.
 * @return {boolean} The result whether is an APP.
 */
export function isApplication(name) {
  return !!name.match(APPLICATION_REG);
}

/**
 * Remove "@app-application".
 * @param {string} name - The name which need to be removed.
 * @return {string} The result which has been removed "@app-application".
 */
export function removeApplicationPrefix(str) {
  const result = str.replace(APPLICATION_REG, '');
  return result;
}

/**
 * Remove "@app-component and @app-module".
 * @param {string} name - The name which need to be removed.
 * @return {string} The result which has been removed "@app-component" and "@app-module".
 */
export function removePrefix(str) {
  const result = str.replace(COMPONENT_REG, '').replace(MODULE_REG, '');
  return result;
}
