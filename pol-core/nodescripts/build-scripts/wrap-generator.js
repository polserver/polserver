// @ts-check

const fs = require('fs'), path = require('path'), Handlebars  = require('handlebars'), 
  xml2js = require('xml2js'), util = require('util');


// Register some helpers
Handlebars.registerHelper("stringify", (val) => JSON.stringify(val));

Handlebars.registerHelper("camelCase", (val) => {
  if (!val) return;
  const ret= `${(val[1]&&val[1].match(/[a-z]/))  ? val[0].toLowerCase() : val[0]}${val.substr(1)}`;
  return ret;
});

Handlebars.registerHelper("trimLast", (val, sepRegex) => {
  if (!val || !sepRegex) return;
  return val.split(sepRegex)[0];
});


Handlebars.registerHelper('exists', function(conditional, options) {
  if(typeof conditional !== "undefined") {
    return options.fn(this);
  } else {
    return options.inverse(this);
  }
});

// We'll try to get the 'polparser' linked internal module. If it exists, we can optimize known members and table functions.

let polparser, internalGetKnownObjMember, internalGetKnownObjMethod, internalGetKnownModuleMethod;
try {
    polparser = process._linkedBinding("polparser");
    internalGetKnownObjMember = polparser.getKnownObjMember;
    internalGetKnownObjMethod = polparser.getKnownObjMethod;
    internalGetKnownModuleMethod = polparser.getKnownModuleMethod;
} catch (e) {

}
async function generateModules(outPath) {
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

  const modules = fs.readdirSync(path.resolve(__dirname,"../../support/scripts"), "utf8").reduce( (prevVal,currVal) => {
    return currVal.endsWith(".em") ? prevVal.concat( [ currVal.substr(0,currVal.length-3) ] ) : prevVal;
  }, []);

  
  const modobj = modules.reduce( (prevVal,modName) => { 
    return { 
      ...prevVal, 
        [modName]: Object.assign({ constants: {}, functions: {}},parser.parse(fs.readFileSync(path.resolve(__dirname,`../../support/scripts/${modName}.em`),"utf-8")))
    };
  }, {});

  if (internalGetKnownModuleMethod) {
    for ( const [modName, modDef] of Object.entries(modobj))
    {
      const { functions } = modDef;
      for ( const funcName in functions ) {
        const internal = internalGetKnownModuleMethod(modName, funcName);
        if (internal) functions[funcName].id = internal.id;
      }
    }
  }

  debugger;

  let templateStr = fs.readFileSync(path.resolve(__dirname,"../templates/modules.hbs"), "utf-8");
  let templateFunc = Handlebars.compile(templateStr);
  let generated = templateFunc( modobj );

  fs.writeFileSync(path.join(outPath,"modules.js"), generated, "utf-8");
}


async function generateObjects(outPath) {
  var parser = new xml2js.Parser();
  let data = await util.promisify(fs.readFile)(path.resolve(__dirname, '../../../docs/docs.polserver.com/pol100/objref.xml'), "utf8");

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

          const internal = internalGetKnownObjMethod && internalGetKnownObjMethod(methodName) 
          if (internal) {
            prototypes[methodName] =  { 
                  id: internal.id
            }; 
          }

          // todo do something with args? maybe for docs? 
          // method.$.proto.substring(index + 1, method.$.proto.length - 1).match(/((\w+)( (\w+))*),?/));
      }
      

      const memberProps = {}; 
      if (clazz.member) for (const member of clazz.member) { 
          const { mname, access } = member.$;

          const knownMember = internalGetKnownObjMember && internalGetKnownObjMember(mname);
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

  debugger;
  let templateStr = fs.readFileSync(path.resolve(__dirname,"../templates/objects.hbs"), "utf-8");
      
  let templateFunc = Handlebars.compile(templateStr);
  let generated = templateFunc( output.map( key => clazzMap[key] ) );

  fs.writeFileSync(path.join(outPath,"objects.js"), generated, "utf-8");
}

module.exports = {
  generateModules,
  generateObjects
}