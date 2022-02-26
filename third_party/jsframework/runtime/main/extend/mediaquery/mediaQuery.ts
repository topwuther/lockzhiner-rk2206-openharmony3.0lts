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

import { Log } from '../../../utils/index';

const MEDIA_QUERY_RULE = {
  CONDITION_WITH_SCREEN: /^(((only|not)screen)|screen)((and|or|,)\([\w\/\.:><=-]+\))*$/,
  CONDITION_WITHOUT_SCREEN: /^\([\w\/\.:><=-]+\)((and|or|,)\([\w\/\.:><=-]+\))*$/,
  CONDITION_WITH_AND: /^\([\/\.a-z0-9:>=<-]+\)(and\([\/\.a-z0-9:>=<-]+\))+/,
  CSS_LEVEL4_MULTI: /^\(([\d\.]+(dpi|dppx|dpcm|px)?)(>|<|>=|<=)[a-z0-9:-]+(>|<|>=|<=)([\d\.]+(dpi|dppx|dpcm|px)?)\)$/,
  CSS_LEVEL4_LEFT: /^\([^m][a-z-]+(>|<|>=|<=)[\d\.]+(dpi|dppx|dpcm|px)?\)$/,
  CSS_LEVEL4_RIGHT: /^\([\d\.]+(dpi|dppx|dpcm|px)?(>|<|>=|<=)[^m][a-z-]+\)$/,
  CSS_LEVEL3_RULE: /^\((min|max)-[a-z-]+:[\d\.]+(dpi|dppx|dpcm|px)?\)$/,
  ORIENTATION_RULE: /^\(orientation:[a-z]+\)/,
  DEVICETYPE_RULE: /^\(device-type:[a-z]+\)/,
  SCREEN_SHAPE_RULE: /^\(round-screen:[a-z]+\)/,
  DARK_MODE: /^\(dark-mode:[a-z]+\)/,
  ASPECT_RATIO: /^\((min|max)?-?(device)?-?aspect-ratio:[\d(\/)?(\d)*]+\)/,
  PATTERN: /^\(pattern:[a-z]+\)/
};

/**
 * Enum for MEDIA ERROR.
 * @enum {string}
 * @readonly
 */
/* eslint-disable no-unused-vars */
enum MEDIAERROR {
  /**
   * SYNTAX Type
   */
  SYNTAX = 'SYNTAX',
  /**
   * NONE Type
   */
  NONE = 'NONE',
}
/* eslint-enable no-unused-vars */

interface MediaMatchInfo {
  status: object;
  result: boolean;
}

const queryHistoryList: Map<string, MediaMatchInfo> = new Map();

/**
 * Match media query condition.
 * @param {string} condition - Media query condition.
 * @param {Object} mediaStatus - The device information.
 * @param {boolean} jsQuery
 * @return {boolean}
 */
export function matchMediaQueryCondition(condition: string, mediaStatus: object, jsQuery: boolean): boolean {
  if (!condition || !mediaStatus) {
    return false;
  }

  // If width and height are not initialized, and the query condition includes 'width' or 'height',
  // return false directly.
  if (mediaStatus['width'] === 0 && (condition.includes('width') || condition.includes('height'))) {
    return false;
  }
  if (jsQuery && queryHistoryList.has(condition)) {
    const queryHistory: MediaMatchInfo = queryHistoryList.get(condition);
    if (queryHistory && JSON.stringify(queryHistory.status) === JSON.stringify(mediaStatus)) {
      return queryHistory.result;
    }
  }
  const result: boolean = doMatchMediaQueryCondition(condition, mediaStatus);
  queryHistoryList.set(condition, {status: mediaStatus, result: result});
  return result;
}

interface FailReason {
  type: MEDIAERROR;
}

/**
 * Match media query condition.
 * @param {string} condition - Media query condition.
 * @param {Object} mediaStatus - The device information.
 * @return {boolean}
 */
