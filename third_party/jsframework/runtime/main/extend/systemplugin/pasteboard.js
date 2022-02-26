import { paramMock } from "./utils"

export function mockPasteBoard() {
  const PasteDataMock = {
    getPrimaryText: function () {
      console.warn("PasteData.getPrimaryText interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return "[PC Preview] unknow primarytext"
    }
  }
  const SystemPasteboardMock = {
    getPasteData: function (...args) {
      console.warn("SystemPasteboard.getPasteData interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, PasteDataMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(PasteDataMock);
        })
      }
    },
    setPasteData: function (...args) {
      console.warn("SystemPasteboard.setPasteData interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    }
  }
  global.systemplugin.pasteboard = {
    createPlainTextData: function () {
      console.warn("pasteboard.createPlainTextData interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      return PasteDataMock;
    },
    getSystemPasteboard: function () {
      console.warn("pasteboard.getSystemPasteboard interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      return SystemPasteboardMock;
    }
  }
}
