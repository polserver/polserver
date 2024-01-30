#ifndef POLSERVER_UNINITIALIZEDVALUE_H
#define POLSERVER_UNINITIALIZEDVALUE_H

#include "bscript/compiler/ast/Value.h"

namespace Pol::Bscript::Compiler
{
class UninitializedValue : public Value
{
public:
  explicit UninitializedValue( const SourceLocation& );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};


}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_UNINITIALIZEDVALUE_H
