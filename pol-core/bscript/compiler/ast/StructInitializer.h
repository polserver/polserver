#ifndef POLSERVER_STRUCTINITIALIZER_H
#define POLSERVER_STRUCTINITIALIZER_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{

class StructInitializer : public Expression
{
public:
  StructInitializer( const SourceLocation& source_location, std::vector<std::unique_ptr<Node>> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_STRUCTINITIALIZER_H
