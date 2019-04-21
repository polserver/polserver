// @ts-check

const fs = require('fs'), path = require('path');

async function createModules() {
  let parser;
  try {
    // If we have jison, we will try to build the parser.
    const jison = require("jison");
    const grammar = fs.readFileSync( path.join(__dirname,"em-parser.jison"),"utf8");
    parser = new jison.Parser(grammar);
  } catch (e) {
    // Otherwise, we'll take the pre-built one.
    parser = new (require("./em-parser").Parser);
  }

  const modules = fs.readdirSync(path.resolve(__dirname,"../support/scripts"), "utf8").reduce( (prevVal,currVal) => {
    return currVal.endsWith(".em") ? prevVal.concat( [ currVal.substr(0,currVal.length-3) ] ) : prevVal;
  }, []);

  console.log("modules are", modules)
  
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

  fs.writeFileSync("modules.js", modSrc, "utf-8")
}


async function run() {

  const Handlebars  = require('handlebars'),
  xml2js = require('xml2js'), util = require('util');
  var parser = new xml2js.Parser();
  let data = await util.promisify(fs.readFile)(__dirname + '/objref.xml', "utf8");

  // @ts-ignore
  let result = await util.promisify(parser.parseString)(data);

  // let classDefMap = {};
  const clazzMap = {};

  const mappedClasses = {
      "Boolean": "boolean",
      "Array": "array",
      "String": "string",
      "Error": "Error",
      "Struct": "object",
      "FunctionObject": undefined
  }, mappedClassesKeys = Object.keys(mappedClasses);

  //Object.keys(mappedClasses).join("|")
  //    /Array|String|Boolean|Error|Struct|FunctionObject/

  result.ESCRIPT.class = result.ESCRIPT.class.filter(clazz => mappedClassesKeys.indexOf(clazz.$.name) === -1);


  for (let clazz of result.ESCRIPT.class) {
      const prototypes = clazz.prototypes = {}, clazzName = clazz.$.name;
      clazzMap[clazzName] = clazz;
      if (clazz.method) for (const method of clazz.method) {
          const index = method.$.proto.indexOf("(");
          let methodName = method.$.proto.substr(0, index);
          
          if (clazzName == "Dictionary" && method.$.proto === "[key]" ) {
              methodName = "get";
          }
          if (!methodName) throw new Error(`Unknown method parsing ${clazzName}::${method.$.proto}`);

          // if (internalGetKnownObjMembers)
              prototypes[methodName] =  { 
                  id: internalGetKnownObjMembers && internalGetKnownObjMembers(methodName) 
              }; // TODO get real id

          // todo do something with args? maybe for docs? 
          // method.$.proto.substring(index + 1, method.$.proto.length - 1).match(/((\w+)( (\w+))*),?/));
      }
      

      const memberProps = {}; 
      if (clazz.member) for (const member of clazz.member) { 
          const { mname, access } = member.$;

          // memberProps[mname]
          const knownMember = internalGetKnownObjMembers && internalGetKnownObjMembers(mname);
          if (knownMember) {
              memberProps[mname] = { id: knownMember.id, ro: access === "r/o" }
          } else {
              memberProps[mname] = { ro: access === "r/o" };
          }
      }
      clazz.memberProps = memberProps;
  }
  var roots = [], nodes = {};

  for (let clazz of result.ESCRIPT.class) {
      const clazzName = clazz.$.name, node = nodes[clazzName] = nodes[clazzName] || {};
      (clazz.child || []).forEach(child => node[child] = (nodes[child] || (nodes[child] = {})));
      if (!clazz.parent)
          roots[clazzName] = node
  }

  var printQueue = Object.entries(roots), clazz, children, output = [];

  while (printQueue.length && ([clazz, children] = printQueue.pop())) {
      output.push(clazz);
      printQueue = printQueue.concat(Object.entries(children));
  }


  // Handlebars.registerPartial("")
  let templateStr = fs.readFileSync("./templates/objects.hbs", "utf-8");
      
  let templateFunc = Handlebars.compile(templateStr);
  let generated = templateFunc( output.map( key => clazzMap[key] ) );
  
  debugger;


  fs.writeFileSync("./o.js", generated, "utf-8");

}