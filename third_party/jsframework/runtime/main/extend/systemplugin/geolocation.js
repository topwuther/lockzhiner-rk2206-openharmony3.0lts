import { hasComplete } from "./utils"

export function mockGeolocation() {
    const data = {
      latitude: '121.61934',
      longitude: '31.257907',
      accuracy: '15',
      time: '160332896544'
    }
    global.systemplugin.geolocation = {
      getLocation: function (...args) {
        console.warn("geolocation.getLocation interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        args[0].success(data)
        hasComplete(args[0].complete)
      },
      getLocationType: function(...args) {
        console.warn("geolocation.getLocationType interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        const info = {types: ['gps', 'network']}
        args[0].success(info)
        hasComplete(args[0].complete)
      },
      getSupportedCoordTypes() {
        console.warn("geolocation.getSupportedCoordTypes interface mocked in the Previewer. How this interface works" +
          " on the Previewer may be different from that on a real device.")
        return ["wgs84"]
      },
      subscribe: function(...args) {
        console.warn("geolocation.subscribe interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        if (!this.unsubscribeLocation) {
          this.unsubscribeLocation = setInterval(() => {
            data.latitude = getRandomArbitrary(121, 122)
            data.longitude = getRandomArbitrary(31, 32)
            data.accuracy = getRandomArbitrary(14, 18)
            args[0].success(data)
          }, 1000)
        }
      },
      unsubscribe: function() {
        console.warn("geolocation.unsubscribe interface mocked in the Previewer. How this interface works on the" +
          " Previewer may be different from that on a real device.")
        clearInterval(this.unsubscribeLocation)
        delete this.unsubscribeLocation
      }
    }
  }