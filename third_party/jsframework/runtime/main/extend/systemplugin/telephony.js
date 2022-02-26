import { paramMock } from "./utils"

export function mockCall() {
  global.systemplugin.telephony = {}
  global.systemplugin.telephony.call = {
    dial: function (...args) {
      console.warn("telephony.call.dial interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    }
  }
}