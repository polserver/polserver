#pragma once

#include <fmt/format.h>

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
};

}  // namespace Pol::Bscript::Compiler

template <>
struct fmt::formatter<Pol::Bscript::Compiler::UserFunctionType> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::Compiler::UserFunctionType& type,
                                        fmt::format_context& ctx ) const
  {
    using namespace Pol::Bscript::Compiler;
    std::string_view name;
    switch ( type )
    {
      using enum UserFunctionType;

    case Static:
      name = "static function";
      break;
    case Constructor:
      name = "class constructor";
      break;
    case Method:
      name = "class method";
      break;
    case Super:
      name = "super function";
      break;
    }
    return fmt::formatter<std::string>::format( name, ctx );
  }
};
