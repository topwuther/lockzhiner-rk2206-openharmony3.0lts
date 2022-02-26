/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export default fileIO;
/**
 * This callback is fileIO error callback
 */
export type fileErrCallBack = (err: Error) => any;
/**
 * This callback is fileIO error callback
 */
export type fileStringCallBack = (err: Error, value: string) => any;
/**
 * This callback is fileIO stat callback
 */
export type fileStatCallBack = (err: Error, stat: Stat) => any;
/**
 * This callback is fileIO dtemp callback
 */
export type fileDtempCallBack = (err: Error, directory: string) => any;
/**
 * This callback is fileIO open callback
 */
export type fileOpenCallBack = (err: Error, fd: number) => any;
/**
 * This callback is fileIO read callback
 */
export type fileReadCallBack = (err: Error, bytesRead: number, buffer: ArrayBuffer) => any;
/**
 * This callback is fileIO write callback
 */
export type fileWriteCallBack = (err: Error, bytesWritten: number) => any;
/**
 * This callback is fileIO Dir callback
 */
export type fileDirCallBack = (err: Error, dir: Dir) => any;
/**
 * This callback is fileIO Dir callback
 */
export type fileDirentCallBack = (err: Error, dirent: Dirent) => any;
/**
 * This callback is fileIO Stream callback
 */
export type fileStreamCallBack = (err: Error, stream: Stream) => any;
/**
 * This callback is fileIO watch listener
 */
