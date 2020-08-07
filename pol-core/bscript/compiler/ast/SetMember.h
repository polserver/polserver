#ifndef POLSERVER_SETMEMBER_H
#define POLSERVER_SETMEMBER_H

#include "Expression.h"

#ifndef OBJMEMBERS_H
#include "objmembers.h"
#endif

namespace Pol::Bscript::Compiler
{
class SetMember : public Expression
{
public:
  SetMember( const SourceLocation&, bool consume, std::unique_ptr<Expression> entity,
             std::string name, std::unique_ptr<Expression> rhs, const Pol::Bscript::ObjMember* );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  Expression& entity();
  Expression& rhs();

  std::unique_ptr<Expression> take_entity();
  std::unique_ptr<Expression> take_rhs();

  const bool consume;
  const std::string name;
  // does the semantic analyze known enough to know something is being assigned to?
  const Pol::Bscript::ObjMember* const known_member;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SETMEMBER_H
