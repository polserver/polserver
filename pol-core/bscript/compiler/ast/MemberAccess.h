#ifndef POLSERVER_MEMBERACCESS_H
#define POLSERVER_MEMBERACCESS_H

#include "bscript/compiler/ast/Expression.h"

#ifndef OBJMEMBERS_H
#include "objmembers.h"
#endif

namespace Pol::Bscript::Compiler
{
class MemberAccess : public Expression
{
public:
  MemberAccess( const SourceLocation&, std::unique_ptr<Expression> lhs, std::string name );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  std::unique_ptr<Expression> take_entity();

  const std::string name;

  const Pol::Bscript::ObjMember* const known_member;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_MEMBERACCESS_H