export type fileWatchFunc = (events: number) => any;
export type ReadOut = {
    bytesRead: number;
    offset: number;
    buffer: ArrayBuffer;
};
declare namespace fileIO {
    export { stat };
    export { statSync };
    export { opendir };
    export { opendirSync };
    export { createStream };
    export { createStreamSync };
    export { fdopenStream };
    export { fdopenStreamSync };
    export { access };
    export { accessSync };
    export { chown };
    export { chownSync };
    export { chmod };
    export { chmodSync };
    export { close };
    export { closeSync };
    export { copyFile };
    export { copyFileSync };
    export { fchown };
    export { fchownSync };
    export { fchmod };
    export { fchmodSync };
    export { fdatasync };
    export { fdatasyncSync };
    export { fstat };
    export { fstatSync };
    export { fsync };
    export { fsyncSync };
    export { ftruncate };
    export { ftruncateSync };
    export { lchown };
    export { lchownSync };
    export { link };
    export { linkSync };
    export { lstat };
    export { lstatSync };
    export { mkdir };
    export { mkdirSync };
    export { mkdtemp };
    export { mkdtempSync };
    export { open };
    export { openSync };
    export { readText };
    export { readTextSync };
    export { read };
    export { readSync };
    export { rename };
    export { renameSync };
    export { rmdir };
    export { rmdirSync };
    export { symlink };
    export { symlinkSync };
    export { truncate };
    export { truncateSync };
    export { unlink };
    export { unlinkSync };
    export { write };
    export { writeSync };
    export { Dir };
    export { Dirent };
    export { Stat };
    export { Stream };
    export { createWatcher };
    export { Watcher };
}
declare interface Stat {
    /**
     * @type {number}
     * @readonly
     */
    readonly dev: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly ino: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly mode: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly nlink: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly uid: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly gid: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly rdev: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly size: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly blocks: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly atime: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly mtime: number;
    /**
     * @type {number}
     * @readonly
     */
    readonly ctime: number;
    /**
     * isBlockDevice.
     * @returns {boolean} is or not
     */
    isBlockDevice(): boolean;
    /**
     * isCharacterDevice.
     * @returns {boolean} is or not
     */
    isCharacterDevice(): boolean;
    /**
     * isDirectory.
     * @returns {boolean} is or not
     */
    isDirectory(): boolean;
    /**
     * isFIFO.
     * @returns {boolean} is or not
     */
    isFIFO(): boolean;
    /**
     * isFile.
     * @returns {boolean} is or not
     */
    isFile(): boolean;
    /**
     * isSocket.
     * @returns {boolean} is or not
     */
    isSocket(): boolean;
    /**
     * isSymbolicLink.
     * @returns {boolean} is or not
     */
    isSymbolicLink(): boolean;
}
declare interface Dir {
    /**
     * read.
     *
     * @param {fileDirCallBack} [callback] - callback.
     * @returns {undefined | Promise<Dirent>} no callback return Promise otherwise return undefined
     * @throws {TypedError} Parameter check failed if read to end, Error.msg = "NoMore"
     */
    read(): Promise<Dirent>;
    read(callback: fileDirentCallBack): undefined;
    /**
     * readSync.
     *
     * @returns {Dirent} Dirent Object
     * @throws {TypedError | Error} read fail if read to end, Error.msg = "NoMore"
     */
    readSync(): Dirent;
    /**
     * closeSync.
     *
     * @param {fileErrCallBack} [callback] - callback.
     * @returns {undefined | Promise<undefined>} close success
     * @throws {TypedError | Error} close fail
     */
    close(): Promise<undefined>;
    close(callback: fileErrCallBack): undefined;
    /**
     * closeSync.
     *
     * @returns {undefined} close success
     * @throws {TypedError | Error} close fail
     */
    closeSync(): undefined;
}
declare interface Stream {
    /**
     * write.
     *
     * @param {ArrayBuffer | string} buffer - file description.
     * @param {Object} [options] - options.
     * @param {number} [options.offset = 0] - offset(bytes) ignored when buffer is string.
     * @param {number} [options.length = -1] - length(bytes) ignored when buffer is string.
     * @param {number} [options.position = -1] - position(bytes) where start to write < 0 use read, else use pread.
     * @param {string} [options.encoding = 'utf-8'] -  encoding.
     * @param {fileWriteCallBack} [callback] - callback.
     * @returns {undefined | Promise<number>} no callback return Promise otherwise return undefined
     * @throws {TypedError} Parameter check failed
     */
    write(buffer: ArrayBuffer | string, options?: {
        offset?: number;
        length?: number;
        position?: number;
        encoding?: string;
    }): Promise<number>;
    write(buffer: ArrayBuffer | string, callback: fileWriteCallBack): undefined;
    write(buffer: ArrayBuffer | string, options: {
        offset?: number;
        length?: number;
        position?: number;
        encoding?: string;
    }, callback: fileWriteCallBack): undefined;
    /**
     * writeSync.
     *
     * @param {ArrayBuffer | string} buffer - file description.
     * @param {Object} [options] - options.
     * @param {number} [options.offset = 0] - offset(bytes) ignored when buffer is string.
     * @param {number} [options.length = -1] - length(bytes) ignored when buffer is string.
     * @param {number} [options.position = -1] - position(bytes) where start to write < 0 use read, else use pread.
     * @param {string} [options.encoding = 'utf-8'] -  encoding.
     * @returns {number} on success number of bytes written
     * @throws {TypedError | Error} write fail
     */
    writeSync(buffer: ArrayBuffer | string, options?: {
        offset?: number;
        length?: number;
        position?: number;
        encoding?: string;
    }): number;
    /**
     * flush.
     *
     * @param {fileErrCallBack} [callback] - callback.
     * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
     * @throws {TypedError} Parameter check failed
     */
    flush(): Promise<undefined>;
    flush(callback: fileErrCallBack): undefined;
    /**
     * flushSync.
     *
     * @returns {undefined} flush success
     * @throws {Error} flush fail
     */
    flushSync(): undefined;
    /**
     * read.
     *
     * @param {ArrayBuffer} buffer - file description.
     * @param {Object} [options] - options.
     * @param {number} [options.offset = 0] - offset.
     * @param {number} [options.length = -1] - length.
     * @param {fileReadCallBack} [callback] - callback.
     * @returns {undefined | Promise<ReadOut>} no callback return Promise otherwise return undefined
     * @throws {TypedError} Parameter check failed
     */
    read(buffer: ArrayBuffer, options?: {
        offset?: number;
        length?: number;
    }): Promise<ReadOut>;
    read(buffer: ArrayBuffer, callback: fileReadCallBack): undefined;
    read(buffer: ArrayBuffer, options: {
        offset?: number;
        length?: number;
    }, callback: fileReadCallBack): undefined;
    /**
     * readSync.
     *
     * @param {ArrayBuffer} buffer - file description.
     * @param {Object} [options] - options.
     * @param {number} [options.offset = 0] - offset.
     * @param {number} [options.length = -1] - length.
     * @returns {number} number of bytesRead
     * @throws {TypedError | Error} read fail
     */
    readSync(buffer: ArrayBuffer, options?: {
        offset?: number;
        length?: number;
    }): number;
    /**
     * closeSync.
     *
     * @param {fileErrCallBack} [callback] - callback.
     * @returns {undefined | Promise<undefined>} close success
     * @throws {TypedError | Error} close fail
     */
    close(): Promise<undefined>;
    close(callback: fileErrCallBack): undefined;
    /**
     * closeSync.
     *
     * @returns {undefined} close success
     * @throws {TypedError | Error} close fail
     */
    closeSync(): undefined;
}

