// @ts-check

// @ts-ignore
const vm = require("vm"),
  fs = require("fs"),
  path = require("path"),
  mod = require("module"),
  { Module } = mod;

const modules = require("./modules");

/**
 * We create a hook into the Module instance method `require` to construct our modwrap objects
 * if appropriate.
 */
let o = Module.prototype.require;
Module.prototype.require = function(id) {
  const { extUoExec } = this;
  if (
    extUoExec &&
    typeof id === "string" &&
    typeof modules[id] === "function"
  ) {
    /**
     * Construct a new instance of NodeModuleWrap<module>
     */
    const mod = new modules[id](extUoExec);

    /**
     * The object above does not have its own properties for the functions. That means,
     * calling `BasicIo.print("")` would work, but `const { print } = BasicIo; print("")`
     * would not. We'll copy them over here. This could also be done in the C++ constructor
     * for NodeModuleWrap<PolModule>.
     */
    // Reflect.ownKeys(Reflect.getPrototypeOf(mod)).forEach(func => {
    //   if (func != "constructor") {
    //     Object.defineProperty(mod, func, {
    //       value: mod[func].bind(mod)
    //     });
    //   }
    // });
    return mod;
  }

  // Return the original require() method
  return o.apply(this, arguments);
};

// Taken from nodejs lib/internal/modules/cjs/helpers.js
function stripBOM(content) {
  if (content.charCodeAt(0) === 0xfeff) {
    content = content.slice(1);
  }
  return content;
}

// Taken from nodejs lib/internal/modules/cjs/helpers.js
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

// Taken from nodejs lib/internal/modules/cjs/helpers.js
function makeRequireFunction(mod) {
  const Module = mod.constructor;

  function require(path) {
    return mod.require(path);
  }

  function resolve(request, options) {
    return Module._resolveFilename(request, mod, false, options);
  }

  require.resolve = resolve;

  function paths(request) {
    return Module._resolveLookupPaths(request, mod, true);
  }

  resolve.paths = paths;

  require.main = mod;

  // Enable support to add extra extension types.
  require.extensions = Module._extensions;

  require.cache = Module._cache;

  return require;
}

/**
 * Returns a function for the script, to be executed later via runScript().
 * @param {string} filename - Filename to load
 * @returns {vm.Script} - Compiled function
 */

function loadScript(filename) {
  /** From https://nodejs.org/api/vm.html#vm_constructor_new_vm_script_code_options
   * Creating a new vm.Script object compiles code but does not run it. The compiled vm.Script
   * can be run later multiple times. The code is not bound to any global object; rather, it is
   * bound before each run, just for that run.
   */

  // TODO return script and contents to core.
  // debugger;
  return new vm.Script(
    `
  // debugger;
// 'this' is the context created from script.runInNewContext
const { contents, module , __filename, __dirname, __polarguments } = this;
let compiledWrapper = require('vm').compileFunction(
  contents,
  ["exports", "require", "module", "__filename", "__dirname"],
  {
    filename: __filename,
    lineOffset: 0
  }
);
compiledWrapper.call(module.exports, module.exports, _require, module,
  __filename, __dirname);

if (typeof module.exports.default === "function") {
  module.exports.default.apply(undefined,__polarguments);
} else {
  1;
}`,
    { filename }
  );
}

const { EventEmitter } = require("events");

/**
 *
 * @param {*} paths
 * @param {*} extUoExec
 * @param {string} filename
 * @param {vm.Script} script
 * @param {*[]} args - Arguments sent to script's default exported function. These
 * are converted BObjectImps from the core, either directly to native JS values (number,
 * string) or a NodeObjectWrap object.
 * @throws
 */
function runScript(extUoExec, filename, script, args) {
  // debugger;
  try {
    // TODO read script _as well as_ contents from core because right now it will read the file for each call, uh oh
    let contents = stripShebang(stripBOM(fs.readFileSync(filename, "utf-8")));

    // Create a new module to load
    // TODO get full filename..
    let _module = new (require("module")).Module(this.filename, null);

    // We're going to pseudo-inherit EventEmitter, so we can do module.emit() / module.on()
    for (const key in EventEmitter.prototype) {
      Object.defineProperty(_module, key, {
        value: EventEmitter.prototype[key],
        writable: true
      });
    }
    EventEmitter.call(_module);

    _module.extUoExec = extUoExec;
    _module.require = _module.require.bind(_module);

    return {
        module: _module,
        value: script.runInNewContext({
          extUoExec,
          __polarguments: args,
          __filename,
          __dirname: path.dirname(filename),
          contents,
          module: _module,
          require,
          _require: makeRequireFunction(_module)
       })
    };

    // We return the module up back to the core. This way, we can module.emit('shutdown')
  
    return _module;
  } catch (e) {
    // Special logging maybe? But throw back up anyway
    throw e;
  }
}

module.exports = exports = {
  loadScript,
  runScript
};
