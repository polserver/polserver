#ifndef POLSERVER_GETMEMBER_H
#define POLSERVER_GETMEMBER_H

#include "Expression.h"

#ifndef OBJMEMBERS_H
#include "objmembers.h"
#endif

namespace Pol::Bscript::Compiler
{
class GetMember : public Expression
{
public:
  GetMember( const SourceLocation&, std::unique_ptr<Expression> lhs, std::string name );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  std::unique_ptr<Expression> take_entity();

  const std::string name;

  const Pol::Bscript::ObjMember* const known_member;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_GETMEMBER_H
