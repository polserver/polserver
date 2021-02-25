#ifndef POLSERVER_VALUE_H
#define POLSERVER_VALUE_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class Value : public Expression
{
public:
  explicit Value( const SourceLocation& );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VALUE_H
