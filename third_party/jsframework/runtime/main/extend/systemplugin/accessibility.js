import { paramMock } from "./utils"

export function mockAccessibility() {
  global.systemplugin.accessibility = {
    isOpenAccessibility: function (...args) {
      console.warn("accessibility.isOpenAccessibility interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
     },
    isOpenTouchExploration: function (...args) {
      console.warn("accessibility.isOpenTouchExploration interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
        const len = args.length
        if (typeof args[len - 1] === 'function') {
          args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock)
        } else {
          return new Promise((resolve) => {
            resolve(paramMock.paramBooleanMock);
        })
      }
    }
  }
}