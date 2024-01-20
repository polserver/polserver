#ifndef POLSERVER_INTERPOLATESTRING_H
#define POLSERVER_INTERPOLATESTRING_H

#include "compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;

class InterpolateString : public Expression
{
public:
  InterpolateString( const SourceLocation& source_location,
                     std::vector<std::unique_ptr<Expression>> );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INTERPOLATESTRING_H
