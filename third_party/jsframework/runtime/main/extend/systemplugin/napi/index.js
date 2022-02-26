import { mockSystemParameter } from './systemParameter'
import { mockFileio } from './fileio'
import { mockWorker } from './worker'
import { mockUtil } from './util'
import { mockPower } from './power'
import { mockRunningLock } from './runningLock'

export function mockRequireNapiFun() {
  global.requireNapi = function (...args) {
    switch (args[0]) {
      case "systemParameter":
        return mockSystemParameter();
      case "fileio":
        return mockFileio();
      case "worker" :
        return mockWorker();
      case "util":
        return mockUtil();
      case "power":
        return mockPower();
      case "runningLock":
        return mockRunningLock();
    }
  }
}
