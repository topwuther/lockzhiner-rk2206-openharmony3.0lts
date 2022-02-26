import { hasComplete } from "./utils"

export function mockSystemPackage() {
    global.systemplugin.package = {
      hasInstalled: function (...args) {
        console.warn("package.hasInstalled interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        args[0].success(true)
        isComplete(args[0].complete)
      }
    }
  }