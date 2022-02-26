# JSFramework

**The JSFramework is a framework for building mobile cross-platform UI written in TypeScript.**

## File Structure
- `runtime/`：all source code
  - `./main/`：JS bundle parsing, data methods, event methods, reactivity, etc.
  - `./preparation/`：JS framework initialization
  - `./utils/`：some utils
  - `./vdom/`：VM compilation
- `test/ut/`：unit test file
- `.eslintrc`：eslint configure
- `BUILD.gn`：compiling file of JS UI framework for NinjaJS
- `build.js`：build JS framework
- `js_framework_build.sh`：script file for JS framework building
- `LICENSE`：Apache License
- `NOTICE`：third party open source software notice
- `package.json`：define what libraries will be installed into node_modules when you run `npm install`
- `tsconfig.json`：the compiler options required to compile the project

## Usage

Here are simplified instructions to how to get started. The following commands are the same both on **Windows** and **Mac** platforms.

### 1. Prerequisites

Please make sure that the following commands work before trying to build:

```
> npm -v
  6.14.8
> node -v
  v12.18.3
```
Your `npm` and `node` should be of a later version. You can upgrade them to the **latest stable version**.
### 2. Installing

RollUp tool for packaging has been configured in `build.js`. So after the preceding conditions are met, we can start installing right now.

First, we go to the root directory of the project:
```
cd ..
cd third_party/jsframework/
```
And then install the dependencies:
```
npm install
```
**Note**: If some errors occur, delete the generated package `node_modules ` and run `npm install` again.

### 3. Building

There are two ways for building JS framework:
```
1. npm run build
2. in the root dir: run the build.sh script, which is built by gn
```

## ESLint

You can config more babel and ESLint plugins in `.eslintrc`.  **You'd better make sure there are no errors of esLint rules after you change them.**
