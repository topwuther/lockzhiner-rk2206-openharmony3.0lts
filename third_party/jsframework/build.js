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

const fs = require('fs');

const path = require('path');

const rollup = require('rollup');

const resolve = require('rollup-plugin-node-resolve');

const commonjs = require('rollup-plugin-commonjs');

const json = require('rollup-plugin-json');

const babel = require('rollup-plugin-babel');

const typescript = require('rollup-plugin-typescript2');

const { uglify } = require('rollup-plugin-uglify');

const {
  eslint
} = require('rollup-plugin-eslint');

const frameworkBanner = `var global=this; var process={env:{}}; ` + `var setTimeout=global.setTimeout;\n`;

const onwarn = warning => {
  // Silence circular dependency warning
  if (warning.code === 'CIRCULAR_DEPENDENCY') {
    return;
  }
  console.warn(`(!) ${warning.message}`);
};

const tsPlugin = typescript({
  tsconfig: path.resolve(__dirname, 'tsconfig.json'),
  check: true
});

const esPlugin = eslint({
  include: ['**/*.ts'],
  exclude: ['node_modules/**', 'lib/**']
});

const configInput = {
  input: path.resolve(__dirname, 'runtime/preparation/index.ts'),
  onwarn,
  plugins: [
    esPlugin,
    tsPlugin,
    json(),
    resolve(),
    commonjs(),
    babel({
      exclude: 'node_moduels/**'
    }),
    uglify()
  ]
};

const configJSAPIMockInput = {
  input: path.resolve(__dirname, 'runtime/main/extend/systemplugin/entry.js'),
  onwarn,
  plugins: [
    esPlugin,
    tsPlugin,
    json(),
    resolve(),
    commonjs(),
    babel({
      exclude: 'node_moduels/**'
    })
  ]
};

const configOutput = {
  file: path.resolve(__dirname, 'dist/strip.native.min.js'),
  format: 'umd',
  banner: frameworkBanner
};

const configJSAPIMockOutput = {
  file: path.resolve(__dirname, 'dist/jsMockSystemPlugin.js'),
  format: 'umd'
};

rollup.rollup(configInput).then(bundle => {
  bundle.write(configOutput).then(() => {
    countSize(configOutput.file);
  });
});

rollup.rollup(configJSAPIMockInput).then(bundle => {
  bundle.write(configJSAPIMockOutput).then(() => {
    countSize(configJSAPIMockOutput.file);
  });
});

function countSize(filePath) {
  const file = path.relative(__dirname, filePath);
  fs.stat(filePath, function(error, stats) {
    if (error) {
      console.error('file size is wrong');
    } else {
      const size = (stats.size / 1024).toFixed(2) + 'KB';
      console.log(`generate snapshot file: ${file}...\nthe snapshot file size: ${size}...`);
    }
  });
}

