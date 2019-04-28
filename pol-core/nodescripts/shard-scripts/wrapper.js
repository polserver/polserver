// @ts-check

/**
 * This binds JavaScript accesses to known methods of our {@link NodeObjectWrap} class in the core.
 * Object used in scripts, such that any JavaScript engine accesses (eg `valueOf` for numeric addition, or property access) access
 * the underlying BObjectImp through the underlying {@link NodeObjectWrap} object.
 * @typedef {Object} NodeProxyObject
 * @property {function(): boolean|number|string} valueOf - Returns the primitive value of this object
 * as defined in https://www.ecma-international.org/ecma-262/6.0/#sec-toobject
 * @property {function(): string} toString - Returns string value
 */

const objects = require("./objects");

const bojectTypeToClassMap = [
  null,
  undefined,
  String,
  Number,
  Number,
  Array,
  null, // ApplicPtr
  null, // ApplicObj
  Error,
  objects.Dictionary,
  Object,
  objects.Packet,
  objects.BinaryFile,
  null, // XMLFile
  null, // XMLNode
  null, // XMLAttributes
  objects.Polcore,
  objects.Account,
  null, // ConfigFileRef
  null, // ConfigElemRef
  objects.Datafile,
  objects.DataFileElement,
  objects.Guild,
  objects.Party,
  null, // BoundingBox
  null, // DebugContext
  objects.Script,
  objects.Package,
  null, // Menu
  objects.Character, // MobileRef
  objects.Character, // OfflineMobileRef
  objects.Item,
  objects.Boat,
  objects.Multi,
  objects.Client,
  null, // SQLConnection
  null, // SQLResultSet
  null, // SQLRow
  Boolean,
  null // FuncRef
];

const scriptIsAToConstructorMap = [
  null, // isA(0) is nothing
  objects.UObject,
  objects.Item,
  objects.Character,
  objects.NPC,
  objects.Lockable,
  objects.Container,
  objects.Corpse,
  objects.Door,
  objects.Spellbook,
  objects.Map,
  objects.House,
  objects.Equipment,
  objects.Armor,
  objects.Weapon
];

// class PolObject {
//   constructor(
    
   function PolObject(wrappedObj, { set_member, get_member, call_method }) {
    Object.defineProperty(wrappedObj, "toString", {
      value: wrappedObj.toString.bind(wrappedObj)
    });

    const scriptObj = new Proxy(wrappedObj, {
      // Listing the properties here, so we know what we have to implement!

      // getPrototypeOf: function(target) {
      //     const clazz = objWrappers[ wrappedObj.typeOfInt() ];
      //     return Reflect.getPrototypeOf(clazz || target);
      // },
      // setPrototypeOf? (target: T, v: any): boolean;
      // isExtensible? (target: T): boolean;
      // preventExtensions? (target: T): boolean;
      // getOwnPropertyDescriptor? (target: T, p: PropertyKey): PropertyDescriptor | undefined;
      // has? (target: T, p: PropertyKey): boolean;
      // has: function(target, key) {
      //   if (key === "then") return false;
      // },

      // set? (target: T, p: PropertyKey, value: any, receiver: any): boolean;
      // deleteProperty? (target: T, p: PropertyKey): boolean;
      // defineProperty? (target: T, p: PropertyKey, attributes: PropertyDescriptor): boolean;
      // enumerate? (target: T): PropertyKey[];
      // ownKeys? (target: T): PropertyKey[];
      // apply? (target: T, thisArg: any, argArray?: any): any;
      // construct? (target: T, argArray: any, newTarget?: any): object;
      get: function(target, p, recv) {
        //   console.log("Request for prop ", p);
        //   debugger;
        if (p === "then") {
          return scriptObj;
        } //Reflect.get(target, p, recv);

        if (p === "toString") {
          return wrappedObj.toString;
        } else if (p === Symbol.toPrimitive) {
          return wrappedObj.toString;
        } else if (p === Symbol.toStringTag) {
          return wrappedObj.toString;
        } else if (p === "_obj") {
          return wrappedObj;
        }

        if (typeof p === "string") return get_member(wrappedObj, p);
        return Reflect.get(target, p, recv);
      }
    });

    // Object.defineProperty(scriptObj, 'valueOf', {
    //     get: () => wrappedObj.valueOf(),
    //     writable: false,
    //     enumerable: false,
    //     configurable: false
    // });

    // Object.defineProperty(scriptObj, 'toString', {
    //     get: () => wrappedObj.toString(),
    //     writable: false,
    //     enumerable: false,
    //     configurable: false
    // });

    return scriptObj;
  }
// }
debugger;
/**
 * This corresponds to the {@link NodeObjectWrap} class in the core. The methods here
 * call those methods defined via Napi's ObjectWrap functionality.
 * @typedef {Object} NodeObjectWrap
 * @property {function(): boolean|number|string} valueOf - Returns the primitive value of this object
 * as defined in https://www.ecma-international.org/ecma-262/6.0/#sec-toobject
 * @property {function(): string} toString - Returns string value
 */

function proxyObject(clazzName = "PolObject", wrappedObj, interalMethods, scriptModule) {
  
  const scriptObj = objects[clazzName] ? 
    new objects[clazzName](wrappedObj, interalMethods, scriptModule) : 
    PolObject(wrappedObj, interalMethods);

  return scriptObj;
}

module.exports = exports = {
  proxyObject
};
