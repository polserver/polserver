#ifndef POLSERVER_MEMBERASSIGNMENT_H
#define POLSERVER_MEMBERASSIGNMENT_H

#include "bscript/compiler/ast/Expression.h"

#ifndef OBJMEMBERS_H
#include "bscript/objmembers.h"
#endif

namespace Pol::Bscript::Compiler
{
class MemberAssignment : public Expression
{
public:
  MemberAssignment( const SourceLocation&, bool consume, std::unique_ptr<Expression> entity,
                    std::string name, std::unique_ptr<Expression> rhs,
                    const Pol::Bscript::ObjMember* );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

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

#endif  // POLSERVER_MEMBERASSIGNMENT_H
