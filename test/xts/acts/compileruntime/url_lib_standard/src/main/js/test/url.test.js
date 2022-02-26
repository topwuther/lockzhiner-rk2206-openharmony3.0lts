/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/lite'
import  Url  from '@ohos.url'

describe('Url_mxa_jk_Test', function () {
    it('Url_append_test_001', 0, function () {
        var that = new Url.URL('http://username:password@host:8080/directory/file?foo=1&bar=2')
        var params = new Url.URLSearchParams(that.search)
        params.append('ma', 'jk')
        var result = params.toString()
        expect(result).assertEqual('foo=1&bar=2&ma=jk')
    })
    it('Url_append_test_002', 0, function () {
        let that = new Url.URL('https://example.com?foo=1&bar=2')
        let params = new Url.URLSearchParams(that.search)
        params.append('ma 大', 'jk￥')
        var result = params.toString()
        expect(result).assertEqual('foo=1&bar=2&ma+%E5%A4%A7=jk%EF%BF%A5')
    })
    it('Url_append_test_003', 0, function () {
        let that = new Url.URL('https://example.com?foo=1&bar=2')
        let params = new Url.URLSearchParams(that.search)
        params.append('foo~!@#$%^&*()_+-=', 'jk')
        var result = params.toString()
        expect(result).assertEqual('foo=1&bar=2&foo%7E%21%40%23%24%25%5E%26*%28%29_%2B-%3D=jk')
    })
    it('Url_delete_test_001', 0, function () {
        let that = new Url.URL('https://example.com?foo=1&bar=2')
        let params = new Url.URLSearchParams(that.search)
        params.delete('foo')
        var result = params.toString()
        expect(result).assertEqual('bar=2')
    })
    it('Url_delete_test_002', 0, function () {
        let that = new Url.URL('https://example.com?foo大=1&bar=2')
        let params = new Url.URLSearchParams(that.search)
        params.delete('foo')
        var result = params.toString()
        expect(result).assertEqual('foo%E5%A4%A7=1&bar=2')
    })
    it('Url_delete_test_003', 0, function () {
        let that = new Url.URL('https://example.com?foo大=1&bar=2')
        let params = new Url.URLSearchParams(that.search)
        params.delete('foo大')
        var result = params.toString()
        expect(result).assertEqual('bar=2')
    })

    it('Url_entries_test_001', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var i = 0;
        var arr = {};
        for(var pair of params.entries()) {
            arr[i] = pair[0];
            i++;
            arr[i] = pair[1];
            i++;
        }
        expect(arr[1]).assertEqual('value1')
    })
    it('Url_entries_test_002', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var i = 0;
        var arr = {};
        for(var pair of params.entries()) {
            arr[i] = pair[0];
            i++;
            arr[i] = pair[1];
            i++;
        }
        expect(arr[2]).assertEqual('key2')
    })
    it('Url_entries_test_003', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        params.append('foo', 'jk')
        var i = 0;
        var arr = {};
        for(var pair of params.entries()) {
            arr[i] = pair[0];
            i++;
            arr[i] = pair[1];
            i++;
        }
        expect(arr[5]).assertEqual('jk')
    })

    it('Url_forEach_test_001', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var arr = {};
        var i = 0;
        function func(str1, str2, params) {
            arr[i] = 'key=' + str1 + ' ' + 'value=' + str2 + ' ' + 'flag=' + params
            i++;
        }
        params.forEach(func)
        expect(arr[0]).assertEqual('key=key1 value=value1 flag=key1=value1&key2=value2')
    })
    it('Url_forEach_test_002', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var arr = {};
        var i = 0;
        function func(str1, str2, params) {
            arr[i] = 'key=' + str1 + ' ' + 'value=' + str2 + ' ' + 'flag=' + params
            i++;
        }
        params.forEach(func)
        expect(arr[1]).assertEqual('key=key2 value=value2 flag=key1=value1&key2=value2')
    })
    it('Url_forEach_test_003', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        params.append('foo', 'jk')
        var arr = {};
        var i = 0;
        function func(str1, str2, params) {
            arr[i] = 'key=' + str1 + ' ' + 'value=' + str2 + ' ' + 'flag=' + params
            i++;
        }
        params.forEach(func)
        expect(arr[2]).assertEqual('key=foo value=jk flag=key1=value1&key2=value2&foo=jk')
    })
    it('Url_get_test_001', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var result = params.get('1')
        expect(result).assertEqual(undefined)
    })
    it('Url_get_test_002', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        var result = params.get('key2')
        expect(result).assertEqual('value2')
    })
    it('Url_get_test_003', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        params.append('5', 'JKL')
        var result = params.get('5')
        expect(result).assertEqual('JKL')
    })
    it('Url_getAll_test_001', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2')
        params.append('key1', 'AAA')
        var result = params.getAll('key1')
        expect(result).assertEqual('value1,AAA')
    })
    it('Url_getAll_test_002', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&8=DEF')
        params.append('8', 'A8A')
        var result =  params.getAll('8')
        expect(result).assertEqual('DEF,A8A')
    })
    it('Url_getAll_test_003', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2&key3=大')
        params.append('key3', 'A3A')
        var result = params.getAll('key3')
        expect(result).assertEqual('大,A3A')
    })
    it('Url_has_test_001', 0, function () {
        let params = new Url.URLSearchParams('key1=value1&key2=value2&key3=大')
        var result = params.has('2')
        expect(result).assertEqual(false)
    })
    it('Url_has_test_002', 0, function () {
        let params = new Url.URLSearchParams('小=value1&key2=value2&key3=大')
        var result = params.has('小')
        expect(result).assertEqual(true)
    })
    it('Url_has_test_003', 0, function () {
        let params = new Url.URLSearchParams('小=value1&￥=value2&key3=大')
        params.append('￥', 'ACA')
        var result = params.has('￥')
        expect(result).assertEqual(true)
    })
    it('Url_keys_test_001', 0, function () {
        let params = new Url.URLSearchParams('小=value1&￥=value2&key3=大')
        var arr = {};
        var i = 0;
        for (var key of params.keys()) {
            arr[i] = key;
            i++;
        }
        expect(arr[0]).assertEqual('小')
    })
    it('Url_keys_test_002', 0, function () {
        let params = new Url.URLSearchParams('小=value1&￥=value2&key3=大')
        var arr = {};
        var i = 0;
        for (var key of params.keys()) {
            arr[i] = key;
            i++;
        }
        expect(arr[1]).assertEqual('￥')
    })
    it('Url_keys_test_003', 0, function () {
        let params = new Url.URLSearchParams('小=value1&￥=value2&key3=大')
        var arr = {};
        var i = 0;
        for (var key of params.keys()) {
            arr[i] = key;
            i++;
        }
        expect(arr[2]).assertEqual('key3')
    })
    it('Url_set_test_001', 0, function () {
        let params = new Url.URLSearchParams('1=value1&2=value2&key3=3')
        params.set('11', 'CCC')
        var res = params.toString()
        expect(res).assertEqual('1=value1&2=value2&key3=3&11=CCC')
    })
    it('Url_set_test_002', 0, function () {
        let params = new Url.URLSearchParams('1=value1&2=value2&key3=3')
        params.set('10', 'BBB')
        var res = params.toString()
        expect(res).assertEqual('1=value1&2=value2&key3=3&10=BBB')
    })
    it('Url_set_test_003', 0, function () {
        let params = new Url.URLSearchParams('1=value1&2=value2&key3=3')
        params.set('ma 大', '10￥')
        var res = params.toString()
        expect(res).assertEqual('1=value1&2=value2&key3=3&ma+%E5%A4%A7=10%EF%BF%A5')
    })
    it('Url_sort_test_001', 0, function () {
        let params = new Url.URLSearchParams('1=value1&3=value3&2=key2')
        params.sort()
        var res = params.toString()
        expect(res).assertEqual('1=value1&2=key2&3=value3')
    })
    it('Url_sort_test_002', 0, function () {
        let params = new Url.URLSearchParams('a=value1&c=value2&b=key2')
        params.sort()
        var res = params.toString()
        expect(res).assertEqual('a=value1&b=key2&c=value2')
    })
    it('Url_sort_test_003', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b=33')
        params.append('a', 'ACA')
        params.sort()
        var res = params.toString()
        expect(res).assertEqual('a=ACA&b=33&c=value2&d=value1')
    })
    it('Url_Values_test_001', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b=大')
        var arr = {};
        var i = 0;
        for (var value of params.values()) {
            arr[i] = value;
            i++;
        }
        expect(arr[0]).assertEqual('value1')
    })
    it('Url_Values_test_002', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b=大')
        var arr = {};
        var i = 0;
        for (var value of params.values()) {
            arr[i] = value;
            i++;
        }
        expect(arr[1]).assertEqual('value2')
    })
    it('Url_Values_test_003', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b=大')
        params.append('a', 'ACA')
        var arr = {};
        var i = 0;
        for (var value of params.values()) {
            arr[i] = value;
            i++;
        }
        expect(arr[3]).assertEqual('ACA')
    })
    it('Url_toString_test_001', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b=大')
        var result = params.toString()
        expect(result).assertEqual('d=value1&c=value2&b=%E5%A4%A7')
    })
    it('Url_toString_test_002', 0, function () {
        let params = new Url.URLSearchParams('d=value1&c=value2&b= 大')
        params.append('1 12', 'QQQ')
        var result = params.toString()
        expect(result).assertEqual('d=value1&c=value2&b=+%E5%A4%A7&1+12=QQQ')
    })
    it('Url_toString_test_003', 0, function () {
        let params = new Url.URLSearchParams('￥=)')
        params.delete('5')
        var result = params.toString()
        expect(result).assertEqual('%EF%BF%A5=%29')
    })
  //new url  SearchParams
    it('Url_construction_test_001', 0, function () {
        let params = new Url.URLSearchParams('?user=abc&query=xyz')
        var result = params.toString()
        expect(result).assertEqual('user=abc&query=xyz')
    })
    it('Url_construction_test_002', 0, function () {
        let params = new Url.URLSearchParams({
            user: 'abc',
            query: ['first', 'second']
        })
        var result = params.toString()
        expect(result).assertEqual('user=abc&query=first%2Csecond')
    })
    it('Url_construction_test_003', 0, function () {
        let params = new Url.URLSearchParams([
            ['user', 'abc'],
            ['query', 'first'],
            ['query', 'second'],
        ])
        var result = params.toString()
        expect(result).assertEqual('user=abc&query=first&query=second')
    })
    it('Url_construction_test_004', 0, function () {
        const map = new Map()
        map.set('user', 'abc')
        map.set('query', 'xyz')
        let params = new Url.URLSearchParams(map)
        var result = params.toString()
        expect(result).assertEqual('user=abc&query=xyz')
    })
    it('Url_construction_test_004', 0, function () {
        function* getQueryPairs() {
            yield ['user', 'abc']
            yield ['query', 'first']
            yield ['query', 'second']
        }
        let params = new Url.URLSearchParams(getQueryPairs())
        var result = params.toString()
        expect(result).assertEqual('user=abc&query=first&query=second')
    })
    //new url - get
    it('Url_to_string_test_001', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.to_string()
        expect(result).assertEqual('http://username:password@host:8080/directory/file?query#fragment')
    })

    it('Url_href_test_002', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.href
        expect(result).assertEqual('http://username:password@host:8080/directory/file?query#fragment')
    })
    it('Url_origin_test_003', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.origin
        expect(result).assertEqual('http://host:8080')
    })
    it('Url_protocol_test_004', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.protocol
        expect(result).assertEqual('http:')
    })
    it('Url_username_test_005', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.username
        expect(result).assertEqual('username')
    })
    it('Url_password_test_006', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.password
        expect(result).assertEqual('password')
    })
    it('Url_host_test_007', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.host
        expect(result).assertEqual('host:8080')
    })
    it('Url_hostname_test_008', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.hostname
        expect(result).assertEqual('host')
    })
    it('Url_port_test_009', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.port
        expect(result).assertEqual('8080')
    })
    it('Url_pathname_test_010', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        var result = params.pathname
        expect(result).assertEqual('/directory/file')
    })
    it('Url_search_test_011', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=qqqq#fragment')
        var result = params.search
        expect(result).assertEqual('?query=qqqq')
    })
    it('Url_search_test_012', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=pppppp#fragment')
        var result = params.search
        expect(result).assertEqual('?query=pppppp')
    })
    it('Url_hash_test_013', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=pppppp#fragment')
        var result = params.hash
        expect(result).assertEqual('#fragment')
    })
    //new url - set
    it('Url_href_test_001', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.href = 'http://[::192.9.5.5]/ipng'
        var result = params.href
        expect(result).assertEqual('http://[::c009:505]/ipng')
    })
    it('Url_protocol_test_002', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.protocol = 'https:'
        var result = params.href
        expect(result).assertEqual('https://username:password@host:8080/directory/file?query#fragment')
    })
    it('Url_username_test_003', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.username = 'username1'
        var result = params.href
        expect(result).assertEqual('http://username1:password@host:8080/directory/file?query#fragment')
    })
    it('Url_password_test_004', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.password = 'passwd'
        var result = params.href
        expect(result).assertEqual('http://username:passwd@host:8080/directory/file?query#fragment')
    })

    it('Url_host_test_005', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.host = 'www.baidu.com:99'
        var result = params.href
        expect(result).assertEqual('http://username:password@www.baidu.com:99/directory/file?query#fragment')
    })

    it('Url_hostname_test_006', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.hostname = 'www.baidu.com'
        var result = params.href
        expect(result).assertEqual('http://username:password@www.baidu.com:8080/directory/file?query#fragment')
    })
    it('Url_port_test_007', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.port = '99'
        var result = params.href
        expect(result).assertEqual('http://username:password@host:99/directory/file?query#fragment')
    })
    it('Url_pathname_test_008', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query#fragment')
        params.pathname = '/path/ddd'
        var result = params.href
        expect(result).assertEqual('http://username:password@host:8080/path/ddd?query#fragment')
    })
    it('Url_search_test_009', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=qqqq#fragment')
        params.search = '?pppppp=1234'
        var result = params.href
        expect(result).assertEqual('http://username:password@host:8080/directory/file?pppppp=1234#fragment')
    })
    it('Url_hash_test_010', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=pppppp#fragment')
        params.hash = '#qwer=da'
        var result = params.href
        expect(result).assertEqual('http://username:password@host:8080/directory/file?query=pppppp#qwer=da')
    })
    //new url JSON
    it('Url_toJSON_test_001', 0, function () {
        let params = new Url.URL('http://username:password@host:8080/directory/file?query=pppppp#qwer=da')
        var result = params.toJSON()
        expect(result).assertEqual('http://username:password@host:8080/directory/file?query=pppppp#qwer=da')
    })
    //new url  IPV6
    it('Url_IPV6_test_001', 0, function () {
        let params = new Url.URL('http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html')
        var result = params.href
        expect(result).assertEqual('http://[fedc:ba98:7654:3210:fedc:ba98:7654:3210]/index.html')
    })
    it('Url_IPV6_test_002', 0, function () {
        let params = new Url.URL('http://[1080:0:0:0:8:800:200C:417A]/index.html')
        var result = params.href
        expect(result).assertEqual('http://[1080::8:800:200c:417a]/index.html')
    })
    it('Url_IPV6_test_003', 0, function () {
        let params = new Url.URL('http://[::FFFF:129.144.52.38]:80/index.html')
        var result = params.href
        expect(result).assertEqual('http://[::ffff:8190:3426]/index.html')
    })
    //new url  IPV4
    it('Url_IPV4_test_001', 0, function () {
        let params = new Url.URL('http://0377.0xff.255.1:80/index.html')
        var result = params.href
        expect(result).assertEqual('http://255.255.255.1/index.html')
    })
    it('Url_IPV4_test_002', 0, function () {
        let params = new Url.URL('http://0377.0xff.255.g/index.html')
        var result = params.href
        expect(result).assertEqual('http://0377.0xff.255.g/index.html')
    })
    it('Url_IPV4_test_003', 0, function () {
        let params = new Url.URL('http://190.254.245.9:80/index.html')
        var result = params.href
        expect(result).assertEqual('http://190.254.245.9/index.html')
    })
    //new url  url_base
    it('Url_base_test_001', 0, function () {
        let params = new Url.URL('https://developer.mozilla.org',)
        var result = params.href
        expect(result).assertEqual('https://developer.mozilla.org/')
    })
    it('Url_base_test_002', 0, function () {
        let params = new Url.URL('https://developer.mozilla.org', 'flie:/developer.mozilla.org')
        var result = params.href
        expect(result).assertEqual('https://developer.mozilla.org/')
    })
    it('Url_base_test_003', 0, function () {
        let params = new Url.URL('https://developer.mozilla.org', 'ftp://www.example.com')
        var result = params.href
        expect(result).assertEqual('https://developer.mozilla.org/')
    })
    it('Url_base_test_004', 0, function () {
        let params = new Url.URL(' ', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/')
    })
    it('Url_base_test_005', 0, function () {
        let params = new Url.URL('.', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/')
    })
    it('Url_base_test_006', 0, function () {
        let params = new Url.URL('../h:', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/h:')
    })
    it('Url_base_test_007', 0, function () {
        let params = new Url.URL('/sca/./path/path/../scasa/jjjjj', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/sca/path/scasa/jjjjj')
    })
    it('Url_base_test_008', 0, function () {
        let params = new Url.URL('sca/./path/path/../scasa/jjjjj', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/sca/path/scasa/jjjjj')
    })
    it('Url_base_test_009', 0, function () {
        let params = new Url.URL('/../sca/./path/path/../scasa/jjjjj', 'http://www.example.com')
        var result = params.href
        expect(result).assertEqual('http://www.example.com/sca/path/scasa/jjjjj')
    })
    it('Url_base_test_010', 0, function () {
        let params = new Url.URL('/../sca/./path/path/../scasa/jjjjj', 'file://www.example.com')
        var result = params.href
        expect(result).assertEqual('file://www.example.com/sca/path/scasa/jjjjj')
    })
    it('Url_base_test_011', 0, function () {
        let params = new Url.URL('/../sca/./path/path/../scasa/jjjjj', 'file1://www.example.com')
        var result = params.href
        expect(result).assertEqual('file1://www.example.com/sca/path/scasa/jjjjj')
    })
})