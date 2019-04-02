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

/**
 * This corresponds to the {@link NodeObjectWrap} class in the core. The methods here 
 * call those methods defined via Napi's ObjectWrap functionality.
 * @typedef {Object} NodeObjectWrap
 * @property {function(): boolean|number|string} valueOf - Returns the primitive value of this object
 * as defined in https://www.ecma-international.org/ecma-262/6.0/#sec-toobject
 * @property {function(): string} toString - Returns string value
 */


function proxyObject(wrappedObj) {
    let scriptObj = new Proxy(wrappedObj, {
        // Listing the properties here, so we know what we have to implement!
        
        // getPrototypeOf? (target: T): object | null;
        // setPrototypeOf? (target: T, v: any): boolean;
        // isExtensible? (target: T): boolean;
        // preventExtensions? (target: T): boolean;
        // getOwnPropertyDescriptor? (target: T, p: PropertyKey): PropertyDescriptor | undefined;
        // has? (target: T, p: PropertyKey): boolean;
        // get? (target: T, p: PropertyKey, receiver: any): any;
        // set? (target: T, p: PropertyKey, value: any, receiver: any): boolean;
        // deleteProperty? (target: T, p: PropertyKey): boolean;
        // defineProperty? (target: T, p: PropertyKey, attributes: PropertyDescriptor): boolean;
        // enumerate? (target: T): PropertyKey[];
        // ownKeys? (target: T): PropertyKey[];
        // apply? (target: T, thisArg: any, argArray?: any): any;
        // construct? (target: T, argArray: any, newTarget?: any): object;
    });

    Object.defineProperty(scriptObj, 'valueOf', {
        get: () => wrappedObj.valueOf(),
        writable: false,
        enumerable: false,
        configurable: false
    });

    Object.defineProperty(scriptObj, 'toString', {
        get: () => wrappedObj.toString(),
        writable: false,
        enumerable: false,
        configurable: false
    });

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
    const mods = ["basic","basicio"];

    const moduleExports = new Proxy( { } /*exports*/, {
        ownKeys: function(target, handler) { return mods },
        get: function(target, property, receiver) {
            if (mods.indexOf(property) === -1) return undefined;
            return new wrapperConstructor(extUoExec,property);
        }
    });
    return moduleExports;
}
// 
debugger;
require('uo');

module.exports = exports = {
    proxyObject,
    createModuleWrap
};