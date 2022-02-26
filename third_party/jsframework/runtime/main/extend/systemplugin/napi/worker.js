import { paramMock } from "../utils"

export function mockWorker() {
  const result = {
    Worker: function (params) {
      console.warn("worker.Worker interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return WorkerMock;
    }
  }
  const WorkerMock = {
    postMessage: function (...args) {
      console.warn("Worker.postMessage interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    terminate: function (...args) {
      console.warn("Worker.terminate interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    on: function (...args) {
      console.warn("Worker.on interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    once: function (...args) {
      console.warn("Worker.once interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    off: function (...args) {
      console.warn("Worker.off interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    addEventListener: function (...args) {
      console.warn("Worker.addEventListener interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    dispatchEvent: function (...args) {
      console.warn("Worker.dispatchEvent interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock;
    },
    removeEventListener: function (...args) {
      console.warn("Worker.removeEventListener interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    removeAllListener: function (...args) {
      console.warn("Worker.removeAllListener interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    onmessage: function (...args) {
      console.warn("Worker.onmessage interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    onmessageerror: function (...agrs) {
      console.warn("Worker.onmessageerror interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    onerror: function (...args) {
      console.warn("Worker.onerror interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    onclose: function (...args) {
      console.warn("Worker.onclose interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    onexit: function (...args) {
      console.warn("Worker.onexit interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    }
  }
  return result;
}