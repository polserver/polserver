// @ts-check

const fs = require('fs'), path = require('path');

let parser;
try {
  // If we have jison, we will try to build the parser.
  const jison = require("jison");
  const grammar = fs.readFileSync( path.join(__dirname,"em-parser.jison"),"utf-8");
  parser = new jison.Parser(grammar);
} catch (e) {
  // Otherwise, we'll take the pre-built one.
  parser = new (require("./em-parser").Parser);
}

const modules = ["basicio","uo","npc"];

let modSrc =
`/**
* NodeModuleWrap native binding. It exports the POL modules as classes inside, eg. \`modwrap.basicio\`.
* These classes take a constructor with a single argument -- the External<UOExecutor> object, which
* bound to the script specific execution instance module's require() function. By "script specific
* execution instance", this is because the script module's own require() function is bound to a
* specific function, and not just the prototype's.
*/
const modwrap = process._linkedBinding("modwrap");\n\n` +
modules.map( mod => {
    let { constants = {}, functions = {} } = parser.parse(fs.readFileSync(path.resolve(__dirname,`../support/scripts/${mod}.em`),"utf-8"));
    let result = [
      `module.exports.${mod} = function ${mod}(extUoExec) {\n  const extMod = new modwrap.${mod}(extUoExec);`,
      Object.entries(constants).map( ([key,val]) => `  this.${key} = ${JSON.stringify(val)};` ).join("\n"),
      Object.entries(functions).map( ([func,args]) => `  this.${  (func[1]&&func[1].match(/[a-z]/))  ? func[0].toLowerCase() : func[0]}${func.substr(1)} = function( ${args.map(a=>a.name).join(", ")} ) { return this.execFunc( "${func}", ${args.map( (a,i) =>typeof a.default !== "undefined"?`arguments.length <= ${i} ? ${JSON.stringify(a.default)} : ${a.name}`:a.name).join(", ")} ); }.bind(extMod);` ).join("\n"),
      '};'
    ];
    return result.join("\n");
}).join("\n");

fs.writeFileSync("modules.js", modSrc, "utf-8");
