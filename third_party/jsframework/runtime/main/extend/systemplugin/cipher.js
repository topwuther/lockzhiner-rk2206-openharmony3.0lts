import { hasComplete } from "./utils"

export function mockCipher() {
    global.systemplugin.cipher = {
      rsa: function(...args) {
        console.warn("cipher.rsa interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        const result = '[PC preview] rsa xxxxxxxxxxxxx'
        args[0].success(result)
        hasComplete(args[0].complete)
      },
      aes: function(...args) {
        console.warn("cipher.aes interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        const result = '[PC preview] aes xxxxxxxxxxxxx'
        args[0].success(result)
        hasComplete(args[0].complete)
      }
    }
  }