/**
 * This callback is fileIO error callback
 * @callback fileErrCallBack
 * @param {Error} err
 */
/**
 * This callback is fileIO stat callback
 * @callback fileStatCallBack
 * @param {Error} err
 * @param {Stat} stat
 */
/**
 * This callback is fileIO dtemp callback
 * @callback fileDtempCallBack
 * @param {Error} err
 * @param {string} directory
 */
/**
 * This callback is fileIO open callback
 * @callback fileOpenCallBack
 * @param {Error} err
 * @param {number} fd
 */
/**
 * This callback is fileIO read callback
 * @callback fileReadCallBack
 * @param {Error} err
 * @param {number} bytesRead
 * @param {ArrayBuffer} buffer
 */
/**
 * This callback is fileIO write callback
 * @callback fileWriteCallBack
 * @param {Error} err
 * @param {bytesWritten} bytesWritten
 */
/**
 * This callback is fileIO Dir callback
 * @callback fileDirCallBack
 * @param {Error} err
 * @param {Dir} Dir Object
 */
/**
 * This callback is fileIO Stream callback
 * @callback fileStreamCallBack
 * @param {Error} err
 * @param {Stream} Dir Object
 */
/**
 * This callback is fileIO watch listener
 * @callback fileWatchFunc
 * @param {number} events
 * @returns {Any} if return false, watch stopped
 */
/**
 * @typedef ReadOut
 * @type {object}
 * @property {number} bytesRead
 * @property {number} offset
 * @property {ArrayBuffer} buffer
 */

/**
 * stat.
 * @static
 * @param {string} path - path.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<Stat>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function stat(path: string): Promise<Stat>;
declare function stat(path: string, callback: fileErrCallBack): undefined;

/**
 * hash.
 * @static
 * @param {string} path - path.
 * @param {string} algorithm - algorithm md5 sha1 sha2-256.
 * @param {fileStringCallBack} [callback] - callback.
 * @returns {undefined | Promise<string>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function hash(path: string, algorithm: string): Promise<string>;
declare function hash(path: string, callback: fileStringCallBack): undefined;

/**
 * statSync.
 * @static
 * @param {string} path - path.
 * @returns {Stat} stat success
 * @throws {TypedError | Error} stat fail
 */
declare function statSync(path: string): Stat;
/**
 * opendir.
 *
 * @param {string} path - directory name.
 * @param {fileDirCallBack} [callback] - callback.
 * @returns {undefined | Promise<Dir>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function opendir(path: string): Promise<Dir>;
declare function opendir(path: string, callback: fileDirCallBack): undefined;
/**
 * opendirSync.
 *
 * @param {string} path - directory name.
 * @returns {Dir} opendir Dir Object
 * @throws {TypedError | Error} opendir fail
 */