function doMatchMediaQueryCondition(condition: string, mediaStatus: object): boolean {
  const noSpace: string = condition.replace(/\s*/g, '');
  let inverse: boolean = false;
  const failReason: FailReason = { type: MEDIAERROR.NONE };
  let noScreen: string;

  // Check if the media query condition is legal.
  if (MEDIA_QUERY_RULE.CONDITION_WITH_SCREEN.exec(noSpace)) {
    if (noSpace.indexOf('notscreen') !== -1) {
      inverse = true;
    }
    const screenPatt: RegExp = /screen[^and:]/g;
    if (screenPatt.exec(noSpace)) {
      return !inverse;
    }
    noScreen = noSpace.replace(/^(only|not)?screen(and)?/g, '');
    if (!noScreen) {
      return !inverse;
    }
  } else if (MEDIA_QUERY_RULE.CONDITION_WITHOUT_SCREEN.exec(noSpace)) {
    noScreen = noSpace;
  } else {
    Log.debug('Illegal condition.');
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }

  // Replace 'or' with comma ','.
  const commaCondition: string = noScreen.replace(/or[(]/g, ',(');

  // Remove screen and modifier.
  const conditionArr: string[] = commaCondition.split(',');
  const len: number = conditionArr.length;
  for (let i = 0; i < len; i++) {
    if (MEDIA_QUERY_RULE.CONDITION_WITH_AND.exec(conditionArr[i])) {
      const result: boolean = parseAndCondtion(conditionArr[i], mediaStatus, failReason);
      if (failReason.type === MEDIAERROR.SYNTAX) {
        return false;
      }
      if (i + 1 === len) {
        return inverse && !result || !inverse && result;
      }
    } else {
      if (parseSingleCondition(conditionArr[i], mediaStatus, failReason)) {
        return !inverse;
      }
      if (failReason.type === MEDIAERROR.SYNTAX) {
        return false;
      }
    }
  }
  return inverse;
}

/**
 * Parse single condition, such as: (100 < width).
 * @param {string} condition - Single condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseSingleCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  if (MEDIA_QUERY_RULE.CSS_LEVEL4_MULTI.exec(condition)) {
    if (parseCss4MultiCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.CSS_LEVEL4_LEFT.exec(condition)) {
    if (parseCss4LeftCondtion(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.CSS_LEVEL4_RIGHT.exec(condition)) {
    if (parseCss4RightCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.CSS_LEVEL3_RULE.exec(condition)) {
    if (parseCss3Condition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.DEVICETYPE_RULE.exec(condition)) {
    if (parseDeviceTypeCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.ORIENTATION_RULE.exec(condition)) {
    if (parseOrientationCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.SCREEN_SHAPE_RULE.exec(condition)) {
    if (parseScreenShapeCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.DARK_MODE.exec(condition)) {
    if (parseDarkModeCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.ASPECT_RATIO.exec(condition)) {
    if (parseAspectRatioCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else if (MEDIA_QUERY_RULE.PATTERN.exec(condition)) {
    if (parsePatternCondition(condition, mediaStatus, failReason)) {
      return true;
    }
  } else {
    Log.debug('Illegal condition');
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return false;
}

/**
 * Parse conditions connect with 'and', such as: (100 < width) and (width < 1000).
 * @param {string} condition - Conditions connect with 'and'.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseAndCondtion(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  // Split and condition to simple conditions.
  const noAnd: string = condition.replace(/and[^a-z]/g, ',(');
  const conditionArr: string[] = noAnd.split(',');
  if (!conditionArr) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  for (let i = 0; i < conditionArr.length; i++) {
    if (!parseSingleCondition(conditionArr[i], mediaStatus, failReason)) {
      return false;
    }
  }
  return true;
}

/**
 * Parse css4 multi-style condition, such as: (100 < width < 1000).
 * @param {string} condition - Css4 multi-style condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseCss4MultiCondition(condition:string, mediaStatus: object, failReason: FailReason): boolean {
  const patt: RegExp = /([a-z-]+|[\d.a-z]+|[><=]+)/g;
  const feature = condition.match(patt);
  if (!feature || feature.length !== 5) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  const rcondition: string = '(' + feature[0] + feature[1] + feature[2] + ')';
  const lcondition: string = '(' + feature[2] + feature[3] + feature[4] + ')';

  return parseCss4RightCondition(rcondition, mediaStatus, failReason) &&
        parseCss4LeftCondtion(lcondition, mediaStatus, failReason);
}

/**
 * Parse css4 style condition, device info is in the left, such as: (width < 1000).
 * @param {string} condition - Css4 style condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseCss4LeftCondtion(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const feature = condition.match(/[a-z-]+|[0-9.]+/g);
  if (!feature || feature.length < 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  const conditionValue: string = feature[1];
  const unit: string = feature.length === 3 ? feature[2] : '';
  const relationship = condition.match(/[><=]+/g);
  const statusValue: number = transferValue(mediaStatus[feature[0]], unit);
  return calculateExpression(statusValue, relationship[0], conditionValue, failReason);
}

/**
 * Parse css4 style condition, device info is in the right, such as: (1000 < width).
 * @param {string} condition - Css4 style condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseCss4RightCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const feature = condition.match(/[a-z-]+|[0-9.]+/g);
  if (!feature || feature.length < 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  const conditionValue: string = feature[0];
  let statusValue: number;
  let unit: string;
  if (feature.length === 3) {
    unit = feature[1];
    statusValue = transferValue(mediaStatus[feature[2]], unit);
  } else {
    unit = '';
    statusValue = transferValue(mediaStatus[feature[1]], unit);
  }
  const relationship = condition.match(/[><=]+/g);
  return calculateExpression(conditionValue, relationship[0], statusValue, failReason);
}

/**
 * Parse css3 style condition, such as: (min-width: 1000).
 * @param {String} condition - Css3 style condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseCss3Condition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const feature = condition.match(/[a-z-]+|[0-9.]+/g);
  if (!feature || feature.length < 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  const conditionValue: string = feature[1];
  const unit: string = feature.length === 3 ? feature[2] : '';
  let relationship: string;
  if (feature[0].match(/^(max-)/)) {
    relationship = '<=';
  } else if (feature[0].match(/^(min-)/)) {
    relationship = '>=';
  } else {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  const status: string = feature[0].replace(/(max|min)-/g, '');
  const statusValue: number = transferValue(mediaStatus[status], unit);
  return calculateExpression(statusValue, relationship, conditionValue, failReason);
}

/**
 * Parse paatern style condition, such as: (pattern: normal).
 * @param {String} condition - pattern style condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parsePatternCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const pattern = condition.match(/[a-z-]+/g);
  if (!pattern || pattern.length !== 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return getVpType(mediaStatus['resolution'], mediaStatus['width']) === pattern[1];
}

/**
 * get the type of vp.
 * @param {number} resolution - the resolution of device.
 * @param {number} width - the width of view page.
 */
function getVpType(resolution: number, width: number): string {
  const value = width / resolution;
  if (value > 0 && value < 320) {
    return 'small';
  } else if (value >= 320 && value < 600) {
    return 'normal';
  } else if (value >= 600 && value < 840) {
    return 'large';
  } else if (value >= 840) {
    return 'xLarge';
  } else {
    return '';
  }
}

/**
 * Parse screen orientation condition, such as: (orientation: portrait).
 * @param {string} condition - Orientation type condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseOrientationCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const orientaton = condition.match(/[a-z-]+/g);
  if (!orientaton || orientaton.length !== 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return orientaton[1] === mediaStatus['orientation'];
}

/**
 * Parse device type condition, such as: (device-type: tv).
 * @param {string} condition - Device type condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseDeviceTypeCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const deviceType = condition.match(/[a-z-]+/g);
  if (!deviceType || deviceType.length !== 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return deviceType[1] === mediaStatus['device-type'];
}

/**
 * Parse screen shape condition, such as: (round-screen: true).
 * @param {string} condition - Screen shape condition.
 * @param {Object} mediaStatus - Device info.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function parseScreenShapeCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const shape = condition.match(/[a-z-]+/g);
  if (!shape || shape.length !== 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return shape[1] === mediaStatus['round-screen'].toString();
}

/**
 * parse dark mode condition, such as: (dark-mode: true)
 * @param {String} condition: dark condition
 * @param {Object} mediaStatus: device info
 * @param {Object} failReason: parse fail reason
 */
function parseDarkModeCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  const darkMode = condition.match(/[a-z-]+/g);
  if (!darkMode || darkMode.length !== 2) {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return darkMode[1] === mediaStatus['dark-mode'].toString();
}

/**
 * parse aspect ratio condition, such as: (aspect-ratio: 8/3)
 * @param {String} condition: (device)?-aspect-ratio condition
 * @param {Object} mediaStatus: aspect-ratio, device-width, device-height
 * @param {Object} failReason: parse fail reason
 */
function parseAspectRatioCondition(condition: string, mediaStatus: object, failReason: FailReason): boolean {
  let conditionValue;
  const aspectRatio = condition.match(/[a-z-\d-\/]+/g);
  let relationship;
  if (aspectRatio[0].match(/^(max-)/)) {
    relationship = '<=';
  } else if (aspectRatio[0].match(/^(min-)/)) {
    relationship = '>=';
  } else {
    relationship = '==';
  }
  let statusValue;
  if (aspectRatio[0].match(/device/)) {
    Log.info('query device status');
    statusValue = mediaStatus['device-width'] / mediaStatus['device-height'];
  } else {
    Log.info('query page status');
    statusValue = mediaStatus['aspect-ratio'];
  }
  const numbers = aspectRatio[1].split('/');
  if (numbers.length === 2) {
    conditionValue = parseInt(numbers[0]) / parseInt(numbers[1]);
  } else {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  return calculateExpression(statusValue, relationship, conditionValue, failReason);
}

/**
 * Transfer unit the same with condition value unit.
 * @param {number} value - Device value should be transfer unit the same with condition value.
 * @param {string} unit - Condition value unit, such as: dpi/dpcm/dppx.
 * @return {number}
 */
function transferValue(value: number, unit: string): number {
  let transfer: number;
  switch (unit) {
    case 'dpi':
      transfer = 96;
      break;
    case 'dpcm':
      transfer = 36;
      break;
    default:
      transfer = 1;
  }
  return value * transfer;
}

/**
 * Calculate expression result.
 * @param {number|string} leftValue - Number device value. String condition value.
 * @param {string} relationship - >=/>/<=/<
 * @param {number|string} rightValue - Number device value. String condition value.
 * @param {FailReason} failReason - Parse fail reason.
 * @return {boolean}
 */
function calculateExpression(leftValue: number | string, relationship: string,
  rightValue: number | string, failReason: FailReason): boolean {
  let lvalue: number | string;
  let rvalue: number | string;
  if (typeof leftValue === 'string') {
    lvalue = leftValue.match(/[\d]+\.[\d]+/) ? parseFloat(leftValue) : parseInt(leftValue);
    rvalue = rightValue;
  } else if (typeof rightValue === 'string') {
    lvalue = leftValue;
    rvalue = rightValue.match(/[\d]+\.[\d]+/) ? parseFloat(rightValue) : parseInt(rightValue);
  } else if (typeof rightValue === 'number') {
    lvalue = leftValue;
    rvalue = rightValue;
  } else {
    failReason.type = MEDIAERROR.SYNTAX;
    return false;
  }
  switch (relationship) {
    case '>=':
      return lvalue >= rvalue;
    case '>':
      return lvalue > rvalue;
    case '<=':
      return lvalue <= rvalue;
    case '<':
      return lvalue < rvalue;
    case '==':
      return lvalue === rvalue;
    default:
      failReason.type = MEDIAERROR.SYNTAX;
  }
  return false;
}
