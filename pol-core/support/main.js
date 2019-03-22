const { print } = process._linkedBinding("basic");
const { start } = process._linkedBinding("tsfn");

let myFunct = function() {
  // print("Called with args: ", Array.from(arguments));
};

// Create the thread-safe function (tsfn). Args:
// 1: function that can be called from core to notify js of events asynchronously
// 2: the require object, so the core can call require('./scripts.js')
// 3: max queue for tsfn. 0: unlimited
if (!start(myFunct, require, 0)) {
  print("COULD NOT START TSFN!");
  // @TODO handle proper error instead of completely shutting down...?
  process.exit(1);
}
