#pragma once

namespace Pol::Bscript::Compiler
{
// This enum is used in the instruction emitter as the offset of the TOK_SPREAD token. The executor
// will then determine how to use the spreaded element.
enum SpreadMode : unsigned short
{
  Array = 0,
  ValueStack = 1,
};
}  // namespace Pol::Bscript::Compiler
