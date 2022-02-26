import {getRandomArbitrary, hasComplete, paramMock} from './utils'

export function mockSensor() {
  global.systemplugin.sensor = {}
  mockAccelerometer()
  mockBarometer()
  mockCompass()
  mockDeviceOrientation()
  mockGyroscope()
  mockHeartRate()
  mockLight()
  mockOnBodyState()
  mockProximity()
  mockStepCounter()
}

function mockAccelerometer() {
  global.systemplugin.sensor.subscribeAccelerometer = function(...args) {
    console.warn("sensor.subscribeAccelerometer interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    const time = {
      normal: 200,
      game: 20,
      ui: 60
    }
    let ret = {}
    let timer = 0
    if (!args[0].interval) {
      timer = time.normal
    } else {
      timer = time[args[0].interval]
    }
    clearInterval(this.unsubscribeAcc)
    delete this.unsubscribeAcc
    this.unsubscribeAcc = setInterval(() => {
      ret.x = Math.ceil(Math.random() * 10)
      ret.y = Math.ceil(Math.random() * 10)
      ret.z = Math.ceil(Math.random() * 10)
      args[0].success(ret)
    }, timer)
  }
  global.systemplugin.sensor.unsubscribeAccelerometer = function() {
    console.warn("sensor.unsubscribeAccelerometer interface mocked in the Previewer. How this interface works on" +
      " the Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeAcc)
    delete this.unsubscribeAcc
  }
}

function mockBarometer() {
  global.systemplugin.sensor.subscribeBarometer = function(...args) {
    console.warn("sensor.subscribeBarometer interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribePressure) {
      let ret = {}
      this.unsubscribePressure = setInterval(() => {
        ret.pressure = getRandomArbitrary(1110, 1111)
        args[0].success(ret)
      }, 500)
    }
  }
  global.systemplugin.sensor.unsubscribeBarometer = function() {
    console.warn("sensor.unsubscribeBarometer interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribePressure)
    delete this.unsubscribePressure
  }
}

function mockCompass() {
  global.systemplugin.sensor.subscribeCompass = function(...args) {
    console.warn("sensor.subscribeCompass interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeDirection) {
      let ret = {}
      this.unsubscribeDirection = setInterval(() => {
        ret.direction = getRandomArbitrary(49, 50)
        args[0].success(ret)
      }, 100)
    }
  }
  global.systemplugin.sensor.unsubscribeCompass = function() {
    console.warn("sensor.unsubscribeCompass interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeDirection)
    delete this.unsubscribeDirection
  }
}

function mockGyroscope() {
  global.systemplugin.sensor.subscribeGyroscope = function(...args) {
    console.warn("sensor.subscribeGyroscope interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    const time = {
      normal: 200,
      game: 20,
      ui: 60
    }
    let ret = {}
    let timer = 0
    if (!args[0].interval) {
      timer = time.normal
    } else {
      timer = time[args[0].interval]
    }
    clearInterval(this.unsubscribeGyr)
    delete this.unsubscribeGyr
    this.unsubscribeGyr = setInterval(() => {
      ret.x = Math.ceil(Math.random() * 10)
      ret.y = Math.ceil(Math.random() * 10)
      ret.z = Math.ceil(Math.random() * 10)
      args[0].success(ret)
    }, timer)
  }
  global.systemplugin.sensor.unsubscribeGyroscope = function() {
    console.warn("sensor.unsubscribeGyroscope interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeGyr)
    delete this.unsubscribeGyr
  }
}

function mockDeviceOrientation() {
  global.systemplugin.sensor.subscribeDeviceOrientation = function(...args) {
    console.warn("sensor.subscribeDeviceOrientation interface mocked in the Previewer. How this interface works on" +
      " the Previewer may be different from that on a real device.")
    const time = {
      normal: 200,
      game: 20,
      ui: 60
    }
    let ret = {}
    let timer = 0
    if (!args[0].interval) {
      timer = time.normal
    } else {
      timer = time[args[0].interval]
    }
    clearInterval(this.unsubscribeDevOri)
    delete this.unsubscribeDevOri
    this.unsubscribeDevOri = setInterval(() => {
      ret.alpha = Math.ceil(Math.random() * 10)
      ret.beta = Math.ceil(Math.random() * 10)
      ret.gamma = Math.ceil(Math.random() * 10)
      args[0].success(ret)
    }, timer)
  }
  global.systemplugin.sensor.unsubscribeDeviceOrientation = function() {
    console.warn("sensor.unsubscribeDeviceOrientation interface mocked in the Previewer. How this interface works" +
      " on the Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeDevOri)
    delete this.unsubscribeDevOri
  }
}

function mockHeartRate() {
  global.systemplugin.sensor.subscribeHeartRate = function(...args) {
    console.warn("sensor.subscribeHeartRate interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeRate) {
      let ret = {}
      this.unsubscribeRate = setInterval(() => {
        ret.heartRate = Math.ceil(Math.random() * 30)
        args[0].success(ret)
      }, 500)
    }
  },
  global.systemplugin.sensor.unsubscribeHeartRate = function() {
    console.warn("sensor.unsubscribeHeartRate interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeRate)
    delete this.unsubscribeRate
  }
}

function mockLight() {
  global.systemplugin.sensor.subscribeLight = function(...args) {
    console.warn("sensor.subscribeLight interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeIntensity) {
      let ret = {}
      this.unsubscribeIntensity = setInterval(() => {
        ret.intensity = getRandomArbitrary(660, 680)
        args[0].success(ret)
      }, 500)
    }
  }
  global.systemplugin.sensor.unsubscribeLight = function() {
    console.warn("sensor.unsubscribeLight interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeIntensity)
    delete this.unsubscribeIntensity
  }
}

function mockOnBodyState() {
  global.systemplugin.sensor.subscribeOnBodyState = function(...args) {
    console.warn("sensor.subscribeOnBodyState interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeBodyState) {
      let ret = {}
      this.unsubscribeBodyState = setInterval(() => {
        ret.value = Math.ceil(Math.random() * 20)
        args[0].success(ret)
      }, 500)
    }
  }
  global.systemplugin.sensor.unsubscribeOnBodyState = function() {
    console.warn("sensor.unsubscribeOnBodyState interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeBodyState)
    delete this.unsubscribeBodyState
  }
  global.systemplugin.sensor.getOnBodyState = function (...args) {
    console.warn("sensor.getOnBodyState interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    let OnBodyStateResponseMock = {
      value: paramMock.paramBooleanMock
    }
    args[0].success(OnBodyStateResponseMock)
    hasComplete(args[0].complete)
  }
}

function mockProximity() {
  global.systemplugin.sensor.subscribeProximity = function(...args) {
    console.warn("sensor.subscribeProximity interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeDistance) {
      let ret = {}
      this.unsubscribeDistance = setInterval(() => {
        ret.distance = Math.ceil(Math.random() * 100)
        args[0].success(ret)
      }, 1000)
    }
  }
  global.systemplugin.sensor.unsubscribeProximity = function() {
    console.warn("sensor.unsubscribeProximity interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeDistance)
    delete this.unsubscribeDistance
  }
}

function mockStepCounter() {
  global.systemplugin.sensor.subscribeStepCounter = function(...args) {
    console.warn("sensor.subscribeStepCounter interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    if (!this.unsubscribeSteps) {
      let ret = {steps: 0}
      this.unsubscribeSteps = setInterval(() => {
        ret.steps += 1
        args[0].success(ret)
      }, 1000)
    }
  }
  global.systemplugin.sensor.unsubscribeStepCounter = function() {
    console.warn("sensor.unsubscribeStepCounter interface mocked in the Previewer. How this interface works on the" +
      " Previewer may be different from that on a real device.")
    clearInterval(this.unsubscribeSteps)
    delete this.unsubscribeSteps
  }
}