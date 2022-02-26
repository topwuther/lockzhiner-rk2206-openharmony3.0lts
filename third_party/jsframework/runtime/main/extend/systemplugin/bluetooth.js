import { hasComplete } from "./utils"

export function mockBluetooth() {
    global.systemplugin.bluetooth = {
      startBLEScan: function (...args) {
        console.warn("bluetooth.startBLEScan interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      stopBLEScan: function (...args) {
        console.warn("bluetooth.stopBLEScan interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        args[0].success()
        hasComplete(args[0].complete)
      },
      subscribeBLEFound: function (...args) {
        console.warn("bluetooth.subscribeBLEFound interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        const options = {
          addrType: 'public',
          addr: 'xx:xx:xx:xx',
          rssi: 123,
          txpower: 'xxx',
          data: 'xxx'
        }
        const min = 1
        const max = 6
        clearInterval(this.unsubscribeBLE)
        delete this.unsubscribeBLE
        this.unsubscribeBLE = setInterval(() => {
          const randomAddr = Math.floor(Math.random() * (max - min)) + min
          const optionsArr = Array(randomAddr).fill(options)
          args[0].success(optionsArr)
        }, 1000)
      },
      unsubscribeBLEFound: function () {
        console.warn("bluetooth.unsubscribeBLEFound interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        clearInterval(this.unsubscribeBLE)
        delete this.unsubscribeBLE
      }
    }
  }
  