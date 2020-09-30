#ifndef POLSERVER_TREEBUILDER_H
#define POLSERVER_TREEBUILDER_H

#include <memory>
#include <string>
#include <vector>

namespace antlr4
{
  class ParserRuleContext;
  namespace tree
  {
    class TerminalNode;
  }
}
namespace Pol::Bscript::Compiler
{
class BuilderWorkspace;
class Expression;
class Report;
class SourceLocation;
class SourceFileIdentifier;

class TreeBuilder
{
public:
  TreeBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  static std::unique_ptr<Expression> consume_expression_result( std::unique_ptr<Expression> );

  SourceLocation location_for( antlr4::ParserRuleContext& ) const;
  SourceLocation location_for( antlr4::tree::TerminalNode& ) const;

  std::string text( antlr4::tree::TerminalNode* );
protected:

  Report& report;

  const SourceFileIdentifier& source_file_identifier;

  BuilderWorkspace& workspace;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_TREEBUILDER_H
