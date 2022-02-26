import { paramMock } from "./utils"

export function mockAbilityFeatureAbility() {
  global.systemplugin.ability = {}
  const WantMock = {
    deviceId: "[PC Preview] unknow deviceId",
    bundleName: "[PC Preview] unknow bundleName",
    abilityName: "[PC Preview] unknow abilityName",
    uri: "[PC Preview] unknow uri",
    type: "[PC Preview] unknow type",
    flag: "[PC Preview] unknow flag",
    action: "[PC Preview] unknow action",
    parameters: "[PC Preview] unknow parameters",
    entities: "[PC Preview] unknow entities"
  }
  const AbilityResultMock = {
    resultCode: "[PC Preview] unknow resultCode",
    want: WantMock
  }
  const DataAbilityHelperMock = {
    openFile: function (...args) {
      console.warn("DataAbilityHelper.openFile interface mocked in the Previewer. How this interface works on the Previewer" +
          " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramNumberMock);
        })
      }
    },
    on: function (...args) {
      console.warn("DataAbilityHelper.on interface mocked in the Previewer. How this interface works on the Previewer" +
          " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      }
    },
    off: function (...args) {
      console.warn("DataAbilityHelper.off interface mocked in the Previewer. How this interface works on the Previewer" +
          " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      }
    },
    call: function (...args) {
      console.warn("DataAbilityHelper.call interface mocked in the Previewer. How this interface works on the Previewer" +
          " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramObjectMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramObjectMock);
        })
      }
    }
  }
  const ContextMock = {
    getCacheDir: function (...args) {
      console.warn("Context.getCacheDir interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock)
      } else {
        return new Promise((resolve) => {
          resolve("[PC Preview] unknow string")
        })
      }
    },
    getFilesDir: function (...args) {
      console.warn("Context.getFilesDir interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock)
      } else {
        return new Promise((resolve) => {
          resolve("[PC Preview] unknow string")
        })
      }
    },
    getExternalCacheDir: function (...args) {
      console.warn("Context.getExternalCacheDir interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock)
      } else {
        return new Promise((resolve) => {
          resolve("[PC Preview] unknow string")
        })
      }
    }
  }
  global.systemplugin.ability.featureAbility = {
    getWant: function (...args) {
      console.warn("ability.featureAbility.getWant interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, WantMock)
      } else {
        return new Promise((resolve) => {
          resolve(WantMock)
        })
      }
    },
    startAbility: function (...args) {
      console.warn("ability.featureAbility.startAbility interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve) => {
          resolve(paramMock.paramNumberMock)
        })
      }
    },
    startAbilityForResult: function (...args) {
      console.warn("ability.featureAbility.startAbilityForResult interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, AbilityResultMock)
      } else {
        return new Promise((resolve) => {
          resolve(AbilityResultMock)
        })
      }
    },
    terminateSelfWithResult: function (...args) {
      console.warn("ability.featureAbility.terminateSelfWithResult interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    terminateSelf: function (...args) {
      console.warn("ability.featureAbility.terminateSelf interface mocked in the Previewer. How this interface works on" +
          " the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    acquireDataAbilityHelper: function (...args) {
      console.warn("ability.featureAbility.acquireDataAbilityHelper mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
      return DataAbilityHelperMock;
    },
    getContext: function () {
      console.warn("ability.featureAbility.getContext interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      return ContextMock;
    }
  }
}
export function mockAbilityParticleAbility() {
  global.systemplugin.ability.particleAbility = {
    startAbility: function (...args) {
      console.warn("ability.particleAbility.startAbility interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve) => {
          resolve(paramMock.paramNumberMock)
        })
      }
    }
  }
}
export function mockAbilityFormManager() {
  const FormInfoMock = {
    bundleName: "[PC Preview] unknow bundleName",
    moduleName: "[PC Preview] unknow moduleName",
    abilityName: "[PC Preview] unknow abilityName",
    name: "[PC Preview] unknow name",
    description: "[PC Preview] unknow description",
    type: "[PC Preview] unknow type",
    jsComponentName: "[PC Preview] unknow jsComponentName",
    colorMode: "[PC Preview] unknow colorMode",
    isDefault: "[PC Preview] unknow isDefault",
    updateEnabled: "[PC Preview] unknow updateEnabled",
    formVisibleNotify: "[PC Preview] unknow formVisibleNotify",
    relatedBundleName: "[PC Preview] unknow relatedBundleName",
    scheduledUpdateTime: "[PC Preview] unknow scheduledUpdateTime",
    formConfigAbility: "[PC Preview] unknow formConfigAbility",
    updateDuration: "[PC Preview] unknow updateDuration",
    defaultDimension: "[PC Preview] unknow defaultDimension",
    supportDimensions: "[PC Preview] unknow supportDimensions",
    customizeDatas: "[PC Preview] unknow customizeDatas",
    jsonObject: "[PC Preview] unknow jsonObject"
  }
  const FormInfoArrayMock = [
    FormInfoMock
  ]
  global.systemplugin.ability.formManager = {
    requestForm: function (...args) {
      console.warn("ability.formManager.requestForm interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    deleteForm: function (...args) {
      console.warn("ability.formManager.deleteForm interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    releaseForm: function (...args) {
      console.warn("ability.formManager.releaseForm interface mocked in the Previewer. How this interface works on the Previewer may" +
          " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    notifyVisibleForms: function (...args) {
      console.warn("ability.formManager.notifyVisibleForms interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    notifyInvisibleForms: function (...args) {
      console.warn("ability.formManager.notifyInvisibleForms interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    enableFormsUpdate: function (...args) {
      console.warn("ability.formManager.enableFormsUpdate interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    disableFormsUpdate: function (...args) {
      console.warn("ability.formManager.disableFormsUpdate interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    getAllFormsInfo: function (...args) {
      console.warn("ability.formManager.getAllFormsInfo interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, FormInfoArrayMock)
      } else {
        return new Promise((resolve) => {
          resolve(FormInfoArrayMock)
        })
      }
    },
    getFormsInfoByApp: function (...args) {
      console.warn("ability.formManager.getFormsInfoByApp interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, FormInfoArrayMock)
      } else {
        return new Promise((resolve) => {
          resolve(FormInfoArrayMock)
        })
      }
    },
    getFormsInfoByModule: function (...args) {
      console.warn("ability.formManager.getFormsInfoByModule interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, FormInfoArrayMock)
      } else {
        return new Promise((resolve) => {
          resolve(FormInfoArrayMock)
        })
      }
    }
  }
}