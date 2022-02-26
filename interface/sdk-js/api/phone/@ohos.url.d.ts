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

declare class URLSearchParams {
    constructor(init ? : string[][] | Record<string, string> | string | URLSearchParams
};

/** Appends a specified key/value pair as a new search parameter.
 */
append(name: string, value : string) : void;

/** Delete the given search parameter and its associated value,
 * from the list of all search parameters.
 */
delete(name: string) : void;

/** Returns all the values associated with a given search parameter
 * as an array.
 */
getAll(name: string) : string[];

/** Returns an iterator allowing to go through all key/value
 * pairs contained in this object.
 */
entries() : IterableIterator<[string, string]>;

/** Allows iteration through all values contained in this object via a callback function.
 */
forEach(callbackfn: (value: string, key : string, parent : this) = > void, thisArg ? : object) : void;

/** Returns the first value associated to the  given search parameter.
 */
get(name: string) : string | null;

/** Returns a Boolean that indicates whether a parameter with the
 * specified name exists.
 */
has(name: string) : boolean;

/**Sets the value associated with a given search parameter to the
 * given value.If there were several matching values, this methods
 * deletes the others. If the search parameter doesn't exist, this
 * methods creates it.
 */
set(name: string, value : string) : void;

/** Sort all key/value pairs contained in this object in place and
  * return undefined.
  */
sort() : void;

/** Returns an iterator allowing to go through all keys contained
  * in this object.
  */
keys() :IterableIterator<string>;

/** Returns an iterator allowing to go through all values contained
  * in this object.
  */
values() : IterableIterator<string>;


/** Returns an iterator allowing to go through all key/value
  * pairs contained in this object.
  */
[Symbol.iterator]() : IterableIterator<[string, string]>;

/** Returns a query string suitable for use in a URL.
   */
toString() :string;
}

/* web api URL interface */
declare class URL {
   /**    constructor of URL
   *    url:absolute URL string or a relative URL string
   *    base:base URL string
   */
    constructor(url: string, base ? : string | URL);

    /* methods */
    createObjectURL(object: object) : string;
    revokeObjectURL(url: string) : void;
    toString() : string;
    toJSON() : string;

    /*    fragment identifier of URL */
hash: string;
    /*    domain: port of URL */
host: string;
    /* domain of URL */
hostname:    string;
    /*    whole URL */
href: string;
    /*    contain the origin    of the URL, readonly */
    readonly origin : string;
    /*    password specified before the domain name */
password: string;
    /* an '/' followed by the path of the URL */
pathname: string;
    /* port of the URL */
port:    string;
    /*    the protocol scheme of the URL, readonly */
protocol:    string;
    /*    indicating the parameter string of the URL */
search: string;
    /* URLSearchParams object which used to access the individual query parameters */
    readonly searchParams : URLSearchParams;
    /* the username before the domain name */
username: string;
}

export {URL, URLSearchParams};