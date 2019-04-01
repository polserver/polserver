// @ts-check

// @ts-ignore
const vm = require("vm"),
  fs = require("fs"),
  path = require("path");

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
 * @returns {function} - Compiled function
 */
function loadScript(filename) {
  let contents = fs.readFileSync(filename, "utf-8");
  contents = stripBOM(contents);
  contents = stripShebang(contents);
  filename = path.resolve(filename);
  debugger;

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

  let compiledWrapper = vm.compileFunction(
    contents,
    ["exports", "require", "module", "__filename", "__dirname"],
    {
      filename,
      lineOffset: 0
    }
  );

  return compiledWrapper;
}

function runScript(paths, extUoExec, filename, compiledWrapper, args) {
  let _module = {
    exports: {},
    id: filename,
    children: [],
    parent: "",
    loaded: true,
    paths
  };
  
  // Delete the modwrap cache, such that a require('modwrap') call will grab the new executor.
  // Probably need to delete everything...?
  if (require.cache) delete require.cache.modwrap;

  compiledWrapper.call(
    _module,
    _module.exports,
    require,
    _module,
    filename,
    path.dirname(filename)
  );
  if (_module.exports && typeof _module.exports.default === "function") {
    return _module.exports.default.apply(undefined,args);
  }

  /** 
   * There was no error, but the script didn't export a default function to run. How should we handle 
   * this...? 
   */
  return 1;
}

module.exports = exports = {
  loadScript,
  runScript: runScript.bind(undefined, module.paths),
};