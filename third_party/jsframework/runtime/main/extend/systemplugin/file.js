import { hasComplete } from "./utils"

export function mockFile() {
    global.systemplugin.file = {
      move: function(...args) {
        console.warn("file.move interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success(args[0].dstUri)
        hasComplete(args[0].complete)
      },
      copy: function(...args) {
        console.warn("file.copy interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success(args[0].dstUri)
        hasComplete(args[0].complete)
      },
      list: function(...args) {
        console.warn("file.list interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        const data = {
          fileList: [{
            uri: '[PC Preview]: no file',
            lastModifiedTime: "[PC Preview]: no file",
            length: "[PC Preview]: no file",
            type: 'file'
          }]
        }
        args[0].success(data)
        hasComplete(args[0].complete)
      },
      get: function(...args) {
        console.warn("file.get interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        const data = {
          uri: '[PC Preview]: no file',
          lastModifiedTime: "[PC Preview]: no file",
          length: "[PC Preview]: no file",
          type: 'file',
          subFiles: ["[PC Preview]: no file", "[PC Preview]: no file"]
        }
        args[0].success(data)
        hasComplete(args[0].complete)
      },
      delete: function(...args) {
        console.warn("file.delete interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      writeText: function(...args) {
        console.warn("file.writeText interface mocked in the Previewer. How this interface works on the Previewer may" +
          " be different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      writeArrayBuffer: function(...args) {
        console.warn("file.writeArrayBuffer interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      readText: function(...args) {
        console.warn("file.readText interface mocked in the Previewer. How this interface works on the Previewer may" +
          " be different from that on a real device.")
        const data = {text: "[PC Preview]: success default"}
        args[0].success(data)
        hasComplete(args[0].complete)
      },
      readArrayBuffer: function(...args) {
        console.warn("file.readArrayBuffer interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        const data = {buffer: ["[PC Preview]: default", "[PC Preview]: default", "[PC Preview]: default"]}
        args[0].success(data)
        hasComplete(args[0].complete)
      },
      access: function(...args) {
        console.warn("file.access interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      mkdir: function(...args) {
        console.warn("file.mkdir interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      rmdir: function(...args) {
        console.warn("file.rmdir interface mocked in the Previewer. How this interface works on the Previewer may be" +
          " different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      }
    }
  }