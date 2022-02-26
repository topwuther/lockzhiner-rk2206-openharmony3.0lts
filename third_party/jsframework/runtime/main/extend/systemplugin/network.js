export function mockNetwork() {
  const data = {
    metered: true,
    type: "5g"
  }
  global.systemplugin.network = {
    getType: function (...args) {
      console.warn("network.getType interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      args[0].success(data)
      args[0].complete()
    },
    subscribe: function (...args) {
      console.warn("network.subscribe interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      if (!this.unsubscribeNetwork) {
        this.unsubscribeNetwork = setInterval(() => {
          args[0].success(data)
        }, 3000)
      }
    },
    unsubscribe: function () {
      console.warn("network.unsubscribe interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      clearInterval(this.unsubscribeNetwork)
      delete this.unsubscribeNetwork
    }
  }
}