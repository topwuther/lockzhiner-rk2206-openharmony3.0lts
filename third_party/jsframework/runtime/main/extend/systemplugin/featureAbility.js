export function mockFeatureAbility() {
  let FeatureAbilityObject = {
    getFeatureAbility: function () {
      const ResultMock = {
        code: '[PC preview] unknow code',
        data: '[PC preview] unknow data'
      }
      const FeatureAbility = {
        startAbility: function (...args) {
          console.warn("Unable to use the FeatureAbility.startAbility method to start another ability in the" +
            " Previewer. Perform this operation on the emulator or a real device instead.")
          return new Promise((resolve, reject) => {
            resolve(ResultMock);
          })
        },
        startAbilityForResult: function (...args) {
          console.warn("Unable to use the FeatureAbility.startAbilityForResult method to start another ability in" +
            " the Previewer. Perform this operation on the emulator or a real device instead.")
          return new Promise((resolve, reject) => {
            resolve(ResultMock);
          })
        },
        finishWithResult: function (...args) {
          console.warn("FeatureAbility.finishWithResult interface mocked in the Previewer. How this interface works" +
            " on the Previewer may be different from that on a real device.")
          return new Promise((resolve, reject) => {
            resolve(ResultMock);
          })
        },
        callAbility: function (...args) {
          console.warn("FeatureAbility.callAbility interface mocked in the Previewer. How this interface works on" +
            " the Previewer may be different from that on a real device.")
          return new Promise((resolve, reject) => {
            resolve(JSON.stringify(ResultMock));
          })
        },
        continueAbility: function (...args) {
          console.warn("FeatureAbility.continueAbility interface mocked in the Previewer. How this interface works on" +
            " the Previewer may be different from that on a real device.")
          return new Promise((resolve, reject) => {
            resolve(ResultMock);
          })
        },
        subscribeAbilityEvent: function (...args) {
          console.warn("FeatureAbility.subscribeAbilityEvent interface mocked in the Previewer. How this interface" +
            " works on the Previewer may be different from that on a real device.")
          return new Promise((resolve, reject) => {
            resolve(JSON.stringify(ResultMock));
          })
        },
        unsubscribeAbilityEvent: function (...args) {
          console.warn("FeatureAbility.unsubscribeAbilityEvent interface mocked in the Previewer. How this interface" +
            " works on the Previewer may be different from that on a real device.")
          return new Promise((resolve, reject) => {
            resolve(JSON.stringify(ResultMock));
          })
        }
      }
      return FeatureAbility
    }
  }
  global.createLocalParticleAbility = function (...args) {
    console.warn("createLocalParticleAbility interface mocked in the Previewer. How this interface" +
      " works on the Previewer may be different from that on a real device.")
    return new Promise((resolve, reject) => {
      resolve(paramMock.paramObjectMock);
    })
  }
  global.FeatureAbility = FeatureAbilityObject.getFeatureAbility();
}