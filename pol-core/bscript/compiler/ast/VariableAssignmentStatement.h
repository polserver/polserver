#ifndef POLSERVER_VARIABLEASSIGNMENTSTATEMENT_H
#define POLSERVER_VARIABLEASSIGNMENTSTATEMENT_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class Identifier;

class VariableAssignmentStatement : public Statement
{
public:
  VariableAssignmentStatement( const SourceLocation&, std::unique_ptr<Identifier> identifier,
                               std::unique_ptr<Node> rhs );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;

  Identifier& identifier();
  Node& rhs();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_VARIABLEASSIGNMENTSTATEMENT_H
