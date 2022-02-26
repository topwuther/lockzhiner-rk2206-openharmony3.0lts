import { hasComplete } from "./utils"

export function mockBrightness() {
  global.systemplugin.brightness = {
    argsV: {
      value: 80
    },
    argsM: {
      mode: 0
    },
    getValue: function (...args) {
      console.warn("brightness.getValue interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      args[0].success(this.argsV)
      hasComplete(args[0].complete)
    },
    setValue: function (...args) {
      console.warn("brightness.setValue interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      if (args[0].value) {
        this.argsV.value = args[0].value
        args[0].success("brightness setValue successfully")
        hasComplete(args[0].complete)
      }
    },
    getMode: function (...args) {
      console.warn("brightness.getMode interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      args[0].success(this.argsM)
      hasComplete(args[0].complete)
    },
    setMode: function (...args) {
      console.warn("brightness.setMode interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      this.argsM.mode = args[0].mode
      args[0].success("brightness setMode successfully")
      hasComplete(args[0].complete)
    },
    setKeepScreenOn: function (...args) {
      console.warn("brightness.setKeepScreenOn interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      args[0].success("brightness setKeepScreenOn successfully")
      hasComplete(args[0].complete)
    }
  }
}