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
 * Returns a module for a script, with <module>.exports.default being the function to call.
 * @param {string} paths - Paths that the loaded module can require from
 * @param {object} extUoExec - External<UOExec>...?
 * @param {string} filename - Script to load in core
 */
function loadScript(paths, extUoExec, filename) {
  let contents = fs.readFileSync(filename, "utf-8");
  contents = stripBOM(contents);
  contents = stripShebang(contents);
  filename = path.resolve(filename);
  debugger;
  let _module = {
    exports: {},
    id: filename,
    children: [],
    parent: "",
    loaded: true,
    paths
  };

  let _require = new Proxy(require, {
    apply: function(target, thisArg, argArray) {
      /** @type {string} */
      let moduleName = argArray && argArray[0];
      if (internalModules.indexOf(moduleName) > -1) {
        console.log("Attempt to load internal module");
        return {
          print: function() {
            console.log.apply(console, arguments);
          }
        }; // new PolModuleWrapper(moduleName, extUoExec, filename).exports;
      }
      // @ts-ignore
      return Reflect.apply(...arguments);
    }
  });

  let compiledWrapper = vm.compileFunction(
    contents,
    ["exports", "require", "module", "__filename", "__dirname"],
    {
      filename,
      lineOffset: 0
    }
  );

  compiledWrapper.call(
    _module,
    _module.exports,
    _require,
    _module,
    filename,
    path.dirname(filename)
  );

  // console.log("result is", result, _module);
  return _module.exports;
}

module.exports = exports = {
  loadScript: loadScript.bind(undefined, module.paths)
};

console.log(__filename,__dirname);