declare function opendirSync(path: string): Dir;
/**
 * createStream.
 *
 * @param {string} path - path.
 * @param {string} mode - mode.
 * @param {fileStreamCallBack} [callback] - callback.
 * @returns {undefined | Promise<Stream>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function createStream(path: string, mode: string): Promise<Stream>;
declare function createStream(path: string, mode: string, callback: fileStreamCallBack): undefined;
/**
 * createStreamSync.
 *
 * @param {string} path - path.
 * @param {string} mode - mode.
 * @returns {Stream} open stream from path
 * @throws {TypedError | Error} open fail
 */
declare function createStreamSync(path: string, mode: string): Stream;
/**
 * fdopenStream.
 *
 * @param {number} fd - fd.
 * @param {string} mode - mode.
 * @param {fileStreamCallBack} [callback] - callback.
 * @returns {undefined | Promise<Stream>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fdopenStream(fd: number, mode: string): Promise<Stream>;
declare function fdopenStream(fd: number, mode: string, callback: fileStreamCallBack): undefined;
/**
 * fdopenStreamSync.
 *
 * @param {number} fd - fd.
 * @param {string} mode - mode.
 * @returns {Stream} open stream from fd
 * @throws {TypedError | Error} open fail
 */
declare function fdopenStreamSync(fd: number, mode: string): Stream;
/**
 * access.
 *
 * @function appendFile
 * @param {string} path - path.
 * @param {number} [mode = 0] - mode.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function access(path: string, mode?: number): Promise<undefined>;
declare function access(path: string, callback: fileErrCallBack): undefined;
declare function access(path: string, mode: number, callback: fileErrCallBack): undefined;
/**
 * accessSync.
 *
 * @function appendFile
 * @param {string} path - path.
 * @param {number} [mode = 0] - mode.
 * @returns {undefined} access success
 * @throws {TypedError | Error} access fail
 */
declare function accessSync(path: string, mode?: number): undefined;
/**
 * chown.
 *
 * @function appendFile
 * @param {string} path - path.
 * @param {number} uid - mode.
 * @param {number} gid - mode.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function chown(path: string, uid: number, gid: number): Promise<undefined>;
declare function chown(path: string, uid: number, gid: number, callback: fileErrCallBack): undefined;
/**
 * chownSync.
 *
 * @function appendFile
 * @param {string} path - path.
 * @param {number} uid - mode.
 * @param {number} gid - mode.
 * @returns {undefined} chown success
 * @throws {TypedError | Error} chown fail
 */
declare function chownSync(path: string, uid: number, gid: number): undefined;
/**
 * chmod.
 *
 * @function chmod
 * @param {string} path - path.
 * @param {number} mode - mode.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function chmod(path: string, mode: number): Promise<undefined>;
declare function chmod(path: string, mode: number, callback: fileErrCallBack): undefined;
/**
 * chmodSync.
 *
 * @function chmodSync
 * @param {string} path - path.
 * @param {number} mode - mode.
 * @returns {undefined} chmod success
 * @throws {TypedError | Error} chmod fail
 */
declare function chmodSync(path: string, mode: number): undefined;
/**
 * close.
 *
 * @function close
 * @param {number} fd - fd.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function close(fd: number): Promise<undefined>;
declare function close(fd: number, callback: fileErrCallBack): undefined;
/**
 * closeSync.
 *
 * @function closeSync
 * @param {number} fd - fd.
 * @returns {undefined} close success
 * @throws {TypedError | Error} close fail
 */
declare function closeSync(fd: number): undefined;
/**
 * copyFile.
 *
 * @function copyFile
 * @param {string | number} src - src.
 * @param {string | number} dest - dest.
 * @param {number} [mode = 0] - mode.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function copyFile(src: string | number, dest: string | number, mode?: number): Promise<undefined>;
declare function copyFile(src: string | number, dest: string | number, callback: fileErrCallBack): undefined;
declare function copyFile(src: string | number, dest: string | number, mode: number, callback: fileErrCallBack): undefined;
/**
 * copyFileSync.
 *
 * @function copyFileSync
 * @param {string | number} src - src.
 * @param {string | number} dest - dest.
 * @param {number} [mode = 0] - mode.
 * @returns {undefined} copyFile success
 * @throws {TypedError | Error} copyFile fail
 */
