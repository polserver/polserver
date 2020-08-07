#ifndef POLSERVER_ERRORINITIALIZER_H
#define POLSERVER_ERRORINITIALIZER_H

#include "Expression.h"

namespace Pol::Bscript::Compiler
{
class ErrorInitializer : public Expression
{
public:
  ErrorInitializer( const SourceLocation&, std::vector<std::string> names,
                    std::vector<std::unique_ptr<Expression>> expressions );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  const std::vector<std::string> names;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_ERRORINITIALIZER_H
