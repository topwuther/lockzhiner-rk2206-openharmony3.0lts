import { paramMock } from "../utils"

export function mockSystemParameter() {
  const systemParameter = {
    getSync: function (...args) {
      console.warn("systemParameter.getSync interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramStringMock
    },
    get: function (...args) {
      console.warn("systemParameter.get interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    setSync: function (...args) {
      console.warn("systemParameter.setSync interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    set: function (...args) {
      console.warn("systemParameter.set interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    }
  }
  return systemParameter
}