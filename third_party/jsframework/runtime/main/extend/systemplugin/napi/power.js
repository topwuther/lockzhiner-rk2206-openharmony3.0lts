import { paramMock } from "../utils"

export function mockPower() {
  const power = {
    shutdownDevice: function (...args) {
      console.warn("power.shutdownDevice interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    rebootDevice: function (...args) {
      console.warn("power.rebootDevice interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    isScreenOn: function (...args) {
      console.warn("power.isScreenOn interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
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
  return power
}
