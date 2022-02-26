import { paramMock } from "../utils"

export function mockFileio() {
  const StatMock = {
    dev: '[PC preview] unknow dev',
    ino: '[PC preview] unknow ino',
    mode: '[PC preview] unknow mode',
    nlink: '[PC preview] unknow nlink',
    uid: '[PC preview] unknow uid',
    gid: '[PC preview] unknow gid',
    rdev: '[PC preview] unknow rdev',
    size: '[PC preview] unknow size',
    blocks: '[PC preview] unknow blocks',
    atime: '[PC preview] unknow atime',
    mtime: '[PC preview] unknow mtime',
    ctime: '[PC preview] unknow ctime',
    isBlockDevice: function () {
      console.warn("Stat.isBlockDevice interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isCharacterDevice: function () {
      console.warn("Stat.isCharacterDevice interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isDirectory: function () {
      console.warn("Stat.isDirectory interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isFIFO: function () {
      console.warn("Stat.isFIFO interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isFile: function () {
      console.warn("Stat.isFile interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isSocket: function () {
      console.warn("Stat.isSocket interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isSymbolicLink: function () {
      console.warn("Stat.isSymbolicLink interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramBooleanMock
    }
  };
  const DirMock = {
    close: function (...args) {
      console.warn("Dir.close interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    closeSync: function (...args) {
      console.warn("Dir.closeSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
    },
    read: function (...args) {
      console.warn("Dir.read interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, DirentMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(DirentMock);
        })
      }
    },
    readSync: function (...args) {
      console.warn("Dir.readSync interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      return DirentMock
    }
  };
  const StreamMock = {
    close: function (...args) {
      console.warn("Stream.close interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    closeSync: function (...args) {
      console.warn("Stream.closeSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
    },
    flush: function (...args) {
      console.warn("Stream.flush interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    flushSync: function (...args) {
      console.warn("Stream.flushSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
    },
    read: function (...args) {
      console.warn("Stream.read interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, ReadOutMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(ReadOutMock);
        })
      }
    },
    readSync: function (...args) {
      console.warn("Stream.readSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramNumberMock;
    },
    write: function (...args) {
      console.warn("Stream.write interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramNumberMock);
        })
      }
    },
    writeSync: function (...args) {
      console.warn("Stream.writeSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramNumberMock;
    }
  };
  const DirentMock = {
    name: "[PC Preview]: unknow name",
    isBlockDevice: function () {
      console.warn("Dirent.isBlockDevice interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isCharacterDevice: function () {
      console.warn("Dirent.isCharacterDevice interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isDirectory: function () {
      console.warn("Dirent.isDirectory interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isFIFO: function () {
      console.warn("Dirent.isFIFO interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isFile: function () {
      console.warn("Dirent.isFile interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isSocket: function () {
      console.warn("Dirent.isSocket interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramBooleanMock
    },
    isSymbolicLink: function () {
      console.warn("Dirent.isSymbolicLink interface mocked in the Previewer. How this interface works on the" +
        " Previewer may be different from that on a real device.")
      return paramMock.paramBooleanMock
    }
  };
  const ReadOutMock = {
    bytesRead: "[PC Preview]: unknow bytesRead",
    offset: "[PC Preview]: unknow offset",
    buffer: "[PC Preview]: unknow buffer"
  };
  const WatcherMock = {
    stop: function () {
      console.warn("Watcher.stop interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    }
  };
  const fileioMock = {
    Dir: DirMock,
    Dirent: DirentMock,
    Stat: StatMock,
    Stream: StreamMock,
    ReadOut: ReadOutMock,
    Watcher: WatcherMock,
    access: function (...args) {
      console.warn("fileio.access interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    accessSync: function (...args) {
      console.warn("fileio.accessSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    chmod: function (...args) {
      console.warn("fileio.chmod interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    chmodSync: function (...args) {
      console.warn("fileio.chmodSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    chown: function (...args) {
      console.warn("fileio.chown interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    chownSync: function (...args) {
      console.warn("fileio.chownSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    close: function (...args) {
      console.warn("fileio.close interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    closeSync: function (...args) {
      console.warn("fileio.closeSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    copyFile: function (...args) {
      console.warn("fileio.copyFile interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    copyFileSync: function (...args) {
      console.warn("fileio.copyFileSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    createStream: function (...args) {
      console.warn("fileio.createStream interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, StreamMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(StreamMock);
        })
      }
    },
    createStreamSync: function (...args) {
      console.warn("fileio.createStreamSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return StreamMock;
    },
    createWatcher: function (...args) {
      console.warn("fileio.createWatcher interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return WatcherMock;
      }
    },
    fchmod: function (...args) {
      console.warn("fileio.fchmod interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    fchmodSync: function (...args) {
      console.warn("fileio.fchmodSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    fchown: function (...args) {
      console.warn("fileio.fchown interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    fchownSync: function (...args) {
      console.warn("fileio.fchownSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    fdatasync: function (...args) {
      console.warn("fileio.fdatasync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    fdatasyncSync: function (...args) {
      console.warn("fileio.fdatasyncSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    fdopenStream: function (...args) {
      console.warn("fileio.fdopenStream interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, StreamMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(StreamMock);
        })
      }
    },
    fdopenStreamSync: function (...args) {
      console.warn("fileio.fdopenStreamSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return StreamMock;
    },
    fstat: function (...args) {
      console.warn("fileio.fstat interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, StatMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(StatMock);
        })
      }
    },
    fstatSync: function (...args) {
      console.warn("fileio.fstatSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return StatMock;
    },
    fsync: function (...args) {
      console.warn("fileio.fsync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    fsyncSync: function (...args) {
      console.warn("fileio.fsyncSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    ftruncate: function (...args) {
      console.warn("fileio.ftruncate interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    ftruncateSync: function (...args) {
      console.warn("fileio.ftruncateSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    hash: function (...args) {
      console.warn("fileio.hash interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    lchown: function (...args) {
      console.warn("fileio.lchown interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    lchownSync: function (...args) {
      console.warn("fileio.lchownSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    lstat: function (...args) {
      console.warn("fileio.lstat interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, StatMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(StatMock);
        })
      }
    },
    lstatSync: function (...args) {
      console.warn("fileio.lstatSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return StatMock;
    },
    mkdir: function (...args) {
      console.warn("fileio.mkdir interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    mkdirSync: function (...args) {
      console.warn("fileio.mkdirSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    mkdtemp: function (...args) {
      console.warn("fileio.mkdtemp interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    mkdtempSync: function (...args) {
      console.warn("fileio.mkdtempSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramStringMock;
    },
    open: function (...args) {
      console.warn("fileio.open interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramNumberMock);
        })
      }
    },
    openSync: function (...args) {
      console.warn("fileio.openSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramNumberMock;
    },
    opendir: function (...args) {
      console.warn("fileio.opendir interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, DirMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(DirMock);
        })
      }
    },
    opendirSync: function (...args) {
      console.warn("fileio.opendirSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return DirMock;
    },
    read: function (...args) {
      console.warn("fileio.read interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, ReadOutMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(ReadOutMock);
        })
      }
    },
    readSync: function (...args) {
      console.warn("fileio.readSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return paramMock.paramNumberMock;
    },
    readText: function (...args) {
      console.warn("fileio.readText interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramStringMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramStringMock);
        })
      }
    },
    readTextSync: function (...args) {
      console.warn("fileio.readTextSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramStringMock;
    },
    rename: function (...args) {
      console.warn("fileio.rename interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    renameSync: function (...args) {
      console.warn("fileio.renameSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    rmdir: function (...args) {
      console.warn("fileio.rmdir interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    rmdirSync: function (...args) {
      console.warn("fileio.rmdirSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    stat: function (...args) {
      console.warn("fileio.stat interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, StatMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(StatMock);
        })
      }
    },
    statSync: function (...args) {
      console.warn("fileio.statSync interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      return StatMock;
    },
    symlink: function (...args) {
      console.warn("fileio.symlink interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    symlinkSync: function (...args) {
      console.warn("fileio.symlinkSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    truncate: function (...args) {
      console.warn("fileio.truncate interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    truncateSync: function (...args) {
      console.warn("fileio.truncateSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return
    },
    unlink: function (...args) {
      console.warn("fileio.unlink interface mocked in the Previewer. How this interface works on the Previewer may" +
        " be different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock);
      } else {
        return new Promise((resolve, reject) => {
          resolve();
        })
      }
    },
    unlinkSync: function (...args) {
      console.warn("fileio.unlinkSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
    },
    write: function (...args) {
      console.warn("fileio.write interface mocked in the Previewer. How this interface works on the Previewer may be" +
        " different from that on a real device.")
      const len = args.length
      if (typeof args[len - 1] === 'function') {
        args[len - 1].call(this, paramMock.businessErrorMock, paramMock.paramNumberMock)
      } else {
        return new Promise((resolve, reject) => {
          resolve(paramMock.paramNumberMock);
        })
      }
    },
    writeSync: function (...args) {
      console.warn("fileio.writeSync interface mocked in the Previewer. How this interface works on the Previewer" +
        " may be different from that on a real device.")
      return paramMock.paramNumberMock;
    }
  }
  return fileioMock
}