import { hasComplete } from "./utils"

export function mockRequest() {
  const uploadResponseMock = {
    code: "[PC Preview]: unknow code",
    data: "[PC Preview]: unknow data",
    headers: "[PC Preview]: unknow headers"
  }
  const downloadResponse = {
    token: "[PC Preview]: unknow token"
  }
  const onDownloadCompleteMock = {
    uri: "[PC Preview]: unknow uri"
  }
  global.systemplugin.request = {
    upload: function (...args) {
      console.warn("system.request.upload interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      args[0].success(uploadResponseMock)
      hasComplete(args[0].complete)
    },
    download: function (...args) {
      console.warn("system.request.download interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      args[0].success(downloadResponse)
      hasComplete(args[0].complete)
    },
    onDownloadComplete: function (...args) {
      console.warn("system.request.onDownloadComplete interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      args[0].success(onDownloadCompleteMock)
      hasComplete(args[0].complete)
    }
  }
}