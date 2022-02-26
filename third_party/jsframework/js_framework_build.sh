#!/bin/bash
# Copyright (c) 2021 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e
echo "copy source code..."
prebuilts_path="../../prebuilts"
# copy dependency file to generate dir of gn
# the params come from .gn

# $2 => node $4 => node_modules
cp -r $3 $9
cp -f $5 $9

if [ -d "$prebuilts_path" ]; then
  echo "copy node_modules..."
  if [ "${11}" == 'true' ];then
    cp -R $4 $9
  else
    cp -r $4 $9
  fi
else
  echo "download node_modules..."
  npm install
  cp -r ./node_modules ../../third_party/jsframework
fi

cp -f $6 $9
cp -f $7 $9
cp -f ${10} $9
cp -f $1 $9
cp -r $8 $9

if [ -d "$prebuilts_path" ]; then
  echo "prebuilts exists"
  # address problme of parallzing compile
  rm -rf "$9/node-v12.18.4-linux-x64"
  rm -rf "$9/node-v12.18.4-darwin-x64"
  cp -r $2 $9
  cd $9
  if [ "${11}" == 'true' ];then
    ./node-v12.18.4-darwin-x64/bin/node build.js
    # run unit test
    ./node-v12.18.4-darwin-x64/bin/node node_modules/.bin/mocha -r ts-node/register test/lib.ts test/ut/**/*.ts test/ut/*.ts
  else
    ./node-v12.18.4-linux-x64/bin/node build.js
    # run unit test
    ./node-v12.18.4-linux-x64/bin/node node_modules/.bin/mocha -r ts-node/register test/lib.ts test/ut/**/*.ts test/ut/*.ts
  fi
else
  npm run build
  # run unit test
  npm run test:unit
fi

# after running, remove dependency file
rm -rf ./node_modules
if [ "${11}" == 'true' ];then
  rm -rf ./node-v12.18.4-darwin-x64
else
  rm -rf ./node-v12.18.4-linux-x64
fi
rm -rf ./runtime
rm -rf ./tsconfig.json
rm -rf ./build.js
rm -rf ./test
rm -rf ./.eslintrc
rm -rf ./.babelrc
rm -rf ./package.json
