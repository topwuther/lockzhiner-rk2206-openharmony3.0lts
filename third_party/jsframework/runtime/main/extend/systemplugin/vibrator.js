import { hasComplete } from "./utils"

export function mockVibrator() {
  global.systemplugin.vibrator = {
    vibrate: function (...args) {
      console.warn("vibrator.vibrate interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      args[0].success()
      hasComplete(args[0].complete)
    }
  }
}