#ifndef POLSERVER_ARRAYINITIALIZER_H
#define POLSERVER_ARRAYINITIALIZER_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class ArrayInitializer : public Expression
{
public:
  ArrayInitializer( const SourceLocation&, std::vector<std::unique_ptr<Expression>> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_ARRAYINITIALIZER_H