declare function copyFileSync(src: string | number, dest: string | number, mode?: number): undefined;
/**
 * fchown.
 *
 * @function fchown
 * @param {number} fd - fd.
 * @param {number} uid - uid.
 * @param {number} gid - gid.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fchown(fd: number, uid: number, gid: number): Promise<undefined>;
declare function fchown(fd: number, uid: number, gid: number, callback: fileErrCallBack): undefined;
/**
 * fchownSync.
 *
 * @function fchownSync
 * @param {number} fd - fd.
 * @param {number} uid - uid.
 * @param {number} gid - gid.
 * @returns {undefined} fchown success
 * @throws {TypedError | Error} fchown fail
 */
declare function fchownSync(fd: number, uid: number, gid: number): undefined;
/**
 * fchmod.
 *
 * @function fchmod
 * @param {number} fd - fd.
 * @param {number} mode - mode.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fchmod(fd: number, mode: number): Promise<undefined>;
declare function fchmod(fd: number, mode: number, callback: fileErrCallBack): undefined;
/**
 * fchmodSync.
 *
 * @function fchmodSync
 * @param {number} fd - fd.
 * @param {number} mode - mode.
 * @returns {undefined} fchmod success
 * @throws {TypedError | Error} fchmod fail
 */
declare function fchmodSync(fd: number, mode: number): undefined;
/**
 * fdatasync.
 *
 * @function fdatasync
 * @param {number} fd - fd.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fdatasync(fd: number): Promise<undefined>;
declare function fdatasync(fd: number, callback: fileErrCallBack): undefined;
/**
 * fdatasyncSync.
 *
 * @function fdatasyncSync
 * @param {number} fd - fd.
 * @returns {undefined} fdatasync success
 * @throws {TypedError | Error} fdatasync fail
 */
declare function fdatasyncSync(fd: number): undefined;
/**
 * fstat.
 *
 * @function fstat
 * @param {number} fd - fd.
 * @param {fileStatCallBack} [callback] - callback.
 * @returns {undefined | Promise<Stat>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fstat(fd: number): Promise<Stat>;
declare function fstat(fd: number, callback: fileStatCallBack): undefined;
/**
 * fstatSync.
 *
 * @function fstatSync
 * @param {number} fd - fd.
 * @returns {Stat}
 * @throws {TypedError | Error} fstat fail
 */
declare function fstatSync(fd: number): Stat;
/**
 * fsync.
 *
 * @function fsync
 * @param {number} fd - fd.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function fsync(fd: number): Promise<undefined>;
declare function fsync(fd: number, callback: fileErrCallBack): undefined;
/**
 * fsyncSync.
 *
 * @function fsyncSync
 * @param {number} fd - fd.
 * @returns {undefined} fsync success
 * @throws {TypedError | Error} fsync fail
 */
declare function fsyncSync(fd: number): undefined;
/**
 * ftruncate.
 *
 * @function ftruncate
 * @param {number} fd - fd.
 * @param {number} [len = 0] - len.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function ftruncate(fd: number, len?: number): Promise<undefined>;
declare function ftruncate(fd: number, callback: fileErrCallBack): undefined;
declare function ftruncate(fd: number, len: number, callback: fileErrCallBack): undefined;
/**
 * ftruncateSync.
 *
 * @function ftruncateSync
 * @param {number} fd - fd.
 * @param {number} [len = 0] - len.
 * @returns {undefined} ftruncate success
 * @throws {TypedError | Error} ftruncate fail
 */
declare function ftruncateSync(fd: number, len?: number): undefined;
/**
 * lchown.
 *
 * @function lchown
 * @param {string} path - path.
 * @param {number} uid - uid.
 * @param {number} gid - gid.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function lchown(path: string, uid: number, gid: number): Promise<undefined>;
declare function lchown(path: string, uid: number, gid: number, callback: fileErrCallBack): undefined;
/**
 * lchownSync.
 *
 * @function lchownSync
 * @param {string} path - path.
 * @param {number} uid - uid.
 * @param {number} gid - gid.
 * @returns {undefined} lchown success
 * @throws {TypedError | Error} lchown fail
 */
