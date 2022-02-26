import Vm from '../model';
import { hasOwn } from '../../utils/index';

/**
 * Get a prop's type.
 * @param {Function} fn - Prop.type.
 * @return {string} - Prop's type.
 */
function getPropType(fn) {
  const match = fn && fn.toString().match(/^\s*function (\w+)/);
  return match ? match[1] : '';
}

/**
 * Get default prop value.
 * @param {Vm} vm - Vm object.
 * @param {PropInterface} prop - Default prop.
 * @return {PropVauleInterfance | undefined} Default prop value or Null.
 */
function getDefaultPropValue(vm, prop) {
  if (!prop) {
    return undefined;
  }
  if (!hasOwn(prop, 'default')) {
    return undefined;
  }
  const __hasDefault = true;
  const def = prop.default;
  const __isDefaultValue =
    typeof def === 'function' && getPropType(prop.type) !== 'Function'
      ? def.call(vm)
      : def;
  return { __hasDefault, __isDefaultValue };
}

export {
  getDefaultPropValue
};
