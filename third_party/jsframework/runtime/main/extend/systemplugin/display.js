import { paramMock } from "./utils"

export function mockDisplay() {
    const displayMock = {
        densityDPI: '[PC preview] unknow densityDPI',
        densityPixels: '[PC preview] unknow densityPixels',
        scaledDensity: '[PC preview] unknow scaledDensity',
        xDPI: '[PC preview] unknow xDPI',
        yDPI: '[PC preview] unknow yDPI',
        width: '[PC preview] unknow width',
        height: '[PC preview] unknow height',
        alive: '[PC preview] unknow alive',
        refreshRate: '[PC preview] unknow refreshRate',
        id: '[PC preview] unknow id',
        state: '[PC preview] unknow state',
        rotation: '[PC preview] unknow rotation',
        name: '[PC preview] unknow name'
    }
    const allDisplayMock = [{
        densityDPI: '[PC preview] unknow densityDPI',
        densityPixels: '[PC preview] unknow densityPixels',
        scaledDensity: '[PC preview] unknow scaledDensity',
        xDPI: '[PC preview] unknow xDPI',
        yDPI: '[PC preview] unknow yDPI',
        width: '[PC preview] unknow width',
        height: '[PC preview] unknow height',
        alive: '[PC preview] unknow alive',
        refreshRate: '[PC preview] unknow refreshRate',
        id: '[PC preview] unknow id',
        state: '[PC preview] unknow state',
        rotation: '[PC preview] unknow rotation',
        name: '[PC preview] unknow name'
    }]
    global.systemplugin.display = {
        getDefaultDisplay: function (...args) {
            console.warn("display.getDefaultDisplay interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            const len = args.length
            if (typeof args[len - 1] === 'function') {
                args[len - 1].call(this, paramMock.businessErrorMock, displayMock)
            } else {
                return new Promise((resolve, reject) => {
                    resolve(displayMock)
                })
            }
        },
        getAllDisplay: function (...args) {
            console.warn("display.getAllDisplay interface mocked in the Previewer. How this interface works on the Previewer" +
                " may be different from that on a real device.")
            const len = args.length
            if (typeof args[len - 1] === 'function') {
                args[len - 1].call(this, paramMock.businessErrorMock, allDisplayMock)
            } else {
                return new Promise((resolve, reject) => {
                    resolve(allDisplayMock)
                })
            }
        },
        on: function (...args) {
            console.warn("display.on interface mocked in the Previewer. How this interface works on the Previewer may be" +
                " different from that on a real device.")
            const len = args.length
            args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
        },
        off: function (...args) {
            console.warn("display.off interface mocked in the Previewer. How this interface works on the Previewer may be" +
                " different from that on a real device.")
            const len = args.length
            args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
        },
    }
}