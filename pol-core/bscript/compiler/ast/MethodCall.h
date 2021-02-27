#ifndef POLSERVER_METHODCALL_H
#define POLSERVER_METHODCALL_H

#include "bscript/compiler/ast/Expression.h"

#ifndef OBJMETHODS_H
#include "objmethods.h"
#endif

namespace Pol::Bscript::Compiler
{
class Expression;
class MethodCallArgumentList;

class MethodCall : public Expression
{
public:
  MethodCall( const SourceLocation&, std::unique_ptr<Expression> lhs,
              std::string methodname, std::unique_ptr<MethodCallArgumentList> );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  unsigned argument_count() const;

  const std::string methodname;
  const Pol::Bscript::ObjMethod* const known_method;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_METHODCALL_H
