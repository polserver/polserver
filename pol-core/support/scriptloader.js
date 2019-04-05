// @ts-check

// @ts-ignore
const vm = require("vm"),
  fs = require("fs"),
  path = require("path"),
  mod = require("module"),
  { Module } = mod;

const { print } = process._linkedBinding("basic");

// let o = Module._load;
// Module._load = function() {
//     console.log("load called with",arguments);
//     if (arguments[0] === "basicio") {
//       debugger;
//         return { print: function() { console.log.apply(console,arguments); } }
//     }
//     var x = o.apply(Module,arguments);
//     console.log("returning",x);
//     return x;
// }

let o = Module.prototype.require;
Module.prototype.require = function(id) {
  // debugger;
  if (id === "basicio") {
    // debugger;
    return {
      print: function() {
        console.log.apply(console, arguments);
      }
    };
  }
  var ret = o.apply(this, arguments);
  return ret;
};

function stripBOM(content) {
  if (content.charCodeAt(0) === 0xfeff) {
    content = content.slice(1);
  }
  return content;
}

function stripShebang(content) {
  // Remove shebang
  if (content.charAt(0) === "#" && content.charAt(1) === "!") {
    // Find end of shebang line and slice it off
    let index = content.indexOf("\n", 2);
    if (index === -1) return "";
    if (content.charAt(index - 1) === "\r") index--;
    // Note that this actually includes the newline character(s) in the
    // new output. This duplicates the behavior of the regular expression
    // that was previously used to replace the shebang line.
    content = content.slice(index);
  }
  return content;
}

// FIXME should come from the native class
const internalModules = ["basic"];

/**
 * Returns a function for the script, to be executed later via runScript().
 * @param {string} filename - Filename to load
 * @returns {vm.Script} - Compiled function
 */

function loadScript(filename) {
  // debugger;

  // let _require = new Proxy(require, {
  //   apply: function(target, thisArg, argArray) {
  //     /** @type {string} */
  //     let moduleName = argArray && argArray[0];
  //     if (internalModules.indexOf(moduleName) > -1) {
  //       console.log("Attempt to load internal module");
  //       return {
  //         print: function() {
  //           console.log.apply(console, arguments);
  //         }
  //       }; // new PolModuleWrapper(moduleName, extUoExec, filename).exports;
  //     }
  //     // @ts-ignore
  //     return Reflect.apply(...arguments);
  //   }
  // });

  // function a() {
  //   // filename = path.resolve(filename);

  // }

  //   contents = `
  // (function (exports, require, module, __filename, __dirname) {
  //   ${contents}
  // })(_module.exports, require, _module, filename, dirname);

  // if (typeof _module.exports.default === "function") {
  //   _module.exports.default.apply(undefined,__polarguments);
  // } else {
  //   print("Script " + filename + " has no default export to run...");
  //   1;
  // }`;

  /** From https://nodejs.org/api/vm.html#vm_constructor_new_vm_script_code_options
   * Creating a new vm.Script object compiles code but does not run it. The compiled vm.Script
   * can be run later multiple times. The code is not bound to any global object; rather, it is
   * bound before each run, just for that run.
   */
  // debugger;
  return new vm.Script(
    `
// debugger;
const modwrap = process._linkedBinding("modwrap");
let _module = new (require('module').Module)(this.filename, null);
let compiledWrapper = require('vm').compileFunction(
  this.contents,
  ["exports", "require", "module", "__filename", "__dirname"],
  {
    filename: this.filename,
    lineOffset: 0
  }
);

function makeRequireFunction(mod) {
  const Module = mod.constructor;

  function require(path) {
    if (path == "basicio") {
      return new (modwrap.basicio)(extUoExec, path);
    }
    return mod.require(path);
  }

  function resolve(request, options) {
    validateString(request, 'request');
    return Module._resolveFilename(request, mod, false, options);
  }

  require.resolve = resolve;

  function paths(request) {
    validateString(request, 'request');
    return Module._resolveLookupPaths(request, mod, true);
  }

  resolve.paths = paths;

  require.main = _module;

  // Enable support to add extra extension types.
  require.extensions = Module._extensions;

  require.cache = Module._cache;

  return require;
}

compiledWrapper.call(_module.exports, _module.exports, makeRequireFunction(_module), _module,
  this.filename, this.dirname);

if (typeof _module.exports.default === "function") {
  _module.exports.default.apply(undefined,this.__polarguments);
} else {
  1;
}`,
    { filename }
  );
}

/**
 *
 * @param {*} paths
 * @param {string} filename
 * @param {*} extUoExec
 * @param {vm.Script} script
 * @param {*[]} args
 * @throws
 */
function runScript(paths, extUoExec, filename, script, args) {
  //
  try {
    //debugger;
    let contents = stripShebang(stripBOM(fs.readFileSync(filename, "utf-8")));

    // let _require = Object.create(require);
    // Object.defineProperty(_require,"ext")

    return script.runInNewContext({
      extUoExec,
      __polarguments: args,
      filename,
      dirname: path.dirname(filename),
      contents,
      require: require,
      process // todo make some pol-process specific object so people can't do process.exit ;)...
    });
  } catch (e) {
    // Special logging maybe? But throw back up anyway
    throw e;
  }
}

module.exports = exports = {
  loadScript,
  runScript: runScript.bind(undefined, module.paths)
};
