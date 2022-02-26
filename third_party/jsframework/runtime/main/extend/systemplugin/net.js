import { paramMock } from "./utils"

export function mockWebSocket() {
  global.systemplugin.net = {}
  const WebSocketMock = {
    connect: function (...args) {
      console.warn("WebSocket.connect interface mocked in the Previewer. How this interface works on the Previewer" +
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
    send: function (...args) {
      console.warn("WebSocket.send interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    },
    close: function (...args) {
      console.warn("WebSocket.close interface mocked in the Previewer. How this interface works on the Previewer " +
        "may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramBooleanMock);
        })
      }
    },
    on: function (...args) {
      console.warn("WebSocket.on interface mocked in the Previewer. How this interface works on the Previewer may " +
        "be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        if (args[0] == 'open') {
          args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramObjectMock);
        } else if (args[0] == 'message') {
          args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
        } else if (args[0] == 'close') {
          args[len - 1].call(this, {
            code: "[PC Preview] unknow code",
            reason: "[PC Preview] unknow reason"
          });
        } else if (args[0] == 'error') {
          args[len - 1].call(this, paramMock.businessErrorMock);
        }
      }
    },
    off: function (...args) {
      console.warn("WebSocket.off interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        if (args[0] == 'open') {
          args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramObjectMock);
        } else if (args[0] == 'message') {
          args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
        } else if (args[0] == 'close') {
          args[len - 1].call(this, {
            code: "[PC Preview] unknow code",
            reason: "[PC Preview] unknow reason"
          });
        } else if (args[0] == 'error') {
          args[len - 1].call(this, paramMock.businessErrorMock);
        }
      }
    }
  }
  global.systemplugin.net.webSocket = {
    createWebSocket: function () {
      console.warn("net.webSocket.createWebSocket interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return WebSocketMock;
    }
  }
}

export function mockHttp() {
  const HttpResponseMock = {
    result: "[PC Preview] unknow result",
    responseCode: "[PC Preview] unknow responseCode",
    header: "[PC Preview] unknow header"
  }
  const HttpRequestMock = {
    request: function (...args) {
      console.warn("HttpRequest.request interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, HttpResponseMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(HttpResponseMock);
        })
      }
    },
    destroy: function () {
      console.warn("HttpRequest.destroy interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    on: function (...args) {
      console.warn("HttpRequest.on interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramObjectMock);
      }
    },
    off: function (...args) {
      console.warn("HttpRequest.off interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramObjectMock);
      }
    }
  }
  global.systemplugin.net.http = {
    createHttp: function () {
      console.warn("net.http.createHttp interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return HttpRequestMock;
    }
  }
}