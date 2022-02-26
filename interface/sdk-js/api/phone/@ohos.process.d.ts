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

declare namespace process {

    export interface ChildProcess {
        readonly pid: number;
        readonly ppid: number;
        readonly exitCode: number;
        readonly killed: boolean;

        wait(): Promise<number>;
        /**
        * Buffer the stdout until EOF and return it as 'Uint8Array'
        */
        getOutput(): Promise<Uint8Array>;
        /**
        * Buffer the stderr until EOF and return it as 'Uint8Array'
        */
        getErrorOutput(): Promise<Uint8Array>;
        /**
        * close the target process
        */
        close(): void;
        /**
        * send a signal to process
        */
        kill(signo: number): void;
    }
    /**
    * spawns a new ChildProcess to run the command
    */
    function runCmd(command: string, options?: RunOptions): ChildProcess;

    function getPid(): number;
    function getPpid(): number;

    /**
    *abort current process
    *@return void
    */
    function abort(): void;

    function on(type: string, listener: EventListener): void;
    function exit(code:number): void;

    /**
    *get current work directory;
    */
    function cwd(): string;
    
    /**
    *change current  directory
    *@param dir
    */
    function chdir(dir: string): void;

    function getEgid(): number;
    function getEuid(): number;

    function getGid(): number;
    function getUid(): number;
    function uptime(): number;

    function getGroups(): number[];
    function kill(signal: number, pid: number): boolean;
    function off(type: string): boolean;
}
export default process;