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

`Dog` is a Constructor function, `eat` is a Method user function, and `StaticFunction` and
`AnotherStaticFunction` are Static functions.

 */
enum class UserFunctionType
{

  Static,
  Constructor,
  Method,
};

}  // namespace Pol::Bscript::Compiler
