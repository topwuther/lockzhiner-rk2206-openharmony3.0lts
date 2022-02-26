export function mockNotification() {
  global.systemplugin.notification = {
    show: function () {
      console.warn("notification.show interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    }
  }
}