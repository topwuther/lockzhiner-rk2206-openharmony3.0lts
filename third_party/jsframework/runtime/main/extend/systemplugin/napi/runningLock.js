import { paramMock } from "../utils"

export function mockRunningLock() {
  const RunningLockMock = {
    lock: function (...args) {
      console.warn("RunningLock.lock interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    isUsed: function () {
      console.warn("RunningLock.isUsed interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    unlock: function () {
      console.warn("RunningLock.unlock interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    }
  };
  const runningLock = {
    isRunningLockTypeSupported: function (...args) {
      console.warn("runningLock.isRunningLockTypeSupported interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    },
    createRunningLock: function (...args) {
      console.warn("runningLock.createRunningLock interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, RunningLockMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(RunningLockMock);
        })
      }
    }
  }
  return runningLock
}