declare function lchownSync(path: string, uid: number, gid: number): undefined;
/**
 * link.
 *
 * @function link
 * @param {string} existingPath - existingPath.
 * @param {string} newPath - newPath.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function link(existingPath: string, newPath: string): Promise<undefined>;
declare function link(existingPath: string, newPath: string, callback: fileErrCallBack): undefined;
/**
 * linkSync.
 *
 * @function linkSync
 * @param {string} existingPath - existingPath.
 * @param {string} newPath - newPath.
 * @returns {undefined} link success
 * @throws {TypedError | Error} link fail
 */
declare function linkSync(existingPath: string, newPath: string): undefined;
/**
 * lstat.
 *
 * @function lstat
 * @param {string} path - path.
 * @param {fileStatCallBack} [callback] - callback.
 * @returns {undefined | Promise<Stat>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function lstat(path: string): Promise<Stat>;
declare function lstat(path: string, callback: fileStatCallBack): undefined;
/**
 * lstatSync.
 *
 * @function lstatSync
 * @param {string} path - path.
 * @returns {Stat} lstat success
 * @throws {TypedError | Error} lstat fail
 */
declare function lstatSync(path: string): Stat;
/**
 * mkdir.
 *
 * @function mkdir
 * @param {string} path - path.
 * @param {string} [mode = 0o775] - path.
 * @param {fileStatCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function mkdir(path: string, mode?: string): Promise<undefined>;
declare function mkdir(path: string, callback: fileStatCallBack): undefined;
declare function mkdir(path: string, mode: string, callback: fileStatCallBack): undefined;
/**
 * mkdirSync.
 *
 * @function mkdirSync
 * @param {string} path - path.
 * @param {string} [mode = 0o775] - path.
 * @returns {undefined} mkdir success
 * @throws {TypedError | Error} mkdir fail
 */
declare function mkdirSync(path: string, mode?: string): undefined;
/**
 * mkdtemp.
 *
 * @function mkdtemp
 * @param {string} prefix - dir prefix.
 * @param {fileDtempCallBack} [callback] - callback.
 * @returns {undefined | Promise<string>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function mkdtemp(prefix: string): Promise<string>;
declare function mkdtemp(prefix: string, callback: fileDtempCallBack): undefined;
/**
 * mkdtempSync.
 *
 * @function mkdtempSync
 * @param {string} prefix - dir prefix.
 * @returns {string} directory name
 * @throws {TypedError | Error} mkdtemp fail
 */
declare function mkdtempSync(prefix: string): string;
/**
 * open.
 *
 * @function open
 * @param {string} path - path.
 * @param {number} [flags = 0] - flags.
 * @param {number} [mode = 0o666] - mode.
 * @param {fileOpenCallBack} [callback] - callback.
 * @returns {undefined | Promise<number>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function open(path: string, flags?: number, mode?: number): Promise<number>;
declare function open(path: string, callback: fileOpenCallBack): undefined;
declare function open(path: string, flags: number, callback: fileOpenCallBack): undefined;
declare function open(path: string, flags: number, mode: number, callback: fileOpenCallBack): undefined;
/**
 * openSync.
 *
 * @function openSync
 * @param {string} path - path.
 * @param {number} [flags = 0] - flags.
 * @param {number} [mode = 0o666] - mode.
 * @returns {number} open fd
 * @throws {TypedError | Error} open fail
 */
declare function openSync(path: string, flags?: number, mode?: number): number;

