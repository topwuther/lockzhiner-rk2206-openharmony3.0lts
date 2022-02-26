import { hasComplete } from "./utils"

export function mockBattery() {
  global.systemplugin.battery = {
    getStatus: function (...args) {
      console.warn("battery.getStatus interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      args[0].success.call(this, {
        level: 1,
        charging: false
      })
      hasComplete(args[0].complete)
    }
  }
}