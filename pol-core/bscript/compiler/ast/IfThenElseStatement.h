#ifndef POLSERVER_IFTHENELSESTATEMENT_H
#define POLSERVER_IFTHENELSESTATEMENT_H

#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
class Block;
class BranchSelector;
class Expression;

class IfThenElseStatement : public Statement
{
public:
  IfThenElseStatement( const SourceLocation&, std::unique_ptr<BranchSelector>,
                       std::unique_ptr<Block> consequent, std::unique_ptr<Node> alternative );
  IfThenElseStatement( const SourceLocation&, std::unique_ptr<BranchSelector>,
                       std::unique_ptr<Block> consequent );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;

  BranchSelector& branch_selector();
  Block& consequent();
  Node* alternative();

  std::unique_ptr<Block> take_consequent();
  std::unique_ptr<Node> take_alternative();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_IFTHENELSESTATEMENT_H
