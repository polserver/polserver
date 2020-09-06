#ifndef POLSERVER_UNINITIALIZEDVALUE_H
#define POLSERVER_UNINITIALIZEDVALUE_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class UninitializedValue : public Expression
{
public:
  explicit UninitializedValue( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};


}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_UNINITIALIZEDVALUE_H
