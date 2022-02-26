import { paramMock } from "./utils"

export function mockDataStorage() {
  global.systemplugin.data = {}
  const paramValueTypeMock = '[PC Preview] unknow ValueType'
  const storageObserverMock = {
    key: "[PC preview] unknow key"
  }
  const storageMock = {
    getSync: function () {
      console.warn("Storage.getSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramValueTypeMock
    },
    get: function (...args) {
      console.warn("Storage.get interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramValueTypeMock)
      } else {
        return new Promise((resolve) => {
          resolve(paramValueTypeMock)
        })
      }
    },
    hasSync: function () {
      console.warn("Storage.hasSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    has: function (...args) {
      console.warn("Storage.has interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock)
      } else {
        return new Promise((resolve) => {
          resolve(paramMock.paramBooleanMock)
        })
      }
    },
    putSync: function () {
      console.warn("Storage.putSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
    },
    put: function (...args) {
      console.warn("Storage.put interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    deleteSync: function () {
      console.warn("Storage.deleteSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    delete: function (...args) {
      console.warn("Storage.delete interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    clearSync: function () {
      console.warn("Storage.clearSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    clear: function (...args) {
      console.warn("Storage.clear interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    flushSync: function () {
      console.warn("Storage.flushSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    flush: function (...args) {
      console.warn("Storage.flush interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    on: function (...args) {
      console.warn("Storage.on interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      args[len - 1].call(this, storageObserverMock)
    },
    off: function (...args) {
      console.warn("Storage.off interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      args[len - 1].call(this, storageObserverMock)
    }
  }
  global.systemplugin.data.storage = {
    getStorageSync: function () {
      console.warn("data.storage.getStorageSync interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return storageMock
    },
    getStorage: function (...args) {
      console.warn("data.storage.getStorage interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, storageMock)
      } else {
        return new Promise((resolve) => {
          resolve(storageMock)
        })
      }
    },
    deleteStorageSync: function () {
      console.warn("data.storage.deleteStorageSync interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
    },
    deleteStorage: function (...args) {
      console.warn("data.storage.deleteStorage interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    },
    removeStorageFromCacheSync: function () {
      console.warn("data.storage.removeStorageFromCacheSync interface mocked in the Previewer. How this interface" +
        " works on the Previewer may be different from that on a real device.")
    },
    removeStorageFromCache: function (...args) {
      console.warn("data.storage.removeStorageFromCache interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock)
      } else {
        return new Promise((resolve) => {
          resolve()
        })
      }
    }
  }
}