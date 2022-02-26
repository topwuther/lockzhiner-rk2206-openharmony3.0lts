import { paramMock } from "../utils"

export function mockUtil() {
    const result = {
        TextDecoder: function(...args) {
            console.warn("util.TextDecoder interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            return TextDecoderMock;
        },
        TextEncoder: function(...args) {
            console.warn("util.TextEncoder interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            return TextEncoderMock;
        }
    }
    const TextDecoderMock = {
        encoding: '[PC preview] unknow encoding',
        fatal: '[PC preview] unknow fatal',
        ignoreBOM: '[PC preview] unknow ignoreBOM',
        decode: function (...args) {
            console.warn("TextDecoder.decode interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            return paramMock.paramStringMock;
        },
    }
    const TextEncoderMock = {
        encoding: '[PC preview] unknow encoding',
        encode: function(...args) {
            console.warn("TextEncoder.encode interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            return paramMock.paramArrayMock;
        },
        encodeInto: function(...args) {
            console.warn("TextEncoder.encodeInto interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            return paramMock.paramObjectMock;
        }
    }
    return result;
}
