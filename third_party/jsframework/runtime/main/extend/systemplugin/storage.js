import { hasComplete } from "./utils"

export function mockStorage() {
    global.systemplugin.storage = {
      get: function(...args) {
        console.warn("storage.get interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success("[PC Preview]: no system")
        hasComplete(args[0].complete)
      },
      set: function(...args) {
        console.warn("storage.set interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success("[PC Preview]: no system")
        hasComplete(args[0].complete)
      },
      clear: function(...args) {
        console.warn("storage.clear interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success("[PC Preview]: no system")
        hasComplete(args[0].complete)
      },
      delete: function(...args) {
        console.warn("storage.delete interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success("[PC Preview]: no system")
        hasComplete(args[0].complete)
      }
    }
  }