import { paramMock } from "./utils"

export function mockMultimediaImage() {
  global.systemplugin.multimedia = {}
  const sizeMock = {
    height: "[PC Preview] unknow height",
    width: "[PC Preview] unknow width"
  }
  const imageInfoMock = {
    size: sizeMock,
    pixelFortmat: "[PC Preview] unknow pixelFortmat",
    colorSpace: "[PC Preview] unknow colorSpace",
    alphaType: "[PC Preview] unknow alphaType"
  }
  const imageSourceMock = {
    getImageInfo: function (...args) {
      console.warn("ImageSource.getImageInfo interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, imageInfoMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(imageInfoMock);
        })
      }
    },
    release: function (...args) {
      console.warn("ImageSource.release interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    supportedFormats: "[PC Preview] unknow supportedFormats"
  }
  const imagePackerMock = {
    packing: function (...args) {
      console.warn("ImagePacker.packing interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramArrayMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramArrayMock);
        })
      }
    },
    release: function (...args) {
      console.warn("ImagePacker.release interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    supportedFormats: "[PC Preview] unknow supportedFormats"
  }
  global.systemplugin.multimedia.image = {
    createImageSource: function () {
      console.warn("multimedia.image.createImageSource interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      return imageSourceMock;
    },
    createImagePacker: function () {
      console.warn("multimedia.image.createImagePacker interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      return imagePackerMock;
    }
  }
}

export function mockMultimediaMedia() {
  const audioPlayerMock = {
    play: function () {
      console.warn("AudioPlayer.play interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    pause: function () {
      console.warn("AudioPlayer.pause interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    stop: function () {
      console.warn("AudioPlayer.stop interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    seek: function (...args) {
      console.warn("AudioPlayer.seek interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    setVolume: function (...args) {
      console.warn("AudioPlayer.setVolume interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return
    },
    release: function () {
      console.warn("AudioPlayer.release interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    src: "[PC Preview] unknow src",
    loop: "[PC Preview] unknow loop",
    currentTime: "[PC Preview] unknow currentTime",
    duration: "[PC Preview] unknow duration",
    state: "[PC Preview] unknow state",
    on: function (...args) {
      console.warn("AudioPlayer.on interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        if (args[0] == 'error') {
          args[len - 1].call(this, paramMock.businessErrorMock);
        } else if (args[0] == 'timeUpdate') {
          args[len - 1].call(this, paramMock.paramNumberMock);
        } else {
          args[len - 1].call(this);
        }
      }
    }
  }
  const audioRecorderMock = {
    prepare: function () {
      console.warn("AudioRecorder.prepare interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    start: function () {
      console.warn("AudioRecorder.start interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    pause: function () {
      console.warn("AudioRecorder.pause interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    resume: function () {
      console.warn("AudioRecorder.resume interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    stop: function () {
      console.warn("AudioRecorder.stop interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    release: function () {
      console.warn("AudioRecorder.release interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
    },
    reset: function () {
      console.warn("AudioRecorder.reset interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    on: function (...args) {
      console.warn("AudioRecorder.on interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        if (args[0] == 'error') {
          args[len - 1].call(this, paramMock.businessErrorMock);
        } else {
          args[len - 1].call(this);
        }
      }
    }
  }
  global.systemplugin.multimedia.media = {
    createAudioPlayer: function () {
      console.warn("multimedia.media.createAudioPlayer interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      return audioPlayerMock;
    },
    createAudioRecorder: function () {
      console.warn("multimedia.media.createAudioRecorder interface mocked in the Previewer. How this interface works" +
        " on the Previewer may be different from that on a real device.")
      return audioRecorderMock;
    }
  }
}

export function mockMultimediaMedialibrary() {
  const MediaLibraryMock = {
    storeMediaAsset: function (...args) {
      console.warn("MediaLibrary.storeMediaAsset interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    startImagePreview: function (...args) {
      console.warn("MediaLibrary.startImagePreview interface mocked in the Previewer. How this interface works on" +
        " the Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    startMediaSelect: function (...args) {
      console.warn("MediaLibrary.startMediaSelect interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramArrayMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramArrayMock);
        })
      }
    }
  }
  global.systemplugin.multimedia.mediaLibrary = {
    getMediaLibrary: function () {
      console.warn("multimedia.mediaLibrary.getMediaLibrary interface mocked in the Previewer. How this interface" +
        " works on the Previewer may be different from that on a real device.")
      return MediaLibraryMock;
    }
  }
}