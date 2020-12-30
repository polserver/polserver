#ifndef POLSERVER_STRUCTINITIALIZER_H
#define POLSERVER_STRUCTINITIALIZER_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class StructMemberInitializer;

class StructInitializer : public Expression
{
public:
  StructInitializer( const SourceLocation& source_location,
                     std::vector<std::unique_ptr<StructMemberInitializer>> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::vector<std::string> identifiers;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_STRUCTINITIALIZER_H
