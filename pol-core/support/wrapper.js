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

class ProxyBase {
    constructor() {

    }
}

const objWrappers = [
  class Unknown {},
  class Uninit {},
  class String {},
  class Long {},
  class Double {},
  class Array {},
  class ApplicPtr {},
  class ApplicObj {},
  class Error {},
  class Dictionary {},
  class Struct {},
  class Packet {},
  class BinaryFile {},
  class XMLFile {},
  class XMLNode {},
  class XMLAttributes {},
  class PolCoreRef {},

  class AccountRef {},
  class ConfigFileRef {},
  class ConfigElemRef {},
  class DataFileRef {},
  class DataElemRef {},
  class GuildRef {},
  class PartyRef {},
  class BoundingBox {},
  class DebugContext {},
  class ScriptExRef {},
  class Package {},
  class MenuRef {},
  class MobileRef {},
  class OfflineMobileRef {},
  class ItemRef {},
  class BoatRef {},
  class MultiRef {},
  class ClientRef {},

  class SQLConnection {},
  class SQLResultSet {},
  class SQLRow {},
  class Boolean {},
  class FuncRef {},
];

/**
 * This corresponds to the {@link NodeObjectWrap} class in the core. The methods here
 * call those methods defined via Napi's ObjectWrap functionality.
 * @typedef {Object} NodeObjectWrap
 * @property {function(): boolean|number|string} valueOf - Returns the primitive value of this object
 * as defined in https://www.ecma-international.org/ecma-262/6.0/#sec-toobject
 * @property {function(): string} toString - Returns string value
 */

function proxyObject(wrappedObj) {
  //   debugger;

  Object.defineProperty(wrappedObj, "toString", {
    value: wrappedObj.toString.bind(wrappedObj)
  });
  let scriptObj = new Proxy(wrappedObj, {
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
    if (p === 'then') {
      return scriptObj;
    }; //Reflect.get(target, p, recv); 

      if (p === "toString") {
        return wrappedObj.toString;
      } else if (p === Symbol.toPrimitive) {
        return wrappedObj.toString;
      } else if (p === Symbol.toStringTag) {
        return wrappedObj.toString;
      } else if (p === "_obj") {
          return wrappedObj;
      }
      
      if (typeof p === "string")
        return wrappedObj.getMember(p);
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

function stripBOM(content) {
  if (content.charCodeAt(0) === 0xfeff) {
    content = content.slice(1);
  }
  return content;
}

function createModuleWrap(extUoExec, wrapperConstructor) {
  // Lazy load a pol module
  /** @type {(number|symbol|string)[]} */
  const mods = ["basic", "basicio"];

  const moduleExports = new Proxy(
    {} /*exports*/,
    {
      ownKeys: function(target, handler) {
        return mods;
      },
      get: function(target, property, receiver) {
        if (mods.indexOf(property) === -1) return undefined;
        return new wrapperConstructor(extUoExec, property);
      }
    }
  );
  return moduleExports;
}
//

module.exports = exports = {
  proxyObject,
  createModuleWrap
};
