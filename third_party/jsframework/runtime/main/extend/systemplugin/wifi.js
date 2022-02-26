import { paramMock } from "./utils"

export function mockWifi() {
  const linkedInfoMock = {
    ssid: '[PC preview] unknow ssid',
    bssid: '[PC preview] unknow bssid',
    networkId: '[PC preview] unknow networkId',
    rssi: '[PC preview] unknow rssi',
    band: '[PC preview] unknow band',
    linkSpeed: '[PC preview] unknow linkSpeed',
    frequency: '[PC preview] unknow frequency',
    isHidden: '[PC preview] unknow isHidden',
    isRestricted: '[PC preview] unknow isRestricted',
    chload: '[PC preview] unknow chload',
    snr: '[PC preview] unknow snr',
    macAddress: '[PC preview] unknow macAddress',
    ipAddress: '[PC preview] unknow ipAddress',
    suppState: '[PC preview] unknow suppState',
    connState: '[PC preview] unknow connState'
  }
  global.systemplugin.wifi = {
    getLinkedInfo: function (...args) {
      console.warn("wifi.getLinkedInfo interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, linkedInfoMock)
      } else {
        return new Promise((resolve) => {
          resolve(linkedInfoMock)
        })
      }
    }
  }
}