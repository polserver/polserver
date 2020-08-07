#ifndef POLSERVER_VALUE_H
#define POLSERVER_VALUE_H

#include "Expression.h"

namespace Pol::Bscript::Compiler
{
class Value : public Expression
{
public:
  explicit Value( const SourceLocation& );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VALUE_H
