#pragma once

namespace Pol::Bscript::Compiler
{
/*

Describes the type of a user function. In the following snippet:

    class Dog
      function Dog( this ) endfunction
      function eat ( this ) endfunction
      function StaticFunction() endfunction
    endclass

    function AnotherStaticFunction() endfunction

`Dog` is a Constructor function, `eat` is a Method user function, and
`StaticFunction` and `AnotherStaticFunction` are Static functions.

`Super` is a compiler-generated function, created when encountering a `super()`
call inside a constructor. A Super function call Constructor functionss
explicitly passing a `this`, whereas regularly a Constructor function call will
_generate_ a `this` argument.
 */
enum class UserFunctionType
{

  Static,
  Constructor,
  Method,
  Super,
  Expression,
};

}  // namespace Pol::Bscript::Compiler
