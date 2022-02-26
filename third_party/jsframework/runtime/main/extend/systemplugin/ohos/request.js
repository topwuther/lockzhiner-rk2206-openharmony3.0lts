import {paramMock} from "../utils";
const NOTIFY = "[PC Preview] unknow"
const Properties = [
  'NETWORK_MOBILE',
  'NETWORK_WIFI',
  'ERROR_CANNOT_RESUME',
  'ERROR_DEVICE_NOT_FOUND',
  'ERROR_FILE_ALREADY_EXISTS',
  'ERROR_FILE_ERROR',
  'ERROR_HTTP_DATA_ERROR',
  'ERROR_INSUFFICIENT_SPACE',
  'ERROR_TOO_MANY_REDIRECTS',
  'ERROR_UNHANDLED_HTTP_CODE',
  'ERROR_UNKNOWN',
  'PAUSED_QUEUED_FOR_WIFI',
  'PAUSED_UNKNOWN',
  'PAUSED_WAITING_FOR_NETWORK',
  'PAUSED_WAITING_TO_RETRY',
  'SESSION_FAILED',
  'SESSION_PAUSED',
  'SESSION_PENDING',
  'SESSION_RUNNING',
  'SESSION_SUCCESSFUL'
]

export function mockOhosRequest() {
  const downloadInfo = {
    downloadId: 1,
    failedReason: 16,
    fileName: "[PC preview] unknown file name",
    filePath: "[PC preview] unknown file path",
    pausedReason: 1,
    status: 1,
    targetURI: "[PC preview] unknown targetURI",
    downloadTitle: "[PC preview] unknown download title",
    downloadTotalBytes: 1024,
    description: "[PC preview] unknown description",
    downloadedBytes: 1
  }
  const DownloadTaskMock = {
    on: function(...args) {
      console.warn("DownloadTask.on interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
          if (args[0] == 'progress') {
              args[len - 1].call(this, paramMock.paramNumberMock, paramMock.paramNumberMock);
          } else if (args[0] == 'complete' || args[0] == 'pause' || args[0] == 'remove') {
              args[len - 1].call(this, paramMock.businessErrorMock);
          } else if (args[0] == 'fail') {
            args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock);
          }
      }
    },
    off: function(...args) {
      console.warn("DownloadTask.off interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
          if (args[0] == 'progress') {
              args[len - 1].call(this, paramMock.paramNumberMock, paramMock.paramNumberMock);
          } else if (args[0] == 'complete' || args[0] == 'pause' || args[0] == 'remove') {
              args[len - 1].call(this, paramMock.businessErrorMock);
          } else if (args[0] == 'fail') {
              args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock);
          }
      }
    },
    remove: function(...args) {
      console.warn("DownloadTask.remove interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    },
    pause: function(...args) {
      console.warn("DownloadTask.pause interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    resume: function(...args) {
      console.warn("DownloadTask.resume interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    queryMimeType: function(...args) {
      console.warn("DownloadTask.queryMimeType interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    query: function(...args) {
      console.warn("DownloadTask.query interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, downloadInfo);
      } else {
        return new Promise((resolve, reject) => {
          resolve(downloadInfo);
        })
      }
    }
  }
  const UploadTaskMock = {
    on: function(...args) {
      console.warn("UploadTask.on interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
          if (args[0] == 'progress') {
              args[len - 1].call(this, paramMock.paramNumberMock, paramMock.paramNumberMock);
	  } else if (args[0] == 'headerReceive') {
              args[len - 1].call(this, paramMock.paramObjectMock);
	  }
      }
    },
    off: function(...args) {
      console.warn("UploadTask.off interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
          if (args[0] == 'progress') {
              args[len - 1].call(this, paramMock.paramNumberMock, paramMock.paramNumberMock);
	  } else if (args[0] == 'headerReceive') {
              args[len - 1].call(this, paramMock.paramObjectMock);
          }
      }
    },
    remove: function(...args) {
      console.warn("UploadTask.remove interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    }
  }
  global.ohosplugin.request = {
    download: function (...args) {
      console.warn("ohos.request.download interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, DownloadTaskMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(DownloadTaskMock);
        })
      }
    },
    upload: function (...args) {
      console.warn("ohos.request.upload interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, UploadTaskMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(UploadTaskMock);
        })
      }
    }
  }

  for(let property of Properties) {
    global.ohosplugin.request[property] = NOTIFY + " " + property
  }
}