/**
 * readText.
 *
 * @function readText
 * @param {string} filePath - file path.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset in bytes.
 * @param {number} [options.length = -1] - length in bytes.
 * @param {number} [options.encoding = 'utf-8'] - encoding.
 * @param {fileStringCallBack} [callback] - callback.
 * @returns {undefined | Promise<string>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function readText(filePath: string, options?: {
    offset?: number;
    length?: number;
    encoding?: string;
}): Promise<string>;
declare function readText(filePath: string, options?: {
    offset?: number;
    length?: number;
    encoding?: string;
}, callback: fileStringCallBack): undefined;

/**
 * readTextSync.
 *
 * @function readTextSync
 * @param {string} filePath - file path.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset in bytes.
 * @param {number} [options.length = -1] - length in bytes.
 * @param {number} [options.encoding = 'utf-8'] - encoding.
 * @returns {string} readout result
 * @throws {TypedError} Parameter check failed
 */
declare function readTextSync(filePath: string, options?: {
    offset?: number;
    length?: number;
    encoding?: string;
}): string;

/**
 * read.
 *
 * @function read
 * @param {number} fd - file description.
 * @param {ArrayBuffer} buffer - file description.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset.
 * @param {number} [options.length = -1] - length.
 * @param {number} [options.position = -1] - position.
 * @param {fileReadCallBack} [callback] - callback.
 * @returns {undefined | Promise<ReadOut>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function read(fd: number, buffer: ArrayBuffer, options?: {
    offset?: number;
    length?: number;
}): Promise<ReadOut>;
declare function read(fd: number, buffer: ArrayBuffer, callback: fileReadCallBack): undefined;
declare function read(fd: number, buffer: ArrayBuffer, options: {
    offset?: number;
    length?: number;
}, callback: fileReadCallBack): undefined;
/**
 * readSync.
 *
 * @function readSync
 * @param {number} fd - file description.
 * @param {ArrayBuffer} buffer - file description.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset.
 * @param {number} [options.length = -1] - length.
 * @param {number} [options.position = -1] - position.
 * @returns {number} number of bytesRead
 * @throws {TypedError | Error} read fail
 */
declare function readSync(fd: number, buffer: ArrayBuffer, options?: {
    offset?: number;
    length?: number;
}): number;
/**
 * rename.
 *
 * @function rename
 * @param {string} oldPath - oldPath.
 * @param {string} newPath - newPath.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function rename(oldPath: string, newPath: string): Promise<undefined>;
declare function rename(oldPath: string, newPath: string, callback: fileErrCallBack): undefined;
/**
 * renameSync.
 *
 * @function renameSync
 * @param {string} oldPath - oldPath.
 * @param {string} newPath - newPath.
 * @returns {undefined} rename success
 * @throws {TypedError | Error} rename fail
 */
declare function renameSync(oldPath: string, newPath: string): undefined;
/**
 * rmdir.
 *
 * @function rmdir
 * @param {string} path - path.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function rmdir(path: string): Promise<undefined>;
declare function rmdir(path: string, callback: fileErrCallBack): undefined;
/**
 * rmdirSync.
 *
 * @function rmdirSync
 * @param {string} path - path.
 * @returns {undefined} rmdir success
 * @throws {TypedError | Error} rmdir fail
 */
declare function rmdirSync(path: string): undefined;
/**
 * symlink.
 *
 * @function symlink
 * @param {string} target - target.
 * @param {string} srcPath - srcPath.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function symlink(target: string, srcPath: string): Promise<undefined>;
declare function symlink(target: string, srcPath: string, callback: fileErrCallBack): undefined;
/**
 * symlinkSync.
 *
 * @function symlinkSync
 * @param {string} target - target.
 * @param {string} srcPath - srcPath.
 * @returns {undefined} symlink success
 * @throws {TypedError | Error} symlink fail
 */
declare function symlinkSync(target: string, srcPath: string): undefined;
/**
 * truncate.
 *
 * @function truncate
 * @param {string} path - path.
 * @param {number} [len = 0] - len.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function truncate(path: string, len?: number): Promise<undefined>;
declare function truncate(path: string, callback: fileErrCallBack): undefined;
declare function truncate(path: string, len: number, callback: fileErrCallBack): undefined;
/**
 * truncateSync.
 *
 * @function truncateSync
 * @param {string} path - path.
 * @param {number} [len = 0] - len.
 * @returns {undefined} truncate success
 * @throws {TypedError | Error} truncate fail
 */
