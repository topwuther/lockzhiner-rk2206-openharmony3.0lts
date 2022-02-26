import { paramMock } from "./utils"

export function mockUserauth() {
  global.systemplugin.userIAM = {}
  const AuthenticatorMock = {
    execuate: function (...args) {
      console.warn("Authenticator.execuate interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramBooleanMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramNumberMock);
        })
      }
    }
  }
  global.systemplugin.userIAM.userAuth = {
    getAuthenticator: function (...args) {
      console.warn("userIAM.userAuth.getAuthenticator interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      return AuthenticatorMock;
    }
  }
}