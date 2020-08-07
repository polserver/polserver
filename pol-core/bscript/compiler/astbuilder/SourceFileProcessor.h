#ifndef POLSERVER_SOURCEFILEPROCESSOR_H
#define POLSERVER_SOURCEFILEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <boost/optional.hpp>
#include <memory>
#include <string>

#include "DeclarationBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class SourceFile;

class SourceFileProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SourceFileProcessor( const SourceFileIdentifier&, BuilderWorkspace&, bool is_src );

public:
  long long use_module( const std::string& name, SourceLocation& including_location );
  void process_source( SourceFile& );

  antlrcpp::Any visitStatement( EscriptGrammar::EscriptParser::StatementContext* ) override;
  antlrcpp::Any visitUnambiguousStatement(
      EscriptGrammar::EscriptParser::UnambiguousStatementContext* ) override;

  SourceLocation location_for( antlr4::ParserRuleContext& ) const;

protected:
  Profile& profile;
  Report& report;
  const SourceFileIdentifier& source_file_identifier;
  BuilderWorkspace& workspace;

  DeclarationBuilder tree_builder;

private:
  const bool is_src;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILEPROCESSOR_H