declare function truncateSync(path: string, len?: number): undefined;
/**
 * unlink.
 *
 * @function unlink
 * @param {string} path - path.
 * @param {fileErrCallBack} [callback] - callback.
 * @returns {undefined | Promise<undefined>} no callback return Promise otherwise return undefined
 * @throws {TypedError} Parameter check failed
 */
declare function unlink(path: string): Promise<undefined>;
declare function unlink(path: string, callback: fileErrCallBack): undefined;
/**
 * unlinkSync.
 *
 * @function unlinkSync
 * @param {string} path - path.
 * @returns {undefined} unlink success
 * @throws {TypedError | Error} unlink fail
 */
declare function unlinkSync(path: string): undefined;
/**
 * write.
 *
 * @function write
 * @param {number} fd - file description.
 * @param {ArrayBuffer | string} buffer - file description.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset(bytes) ignored when buffer is string.
 * @param {number} [options.length = -1] - length(bytes) ignored when buffer is string.
 * @param {number} [options.position = -1] - position(bytes) where start to write < 0 use read, else use pread.
 * @param {string} [options.encoding = 'utf-8'] - encoding.
 * @param {fileWriteCallBack} [callback] - callback.
 * @returns {undefined | Promise<number>} no callback return Promise otherwise return undefined
 * @throws {TypedError | RangeError} Parameter check failed
 */
declare function write(fd: number, buffer: ArrayBuffer | string, options?: {
    offset?: number;
    length?: number;
    position?: number;
    encoding?: string;
}): Promise<number>;
declare function write(fd: number, buffer: ArrayBuffer | string, callback: fileWriteCallBack): undefined;
declare function write(fd: number, buffer: ArrayBuffer | string, options: {
    offset?: number;
    length?: number;
    position?: number;
    encoding?: string;
}, callback: fileWriteCallBack): undefined;
/**
 * writeSync.
 *
 * @function writeSync
 * @param {number} fd - file description.
 * @param {ArrayBuffer | string} buffer - file description.
 * @param {Object} [options] - options.
 * @param {number} [options.offset = 0] - offset(bytes) ignored when buffer is string.
 * @param {number} [options.length = -1] - length(bytes) ignored when buffer is string.
 * @param {number} [options.position = -1] - position(bytes) where start to write < 0 use read, else use pread.
 * @param {string} [options.encoding = 'utf-8'] -  encoding.
 * @returns {number} on success number of bytesRead
 * @throws {TypedError | RangeError | Error} write fail
 */
declare function writeSync(fd: number, buffer: ArrayBuffer | string, options?: {
    offset?: number;
    length?: number;
    position?: number;
    encoding?: string;
}): number;
/**
 * watch.
 *
 * @param {string} filename - filename.
 * @param {number} events - events(depends on OS & filesystem).
 * @param {fileWatchFunc} listener - listener.
 * @returns {Watcher} watch success
 * @throws {TypedError | Error} watch fail
 */
declare function createWatcher(filename: string, events: number, listener: fileWatchFunc): Watcher;

declare interface Dirent {
    /**
     * @type {string}
     * @readonly
     */
    readonly name: string;
    /**
     * isBlockDevice.
     * @returns {boolean} is or not
     */
    isBlockDevice(): boolean;
    /**
     * isCharacterDevice.
     * @returns {boolean} is or not
     */
    isCharacterDevice(): boolean;
    /**
     * isDirectory.
     * @returns {boolean} is or not
     */
    isDirectory(): boolean;
    /**
     * isFIFO.
     * @returns {boolean} is or not
     */
    isFIFO(): boolean;
    /**
     * isFile.
     * @returns {boolean} is or not
     */
    isFile(): boolean;
    /**
     * isSocket.
     * @returns {boolean} is or not
     */
    isSocket(): boolean;
    /**
     * isSymbolicLink.
     * @returns {boolean} is or not
     */
    isSymbolicLink(): boolean;
}
declare interface Watcher {
    /**
     * stop
     *
     * @returns {undefined}
     */
    stop(): undefined;
}
