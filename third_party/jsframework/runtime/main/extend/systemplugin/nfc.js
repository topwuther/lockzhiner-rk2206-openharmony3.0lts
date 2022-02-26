import { paramMock } from "./utils"

export function mockNfcCardEmulation() {
  global.systemplugin.nfc = {}
  global.systemplugin.nfc.cardEmulation = {
    isSupported: function (...args) {
      console.warn("nfc.cardEmulation.isSupported interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock;
    }
  }
}
