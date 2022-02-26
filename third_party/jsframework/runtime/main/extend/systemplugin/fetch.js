export function mockFetch() {
  global.sendGroupMessage = global.group.sendGroupMessage;
  let GroupMessenger = {
    create: function () {
      let messenger = {};
      messenger.send = function (groupName, functionName, ...args) {
        return new Promise(function (resolve, reject) {
          let params = messenger.prepareArgs(...args);
          sendGroupMessage(function (result) {
            resolve(messenger.parseJsonResult(result));
          }, function (error) {
            reject(messenger.parseJsonResult(error));
          }, groupName, functionName, ...params);
        })
      };
      messenger.parseJsonResult = function (data) {
        if (data && data.constructor == String) {
          try {
            data = JSON.parse(data);
          } catch (jsonParseErr) {
            console.warn("parse result exception: " + JSON.stringify(jsonParseErr));
          }
        }
        return data;
      };
      messenger.prepareArgs = function (...args) {
        let result = [...args];
        for (let i = 0; i < result.length; i++) {
          if (typeof result[i] === 'function') {
            result[i] = messenger.packageCallback(result[i]);
          }
        }
        return result;
      };
      messenger.packageCallback = function (func) {
        return function (data) {
          data = messenger.parseJsonResult(data);
          if (!Array.isArray(data)) {
            func(data);
          } else {
            func(...data);
          }
        };
      };
      return messenger;
    }
  };
  
  let CommonCallback = {
    commonCallback: function commonCallback(callback, flag, data, code) {
      if (typeof callback === 'function') {
        switch (flag) {
          case 'success':
            callback(data);
            break;
          case 'fail':
            callback(data, code);
            break;
          case 'cancel':
            callback(data);
            break;
          case 'complete':
            callback();
            break;
          default:
            break;
        }
      } else {
        console.warn('callback.' + flag + ' is not function or not present');
      }
    }
  };
  global.commonCallback = CommonCallback.commonCallback;
  let CommonCallbackEx = {
    commonCallbackEx: function commonCallbackEx(callback, result, pluginError) {
      if ((callback === undefined) || ((callback.success === undefined) && (callback.fail === undefined) && (callback.complete === undefined))) {
        return CommonCallbackEx.promiseMethod(result, pluginError);
      } else {
        return CommonCallbackEx.callbackMethod(callback, result, pluginError);
      }
    },
    promiseMethod: function promiseMethod(result, pluginError) {
      if (pluginError != undefined) {
        throw pluginError;
      }
      return result;
    },
    callbackMethod: function callbackMethod(callback, result, pluginError) {
      if (pluginError != undefined) {
        commonCallback(callback.fail, 'fail', pluginError.data, pluginError.code);
        commonCallback(callback.complete, 'complete');
        throw pluginError;
      }
      commonCallback(callback.success, 'success', result.data);
      commonCallback(callback.complete, 'complete');
      return result;
    },
    catching: function catching(promise, param) {
      return promise.then(ret => commonCallbackEx(param, ret))
        .catch(err => commonCallbackEx(param, null, err));
    }
  };
  global.commonCallbackEx = CommonCallbackEx.commonCallbackEx;
  global.systemplugin.catching = CommonCallbackEx.catching;
  
  let FetchObject = {
    getFetch: function () {
      let fetch = {}
      fetch.messenger = GroupMessenger.create();
      fetch.fetch = async function (param) {
        return await CommonCallbackEx.catching(this.messenger.send("groupName", "fetch", param), param);
      }
      return fetch
    }
  }
  
  global.systemplugin.fetch = FetchObject.getFetch();
}