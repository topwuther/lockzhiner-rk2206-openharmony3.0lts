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
declare namespace util {
	/** return a formatted string using the first argument as a printf-like format. */
	/**     %s: String will be used to convert all values except BigInt, Object and -0. BigInt values will 
	be represented with an n and Objects that have no user defined toString function are inspected using
	util.inspect() with options { depth: 0, colors: false, compact: 3 }.
			%d: Number will be used to convert all values except BigInt and Symbol.
			%i: parseInt(value, 10) is used for all values except BigInt and Symbol.
			%f: parseFloat(value) is used for all values except Bigint and Symbol.
			%j: JSON. Replaced whith the string '[Circular]' if the argument contains circular references.
			%o: Object. A string representation of an object with generic JavaScript object formatting.
			Similar to util.inspect() with options { showHidden: true, showProxy: true}. This will show the
			full object including non-enumerable properties and proxies.
			%O: Object. A string representation of an object with generic JavaScript object formatting.
			Similar to util.inspect() without options. This will show the full object not including
			non-enumerable properties and proxies.
			%c: CSS. This specifier is ignored and will skip any CSS passed in.
			%%: single percent sign ('%'). This does not consume an argument.
			Returns: <string> The formatted string */
	function printf(format: string, ...args: Object[]): string;
		
	/** return the string name of a system errno*/
	function getErrorString(errno: number): string;
		
	/** Takes an async function (or a function that returns a Promise) and returns a function following the
	error-first callback style */
	function callbackWrapper(original: Function): (err: Object, value: Object) => void;
		
	/** Takes a function following the common error-first callback style, i.e taking an (err, value) => ...
	callback as the last argument, and return a version that returns promises */
	function promiseWrapper(original: (err: Object, value: Object) => void): Object;
	
	/**
     *  encoding name: support full encoding in ICU data utf-8 utf-16 iso8859 must support in all device
     */
    class TextDecoder {
        /** the source encoding's name, lowercased. */
        readonly encoding: string;
        /** Returns `true` if error mode is "fatal", and `false` otherwise. */
        readonly fatal: boolean;
        /** Returns `true` if ignore BOM flag is set, and `false` otherwise. */
        readonly ignoreBOM = false;
        constructor(
            encoding?: string,
            options?: { fatal?: boolean; ignoreBOM?: boolean },
        );
        /** Returns the result of running encoding's decoder. */
        decode(input?: ArrayBuffer | ArrayBufferView, options?: { stream?: false }): string;
    }
 /**
     * TextEncoder takes a stream of code points as input and emits a stream of UTF-8 bytes.
     */
    class TextEncoder {
        readonly encoding = "utf-8";

        constructor();

        /** Returns the result of encoder. */
        encode(input?: string): Uint8Array;

        /** encode string, write the result to dest array */
        encodeInto(
            input: string,
            dest: Uint8Array,
        ): { read: number; written: number };
    }
}

export default util;